/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_stun.c
* universal-network-c
*/

#include "test.h"
#include "stun_utils.h"
#include "stun_protocol.h"
#include "stun.h"

static void test_stun_utils()
{
	uint8_t idA[kStunTransactionIdLen] = {0xE7,0x53,0xD7,0x6E,0xA8,0x57,0xA2,0x4D,0xA3,0x8A,0x22,0x9F}; // A = B
	uint8_t idB[kStunTransactionIdLen] = {0xE7,0x53,0xD7,0x6E,0xA8,0x57,0xA2,0x4D,0xA3,0x8A,0x22,0x9F}; // A = B
	uint8_t idC[kStunTransactionIdLen] = {0xAC,0x53,0xD7,0x6E,0xA8,0x57,0xA2,0x12,0xA3,0x3A,0x42,0x9F};
	uint8_t idD[kStunTransactionIdLen] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	
	StunTransactionId validTransactionIdA;
	StunTransactionId validTransactionIdB;
	StunTransactionId validTransactionIdC;
	StunTransactionId validTransactionIdD;
	
	memcpy(&validTransactionIdA.id, idA, kStunTransactionIdLen);
	memcpy(&validTransactionIdB.id, idB, kStunTransactionIdLen);
	memcpy(&validTransactionIdC.id, idC, kStunTransactionIdLen);
	memcpy(&validTransactionIdD.id, idD, kStunTransactionIdLen);
	
	assert(stunIsValidTransactionId(&validTransactionIdA) == true);
	assert(stunIsValidTransactionId(&validTransactionIdB) == true);
	assert(stunIsValidTransactionId(&validTransactionIdC) == true);
	assert(stunIsValidTransactionId(&validTransactionIdD) == false);
	
	assert(stunIsEqualTransactionId(&validTransactionIdA, &validTransactionIdB) == true);
	assert(stunIsEqualTransactionId(&validTransactionIdA, &validTransactionIdC) == false);
	
	StunTransactionId testTransactionIdA;
	StunTransactionId testTransactionIdB;
	
	stunGenerateTransactionId(&testTransactionIdA);
	stunGenerateTransactionId(&testTransactionIdB);
	
	assert(stunIsValidTransactionId(&testTransactionIdA) == true);
	assert(stunIsValidTransactionId(&testTransactionIdB) == true);
	
	assert(stunIsEqualTransactionId(&testTransactionIdA, &testTransactionIdB) == false); // Random, should be different
	
	LOG_TEST_END;
}

static void test_stun_protocol()
{	
	uint8_t validHeaderData[kStunHeaderLen] = {0x01,0x01,0x00,0x44,0x21,0x12,0xa4,0x42,0x5e,0x39,0xb8,0x4d,0x76,0xf9,0x79,0xe5,0x00,0x00,0x00,0x00};

	StunMsgType validType = StunMsgTypeBinding;
	StunMsgClass validClass = StunMsgClassSuccessResponse;
	unsigned int validLength = 68;
	uint8_t id[kStunTransactionIdLen] = {0x00,0x00,0x00,0x00,0xe5,0x79,0xf9,0x76,0x4d,0xb8,0x39,0x5e};
	StunTransactionId validTransactionId;
	memcpy(&validTransactionId.id, id, kStunTransactionIdLen);
	
	uint8_t testHeaderData[kStunHeaderLen];	
	bitstream_t testHeaderBitstream = bitstream_create(testHeaderData, kStunHeaderLen);
	
	stunProtocolPackHeader(&testHeaderBitstream, validType, validClass, validTransactionId, validLength);
	
	assert(memcmp(validHeaderData, testHeaderData, kStunHeaderLen) == 0); // Headers must be equal
	
	bitstream_t testUnpackHeaderBitstream = bitstream_create(validHeaderData, kStunHeaderLen);
	
	StunMsgType testType;
	StunMsgClass testClass;
	unsigned int testLength;
	StunTransactionId testTransactionId;
	
	stunProtocolUnpackHeader(&testUnpackHeaderBitstream, &testType, &testClass, &testTransactionId, &testLength);
	
	assert(validType == testType); // Test and Valid values must be equal
	assert(validClass == testClass);
	assert(validLength == testLength);
	assert(stunIsEqualTransactionId(&validTransactionId, &testTransactionId) == true);
	
	uint8_t validBodyData[68] = {0x00,0x01,0x00,0x08,0x00,0x01,0xf5,0x8a,0xbc,0x52,0xc0,0x06,0x00,0x04,0x00,0x08,0x00,0x01,0x0d,0x96,0xd0,0x6e,0x84,0x84,0x00,0x05,0x00,0x08,0x00,0x01,0x0d,0x97,0xd0,0x6e,0x84,0x85,0x80,0x20,0x00,0x08,0x00,0x01,0xd4,0x98,0x9d,0x40,0x64,0x44,0x80,0x22,0x00,0x10,0x56,0x6f,0x76,0x69,0x64,0x61,0x2e,0x6f,0x72,0x67,0x20,0x30,0x2e,0x39,0x36,0x00};
	bitstream_t validBodyBitstream = bitstream_create(validBodyData, 68);
	
	StunBindingResponse validBindingResponse;
	validBindingResponse.hasResponseOrigin = false;
	validBindingResponse.hasMappedAddress = true;
	net_addr_resolve(&validBindingResponse.mappedAddressAddr, "188.82.192.6", 62858);
	validBindingResponse.hasXorMappedAddress = false;
	validBindingResponse.hasAlternateServer = false;
	validBindingResponse.hasErrorCode = false;
	
	StunBindingResponse testBindingResponse;
	
	assert(stunProtocolUnpackBindingResponseBody(&validBodyBitstream, 68, &testBindingResponse) == kStunValid);
	
	assert(validBindingResponse.hasResponseOrigin == testBindingResponse.hasResponseOrigin);
	assert(validBindingResponse.hasMappedAddress == testBindingResponse.hasMappedAddress);
	assert(net_addr_is_equal(&validBindingResponse.mappedAddressAddr, &testBindingResponse.mappedAddressAddr) == true);
	assert(validBindingResponse.hasXorMappedAddress == testBindingResponse.hasXorMappedAddress);
	assert(validBindingResponse.hasAlternateServer == testBindingResponse.hasAlternateServer);
	assert(validBindingResponse.hasErrorCode == testBindingResponse.hasErrorCode);
	
	LOG_TEST_END;
}

static void test_stun_test()
{
	
	LOG_TEST_END;
}

static void test_stun_client1()
{
	NetError netError;
	net_socket_t socket = net_socket_create(&netError, AF_INET, 0, 0);
	
	const char * testStunServerHostname = "backend.laugga.com";
	// const unsigned short testStunServerPort = kStunServerDefaultPort;
	// 
	// net_addr_t testStunServerAddr;
	// 
	// NetError netError;
	// 
	// netError = net_addr_resolve(&testStunServerAddr, testStunServerHostname, testStunServerPort);
	// 
	// if(netError)
	// {
	// 	netErrorLog(netError);
	// 	if(netError == NetNotKnownError)
	// 		printf("Hostname '%s' does not exist\n", testStunServerHostname);
	// }
	// 
	// assert(netError == NetNoError);
	
	StunDidResolve clientDidResolve = ^(stun_t c) {
		assert(c->didResolve == true);
		assert(c->results.didBindingTestSuccess == true);
		
		net_addr_t zeroAddr;
		net_addr_set(&zeroAddr, 0, 0, false);
		assert(net_addr_is_equal(&c->results.localAddr, &zeroAddr) == false);
		assert(net_addr_is_equal(&c->results.mappedAddr, &zeroAddr) == false);
		
		if(c->results.isMappingDirect)
			assert(net_addr_is_equal(&c->results.localAddr, &c->results.mappedAddr) == true);
		else
			assert(net_addr_is_equal(&c->results.localAddr, &c->results.mappedAddr) == false);
	
		stunLog(c);
	};
	
	StunDidFailResolve clientDidFailResolve = ^(stun_t c) {
	};
	
	stun_t stun = stunCreate(socket, testStunServerHostname, clientDidResolve, clientDidFailResolve);
	
	stunResolve(stun);
	
	sleep(4); // Should wait to resolve
	
	assert(stun->didResolve = true);
	
	stunDestroy(stun);
	
	LOG_TEST_END;
}

static void test_stun_client2()
{
	NetError netError;
	net_socket_t socket = net_socket_create(&netError, AF_INET, 0, 0);
	
	const char * testStunServerHostname = "www.laugga.com"; // No STUN server should be running on web
	// const unsigned short testStunServerPort = kStunServerDefaultPort;
	// 
	// net_addr_t testStunServerAddr;
	// 
	// NetError netError;
	// 
	// netError = net_addr_resolve(&testStunServerAddr, testStunServerHostname, testStunServerPort);
	// 
	// if(netError)
	// {
	// 	netErrorLog(netError);
	// 	if(netError == NetNotKnownError)
	// 		printf("Hostname '%s' does not exist\n", testStunServerHostname);
	// }
	// 
	// assert(netError == NetNoError);
	
	bool didFail = true;
	
	StunDidResolve clientDidResolve = ^(stun_t c) {
		assert(didFail == false);		
		// exit(0);
	};
	
	StunDidFailResolve clientDidFailResolve = ^(stun_t c) {
		stunDestroy(c);
		assert(didFail == true);	
		LOG_TEST_END;
	};
	
	stun_t stun = stunCreate(socket, testStunServerHostname, clientDidResolve, clientDidFailResolve);
	
	stunResolve(stun);
	
	sleep(3); // Should wait for all 3 timeouts fire
}

const int testPacketsCount = 10;
size_t testPacketsLen[testPacketsCount];
int receivedTestPacketsCount = 0;
size_t receivedTestPacketsLen[testPacketsCount];

net_socket_t test_receiveSocket, test_sendSocket;

static void test_socket_stun_receiveCallback(void * context, net_packet_t packet)
{
	receivedTestPacketsLen[receivedTestPacketsCount++] = net_packet_len(packet);
	net_packet_release(test_receiveSocket, packet);
	
	net_socket_t contextSocket = (net_socket_t)context;
	assert(test_receiveSocket == contextSocket);
}

static void test_socket_stun()
{
	LOG_TEST_START;
	
 	NetError netError;
	
	const int sendPort = 45300;
	const int receivePort = 45301;
	static const char * localhost = "0.0.0.0";
		
	test_receiveSocket = net_socket_create(&netError, AF_INET, localhost, receivePort);
	assert(netError == NetNoError);
	test_sendSocket = net_socket_create(&netError, AF_INET, localhost, sendPort);
	assert(netError == NetNoError);
	
	assert(test_receiveSocket);
	assert(test_sendSocket);
	
	net_socket_set_receive_callback(test_receiveSocket, test_receiveSocket, test_socket_stun_receiveCallback);
	
	// Stun test
	StunDidResolve clientDidResolve = ^(stun_t c) {
	};
	StunDidFailResolve clientDidFailResolve = ^(stun_t c) {
		stunDestroy(c);
	};
	const char * testStunServerHostname = "backend.laugga.com";
	stun_t stun = stunCreate(test_receiveSocket, testStunServerHostname, clientDidResolve, clientDidFailResolve);
	assert(stun);
	stunResolve(stun);
	
	sleep(3); // wait 3 seconds
	
	// Check results
	assert(stun->didResolve);
	assert(stun->results.didBindingTestSuccess);
	assert(stun->results.didBehaviorTestSuccess);
	assert(stun->results.didFilteringTestSuccess);

	// Proceed with socket testing
	//dispatch_async(dispatch_get_main_queue(), ^{	
		for(int i=0; i<testPacketsCount; ++i)
		{
			net_packet_t testPacket = net_packet_alloc(test_sendSocket);			
			net_packet_set(testPacket, localhost, receivePort);		
			
			testPacketsLen[i] = (1+rand()%10); // N (1, 10) Bytes for testing
			receivedTestPacketsLen[i] = 0;
	
			for(int b=0; b<testPacketsLen[i]; ++b)
				bitstream_write_uint8(&testPacket->bitstream, 0xFF); // Write up testPacketsLen[i] bytes for testing
			
			net_socket_send(test_sendSocket, testPacket);
			net_packet_release(test_sendSocket, testPacket);
			
			usleep(100); // wait 100 microseconds
		}
	//});
	
	dispatch_time_t waitDelay = dispatch_time(DISPATCH_TIME_NOW, 4ull * NSEC_PER_SEC);
	dispatch_after(waitDelay, dispatch_get_main_queue(), ^{
		assert(receivedTestPacketsCount == testPacketsCount);

		int totalSent = 0;
		int totalReceived = 0;

		for(int i=0; i<testPacketsCount; ++i)
		{
			totalSent += testPacketsLen[i];
			totalReceived += receivedTestPacketsLen[i];
		}
		
		assert(totalReceived == totalSent);

		LOG_TEST_END;
		exit(0);
	});
	
	//dispatch_main();
	
	sleep(4);
	
	LOG_TEST_END;
}

const int forward_testPacketsCount = 3;
size_t forward_testPacketsLen[testPacketsCount];
int forward_receivedTestPacketsCount = 0;
size_t forward_receivedTestPacketsLen[testPacketsCount];

net_socket_t forward_test_receiveSocket, forward_test_sendSocket;

static void forward_test_socket_stun_receiveCallback(void * context, net_packet_t packet)
{
	printf("GOT FORWARDED PACKET\n");
	
	forward_receivedTestPacketsLen[forward_receivedTestPacketsCount++] = net_packet_len(packet);
	net_packet_release(forward_test_receiveSocket, packet);
	
	net_socket_t contextSocket = (net_socket_t)context;
	assert(forward_test_receiveSocket == contextSocket);
}

static void test_socket_stun_forward()
{
	LOG_TEST_START;
	
 	NetError netError;
	
	const int sendPort = 45400;
	const int receivePort = 45401;
	static const char * localhost = "0.0.0.0";
		
	forward_test_receiveSocket = net_socket_create(&netError, AF_INET, localhost, receivePort);
	assert(netError == NetNoError);
	forward_test_sendSocket = net_socket_create(&netError, AF_INET, localhost, sendPort);
	assert(netError == NetNoError);
	
	assert(forward_test_receiveSocket);
	assert(forward_test_sendSocket);
	
	net_socket_set_receive_callback(forward_test_receiveSocket, forward_test_receiveSocket, forward_test_socket_stun_receiveCallback);
	
	// Stun test
	StunDidResolve clientDidResolve = ^(stun_t c) {
	};
	StunDidFailResolve clientDidFailResolve = ^(stun_t c) {
		stunDestroy(c);
	};
	const char * testStunServerHostname = "backend.laugga.com";
	stun_t stun = stunCreate(forward_test_receiveSocket, testStunServerHostname, clientDidResolve, clientDidFailResolve);
	assert(stun);
	stunResolve(stun);
	
	// Proceed with socket testing
	for(int i=0; i<forward_testPacketsCount; ++i)
	{
		net_packet_t testPacket = net_packet_alloc(forward_test_sendSocket);			
		net_packet_set(testPacket, localhost, receivePort);		
		
		forward_testPacketsLen[i] = (1+rand()%10); // N (1, 10) Bytes for testing
		forward_receivedTestPacketsLen[i] = 0;

		for(int b=0; b<forward_testPacketsLen[i]; ++b)
			bitstream_write_uint8(&testPacket->bitstream, 0xFF); // Write up testPacketsLen[i] bytes for testing
		
		net_socket_send(forward_test_sendSocket, testPacket);
		net_packet_release(forward_test_sendSocket, testPacket);
		
		usleep(205000); // wait 100 microseconds
	}
	
	sleep(3); // wait 3 seconds
	
	// Check results
	assert(stun->didResolve);
	assert(stun->results.didBindingTestSuccess);
	assert(stun->results.didBehaviorTestSuccess);
	assert(stun->results.didFilteringTestSuccess);
	
	dispatch_time_t waitDelay = dispatch_time(DISPATCH_TIME_NOW, 4ull * NSEC_PER_SEC);
	dispatch_after(waitDelay, dispatch_get_main_queue(), ^{
		assert(forward_receivedTestPacketsCount == forward_testPacketsCount);

		int totalSent = 0;
		int totalReceived = 0;

		for(int i=0; i<forward_testPacketsCount; ++i)
		{
			totalSent += forward_testPacketsLen[i];
			totalReceived += forward_receivedTestPacketsLen[i];
		}
		
		assert(totalReceived == totalSent);

		LOG_TEST_END;
		exit(0);
	});
	
	dispatch_main();
	
	sleep(4);
	
	LOG_TEST_END;
}

int main(void)
{
	LOG_SUITE_START("stun");
		
	test_stun_utils();
	test_stun_protocol();
	//test_stun_test();
	test_stun_client1();
	//test_stun_client2();
	test_socket_stun();
	test_socket_stun_forward();
		
	return 0;
}
