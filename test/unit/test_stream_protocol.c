/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_stream_protocol.c
* universal-network-c
*/

#include "test.h"
#include "protocol.h"
#include "stream_protocol.h"

static void test_stream_protocol_header()
{
	LOG_TEST_START;
	
	const size_t data_length = 100;
	uint8_t data[data_length];
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	Sequence test_sequence = 23;
	Ack test_ack = 22;
	AckBitField test_ackBitField = 0x9323ac0d;
	
	Sequence unpack_test_sequence = 0;
	Ack unpack_test_ack = 0;
	AckBitField unpack_test_ackBitField = 0;
	
	streamProtocolPackHeader(&bitstream, test_sequence, test_ack, test_ackBitField);
	bitstream_reset(&bitstream);
	streamProtocolUnpackHeader(&bitstream, &unpack_test_sequence, &unpack_test_ack, &unpack_test_ackBitField);
	
	assert(unpack_test_sequence == test_sequence);
	assert(unpack_test_ack == test_ack);
	assert(unpack_test_ackBitField == test_ackBitField);
	
	LOG_TEST_END;
}

static void test_stream_protocol_data()
{
	LOG_TEST_START;
	
	const size_t data_length = 100;
	uint8_t data[data_length];
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	const char * test_string = "test_stream_protocol_data\0";
	
	StreamObject test_object;
	streamObjectSetup(&test_object);
  test_object.tag = 49284398493;
  streamObjectCopyData(&test_object, (uint8_t *)test_string, strlen(test_string));
  
	StreamObject unpack_test_object;
	streamObjectSetup(&unpack_test_object);
	
	streamProtocolPackData(&bitstream, &test_object);
	bitstream_reset(&bitstream);
	streamProtocolUnpackData(&bitstream, &unpack_test_object);
	
	assert(unpack_test_object.tag == test_object.tag);
	assert(unpack_test_object.length == test_object.length);
	assert(memcmp(unpack_test_object.data, test_object.data, test_object.length) == 0);
	
	LOG_TEST_END;
}

int main(void)
{	
	LOG_SUITE_START("stream_protocol");

	test_stream_protocol_header();
	test_stream_protocol_data();
	
	return 0;
}
