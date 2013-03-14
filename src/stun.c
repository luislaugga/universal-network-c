/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stun.c
* Universal
*/

#include "stun.h"
#include "stun_internal.h"
#include "network.h"

stun_t stunCreate(net_socket_t socket, const char * serverHostname, StunDidResolve stunDidResolve, StunDidFailResolve stunDidFailResolve)
{
	stun_t stun = malloc(sizeof(struct StunStruct)); // Alloc
	
	if(stun)
	{		
		// Stun configuration
		stun->didResolve = false;
		stun->stunDidResolve = Block_copy(stunDidResolve);
		stun->stunDidFailResolve = Block_copy(stunDidFailResolve);
		
		// Test configuration
		stun->config.testDidComplete = Block_copy(^(StunTest * test) {
			stunRunNextTest(stun, test);
		});
		
		// Transmission configuration
		stun->config.rto = kStunDefaultRto;
		stun->config.maxRetries = kStunDefaultRetries;
						
		// Results reset
		memset(&stun->results, 0, sizeof(StunResults));
		
		// Associate socket
		stun->config.socket_info.socket = socket;
		
		// Create queue
		stun->config.stunDispatchQueue = dispatch_queue_create("com.laugga.stunDispatchQueue", NULL);
        
        // Store server hostname
        strncpy(stun->config.primaryServerHostname, serverHostname, 64);
	}
	
	return stun;
}

void stunDestroy(stun_t stun)
{
	if(stun)
	{
		// Release any pending tests
		if(!stun->bindingTest1.isCompleted) stunTestRelease(&stun->bindingTest1, &stun->config);
		if(!stun->behaviorTest1.isCompleted) stunTestRelease(&stun->behaviorTest1, &stun->config);
		if(!stun->behaviorTest2.isCompleted) stunTestRelease(&stun->behaviorTest2, &stun->config);
		if(!stun->filteringTest1.isCompleted) stunTestRelease(&stun->filteringTest1, &stun->config);
		if(!stun->filteringTest2.isCompleted) stunTestRelease(&stun->filteringTest2, &stun->config);
		
		// WARNING: don't call stunDestroy from any of didResolve/didFailResolve blocks because those are dispatched to this queue
		dispatch_release(stun->config.stunDispatchQueue); // Release dispatch queue
		
		if(stun->config.testDidComplete)
			Block_release(stun->config.testDidComplete); // Release test did complete block
		
		if(stun->stunDidResolve)
			Block_release(stun->stunDidResolve); // Release did resolve block
		
		if(stun->stunDidFailResolve)
			Block_release(stun->stunDidFailResolve); // Release did fail resolve block
	
		free(stun); // Release self
		stun = NULL; // Nil pointer
	}
}

void stunResolve(stun_t c)
{
	dispatch_async(c->config.stunDispatchQueue, ^{ 
        
        // Server address resolution
		NetError netError = net_addr_resolve(&c->config.primaryServerAddr, c->config.primaryServerHostname, kStunServerDefaultPort);
		
        // Won't proceed if resolve fails
        if(netError)
		{
			netErrorLog(netError);
			if(netError == NetNotKnownError)
				mNetworkLog("Hostname '%s' does not exist", c->config.primaryServerHostname);
            
            c->stunDidFailResolve(c); // notify
		}
        else // no error
        {
            // Copy socket callback and block callback pointers
            net_socket_copy_info(c->config.socket_info.socket, &c->config.socket_info);
            
            // Run first test: basic binding
            stunTestStartBasicBinding(&c->bindingTest1, &c->config, &c->results);
        }
	});
}

void stunRunNextTest(stun_t c, StunTest * previousTest)
{
	dispatch_async(c->config.stunDispatchQueue, ^{ 
		if(previousTest == &c->bindingTest1) // Run test: behavior 1
			stunTestStartBehavior1(&c->behaviorTest1, &c->config, &c->results);
		else if(previousTest == &c->behaviorTest1) // Run test: behavior 2
			stunTestStartBehavior2(&c->behaviorTest2, &c->config, &c->results);
		else if(previousTest == &c->behaviorTest2) // Run test: filtering 1
			stunTestStartFiltering1(&c->filteringTest1, &c->config, &c->results);
		else if(previousTest == &c->filteringTest1) // Run test: filtering 2
			stunTestStartFiltering2(&c->filteringTest2, &c->config, &c->results);
		else if(c->results.didBindingTestSuccess) // Finish
		{
			// Copy back socket callback and block callback pointers
			net_socket_set_info(c->config.socket_info.socket, &c->config.socket_info);
		
			// Notify
			c->didResolve = true;
			c->stunDidResolve(c);	
		}
		else
		{
			c->stunDidFailResolve(c); // If fails basic binding test, no point going further
		}
	});
}

void stunLog(stun_t c)
{
	mNetworkLog("STUN report");
	mNetworkLog("Server Address:");
	net_addr_log(&c->config.primaryServerAddr);
	
	if(c->results.didBindingTestSuccess)
	{
		mNetworkLog("BindingTest: Success");
		mNetworkLog("Local Address:");
		net_addr_log(&c->results.localAddr);
		mNetworkLog("Mapped Address:");
		net_addr_log(&c->results.mappedAddr);
		if(c->results.hasOtherServerAddress)
		{
			mNetworkLog("Server Another Address: True");
			mNetworkLog("Server Alternate Addresses:");
			net_addr_log(&c->results.otherServerPrimaryHostAddr);
			net_addr_log(&c->results.otherServerPrimaryPortAddr);
			net_addr_log(&c->results.otherServerAddr);
		}
		else
		{
			mNetworkLog("Server Another Address: False");
		}
	}
	else
	{
		mNetworkLog("BindingTest: Fail");
	}
	
	if(c->results.didBehaviorTestSuccess)
	{
		mNetworkLog("BehaviorTest: Success");
		switch(c->results.natBehavior)
		{
			case DirectMapping:
			mNetworkLog("NAT Behavior: DirectMapping");
			break;
			case EndpointIndependentMapping:
			mNetworkLog("NAT Behavior: EndpointIndependentMapping");
			break;
			case AddressDependentMapping:
			mNetworkLog("NAT Behavior: AddressDependentMapping");
			break;
			case AddressAndPortDependentMapping:
			mNetworkLog("NAT Behavior: AddressAndPortDependentMapping");
			break;
			default:
			mNetworkLog("NAT Behavior: Unknown");
		}
	}
	else
	{
		mNetworkLog("BehaviorTest: Fail");
	}
	
	if(c->results.didFilteringTestSuccess)
	{
		mNetworkLog("FilteringTest: Success");
		switch(c->results.natFiltering)
		{
			case DirectConnectionFiltering:
			mNetworkLog("NAT Filtering: DirectConnectionFiltering");
			break;
			case EndpointIndependentFiltering:
			mNetworkLog("NAT Filtering: EndpointIndependentFiltering");
			break;
			case AddressDependentFiltering:
			mNetworkLog("NAT Filtering: AddressDependentFiltering");
			break;
			case AddressAndPortDependentFiltering:
			mNetworkLog("NAT Filtering: AddressAndPortDependentFiltering");
			break;
			default:
			mNetworkLog("NAT Filtering: Unknown");
		}
	}
	else
	{
		mNetworkLog("FilteringTest: Fail");
	}
}