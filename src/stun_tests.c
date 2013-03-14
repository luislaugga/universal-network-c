/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stun_tests.c
* lUniversal-Network
*/

#include "stun_tests.h"
#include "stun_utils.h"
#include "universal_network_c.h"

void stunTestStartBasicBinding(StunTest * test, StunConfig * config, StunResults * results)
{	
	mNetworkLog("Testing: Binding");
		
	// Reset
	memset(test, 0, sizeof(StunTest));
	
	// Test setup
	stunTestSetup(test, config, &config->primaryServerAddr);
	
	// Retrieve local address
	net_socket_local_addr(config->socket_info.socket, &results->localAddr);
	
	// Build STUN request
	stunProtocolPackBindingRequest(&test->packet->bitstream, test->transactionId); // Binding request
	
	// Set STUN response block (simplifies access to config and results)
	test->receiveBlock = Block_copy(^(net_packet_t packet) {
		dispatch_async(config->stunDispatchQueue, ^{ 
			if(test->isCompleted == false) // If test has not been completed
			{
				StunTransactionId transactionId; // Unpack response
				StunBindingResponse bindingResponse;
			
				if(stunProtocolUnpackBindingResponse(&packet->bitstream, &transactionId, &bindingResponse) == kStunValid)
				{
					stunTestDestroyTimeout(test, config); // Stop timeout
				
					if(stunIsEqualTransactionId(&test->transactionId, &transactionId))
					{
						if(bindingResponse.hasMappedAddress)
							net_addr_copy(&results->mappedAddr, &bindingResponse.mappedAddressAddr);
						if(bindingResponse.hasXorMappedAddress)
							net_addr_copy(&results->mappedAddr, &bindingResponse.xorMappedAddressAddr);
						if(bindingResponse.hasAlternateServer)
						{
							results->hasOtherServerAddress = true;
							net_addr_copy(&results->otherServerAddr, &bindingResponse.alternateServerAddr);
							net_addr_set(&results->otherServerPrimaryHostAddr, config->primaryServerAddr.sin_addr.s_addr, results->otherServerAddr.sin_port, false);
							net_addr_set(&results->otherServerPrimaryPortAddr, results->otherServerAddr.sin_addr.s_addr, config->primaryServerAddr.sin_port, false);
						}
					
						if(net_addr_is_equal(&results->mappedAddr, &results->localAddr))
							results->isMappingDirect = true;
						
						results->didBindingTestSuccess = true; // Set binding test results
						stunTestSetComplete(test, config);
					}
				}
				else
				{
					stunTestForward(test, config, packet); // Forward if not valid stun (might belong to other protocol running on same socket)
				}
			}

			net_packet_release(config->socket_info.socket, packet); // Release response packet
		});
	});
	
	// Send STUN request
	stunTestSend(test, config, &results->didBindingTestSuccess);
}

void stunTestStartBehavior1(StunTest * test, StunConfig * config, StunResults * results)
{	
	mNetworkLog("Testing: Behavior 1");
	
	// Reset
	memset(test, 0, sizeof(StunTest));
	
	// Not need to run if it is direct
	if(results->didBindingTestSuccess && results->isMappingDirect)
	{
		results->natBehavior = DirectMapping;
		results->didBehaviorTestSuccess = true;
		stunTestSetComplete(test, config);
	}	
	// Set complete but fail if no alternate server address
	else if(results->hasOtherServerAddress == false)
	{
		results->didBehaviorTestSuccess = false;
		stunTestSetComplete(test, config);
	}
	else
	{
		// Send STUN request to otherServerPrimaryPortAddr
		// Test setup
		stunTestSetup(test, config, &results->otherServerPrimaryPortAddr);

		// Build STUN request
		stunProtocolPackBindingRequest(&test->packet->bitstream, test->transactionId); // Binding request
		
		// Set STUN response block (simplifies access to config and results)
		test->receiveBlock = Block_copy(^(net_packet_t packet) {
			dispatch_async(config->stunDispatchQueue, ^{ 
				if(test->isCompleted == false) // If test has not been completed
				{
					StunTransactionId transactionId; // Unpack response
					StunBindingResponse bindingResponse;
					if(stunProtocolUnpackBindingResponse(&packet->bitstream, &transactionId, &bindingResponse) == kStunValid)
					{
						stunTestDestroyTimeout(test, config); // Stop timeout
					
						if(stunIsEqualTransactionId(&test->transactionId, &transactionId))
						{
							if(bindingResponse.hasMappedAddress)
								net_addr_copy(&results->mappedOtherServerPrimaryPortAddr, &bindingResponse.mappedAddressAddr);
							if(bindingResponse.hasXorMappedAddress)
								net_addr_copy(&results->mappedOtherServerPrimaryPortAddr, &bindingResponse.xorMappedAddressAddr);
						
							if(bindingResponse.hasMappedAddress || bindingResponse.hasXorMappedAddress)
					        	if (net_addr_is_equal(&results->mappedOtherServerPrimaryPortAddr, &results->mappedAddr)) // mappedAddr = mappedAddr with otherAddr:primaryPort
					        	{
									results->didBehaviorTestSuccess = true;
					            	results->natBehavior = EndpointIndependentMapping;
					        	}
						
							 // Set behavior test 1 results
							stunTestSetComplete(test, config);
						}
					}
					else
					{
						stunTestForward(test, config, packet); // Forward if not valid stun (might belong to other protocol running on same socket)
					}
				}

				net_packet_release(config->socket_info.socket, packet); // Release response packet
			});
		});

		// Send STUN request
		stunTestSend(test, config, &results->didBindingTestSuccess);
	}
}

void stunTestStartBehavior2(StunTest * test, StunConfig * config, StunResults * results)
{		
	mNetworkLog("Testing: Behavior 2");

	// Reset
	memset(test, 0, sizeof(StunTest));
	
	// Not need to run if behavior was found
	if(results->didBehaviorTestSuccess)
	{
		stunTestSetComplete(test, config);
	}	
	// Behavior test 1 error
	else if(net_addr_is_valid(&results->mappedOtherServerPrimaryPortAddr) == false)
	{
		results->didBehaviorTestSuccess = false;
		stunTestSetComplete(test, config);
	}
	else
	{
		// Send STUN request to otherServerAddr
		// Test setup
		stunTestSetup(test, config, &results->otherServerAddr);

		// Build STUN request
		stunProtocolPackBindingRequest(&test->packet->bitstream, test->transactionId); // Binding request
		
		// Set STUN response block (simplifies access to config and results)
		test->receiveBlock = Block_copy(^(net_packet_t packet) {
			dispatch_async(config->stunDispatchQueue, ^{ 
				if(test->isCompleted == false) // If test has not been completed
				{
					StunTransactionId transactionId; // Unpack response
					StunBindingResponse bindingResponse;

					if(stunProtocolUnpackBindingResponse(&packet->bitstream, &transactionId, &bindingResponse) == kStunValid)
					{
						stunTestDestroyTimeout(test, config); // Stop timeout
					
						if(stunIsEqualTransactionId(&test->transactionId, &transactionId))
						{
							if(bindingResponse.hasMappedAddress)
								net_addr_copy(&results->mappedOtherServerAddr, &bindingResponse.mappedAddressAddr);
							if(bindingResponse.hasXorMappedAddress)
								net_addr_copy(&results->mappedOtherServerAddr, &bindingResponse.xorMappedAddressAddr);
						
							if(bindingResponse.hasMappedAddress || bindingResponse.hasXorMappedAddress)
							{
					        	if (net_addr_is_equal(&results->mappedOtherServerAddr, &results->mappedAddr)) // mappedAddr = mappedAddr with otherAddr:otherPort	
					            	results->natBehavior = AddressDependentMapping;
					        	else
									results->natBehavior = AddressAndPortDependentMapping;
								
								results->didBehaviorTestSuccess = true;	
							}
						
							 // Set behavior test 1 results
							stunTestSetComplete(test, config);
						}
					}
					else
					{
						stunTestForward(test, config, packet); // Forward if not valid stun (might belong to other protocol running on same socket)
					}
				}

				net_packet_release(config->socket_info.socket, packet); // Release response packet
			});
		});

		// Send STUN request
		stunTestSend(test, config, &results->didBindingTestSuccess);
	}
}

void stunTestStartFiltering1(StunTest * test, StunConfig * config, StunResults * results)
{
	mNetworkLog("Testing: Filtering 1");
	
	// Reset
	memset(test, 0, sizeof(StunTest));
	
	// Not need to run if it is direct
	if(results->didBindingTestSuccess && results->isMappingDirect)
	{
		results->natFiltering = EndpointIndependentFiltering;
		results->didFilteringTestSuccess = true;
		stunTestSetComplete(test, config);
	}	
	// Set complete but fail if no alternate server address
	else if(results->hasOtherServerAddress == false)
	{
		results->didFilteringTestSuccess = false;
		stunTestSetComplete(test, config);
	}
	else
	{
		// Test setup
		stunTestSetup(test, config, &config->primaryServerAddr); // Primary server address

		// Build STUN binding-change-request
		bool doChangeHost = true;
		bool doChangePort = true;
		stunProtocolPackBindingChangeRequest(&test->packet->bitstream, test->transactionId, doChangeHost, doChangePort); // Bind request with change host+port
		
		// Set STUN response block (simplifies access to config and results)
		test->receiveBlock = Block_copy(^(net_packet_t packet) {
			dispatch_async(config->stunDispatchQueue, ^{ 
				if(test->isCompleted == false) // If test has not been completed
				{
					StunTransactionId transactionId; // Unpack response
					StunBindingResponse bindingResponse;
					if(stunProtocolUnpackBindingResponse(&packet->bitstream, &transactionId, &bindingResponse) == kStunValid)
					{
						stunTestDestroyTimeout(test, config); // Stop timeout
					
						if(stunIsEqualTransactionId(&test->transactionId, &transactionId))
						{
							// if we got a response back from the other IP and Port, then we have independent filtering
							if (net_addr_is_equal(&results->otherServerAddr, &packet->addr))
					        	{
									results->didFilteringTestSuccess = true;
					            	results->natFiltering = EndpointIndependentFiltering;
					        	}
						
	                     	 // Set behavior test 1 results
							stunTestSetComplete(test, config);
						}
					}
					else
					{
						stunTestForward(test, config, packet); // Forward if not valid stun (might belong to other protocol running on same socket)
					}
				}

				net_packet_release(config->socket_info.socket, packet); // Release response packet
			});
		});

		// Send STUN request
		stunTestSend(test, config, &results->didFilteringTestSuccess);
	}
}

void stunTestStartFiltering2(StunTest * test, StunConfig * config, StunResults * results)
{
	mNetworkLog("Testing: Filtering 2");
	
	// Reset
	memset(test, 0, sizeof(StunTest));
	
	// Not need to run if filtering was found
	if(results->didFilteringTestSuccess)
	{
		stunTestSetComplete(test, config);
	}
	// Set complete but fail if no alternate server address
	else if(results->hasOtherServerAddress == false)
	{
		results->didFilteringTestSuccess = false;
		stunTestSetComplete(test, config);
	}
	else
	{
		// Test setup
		stunTestSetup(test, config, &config->primaryServerAddr); // Primary server address

		// Build STUN binding-change-request
		bool doChangeHost = false;
		bool doChangePort = true;
		stunProtocolPackBindingChangeRequest(&test->packet->bitstream, test->transactionId, doChangeHost, doChangePort); // Bind request with change host+port
		
		// Set STUN response block (simplifies access to config and results)
		test->receiveBlock = Block_copy(^(net_packet_t packet) {
			dispatch_async(config->stunDispatchQueue, ^{ 
				if(test->isCompleted == false) // If test has not been completed
				{
					StunTransactionId transactionId; // Unpack response
					StunBindingResponse bindingResponse;
					if(stunProtocolUnpackBindingResponse(&packet->bitstream, &transactionId, &bindingResponse) == kStunValid)
					{
						stunTestDestroyTimeout(test, config); // Stop timeout
					
						if(stunIsEqualTransactionId(&test->transactionId, &transactionId))
						{
							// if we got a response back from the other Port, then we have independent filtering
							if (net_addr_is_equal(&results->otherServerAddr, &packet->addr))
					        	{
									results->didFilteringTestSuccess = true;
					            	results->natFiltering = AddressDependentFiltering;
					        	}
						
							 // Set behavior test 1 results
							stunTestSetComplete(test, config);
						}
					}
					else
					{
						stunTestForward(test, config, packet); // Forward if not valid stun (might belong to other protocol running on same socket)
					}
				}

				net_packet_release(config->socket_info.socket, packet); // Release response packet
			});
		});

		// Send STUN request
		stunTestSend(test, config, &results->didFilteringTestSuccess);
	}
}

void stunTestSetup(StunTest * test, StunConfig * config, net_addr_t * destAddr)
{		
	mNetworkPrettyLog;
	
	// Packet setup
	test->packet = net_packet_alloc(config->socket_info.socket); // Stun test owns packet. Responsible for releasing it.
	net_packet_addr(test->packet, destAddr); // Set STUN server address
	
	// STUN setup
	stunGenerateTransactionId(&test->transactionId); // Generate random trans. id
}

void stunTestSend(StunTest * test, StunConfig * config, bool * testSuccessCheck)
{
	// Define timeout block (simplifies access to config and results)
	test->timeoutBlock = Block_copy(^(void) {
      	dispatch_async(config->stunDispatchQueue, ^{ 
			stunTestRetryOrFail(test, config, testSuccessCheck); // If fails will set *testSuccessCheck = false
		});
	});
	
	// Socket set receive block
	net_socket_set_receive_block(config->socket_info.socket, test->receiveBlock);
	
	// Set timeout
	timeout_create_block(&test->timeout, test->timeoutBlock, config->rto);
	
	// Socket send packet
	net_socket_send(config->socket_info.socket, test->packet);
}

void stunTestDestroyTimeout(StunTest * test, StunConfig * config)
{
	mNetworkPrettyLog;
    
    // WARNING: Could be called several times for same timeout if 
    // test times out, sends another request and server replies back twice
    // but responses come back later and together with same stun transaction id
	timeout_destroy(&test->timeout); // Destroy timeout 
}

void stunTestSetComplete(StunTest * test, StunConfig * config)
{
	mNetworkPrettyLog;
		
    // Finish test asynchronously
	mNetworkLog("Test is complete");
	stunTestRelease(test, config); // Results set, can release test objects now
	test->isCompleted = true; // Mark as completed
	config->testDidComplete(test); // Notify
}

void stunTestRetryOrFail(StunTest * test, StunConfig * config, bool * testSuccessCheck)
{
	mNetworkPrettyLog;
	
	if(test->isCompleted == false) // If test has not been completed
	{			
		stunTestDestroyTimeout(test, config);
		if(test->retries < config->maxRetries) // Retry
		{
			test->retries++; // Update retries
            long timeoutRto = config->rto * (3ull * (long)test->retries); // Calculate new rto
			timeout_create_block(&test->timeout, test->timeoutBlock, timeoutRto); // Set timeout
			net_socket_send(config->socket_info.socket, test->packet); // Retransmit
            
            mNetworkLog("Retry nr: %u Timeout: %ld", test->retries, timeoutRto);
		}
		else // Fail
		{	
			*testSuccessCheck = false; // Set test result to failed
			stunTestSetComplete(test, config);
		}
	}
}

void stunTestRelease(StunTest * test, StunConfig * config)
{
	mNetworkPrettyLog;
	
	stunTestDestroyTimeout(test, config); // Destroy timeout (re-release)

	if(test->receiveBlock)
		Block_release(test->receiveBlock); // Release receive block
	
	if(test->timeoutBlock)
		Block_release(test->timeoutBlock); // Release timeout block
	
	if(test->packet)
		net_packet_release(config->socket_info.socket, test->packet); // Release request packet
}

void stunTestForward(StunTest * test, StunConfig * config, net_packet_t packet)
{
	mNetworkPrettyLog;
	
	// 1. Reset bitstream
	bitstream_reset(&packet->bitstream);
	
	// 2. Forward using config's socket_info
	net_socket_info_s socket_info = config->socket_info;	
	
	if(socket_info.receiveBlock) // block
	{
		socket_info.receiveBlock(packet);
	}
	else if(socket_info.receiveCallback) // alternative callback
	{
		socket_info.receiveCallback(socket_info.receiveCallbackContext, packet);
	}
}