/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_net.c
* universal-network-c
*/

#include "test.h"
#include "queue.h"
#include "pool.h"
#include "net.h"

const int testPacketsCount = 100;
size_t testPacketsLen[testPacketsCount];
int receivedTestPacketsCount = 0;
size_t receivedTestPacketsLen[testPacketsCount];

net_socket_t test_receiveSocket, test_sendSocket;

static void test_receiveCallback(void * context, net_packet_t packet)
{
	receivedTestPacketsLen[receivedTestPacketsCount++] = net_packet_len(packet);
	net_packet_release(test_receiveSocket, packet);
}

static void test_net_callback()
{
	LOG_TEST_START;
	
	NetError netError;
	
	const int sendPort = 45300;
	const int receivePort = 45301;
	static const char * localhost = "127.0.0.1";
		
	test_receiveSocket = net_socket_create(&netError, AF_INET, localhost, receivePort);
	test_sendSocket = net_socket_create(&netError, AF_INET, localhost, sendPort);
	
	assert(test_receiveSocket);
	assert(test_sendSocket);
	
	net_socket_set_receive_callback(test_receiveSocket, NULL, test_receiveCallback);
	
	dispatch_async(dispatch_get_main_queue(), ^{	
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
	});
	
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
	});
	
	//dispatch_main();
	
	sleep(4);
}

static void test_net_block()
{
	LOG_TEST_START;
	
	NetError netError;
	
	const int sendPort = 45300;
	const int receivePort = 45301;
	static const char * localhost = "127.0.0.1";
		
	test_receiveSocket = net_socket_create(&netError, AF_INET, localhost, receivePort);
	test_sendSocket = net_socket_create(&netError, AF_INET, localhost, sendPort);
	
	assert(test_receiveSocket);
	assert(test_sendSocket);
	
	net_socket_receive_block_t test_receiveBlock = Block_copy(^(net_packet_t packet) {
		receivedTestPacketsLen[receivedTestPacketsCount++] = net_packet_len(packet);
		net_packet_release(test_receiveSocket, packet);
	});
	
	net_socket_set_receive_block(test_receiveSocket, test_receiveBlock);
	
	dispatch_async(dispatch_get_main_queue(), ^{	
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
	});
	
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
	
	dispatch_main();
	
	sleep(4);
	
	Block_release(test_receiveBlock);
}

int main(void)
{
	LOG_SUITE_START("net");

	//test_net_callback();
	test_net_block();
	
	return 0;
}
