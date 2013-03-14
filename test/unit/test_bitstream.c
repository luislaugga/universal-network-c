/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_bitstream.c
* universal-network-c
*/

#include "test.h"
#include "bitstream.h"

void test_bytes()
{
	const size_t data_length = 16; // 16 Bytes
	
	uint8_t data[data_length];
	uint8_t write_bytes[data_length];
	uint8_t read_bytes[data_length];
	
	for(int i=0; i<data_length; ++i)
	{
		write_bytes[i] = i;
		read_bytes[i] = 0;
	}
		
	bitstream_t bitstream = bitstream_create(data, data_length);
	bitstream_write_bytes(&bitstream, write_bytes, data_length);
	bitstream_reset(&bitstream);
	bitstream_read_bytes(&bitstream, read_bytes, data_length);
	
	assert(memcmp(write_bytes, read_bytes, data_length) == 0);

	LOG_TEST_END;
}

static void test_uint8()
{
	LOG_TEST_START;
	
	const size_t data_length = 1; // 1 Byte
	
	uint8_t data[data_length];
	unsigned int write_value = 56;
	unsigned int read_value = 0;
	uint8_t write_value_uint8 = 56;
	uint8_t read_value_uint8 = 0;
	
	bitstream_t bitstream = bitstream_create(data, data_length);
	bitstream_write_uint8(&bitstream, write_value);
	bitstream_reset(&bitstream);
	bitstream_read_uint8(&bitstream, &read_value);
	
	assert(read_value == write_value);
	
	bitstream_t bitstream_uint8 = bitstream_create(data, data_length);
	bitstream_write_uint8(&bitstream_uint8, (unsigned int)write_value_uint8);
	bitstream_reset(&bitstream_uint8);
	bitstream_read_uint8(&bitstream_uint8, (unsigned int *)&read_value_uint8);
	
	assert(read_value_uint8 == write_value_uint8);

	LOG_TEST_END;
}

void test_uint16()
{
	const size_t data_length = 2; // 2 Bytes
	
	uint8_t data[data_length];
	unsigned int write_value = 13;
	unsigned int read_value = 2393992;
	unsigned short write_value_short = 5613;
	unsigned short read_value_short = 38382;
	
	bitstream_t bitstream = bitstream_create(data, data_length);
	bitstream_write_uint16(&bitstream, write_value);
	bitstream_reset(&bitstream);
	bitstream_read_uint16(&bitstream, &read_value);
	
	assert(read_value == write_value);
	
	bitstream_t bitstream_short = bitstream_create(data, data_length);
	bitstream_write_uint16(&bitstream_short, (unsigned int)write_value_short);
	bitstream_reset(&bitstream_short);
	bitstream_read_uint16(&bitstream_short, (unsigned int *)&read_value_short);
	
	assert(read_value_short == write_value_short);

	LOG_TEST_END;
}

void test_uint16_endian()
{
	const size_t data_length = 2; // 2 Bytes
	
	uint8_t data[data_length];
	unsigned int write_value = 13;
	unsigned int read_value = 2393992;
	unsigned short write_value_short = 5613;
	unsigned short read_value_short = 38382;
	
	bitstream_t bitstream = bitstream_create(data, data_length);
	bitstream_write_uint16_endian(&bitstream, write_value);
	bitstream_reset(&bitstream);
	bitstream_read_uint16_endian(&bitstream, &read_value);
	
	assert(read_value == write_value);
	
	bitstream_t bitstream_short = bitstream_create(data, data_length);
	bitstream_write_uint16_endian(&bitstream_short, (unsigned int)write_value_short);
	bitstream_reset(&bitstream_short);
	bitstream_read_uint16_endian(&bitstream_short, (unsigned int *)&read_value_short);
	
	assert(read_value_short == write_value_short);

	LOG_TEST_END;
}

void test_uint32()
{
	const size_t data_length = 4; // 4 Bytes
	
	uint8_t data[data_length]; 
	unsigned int write_value = 0x56133499;
	unsigned int read_value = 0x00000000;
	
	bitstream_t bitstream = bitstream_create(data, data_length);
	bitstream_write_uint32(&bitstream, write_value);
	bitstream_reset(&bitstream);
	bitstream_read_uint32(&bitstream, &read_value);
	
	assert(read_value == write_value);

	LOG_TEST_END;
}

void test_uint32_endian()
{
	const size_t data_length = 4; // 4 Bytes
	
	uint8_t data[data_length]; 
	unsigned int write_value = 0x56133499;
	unsigned int read_value = 0x00000000;
	
	bitstream_t bitstream = bitstream_create(data, data_length);
	bitstream_write_uint32_endian(&bitstream, write_value);
	bitstream_reset(&bitstream);
	bitstream_read_uint32_endian(&bitstream, &read_value);
	
	assert(read_value == write_value);

	LOG_TEST_END;
}

void test_uint64()
{
	const size_t data_length = 8; // 8 Bytes
	
	uint8_t data[data_length]; 
	uint64_t write_value1 = 0x354ddccede51c3f2;
	uint64_t read_value1 = 0x0;
	
	bitstream_t bitstream = bitstream_create(data, data_length);
	bitstream_write_uint64(&bitstream, write_value1);
	bitstream_reset(&bitstream);
	bitstream_read_uint64(&bitstream, &read_value1);
	
	assert(read_value1 == write_value1);
	
	uint64_t write_value2 = 0x0;
	uint64_t read_value2 = 0x354ddccede51c3f2;

	bitstream_reset(&bitstream);	
	bitstream_write_uint64(&bitstream, write_value2);
	bitstream_reset(&bitstream);
	bitstream_read_uint64(&bitstream, &read_value2);
	
	assert(read_value2 == write_value2);

	LOG_TEST_END;
}

void test_float32()
{
	const size_t data_length = 4; // 4 Bytes
	
	uint8_t data[data_length]; 
	float write_value;
	float read_value;
	
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	write_value = 3.14f;
	read_value = 0.0;
	
	bitstream_write_float32(&bitstream, write_value);
	bitstream_reset(&bitstream);
	bitstream_read_float32(&bitstream, &read_value);
	bitstream_reset(&bitstream);
	
	mNetworkLog("%f %f", read_value, write_value);
	assert(read_value == write_value);
	
	write_value = 0.0f;
	read_value = 0.0;
	
	bitstream_write_float32(&bitstream, write_value);
	bitstream_reset(&bitstream);
	bitstream_read_float32(&bitstream, &read_value);
	bitstream_reset(&bitstream);
	
	mNetworkLog("%f %f", read_value, write_value);
	assert(read_value == write_value);
	
	write_value = -3.14f;
	read_value = 0.0;
	
	bitstream_write_float32(&bitstream, write_value);
	bitstream_reset(&bitstream);
	bitstream_read_float32(&bitstream, &read_value);
	bitstream_reset(&bitstream);
	
	mNetworkLog("%f %f", read_value, write_value);
	assert(read_value == write_value);
	
	write_value = -0.0f;
	read_value = 0.0;
	
	bitstream_write_float32(&bitstream, write_value);
	bitstream_reset(&bitstream);
	bitstream_read_float32(&bitstream, &read_value);
	bitstream_reset(&bitstream);
	
	mNetworkLog("%f %f", read_value, write_value);
	assert(read_value == write_value);
	
	write_value = 10406.45678f;
	read_value = 0.0;
	
	bitstream_write_float32(&bitstream, write_value);
	bitstream_reset(&bitstream);
	bitstream_read_float32(&bitstream, &read_value);
	bitstream_reset(&bitstream);
	
	mNetworkLog("%f %f", read_value, write_value);
	assert(read_value == write_value);
	
	write_value = -10406.45678f;
	read_value = 0.0;
	
	bitstream_write_float32(&bitstream, write_value);
	bitstream_reset(&bitstream);
	bitstream_read_float32(&bitstream, &read_value);
	bitstream_reset(&bitstream);

	mNetworkLog("%f %f", read_value, write_value);
	assert(read_value == write_value);

	LOG_TEST_END;
}

void test_str()
{
	const size_t data_fit_length1 = 34+1; // 34+1 Bytes (write string will fit and occuppy all data's memory)
	const size_t data_fit_length2 = 100; // n >> str. length (write string will fit)
	const size_t data_nofit_length1 = 33+1; // 33+1 Bytes (write string won't fit by one byte)
	const size_t data_nofit_length2 = 5; // n << str. length (write string won't fit by one byte)
		
	uint8_t data_fit1[data_fit_length1];
	uint8_t data_fit2[data_fit_length2];
	uint8_t data_nofit1[data_nofit_length1];
	uint8_t data_nofit2[data_nofit_length2];
	
	const char * write_string = "abcdefghijklmnopqrstuvwxyzABCDEFGH"; // 34+1 bytes
	
	char read_string_fit1[data_fit_length1];
	char read_string_fit2[data_fit_length2];
	char read_string_nofit1[data_fit_length1];
	char read_string_nofit2[data_fit_length1];
	
	// Test Fit 1
	bitstream_t bitstream_fit1 = bitstream_create(data_fit1, data_fit_length1);
	size_t write_bytes_fit1 = bitstream_write_str(&bitstream_fit1, write_string);
	bitstream_reset(&bitstream_fit1);
	size_t read_bytes_fit1 = bitstream_read_str(&bitstream_fit1, read_string_fit1, data_fit_length1);
	
	assert(write_bytes_fit1 == data_fit_length1-1);
	assert(strcmp(read_string_fit1, write_string) == 0);
	assert(read_bytes_fit1 == data_fit_length1-1);
	
	// Test Fit 2
	bitstream_t bitstream_fit2 = bitstream_create(data_fit2, data_fit_length2);
	size_t write_bytes_fit2 = bitstream_write_str(&bitstream_fit2, write_string);
	bitstream_reset(&bitstream_fit2);
	size_t read_bytes_fit2 = bitstream_read_str(&bitstream_fit2, read_string_fit2, data_fit_length2);
	bitstream_reset(&bitstream_fit2);
	size_t read_bytes_fit2_fail = bitstream_read_str(&bitstream_fit2, read_string_fit2, 0);
	
	assert(write_bytes_fit2 == data_fit_length1-1);
	assert(strcmp(read_string_fit2, write_string) == 0);
	assert(read_bytes_fit2 == data_fit_length1-1);
	assert(read_bytes_fit2_fail == 0);
	
	// Test No Fit 1
	bitstream_t bitstream_nofit1 = bitstream_create(data_nofit1, data_nofit_length1);
	size_t write_bytes_nofit1 = bitstream_write_str(&bitstream_nofit1, write_string);
	bitstream_reset(&bitstream_nofit1);
	size_t read_bytes_nofit1 = bitstream_read_str(&bitstream_nofit1, read_string_nofit1, data_nofit_length1);
	
	assert(write_bytes_nofit1 == 0);
	assert(strcmp(read_string_nofit1, write_string) != 0);
	assert(read_bytes_nofit1 == 0);
	
	// Test No Fit 2
	bitstream_t bitstream_nofit2 = bitstream_create(data_nofit2, data_nofit_length2);
	size_t write_bytes_nofit2 = bitstream_write_str(&bitstream_nofit2, write_string);
	bitstream_reset(&bitstream_nofit2);
	size_t read_bytes_nofit2 = bitstream_read_str(&bitstream_nofit2, read_string_nofit2, data_nofit_length2);
	
	assert(write_bytes_nofit2 == 0);
	assert(strcmp(read_string_nofit2, write_string) != 0);
	assert(read_bytes_nofit2 == 0);

	LOG_TEST_END;
}

void test_snapshot()
{
	const size_t data_length = 100; // 4 Bytes
	
	uint8_t data[data_length]; 
	unsigned int write_value1 = 0x56133499;
	unsigned int write_value2 = 0x10031400;
	unsigned int write_value3_first = 0x49302133;
	unsigned int write_value3_snapshot = 0x49302233;
	unsigned int read_value1 = 0x00000000;
	unsigned int read_value2 = 0x00000000;
	unsigned int read_value3 = 0x00000000;
	
	bitstream_t bitstream = bitstream_create(data, data_length);
	
	bitstream_snapshot_t snapshot1 = bitstream_snapshot(&bitstream); 
	bitstream_skip_bytes(&bitstream, 4);
	
	bitstream_write_uint32(&bitstream, write_value2);
		
	bitstream_rollback(&bitstream, &snapshot1);
	bitstream_write_uint32(&bitstream, write_value1);
	bitstream_rollover(&bitstream, &snapshot1);

	bitstream_snapshot_t snapshot3 = bitstream_snapshot(&bitstream); 
	bitstream_write_uint32(&bitstream, write_value3_first);	
	bitstream_rollback(&bitstream, &snapshot3);
	bitstream_write_uint32(&bitstream, write_value3_snapshot);
	bitstream_rollover(&bitstream, &snapshot3);
	
	bitstream_reset(&bitstream);
	
	bitstream_read_uint32(&bitstream, &read_value1);
	bitstream_read_uint32(&bitstream, &read_value2);
	bitstream_read_uint32(&bitstream, &read_value3);	
	
	assert(read_value1 == write_value1);
	assert(read_value2 == write_value2);
	assert(read_value3 != write_value3_first);
	assert(read_value3 == write_value3_snapshot);
	
	LOG_TEST_END;	
}

int main(void)
{
	LOG_SUITE_START("bitstream");
	
	test_bytes();
	test_uint8();
	test_uint16();
	test_uint16_endian();
	test_uint32();
	test_uint32_endian();
	test_uint64();
	test_float32();
	test_str();
	test_snapshot();
	
	return 0;
}
