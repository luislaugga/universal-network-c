/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* bitstream.h
* universal-network-c
*/

#ifndef __universal_network_bitstream_h__
#define __universal_network_bitstream_h__

/*!
 * bitstream.h is a Universal.framework public header
 * Due to c++ function name mangling, we need to explicitly tell linker this is a c header (c function names are not mangled)
 * This is required in order to link Universal.framework library with any c++/obj-c++ code
 */
#ifdef __cplusplus 
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define __LITTLE_ENDIAN__
#elif __BYTE_ORDER == __BIG_ENDIAN
#define __BIG_ENDIAN__
#endif
#endif

/*!
 * @typedef bitstream_t
 *
 * @abstract Bit stream associated with a memory block. Allows writing/reading operations.
 * @discussion 
 * bitstream_t is always associated to a block of memory 'data' and has a memory size 'bound' defined to avoid overflow.
 * It is used for network serialization purposes. All serialized data is in big-endian format.
 * Whether reading/writing it always starts at 'data' first byte and updates 'offset' by the respective nr. of bytes after each operation.
 */
typedef struct {
	uint8_t * data; // Data containing serialized values
	size_t offset;  // Byte offset (read or write)
	size_t bound; 	// Byte bound of memory pointed by data
} bitstream_t;

bitstream_t bitstream_create(uint8_t * data, size_t bound);
void bitstream_reset(bitstream_t * stream);
void bitstream_skip_bytes(bitstream_t * stream, size_t length);

/*!
 * @typedef bitstream_mark_t
 *
 * @abstract Snapshot of the bitstream current state for later rollback
 * @discussion 
 * A snapshot initially registers a rollback+rollover offset on the bitstream current offset.
 * A rollback operation:
	* 1. Set a rollover offset on the bitstream current offset
 	* 2. Put the bitstream back on the rollback offset, set during snapshot. 
 * A rollover operation:
	* 1. Set a rollback offset on the bitstream current offset
 	* 2. Put the bitstream back on the rollover offset, set during previous rollback.
 */
typedef struct {
	size_t rollback_offset; // Snapshot rollback offset, set on rollover
	size_t rollover_offset; // Snapshot rollover offset, set on rollback
	bitstream_t * bitstream;  // Bitstream associated
} bitstream_snapshot_t;

bitstream_snapshot_t bitstream_snapshot(bitstream_t * stream);
void bitstream_rollback(bitstream_t * stream, bitstream_snapshot_t * snapshot);
void bitstream_rollover(bitstream_t * stream, bitstream_snapshot_t * snapshot);

// n Bytes
int bitstream_write_bytes(bitstream_t * stream, uint8_t * src, size_t length);
int bitstream_read_bytes(bitstream_t * stream, uint8_t * dest, size_t length);

// 1 Byte, unsigned integer
void bitstream_write_uint8(bitstream_t * stream, unsigned int src);
void bitstream_read_uint8(bitstream_t * stream, unsigned int * dest);

// 2 Byte, unsigned integer
void bitstream_write_uint16(bitstream_t * stream, unsigned int src);
void bitstream_read_uint16(bitstream_t * stream, unsigned int * dest);

// 2 Byte, unsigned integer (no endianness convertion to big-endian/network-byte-order)
void bitstream_write_uint16_endian(bitstream_t * stream, unsigned int src);
void bitstream_read_uint16_endian(bitstream_t * stream, unsigned int * dest);

// 4 Byte, unsigned integer
void bitstream_write_uint32(bitstream_t * stream, unsigned int src);
void bitstream_read_uint32(bitstream_t * stream, unsigned int * dest);

// 4 Byte, unsigned integer (no endianness convertion to big-endian/network-byte-order)
void bitstream_write_uint32_endian(bitstream_t * stream, unsigned int src);
void bitstream_read_uint32_endian(bitstream_t * stream, unsigned int * dest);

// 8 Byte, unsigned integer
void bitstream_write_uint64(bitstream_t * stream, uint64_t src);
void bitstream_read_uint64(bitstream_t * stream, uint64_t * dest);

// 4 Byte, float
void bitstream_write_float32(bitstream_t * stream, float src);
void bitstream_read_float32(bitstream_t * stream, float * dest);

// n Bytes, string, char *
int bitstream_write_str(bitstream_t * stream, const char * src);
int bitstream_read_str(bitstream_t * stream, char * dest, const size_t maxlength);

#ifdef __cplusplus
}
#endif    
    
#endif