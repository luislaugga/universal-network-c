/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_stream_protocol.c
* universal-network-c
*/

#include "test.h"
#include "transaction.h"
#include "transaction_protocol.h"

#pragma mark -
#pragma mark Attribute Address

static void test_pack_unpack_attribute_address()
{
	LOG_TEST_START;
	
	net_addr_t test_localAddress;
	net_addr_local(&test_localAddress);
	
	net_addr_t test_zeroAddress;
	net_addr_zero(&test_zeroAddress);
	
	const size_t data_length = 100; // 100 Bytes
	uint8_t data[data_length]; 
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	transactionProtocolPackAttributeAddress(&bitstream, &test_localAddress);
	transactionProtocolPackAttributeAddress(&bitstream, &test_zeroAddress);
	
	bitstream_reset(&bitstream);
	
	net_addr_t read_localAddress;
	net_addr_t read_zeroAddress;
	
	transactionProtocolUnpackAttributeAddress(&bitstream, &read_localAddress);
	transactionProtocolUnpackAttributeAddress(&bitstream, &read_zeroAddress);
	
	assert(net_addr_is_equal(&test_localAddress, &read_localAddress));
	assert(net_addr_is_equal(&test_zeroAddress, &read_zeroAddress));
	
	LOG_TEST_END;
}

#pragma mark -
#pragma mark User Objects

static void test_pack_unpack_UserCreateObject()
{
	LOG_TEST_START;
	
	TransactionObjectUserCreate packUserCreate;
	TransactionObjectUserCreate unpackUserCreate;

  packUserCreate.uid = 29293299292; // random
	sprintf(packUserCreate.name, "First Last");
	sprintf(packUserCreate.email, "user@domain.com");
		
	uint8_t bitstream_data[kNetPacketMaxLen]; 
	bitstream_t bitstream = bitstream_create(bitstream_data, kNetPacketMaxLen);	
	transactionProtocolPackObjectUserCreate(&bitstream, &packUserCreate);	
	bitstream_reset(&bitstream);
	transactionProtocolUnpackObjectUserCreate(&bitstream, &unpackUserCreate);
	
	assert(packUserCreate.uid == unpackUserCreate.uid);
	assert(strcmp(packUserCreate.name, unpackUserCreate.name) == 0);
	assert(strcmp(packUserCreate.email, unpackUserCreate.email) == 0);
	
	LOG_TEST_END;
}

#pragma mark -
#pragma mark Resource Objects

static void test_pack_unpack_ResourceCreateObject()
{
	LOG_TEST_START;
	
	TransactionObjectResourceCreate packResourceCreate;
	TransactionObjectResourceCreate unpackResourceCreate;

  packResourceCreate.rid = 393939392329;
  packResourceCreate.uid = 2323289382938;
	uint8_t data[10] = "0123456789";
	memcpy(packResourceCreate.data, data, 10);
	packResourceCreate.length = 10;
		
	uint8_t bitstream_data[kNetPacketMaxLen]; 
	bitstream_t bitstream = bitstream_create(bitstream_data, kNetPacketMaxLen);	
	transactionProtocolPackObjectResourceCreate(&bitstream, &packResourceCreate);	
	bitstream_reset(&bitstream);
	transactionProtocolUnpackObjectResourceCreate(&bitstream, &unpackResourceCreate);
	
	assert(packResourceCreate.rid == unpackResourceCreate.rid);
	assert(packResourceCreate.uid == unpackResourceCreate.uid);
	assert(packResourceCreate.length == unpackResourceCreate.length);
	assert(memcmp(packResourceCreate.data, unpackResourceCreate.data, packResourceCreate.length) == 0);
	
	LOG_TEST_END;
}

#pragma mark -
#pragma mark Online Object

const char * test_peerId = "test_Peer1";
net_addr_t test_streamAddress;
const unsigned int test_localAddressHost = 1039302;
const unsigned short test_localAddressPort = 1000;
net_addr_t test_localAddress;
const unsigned int test_mappedAddressHost = 9038492;
const unsigned short test_mappedAddressPort = 3000;
net_addr_t test_mappedAddress;
const unsigned int test_expireSeconds = 15;
bool requestOk;

void test_object_online_receiveCallback(void * context, net_addr_t * addr, TransactionObject * object)
{
/*	// Check object type
	if(object->type == TransactionOnline)
	{
		// Cast
		TransactionObjectOnline * online = (TransactionObjectOnline *)object; 
		
		// Read values
		assert(strcmp(online->peerId, test_peerId) == 0);
		assert(net_addr_is_equal(&online->localAddress, &test_localAddress));
		assert(net_addr_is_equal(&online->mappedAddress, &test_mappedAddress));
		assert(online->expireSeconds == test_expireSeconds);
		
		requestOk = true;
	}
	else
	{
		requestOk = false;
	}*/
}

void test_object_online_errorCallback(void * context, net_addr_t * addr)
{
	requestOk = false;
}

static void test_object_online()
{
	LOG_TEST_START;
	
	// Setup
/*	TransactionConfiguration testSetup;
	TransactionConfiguration * testSetupPtr = &testSetup;
	transactionSetup(testSetupPtr, 0, NULL, &test_object_online_receiveCallback, &test_object_online_errorCallback);
	net_addr_t socketAddress;
	net_socket_local_addr(testSetup.socket, &socketAddress);
	requestOk = false;
	
	// Test
	TransactionObjectOnline online;
	online.type = TransactionOnline;
	strcpy(online.peerId, test_peerId);
	net_addr_set(&test_localAddress, test_localAddressHost, test_localAddressPort, true);
	net_addr_set(&test_mappedAddress, test_mappedAddressHost, test_mappedAddressPort, true);
	net_addr_copy(&online.localAddress, &test_localAddress);
	net_addr_copy(&online.mappedAddress, &test_mappedAddress);
	online.expireSeconds = test_expireSeconds;
	
	transactionRequest(testSetupPtr, &socketAddress, (TransactionObject *)&online);
	
	sleep(1);
	
	assert(requestOk);
		*/
	LOG_TEST_END;
}

#pragma mark -
#pragma mark Offline Object

void test_object_offline_receiveCallback(void * context, net_addr_t * addr, TransactionObject * object)
{
	// Check object type
	if(object->type == TransactionOffline)
	{
		// Cast
		TransactionObjectOffline * offline = (TransactionObjectOffline *)object; 
		
		// Read values
		assert(strcmp(offline->peerId, test_peerId) == 0);
		
		requestOk = true;
	}
	else
	{
		requestOk = false;
	}
}

void test_object_offline_errorCallback(void * context, net_addr_t * addr)
{
	requestOk = false;
}

static void test_object_offline()
{
	LOG_TEST_START;
	
	// Setup
	TransactionConfiguration testSetup;
	TransactionConfiguration * testSetupPtr = &testSetup;
	transactionSetup(testSetupPtr, 0, NULL, &test_object_offline_receiveCallback, &test_object_offline_errorCallback);
	net_addr_t socketAddress;
	net_socket_local_addr(testSetup.socket, &socketAddress);
	requestOk = false;
	
	// Test
	TransactionObjectOffline offline;
	offline.type = TransactionOffline;
	strcpy(offline.peerId, test_peerId);
	
	transactionRequest(testSetupPtr, &socketAddress, (TransactionObject *)&offline);
	
	sleep(1);
	
	assert(requestOk);
		
	LOG_TEST_END;
}

#pragma mark -
#pragma mark PeerList Object

const unsigned int test_peerCount = 5;
TransactionObjectPeer test_peerList[test_peerCount];

void test_object_peerlist_receiveCallback(void * context, net_addr_t * addr, TransactionObject * object)
{
	// Check object type
	if(object->type == TransactionPeerList)
	{
		// Cast
		TransactionObjectPeerList * objectPeerList = (TransactionObjectPeerList *)object; 
		
		// Read values
		assert(objectPeerList->count == test_peerCount);
		
		for(int i=0; i<test_peerCount; ++i)
		{
			assert(strcmp(test_peerList[i].peerId, objectPeerList->list[i].peerId) == 0);
			assert(net_addr_is_equal(&test_peerList[i].localAddress, &objectPeerList->list[i].localAddress));
			assert(net_addr_is_equal(&test_peerList[i].mappedAddress, &objectPeerList->list[i].mappedAddress));
		}
		
		requestOk = true;
	}
	else
	{
		requestOk = false;
	}
}

void test_object_peerlist_errorCallback(void * context, net_addr_t * addr)
{
	requestOk = false;
}

static void test_object_peerlist()
{
	LOG_TEST_START;
	
	// Setup
	TransactionConfiguration testSetup;
	TransactionConfiguration * testSetupPtr = &testSetup;
	transactionSetup(testSetupPtr, 0, NULL, &test_object_peerlist_receiveCallback, &test_object_peerlist_errorCallback);
	net_addr_t socketAddress;
	net_socket_local_addr(testSetup.socket, &socketAddress);
	requestOk = false;
	
	// Test
	TransactionObjectPeerList objectPeerlist;
	transactionProtocolInitializeObjectPeerList(&objectPeerlist);
	objectPeerlist.type = TransactionPeerList;
	
	for(int i=0; i<test_peerCount; ++i)
	{
		sprintf(test_peerList[i].peerId, "peer%d", i);
		net_addr_zero(&test_peerList[i].localAddress);
		net_addr_zero(&test_peerList[i].mappedAddress);
		transactionProtocolAddPeerToObjectPeerList(&objectPeerlist, &test_peerList[i]);
	}
	
	transactionRequest(testSetupPtr, &socketAddress, (TransactionObject *)&objectPeerlist);
	
	sleep(1);
	
	assert(requestOk);
		
	LOG_TEST_END;
}

#pragma mark -
#pragma mark Connect Object

void test_object_connect_receiveCallback(void * context, net_addr_t * addr, TransactionObject * object)
{
	// Check object type
	if(object->type == TransactionConnect)
	{
		// Cast
		TransactionObjectConnect * connect = (TransactionObjectConnect *)object; 
		
		// Read values
		assert(strcmp(connect->peerId, test_peerId) == 0);
		assert(net_addr_is_equal(&connect->streamAddress, &test_streamAddress));
		
		requestOk = true;
	}
	else
	{
		requestOk = false;
	}
}

void test_object_connect_errorCallback(void * context, net_addr_t * addr)
{
	requestOk = false;
}

static void test_object_connect()
{
	LOG_TEST_START;
	
	// Setup
	TransactionConfiguration testSetup;
	TransactionConfiguration * testSetupPtr = &testSetup;
	transactionSetup(testSetupPtr, 0, NULL, &test_object_connect_receiveCallback, &test_object_connect_errorCallback);
	net_addr_t socketAddress;
	net_socket_local_addr(testSetup.socket, &socketAddress);
	requestOk = false;
	
	// Stream specific values
	net_addr_local(&test_streamAddress);
	
	// Test
	TransactionObjectConnect connect;
	connect.type = TransactionConnect;
	strcpy(connect.peerId, test_peerId);
	net_addr_copy(&connect.streamAddress, &test_streamAddress);
	
	transactionRequest(testSetupPtr, &socketAddress, (TransactionObject *)&connect);
	
	sleep(1);
	
	assert(requestOk);
		
	LOG_TEST_END;
}

#pragma mark -
#pragma mark ConnectAccept Object

void test_object_connectaccept_receiveCallback(void * context, net_addr_t * addr, TransactionObject * object)
{
	// Check object type
	if(object->type == TransactionConnectAccept)
	{
		// Cast
		TransactionObjectConnectAccept * connectaccept = (TransactionObjectConnectAccept *)object; 
		
		// Read values
		assert(strcmp(connectaccept->peerId, test_peerId) == 0);
		assert(net_addr_is_equal(&connectaccept->streamAddress, &test_streamAddress));
	
		requestOk = true;
	}
	else
	{
		requestOk = false;
	}
}

void test_object_connectaccept_errorCallback(void * context, net_addr_t * addr)
{
	requestOk = false;
}

static void test_object_connectaccept()
{
	LOG_TEST_START;
	
	// Setup
	TransactionConfiguration testSetup;
	TransactionConfiguration * testSetupPtr = &testSetup;
	transactionSetup(testSetupPtr, 0, NULL, &test_object_connectaccept_receiveCallback, &test_object_connectaccept_errorCallback);
	net_addr_t socketAddress;
	net_socket_local_addr(testSetup.socket, &socketAddress);
	requestOk = false;
	
	// Stream specific values
	net_addr_local(&test_streamAddress);
	
	// Test
	TransactionObjectConnectAccept connectaccept;
	connectaccept.type = TransactionConnectAccept;
	strcpy(connectaccept.peerId, test_peerId);
	net_addr_copy(&connectaccept.streamAddress, &test_streamAddress);
	
	transactionRequest(testSetupPtr, &socketAddress, (TransactionObject *)&connectaccept);
	
	sleep(1);
	
	assert(requestOk);
		
	LOG_TEST_END;
}

#pragma mark -
#pragma mark ConnectRefuse Object

void test_object_connectrefuse_receiveCallback(void * context, net_addr_t * addr, TransactionObject * object)
{
	// Check object type
	if(object->type == TransactionConnectRefuse)
	{
		// Cast
		TransactionObjectConnectRefuse * connectrefuse = (TransactionObjectConnectRefuse *)object; 
		
		// Read values
		assert(strcmp(connectrefuse->peerId, test_peerId) == 0);
	
		requestOk = true;
	}
	else
	{
		requestOk = false;
	}
}

void test_object_connectrefuse_errorCallback(void * context, net_addr_t * addr)
{
	requestOk = false;
}

static void test_object_connectrefuse()
{
	LOG_TEST_START;
	
	// Setup
	TransactionConfiguration testSetup;
	TransactionConfiguration * testSetupPtr = &testSetup;
	transactionSetup(testSetupPtr, 0, NULL, &test_object_connectrefuse_receiveCallback, &test_object_connectrefuse_errorCallback);
	net_addr_t socketAddress;
	net_socket_local_addr(testSetup.socket, &socketAddress);
	requestOk = false;
	
	// Stream specific values
	net_addr_local(&test_streamAddress);
	
	// Test
	TransactionObjectConnectRefuse connectrefuse;
	connectrefuse.type = TransactionConnectRefuse;
	strcpy(connectrefuse.peerId, test_peerId);
	
	transactionRequest(testSetupPtr, &socketAddress, (TransactionObject *)&connectrefuse);
	
	sleep(1);
	
	assert(requestOk);
		
	LOG_TEST_END;
}

#pragma mark -
#pragma mark Disconnect Object

void test_object_disconnect_receiveCallback(void * context, net_addr_t * addr, TransactionObject * object)
{
	// Check object type
	if(object->type == TransactionDisconnect)
	{
		// Cast
		TransactionObjectDisconnect * disconnect = (TransactionObjectDisconnect *)object; 
		
		// Read values
		assert(strcmp(disconnect->peerId, test_peerId) == 0);
	
		requestOk = true;
	}
	else
	{
		requestOk = false;
	}
}

void test_object_disconnect_errorCallback(void * context, net_addr_t * addr)
{
	requestOk = false;
}

static void test_object_disconnect()
{
	LOG_TEST_START;
	
	// Setup
	TransactionConfiguration testSetup;
	TransactionConfiguration * testSetupPtr = &testSetup;
	transactionSetup(testSetupPtr, 0, NULL, &test_object_disconnect_receiveCallback, &test_object_disconnect_errorCallback);
	net_addr_t socketAddress;
	net_socket_local_addr(testSetup.socket, &socketAddress);
	requestOk = false;
	
	// Stream specific values
	net_addr_local(&test_streamAddress);
	
	// Test
	TransactionObjectDisconnect disconnect;
	disconnect.type = TransactionDisconnect;
	strcpy(disconnect.peerId, test_peerId);
	
	transactionRequest(testSetupPtr, &socketAddress, (TransactionObject *)&disconnect);
	
	sleep(1);
	
	assert(requestOk);
		
	LOG_TEST_END;
}

int main(void)
{	
	LOG_SUITE_START("transaction_protocol");

	// attributes pack/unpack
	test_pack_unpack_attribute_address();
	
	// user objects pack/unpack
	test_pack_unpack_UserCreateObject();
	
	// resource objects pack/unpack
	test_pack_unpack_ResourceCreateObject();
	
	// objects
	test_object_online();
	test_object_offline();
	test_object_peerlist();
	test_object_connect();
	test_object_connectaccept();
	test_object_connectrefuse();
	test_object_disconnect();
	
	return 0;
}
