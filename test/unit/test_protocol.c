/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_protocol.c
* universal-network-c
*/

#include "test.h"
#include "protocol.h"

static void test_header()
{
	LOG_TEST_START;
	
	const size_t data_length = 256;
	uint8_t data[data_length];
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	ProtocolType msgType1 = ProtocolTypeTransaction;
	ProtocolType msgType2 = ProtocolTypeStream;

	ProtocolId msgId1 = kProtocolDefaultId;
	ProtocolId msgId2 = 0x31;
	
	ProtocolVersion msgVersion1 = kProtocolDefaultVersion;
	ProtocolVersion msgVersion2 = 0x02;
		
	protocolPackHeader(&bitstream, msgId1, msgVersion1, msgType1);
	protocolPackHeader(&bitstream, msgId2, msgVersion2, msgType2);
	
	bitstream_reset(&bitstream);
	
	ProtocolType test_msgType1 = ProtocolTypeTransaction;
	ProtocolType test_msgType2 = ProtocolTypeStream;
	
	ProtocolId test_msgId1 = kProtocolDefaultId;
	ProtocolId test_msgId2 = kProtocolDefaultId; // Should be 0x31, will fail
	
	ProtocolVersion test_msgVersion1 = kProtocolDefaultVersion;
	ProtocolVersion test_msgVersion2 = 0x02;
	
	assert(msgType1 == test_msgType1);
	assert(msgType2 == test_msgType2);
	assert(msgId1 == test_msgId1);
	assert(msgId2 != test_msgId2);  // Should be 0x31, will fail
	assert(msgVersion1 == test_msgVersion1);
	assert(msgVersion2 == test_msgVersion2);
	
	assert(protocolUnpackHeader(&bitstream, test_msgId1, test_msgVersion1, test_msgType1) == UnpackValid);
	assert(protocolUnpackHeader(&bitstream, test_msgId2, test_msgVersion2, test_msgType2) == UnpackInvalid); // Must be invalid
	
	LOG_TEST_END;
}

/*static void test_attributeId()
{
	LOG_TEST_START;
	
	const size_t data_length = 256;
	uint8_t data[data_length];
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	char * str1 = "teste string teste string 123 string@teste.com";
	char * str2 = "com.laugga.backend";
	
	protocolPackAttributeId(&bitstream, str1);
	protocolPackAttributeId(&bitstream, str2);
		
	bitstream_reset(&bitstream);

	char test_str1[data_length];
	char test_str2[data_length];

	assert(protocolUnpackAllAttributeId(&bitstream, test_str1, data_length) == UnpackValid);
	assert(protocolUnpackAllAttributeId(&bitstream, test_str2, data_length) == UnpackValid);
	
	assert(strcmp(str1, test_str1) == 0);
	assert(strcmp(str2, test_str2) == 0);
	
	LOG_TEST_END;
}

static void test_attributeAddress()
{
	LOG_TEST_START;
	
	const size_t data_length = 256;
	uint8_t data[data_length];
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	net_addr_t addr1;
	net_addr_local(&addr1);
	net_addr_t addr2;
	net_addr_set(&addr2, 123567, 1234, true);
	net_addr_t addr3;
	net_addr_set(&addr3, 123567, 1234, true);
	
	protocolPackAttributeAddress(&bitstream, ProtocolAttributeLocalAddress, &addr1);
	protocolPackAttributeAddress(&bitstream, ProtocolAttributeMappedAddress, &addr2);
	protocolPackAttributeAddress(&bitstream, ProtocolAttributeMappedAddress, &addr3);
		
	bitstream_reset(&bitstream);

	net_addr_t test_addr1;
	net_addr_t test_addr2;
	net_addr_t test_addr3;

	assert(protocolUnpackAllAttributeAddress(&bitstream, ProtocolAttributeLocalAddress, &test_addr1) == UnpackValid);
	assert(protocolUnpackAllAttributeAddress(&bitstream, ProtocolAttributeLocalAddress, &test_addr2) == UnpackUnexpected);
	assert(protocolUnpackAllAttributeAddress(&bitstream, ProtocolAttributeMappedAddress, &test_addr3) == UnpackValid);
		
	assert(net_addr_is_equal(&addr1, &test_addr1));
	assert(net_addr_is_equal(&addr2, &test_addr2));
	assert(net_addr_is_equal(&addr3, &test_addr3));
	
	LOG_TEST_END;
}

static void test_attributeExpireSeconds()
{
	LOG_TEST_START;
	
	const size_t data_length = 256;
	uint8_t data[data_length];
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	time_t seconds1;
	time_t seconds2;

	seconds1 = 3453;
	seconds2 = 4294967295;
	
	protocolPackAttributeExpireSeconds(&bitstream, seconds1);
	protocolPackAttributeExpireSeconds(&bitstream, seconds2);
		
	bitstream_reset(&bitstream);

	time_t test_seconds1;
	time_t test_seconds2;

	assert(protocolUnpackAllAttributeExpireSeconds(&bitstream, &test_seconds1) == UnpackValid);
	assert(protocolUnpackAllAttributeExpireSeconds(&bitstream, &test_seconds2) == UnpackValid);

	assert(seconds1 == test_seconds1);
	assert(seconds2 == test_seconds2);
	
	LOG_TEST_END;
}

static void test_presenceRequest1()
{
	LOG_TEST_START;
	
	const size_t data_length = 256;
	uint8_t data[data_length];
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	char * strid = "string@teste.com";
	
	net_addr_t addr1;
	net_addr_local(&addr1);
	net_addr_t addr2;
	net_addr_set(&addr2, 123567, 1234, true);

	protocolPackPresenceRequest(&bitstream, true, strid, &addr1, &addr2);
	
	bitstream_reset(&bitstream);

	ProtocolPresenceRequest presenceRequest;
	protocolUnpackPresenceRequest(&bitstream, &presenceRequest);
	
	assert(strcmp(strid, presenceRequest.id) == 0);
	assert(net_addr_is_equal(&addr1, &presenceRequest.localAddress));
	assert(net_addr_is_equal(&addr2, &presenceRequest.mappedAddress));
	
	LOG_TEST_END;
}

static void test_presenceRequest2()
{
	LOG_TEST_START;
	
	const size_t data_length = 256;
	uint8_t data[data_length];
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	char * strid = "string@teste.com";

	protocolPackPresenceRequest(&bitstream, false, strid, NULL, NULL);
	
	bitstream_reset(&bitstream);

	ProtocolPresenceRequest presenceRequest;
	protocolUnpackPresenceRequest(&bitstream, &presenceRequest);
	
	assert(strcmp(strid, presenceRequest.id) == 0);
	assert(net_addr_is_valid(&presenceRequest.localAddress) == false);
	assert(net_addr_is_valid(&presenceRequest.mappedAddress) == false);
	
	LOG_TEST_END;
}

static void test_notifyRequest()
{
	LOG_TEST_START;
	
	const size_t data_length = 256;
	uint8_t data[data_length];
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	char * strid1 = "string@teste.com";
	
	net_addr_t addr11;
	net_addr_local(&addr11);
	net_addr_t addr12;
	net_addr_set(&addr12, 123567, 1234, true);
	
	char * strid2 = "peer@string.com";
	
	net_addr_t addr21;
	net_addr_set(&addr21, 99999, 22, true);
	net_addr_t addr22;
	net_addr_local(&addr22);

	protocolPackNotifyRequestCount(&bitstream, 2);
	protocolPackNotifyRequestPeer(&bitstream, strid1, &addr11, &addr12);
	protocolPackNotifyRequestPeer(&bitstream, strid2, &addr21, &addr22);
	
	bitstream_reset(&bitstream);

	char test_strid1[60];
	char test_strid2[60];
	net_addr_t test_addr11, test_addr12, test_addr21, test_addr22;
	
	ProtocolNotifyRequest notifyRequest;
	protocolUnpackNotifyRequest(&bitstream, &notifyRequest);
	
	assert(notifyRequest.peersCount == 2);
	
	assert(strcmp(notifyRequest.peersArray[0].id, strid1) == 0);
	assert(strcmp(notifyRequest.peersArray[1].id, strid2) == 0);
	assert(net_addr_is_equal(&notifyRequest.peersArray[0].localAddress, &addr11));
	assert(net_addr_is_equal(&notifyRequest.peersArray[1].localAddress, &addr21));
	assert(net_addr_is_equal(&notifyRequest.peersArray[0].mappedAddress, &addr12));
	assert(net_addr_is_equal(&notifyRequest.peersArray[1].mappedAddress, &addr22));
	
	LOG_TEST_END;
}*/

int main(void)
{	
	LOG_SUITE_START("protocol");

	test_header();
	/*test_attributeId();
	test_attributeAddress();
	test_attributeExpireSeconds();
	test_presenceRequest1();
	test_presenceRequest2();
	test_notifyRequest();
	*/
	
	return 0;
}
