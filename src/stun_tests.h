/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stun_test.h
* Universal
*/

#ifndef __lNetwork_stun_test_h__
#define __lNetwork_stun_test_h__

#include "timeout.h"
#include "net.h"

#include "stun_protocol.h"

typedef enum {
    UnknownBehavior = 0,
    DirectMapping,                  // IP address and port are the same between client and server view (NO NAT)
    EndpointIndependentMapping,     // same mapping regardless of IP:port original packet sent to (the kind of NAT we like)
    AddressDependentMapping,        // mapping changes for local socket based on remote IP address only, but remote port can change (partially symmetric, not great)
    AddressAndPortDependentMapping  // different port mapping if the ip address or port change (symmetric NAT, difficult to predict port mappings)
} StunNatBehavior;

typedef enum {
    UnknownFiltering = 0,
    DirectConnectionFiltering,
    EndpointIndependentFiltering,    // shouldn't be common unless connection is already direct (can receive on mapped address from anywhere regardless of where the original send went)
    AddressDependentFiltering,       // IP-restricted NAT
    AddressAndPortDependentFiltering // port-restricted NAT
} StunNatFiltering;

typedef struct {
	bool didBindingTestSuccess; // Basic binding test results
	net_addr_t localAddr;   	// local address
	net_addr_t mappedAddr; 		// mapped address from primaryServerAddr for local address
	bool isMappingDirect;  		// true if localAddr == mappedAddr
		
	bool hasOtherServerAddress; 		   // true if the basic binding request got an "other adddress" back from the server
	net_addr_t otherServerPrimaryHostAddr; // other address (Primary Host:Other Port) as identified by the basic binding request
	net_addr_t otherServerPrimaryPortAddr; // other address (Other Host:Primary Port) as identified by the basic binding request
	net_addr_t otherServerAddr;  	 	   // other address (Other Host:Other Port) as identified by the basic binding request

    bool didBehaviorTestSuccess; // Behavior test results
    StunNatBehavior natBehavior;
    net_addr_t mappedOtherServerPrimaryPortAddr; // result of binding request for otherServerPrimaryPortAddr (behavior test 1)
    net_addr_t mappedOtherServerAddr; 		   	 // result of binding request for otherServerAddr (behavior test 2)

	bool didFilteringTestSuccess; // Filter test results
    StunNatFiltering natFiltering;
} StunResults;

typedef struct {
	bool isCompleted;
	StunTransactionId transactionId; // The transaction ID is a 96-bit identifier, used to uniquely identify STUN transactions
	net_packet_t packet;
	net_socket_receive_block_t receiveBlock; // receiveBlock needs to be released (Block_Copy on start)
	unsigned int retries; // Nr. of retransmissions
	timeout_t timeout;
	timeout_block_t timeoutBlock; // timeoutBlock needs to be released (Block_Copy on start)
} StunTest;

typedef void (^StunTestDidComplete)(StunTest *);

typedef struct {
	long rto;     // Retransmission timeout in milliseconds
	unsigned int maxRetries; // Max. number of retransmissions after first request
	
	net_socket_info_s socket_info;
	dispatch_queue_t stunDispatchQueue;
	
    char primaryServerHostname[64]; // WEAK 64 bytes is enough?
	net_addr_t primaryServerAddr;
	StunTestDidComplete testDidComplete; // Block is called whenever some test finishes
} StunConfig;

void stunTestStartBasicBinding(StunTest * t, StunConfig * config, StunResults * results);
void stunTestStartBehavior1(StunTest * t, StunConfig * config, StunResults * results);
void stunTestStartBehavior2(StunTest * t, StunConfig * config, StunResults * results);
void stunTestStartFiltering1(StunTest * t, StunConfig * config, StunResults * results);
void stunTestStartFiltering2(StunTest * t, StunConfig * config, StunResults * results);

void stunTestSetup(StunTest * test, StunConfig * config, net_addr_t * destAddr);
void stunTestSend(StunTest * test, StunConfig * config, bool * testSuccessCheck);
void stunTestDestroyTimeout(StunTest * test, StunConfig * config);
void stunTestSetComplete(StunTest * test, StunConfig * config);
void stunTestRetryOrFail(StunTest * test, StunConfig * config, bool * testSuccessCheck);
void stunTestRelease(StunTest * test, StunConfig * config);

void stunTestForward(StunTest * test, StunConfig * config, net_packet_t packet); // Forward packet to socket's original callback in case it is STUN invalid

#endif