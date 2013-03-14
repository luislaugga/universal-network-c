/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_transaction.c
* universal-network-c
*/

#include "test.h"
#include "transaction.h"
#include "transaction_internal.h"

#pragma mark -
#pragma mark Request

uint8_t testRequest1[] = {
	kProtocolDefaultId, // protocol
	kProtocolDefaultVersion,  // version
	ProtocolTypeTransaction,  // transaction
	0x00,			     // Transaction id
	TransactionTypeRequest,    // Transaction type
 	TransactionEmpty, // Transaction request type
}; 

const unsigned int testRequest1_length = 6;

uint8_t testResponse1[] = {
	kProtocolDefaultId, // protocol
	kProtocolDefaultVersion,  // version
	ProtocolTypeTransaction,  // transaction
	0x00,			     // Transaction id
	TransactionTypeResponse,    // Transaction type
 	TransactionResponseTypeSuccess, // Transaction response type
}; 

const unsigned int testResponse1_length = 6;

void test_request_receiveCallback(void * context, net_addr_t * addr, TransactionObject * object)
{

}

void test_request_errorCallback(void * context, net_addr_t * addr)
{
	bool * requestAck = (bool *)context;
	*requestAck = false;
}

static void test_request()
{
	LOG_TEST_START;
	
	__block unsigned int requestCount = 0;
	bool requestAck = true; // true unless error
	
	NetError netError;
	static const char * localhost = "0.0.0.0"; // Listening on all network interfaces
	
	// Receive Socket
	net_socket_t receiveSocket = net_socket_create(&netError, AF_INET, localhost, 0);
	assert(!netError);
	net_addr_t receiveSocketAddr;
	net_socket_local_addr(receiveSocket, &receiveSocketAddr);
	
	net_socket_receive_block_t testReceiveSocketBlock = Block_copy(^(net_packet_t packet) {
		assert(net_packet_len(packet) == testRequest1_length); // testRequest1
		assert(memcmp(testRequest1, packet->data, net_packet_len(packet)) == 0); // testRequest1
		++requestCount;
		net_packet_set_data(packet, testResponse1, testResponse1_length);
		net_socket_send(receiveSocket, packet); // testResponse1
		net_packet_release(receiveSocket, packet);
	});
	
	net_socket_set_receive_block(receiveSocket, testReceiveSocketBlock);

	// Send Socket	
	TransactionConfiguration testSetup;
	TransactionConfiguration * testSetupPtr = &testSetup;
	transactionSetup(testSetupPtr, 0, &requestAck, &test_request_receiveCallback, &test_request_errorCallback);
	
	net_addr_t sendSocketAddr;
	net_socket_local_addr(testSetup.socket, &sendSocketAddr);
		
	TransactionObject object;
	object.type = TransactionEmpty;
	transactionRequest(testSetupPtr, &receiveSocketAddr, &object);

	sleep(1);
	
	assert(requestCount == 1);
	assert(requestAck == true);
		
	Block_release(testReceiveSocketBlock);
	
	LOG_TEST_END;
}

#pragma mark -
#pragma mark Response

bool test_response_requestAck = true; // true unless error
bool test_response_requestReceived = false;

void test_response_receiveCallback(void * context, net_addr_t * addr, TransactionObject * object)
{
	test_response_requestReceived = true;	
}

void test_response_errorCallback(void * context, net_addr_t * addr)
{
	test_response_requestAck = false;
}

static void test_response()
{
	LOG_TEST_START;
	
	__block unsigned int responseCount = 0;
		
	NetError netError;
	static const char * localhost = "0.0.0.0"; // Listening on all network interfaces
	
	// Send Socket
	net_socket_t sendSocket = net_socket_create(&netError, AF_INET, localhost, 0);
	assert(!netError);
	net_addr_t sendSocketAddr;
	net_socket_local_addr(sendSocket, &sendSocketAddr);
	
	net_socket_receive_block_t testSendSocketBlock = Block_copy(^(net_packet_t packet) {
		assert(net_packet_len(packet) == testResponse1_length); // testResponse1
		assert(memcmp(testResponse1, packet->data, net_packet_len(packet)) == 0); // testResponse1
		++responseCount;
	});
	
	net_socket_set_receive_block(sendSocket, testSendSocketBlock);

	// Receive Socket
	TransactionConfiguration testSetup;
	TransactionConfiguration * testSetupPtr = &testSetup;
	transactionSetup(testSetupPtr, 0, NULL, test_response_receiveCallback, test_response_errorCallback);
	
	net_addr_t receiveSocketAddr;
	net_socket_local_addr(testSetup.socket, &receiveSocketAddr);
	
	net_packet_t testRequestPacket = net_packet_alloc(sendSocket);
	net_packet_set_data(testRequestPacket, testRequest1, testRequest1_length);
	net_packet_addr(testRequestPacket, &receiveSocketAddr);
	net_socket_send(sendSocket, testRequestPacket);

	sleep(1);
	
	assert(responseCount == 1);
	assert(test_response_requestReceived == true);
	assert(test_response_requestAck == true);
		
	Block_release(testSendSocketBlock);
	
	LOG_TEST_END;
}

#pragma mark -
#pragma mark Enable/Disable

void test_enable_disable_receiveCallback(void * context, net_addr_t * addr, TransactionObject * object)
{
	// should never be called...
}

void test_enable_disable_errorCallback(void * context, net_addr_t * addr)
{
	mNetworkPrettyLog;
	
	bool * didGetError = (bool *)context;
	*didGetError = true;
}

static void test_enable_disable()
{
	LOG_TEST_START;
	
	// flag indicates if error callback was called
	bool didGetError = false;
	
	NetError netError;
	static const char * localhost = "0.0.0.0"; // Listening on all network interfaces
	
	// Receive Socket
	net_socket_t receiveSocket = net_socket_create(&netError, AF_INET, localhost, 0);
	assert(!netError);
	net_addr_t receiveSocketAddr;
	net_socket_local_addr(receiveSocket, &receiveSocketAddr);
	
	net_socket_receive_block_t testReceiveSocketBlock = Block_copy(^(net_packet_t packet) {
		// empty, never reply back
	});
	
	net_socket_set_receive_block(receiveSocket, testReceiveSocketBlock);

	// Send Socket	
	TransactionConfiguration testSetup;
	TransactionConfiguration * testSetupPtr = &testSetup;
	transactionSetup(testSetupPtr, 0, &didGetError, &test_enable_disable_receiveCallback, &test_enable_disable_errorCallback);
	
	net_addr_t sendSocketAddr;
	net_socket_local_addr(testSetup.socket, &sendSocketAddr);
		
	TransactionObject object;
	object.type = TransactionEmpty;
	transactionRequest(testSetupPtr, &receiveSocketAddr, &object);
	
	assert(didGetError == false);
	sleep(11);
	assert(didGetError == true);
	
	// reset
	didGetError = false;
	
	// send again
	transactionRequest(testSetupPtr, &receiveSocketAddr, &object);
	
	// disable
	transactionDisable(testSetupPtr);
	
	assert(didGetError == false);
	sleep(11); // wait
	assert(didGetError == false);
		
	Block_release(testReceiveSocketBlock);
	
	LOG_TEST_END;
}

int main(void)
{
	LOG_SUITE_START("transaction");

	test_request();
	test_response();
	test_enable_disable();
	
	return 0;
}
