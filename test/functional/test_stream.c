/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_stream.c
* universal-network-c
*/

#include "test.h"
#include "stream.h"
#include "stream_internal.h"

#pragma mark -
#pragma mark Stream

static void test_stream()
{
	LOG_TEST_START;
	
	net_addr_t localAddress;
	net_addr_local(&localAddress);
	net_addr_t zeroAddress;
	net_addr_zero(&zeroAddress);
	
	Stream * streamA = streamCreate(&localAddress);
	Stream * streamB = streamCreate(&zeroAddress);	
	
	assert(net_addr_is_equal(&streamA->address, &localAddress));
	assert(net_addr_is_equal(&streamB->address, &zeroAddress));
	
	assert(streamA->state == StreamWaiting);
	assert(streamB->state == StreamWaiting);
	
	streamDestroy(&streamA);
	streamDestroy(&streamB);
	
	assert(streamA == NULL);
	assert(streamB == NULL);
	
	LOG_TEST_END;
}

#pragma mark -
#pragma mark StreamList

// static void test_stream_list()
// {
// 	LOG_TEST_START;
// 	
// 	net_addr_t addr1, addr2;
// 	net_addr_set(&addr1, 1, 1, true);
// 	net_addr_set(&addr2, 2, 2, true);
// 	
// 	net_addr_t addrZero;
// 	net_addr_zero(&addrZero);
// 	
// 	StreamList list = streamListCreate();
// 	
// 	assert(streamListIsEmpty(&list) == true);
// 	
// 	streamListAdd(&list, &addr1);
// 	
// 	assert(list.count == 1);
// 	assert(streamListIsEmpty(&list) == false);
// 	
// 	streamListAdd(&list, &addr2);
// 	
// 	assert(list.count == 2);
// 	assert(streamListIsEmpty(&list) == false);
// 	
// 	streamListAdd(&list, &addr2); // repeat, already added so won't have any effect
// 	
// 	assert(list.count == 2);
// 	assert(streamListIsEmpty(&list) == false);
// 	
// 	Stream * stream1 = streamListFind(&list, &addr1);
// 	Stream * stream2 = streamListFind(&list, &addr2);
// 		
// 	assert(stream1 != NULL);
// 	assert(stream2 != NULL);
// 	
// 	Stream * streamNot = streamListFind(&list, &addrZero);
// 	
// 	assert(streamNot == NULL);
// 	
// 	streamListRemove(&list, stream1);
// 	streamListRemove(&list, stream2);
// 		
// 	assert(streamListIsEmpty(&list) == true);
// 	
// 	streamListDestroy(&list);
// 	
// 	LOG_TEST_END;
// }

#pragma mark -
#pragma mark Add/Remove Stream

void test_stream_add_remove_update(void * context, bitstream_t * bitstream) {/* empty */ }
void test_stream_add_remove_timeout(void * context, net_addr_t * address) {/* empty */ }
void test_stream_add_remove_receive(void * context, net_addr_t * address, bitstream_t * bitstream) {/* empty */ }
void test_stream_add_remove_suspend(void * context) {/* empty */ }

static void test_stream_add_remove()
{
	LOG_TEST_START;
	
	LOG_TEST_START;
	
	__block unsigned int receiveCount = 0; // must be 1
	
	static const char * localhost = "0.0.0.0"; // Listening on all network interfaces
		
	// Auxiliary Socket
	NetError netError;
	net_socket_t auxSocket = net_socket_create(&netError, AF_INET, localhost, 0);
	assert(!netError);
	net_addr_t auxSocketAddr;
	net_socket_local_addr(auxSocket, &auxSocketAddr);
	
	// Setup stream
	__block StreamConfiguration configuration;
	NetError streamError = streamSetup(&configuration, 0, NULL, test_stream_add_remove_update, test_stream_add_remove_receive, test_stream_add_remove_timeout, test_stream_add_remove_suspend);
	assert(streamError == NetNoError);
	net_addr_t streamSocketAddr;
	net_socket_local_addr(configuration.socket, &streamSocketAddr);
	
	// Set Auxiliary Socket block
	net_socket_receive_block_t auxSocketReceiveBlock = Block_copy(^(net_packet_t packet) {
		
		assert(streamDoesExist(&configuration, &auxSocketAddr) == true); // Check it exists
		
		++receiveCount; // Update receive count
		streamRemove(&configuration, &auxSocketAddr); // Remove dummy stream address
		
		assert(net_addr_is_equal(&packet->addr, &streamSocketAddr)); // check auxiliary is receiving from stream socket address
		net_packet_release(auxSocket, packet);
	});
	net_socket_set_receive_block(auxSocket, auxSocketReceiveBlock);

	// Start
	streamResume(&configuration);
	
	// Add dummy stream address
	streamAdd(&configuration, &auxSocketAddr);
	
	// Wait
	sleep(kStreamTimeout);
	
	// Check
	assert(receiveCount == 1);
	assert(streamDoesExist(&configuration, &auxSocketAddr) == false); // Check it DOES NOT exist
	
	
	streamSuspend(&configuration);
	streamTeardown(&configuration);
	Block_release(auxSocketReceiveBlock);

	LOG_TEST_END;
}

#pragma mark -
#pragma mark Timeout

void test_stream_timeout_update(void * context, bitstream_t * bitstream) { /* empty */ }
void test_stream_timeout_suspend(void * context) { /* empty */ }

void test_stream_timeout_receive(void * context, net_addr_t * address, bitstream_t * bitstream)
{
	bool * didTimeout = (bool *)context;
	*didTimeout = false;
}

void test_stream_timeout_timeout(void * context, net_addr_t * address)
{
	bool * didTimeout = (bool *)context;
	*didTimeout = true;
}

static void test_stream_timeout()
{
	LOG_TEST_START;
	
	bool didTimeout = false; // must be true
	__block bool didSend = false; // must be true
	
	static const char * localhost = "0.0.0.0"; // Listening on all network interfaces
	
	// Auxiliary Socket
	NetError netError;
	net_socket_t auxSocket = net_socket_create(&netError, AF_INET, localhost, 0);
	assert(!netError);
	net_addr_t auxSocketAddr;
	net_socket_local_addr(auxSocket, &auxSocketAddr);
	
	// Setup stream
	__block StreamConfiguration configuration;
	NetError streamError = streamSetup(&configuration, 0, &didTimeout, &test_stream_timeout_update, &test_stream_timeout_receive, &test_stream_timeout_timeout, test_stream_timeout_suspend);
	assert(!streamError);
	net_addr_t streamSocketAddr;
	net_socket_local_addr(configuration.socket, &streamSocketAddr);
	
	// Set Auxiliary Socket block
	net_socket_receive_block_t auxSocketReceiveBlock = Block_copy(^(net_packet_t packet) {
		didSend = true;
		assert(net_addr_is_equal(&packet->addr, &streamSocketAddr)); // check auxiliary is receiving from stream socket address
		net_packet_release(auxSocket, packet);
	});
	
	net_socket_set_receive_block(auxSocket, auxSocketReceiveBlock);

	// Start
	streamResume(&configuration);
	
	// Add dummy stream address
	streamAdd(&configuration, &auxSocketAddr);

	// Wait
	sleep(kStreamTimeout+1);
	
	// Check
	assert(didSend == true);
	assert(didTimeout == true);
	
	streamSuspend(&configuration);
	streamTeardown(&configuration);
		
	Block_release(auxSocketReceiveBlock);

	LOG_TEST_END;	
}

#pragma mark -
#pragma mark Update

const char * test_stream_update_string = "test_stream_update_string";
unsigned int test_stream_update_string_len = 25;
unsigned int test_stream_update_len = 26; // packed as {length (1), string (25)}

char unpack_test_stream_update_string[26]; // string + '\0'
unsigned int unpack_test_stream_update_string_len;

void test_stream_update_receive(void * context, net_addr_t * address, bitstream_t * bitstream) { /* empty */ }
void test_stream_update_timeout(void * context, net_addr_t * address) { /* empty */ }
void test_stream_update_suspend(void * context) { /* empty */ }

void test_stream_update_update(void * context, bitstream_t * bitstream)
{
	bitstream_write_str(bitstream, test_stream_update_string);
}

static void test_stream_update()
{
	LOG_TEST_START;
		
	__block unsigned int receiveCount = 0; // must be 1
	
	static const char * localhost = "0.0.0.0"; // Listening on all network interfaces
	
	// Auxiliary Socket
	NetError netError;
	net_socket_t auxSocket = net_socket_create(&netError, AF_INET, localhost, 0);
	assert(!netError);
	net_addr_t auxSocketAddr;
	net_socket_local_addr(auxSocket, &auxSocketAddr);
	
	// Setup stream
	__block StreamConfiguration configuration;
	NetError streamError = streamSetup(&configuration, 0, NULL, test_stream_update_update, test_stream_update_receive, test_stream_update_timeout, test_stream_update_suspend);
	assert(!streamError);
	net_addr_t streamSocketAddr;
	net_socket_local_addr(configuration.socket, &streamSocketAddr);
	
	// Set Auxiliary Socket block
	net_socket_receive_block_t auxSocketReceiveBlock = Block_copy(^(net_packet_t packet) {	
		++receiveCount; // Update receive count
		streamRemove(&configuration, &auxSocketAddr); // Remove dummy stream address
		
		assert(net_addr_is_equal(&packet->addr, &streamSocketAddr)); // check auxiliary is receiving from stream socket address
		
		Sequence sequence;
		Ack ack;
		AckBitField ackBitField;
		StreamObject object;
		streamObjectSetup(&object);
		streamProtocolUnpackHeader(&packet->bitstream, &sequence, &ack, &ackBitField);
		streamProtocolUnpackData(&packet->bitstream, &object);
		
		assert(sequence == 0);
		assert(ack == 0);
		assert(ackBitField == 0);
		
		assert(object.length == test_stream_update_len);
		assert(object.bitstream.offset == 0); // before unpack
		
		unpack_test_stream_update_string_len = bitstream_read_str(&object.bitstream, unpack_test_stream_update_string, test_stream_update_string_len+1);
			
		assert(unpack_test_stream_update_string_len == test_stream_update_string_len);
		assert(strncmp(unpack_test_stream_update_string, test_stream_update_string, test_stream_update_string_len) == 0);
		assert(object.bitstream.offset == object.length); // after unpack
		
		net_packet_release(auxSocket, packet);
	});
	
	net_socket_set_receive_block(auxSocket, auxSocketReceiveBlock);

	// Start
	streamResume(&configuration);
	
	// Add dummy stream address
	streamAdd(&configuration, &auxSocketAddr);

	// Wait
	sleep(kStreamTimeout);
	
	// Check
	assert(receiveCount == 1);
	
	streamSuspend(&configuration);
	streamTeardown(&configuration);
		
	Block_release(auxSocketReceiveBlock);

	LOG_TEST_END;
}

int main(void)
{	
	LOG_SUITE_START("stream");

	test_stream();
	//test_stream_list();
	test_stream_add_remove();
	test_stream_timeout();
	test_stream_update();
	
	return 0;
}
