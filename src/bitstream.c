/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* bitstream.c
* universal-network-c
*/

#include "bitstream.h"

#define mBitstreamBytesDoFit(stream, bytes) ((bytes) <= (stream->bound-stream->offset))
#define mBitstreamHasEnoughBytes(stream, bytes) ((bytes) <= (stream->bound-stream->offset))

bitstream_t bitstream_create(uint8_t * data, size_t bound)
{
	bitstream_t b;
	b.offset = 0; 	 // First byte
	b.bound = bound; // Offset bound (ie. Bound = 10, means last valid offset should be 10, 10 Byte packet)
	b.data = data;
	return b;
}

void bitstream_reset(bitstream_t * stream)
{
	stream->offset = 0;
}

void bitstream_skip_bytes(bitstream_t * stream, size_t length)
{
	if(!mBitstreamHasEnoughBytes(stream, length)) // Either skip all 'length; bytes or nothing if overflows bounds
		return;
		
	stream->offset += length;
	if(stream->offset > stream->bound)
		stream->offset = stream->bound; // Reached end of stream
}

bitstream_snapshot_t bitstream_snapshot(bitstream_t * stream)
{
	bitstream_snapshot_t snapshot;
	snapshot.rollover_offset = snapshot.rollback_offset = stream->offset;
	snapshot.bitstream = stream;
	
	return snapshot;
}

void bitstream_rollback(bitstream_t * stream, bitstream_snapshot_t * snapshot)
{
	if(snapshot->bitstream)
	{
		snapshot->rollover_offset = stream->offset;
		stream->offset = snapshot->rollback_offset;
	}
}

void bitstream_rollover(bitstream_t * stream, bitstream_snapshot_t * snapshot)
{
	if(snapshot->bitstream)
	{	
		snapshot->rollback_offset = stream->offset;
		stream->offset = snapshot->rollover_offset;
	}
}

int bitstream_write_bytes(bitstream_t * stream, uint8_t * src, size_t length)
{
	if(!mBitstreamBytesDoFit(stream, length)) // Either write all bytes or nothing if doesn't fit
		return 0;
		
#ifdef __LITTLE_ENDIAN__
	for(int b=0; b<length; ++b)
		stream->data[stream->offset+length-b-1] = src[b]; // Swaps byte order
#else
	memcpy(&stream->data[stream->offset], src, length); // DOES NOT Swap byte order (only works in big-endian systems)
#endif
	stream->offset += length;
	
	return length;
}

int bitstream_read_bytes(bitstream_t * stream, uint8_t * dest, size_t length)
{
	if(!mBitstreamHasEnoughBytes(stream, length)) // Either read all bytes or nothing if there's less then 'length' bytes left in bitstream
		return 0;
		
#ifdef __LITTLE_ENDIAN__
	for(int b=0; b<length; ++b)
		dest[b] = stream->data[stream->offset+length-b-1]; // Swaps byte order
#else
	memcpy(dest, &stream->data[stream->offset], length); // DOES NOT Swap byte order (only works in big-endian systems)
#endif		
	stream->offset += length;
	
	return length;
}

void bitstream_write_uint8(bitstream_t * stream, unsigned int src)
{
	if(!mBitstreamBytesDoFit(stream, 1)) // Do nothing if doesn't fit
		return;
		
	stream->data[stream->offset++] = (uint8_t)(src & 0xFF);
}

void bitstream_read_uint8(bitstream_t * stream, unsigned int * dest)
{
	*dest = (unsigned int)(stream->data[stream->offset++]);
}

void bitstream_write_uint16(bitstream_t * stream, unsigned int src)
{
	if(!mBitstreamBytesDoFit(stream, 2)) // Do nothing if doesn't fit
		return;
		
#ifdef __LITTLE_ENDIAN__
	stream->data[stream->offset++] = (uint8_t)((src >> 8) & 0xFF); // Swaps byte order (only works in little-endian systems)
	stream->data[stream->offset++]= (uint8_t)(src & 0xFF);
#else
	stream->data[stream->offset++] = (uint8_t)(src & 0xFF); // Don't swap byte order
	stream->data[stream->offset++]= (uint8_t)((src >> 8) & 0xFF);
#endif
}

void bitstream_read_uint16(bitstream_t * stream, unsigned int * dest)
{
#ifdef __LITTLE_ENDIAN__
	*dest = (((unsigned int) stream->data[stream->offset++] << 8) | // Swaps byte order (only works in little-endian systems)
             ((unsigned int) stream->data[stream->offset++] ));
#else
	*dest = (((unsigned int) stream->data[stream->offset++]) | // Don't swap byte order
             ((unsigned int) stream->data[stream->offset++] << 8 ));
#endif
}

void bitstream_write_uint16_endian(bitstream_t * stream, unsigned int src)
{
	if(!mBitstreamBytesDoFit(stream, 2)) // Do nothing if doesn't fit
		return;
		
	stream->data[stream->offset++] = (uint8_t)(src & 0xFF); // Don't swap byte order
	stream->data[stream->offset++]= (uint8_t)((src >> 8) & 0xFF);
}

void bitstream_read_uint16_endian(bitstream_t * stream, unsigned int * dest)
{
	*dest = (((unsigned int) stream->data[stream->offset++]) | // Don't swap byte order
             ((unsigned int) stream->data[stream->offset++] << 8 ));
}

void bitstream_write_uint32(bitstream_t * stream, unsigned int src)
{
	if(!mBitstreamBytesDoFit(stream, 4)) // Do nothing if doesn't fit
		return;
		
#ifdef __LITTLE_ENDIAN__
	stream->data[stream->offset++] = (uint8_t)(src >> 24);// Swaps byte order (only works in little-endian systems)
	stream->data[stream->offset++] = (uint8_t)((src >> 16) & 0xFF);
	stream->data[stream->offset++] = (uint8_t)((src >> 8) & 0xFF);
	stream->data[stream->offset++] = (uint8_t)(src & 0xFF);
#else
	stream->data[stream->offset++] = (uint8_t)(src & 0xFF);// Don't swap byte order
	stream->data[stream->offset++] = (uint8_t)((src >> 8) & 0xFF);
	stream->data[stream->offset++] = (uint8_t)((src >> 16) & 0xFF);
	stream->data[stream->offset++] = (uint8_t)(src >> 24);
#endif
}

void bitstream_read_uint32(bitstream_t * stream, uint32_t * dest)
{
#ifdef __LITTLE_ENDIAN__
	*dest = (((unsigned int)stream->data[stream->offset++] << 24) | // WEAK Swaps byte order (only works in little-endian systems)
             ((unsigned int)stream->data[stream->offset++] << 16) | 
             ((unsigned int)stream->data[stream->offset++] << 8)  | 
             ((unsigned int)stream->data[stream->offset++]));
#else
	*dest = (((unsigned int)stream->data[stream->offset++]) | // WEAK Swaps byte order (only works in little-endian systems)
             ((unsigned int)stream->data[stream->offset++] << 8) | 
             ((unsigned int)stream->data[stream->offset++] << 16) | 
             ((unsigned int)stream->data[stream->offset++] << 24));
#endif
}

void bitstream_write_uint32_endian(bitstream_t * stream, unsigned int src)
{
	if(!mBitstreamBytesDoFit(stream, 4)) // Do nothing if doesn't fit
		return;
		
	stream->data[stream->offset++] = (uint8_t)(src & 0xFF);// Don't swap byte order
	stream->data[stream->offset++] = (uint8_t)((src >> 8) & 0xFF);
	stream->data[stream->offset++] = (uint8_t)((src >> 16) & 0xFF);
	stream->data[stream->offset++] = (uint8_t)(src >> 24);
}

void bitstream_read_uint32_endian(bitstream_t * stream, uint32_t * dest)
{
	*dest = (((unsigned int)stream->data[stream->offset++]) | // WEAK Swaps byte order (only works in little-endian systems)
             ((unsigned int)stream->data[stream->offset++] << 8) | 
             ((unsigned int)stream->data[stream->offset++] << 16) | 
             ((unsigned int)stream->data[stream->offset++] << 24));
}

void bitstream_write_uint64(bitstream_t * stream, uint64_t src)
{
	if(!mBitstreamBytesDoFit(stream, 8)) // Do nothing if doesn't fit
		return;
		
#ifdef __LITTLE_ENDIAN__
	stream->data[stream->offset++] = (uint8_t)(src >> 56);// Swaps byte order (only works in little-endian systems)
	stream->data[stream->offset++] = (uint8_t)(src >> 48);
	stream->data[stream->offset++] = (uint8_t)(src >> 40);
	stream->data[stream->offset++] = (uint8_t)(src >> 32);
	stream->data[stream->offset++] = (uint8_t)(src >> 24);
	stream->data[stream->offset++] = (uint8_t)((src >> 16) & 0xFF);
	stream->data[stream->offset++] = (uint8_t)((src >> 8) & 0xFF);
	stream->data[stream->offset++] = (uint8_t)(src & 0xFF);
#else
	stream->data[stream->offset++] = (uint8_t)(src & 0xFF);// Don't swap byte order
	stream->data[stream->offset++] = (uint8_t)((src >> 8) & 0xFF);
	stream->data[stream->offset++] = (uint8_t)((src >> 16) & 0xFF);
	stream->data[stream->offset++] = (uint8_t)(src >> 24);
	stream->data[stream->offset++] = (uint8_t)(src >> 32);
	stream->data[stream->offset++] = (uint8_t)(src >> 40);
	stream->data[stream->offset++] = (uint8_t)(src >> 48);
	stream->data[stream->offset++] = (uint8_t)(src >> 56);
#endif
}

void bitstream_read_uint64(bitstream_t * stream, uint64_t * dest)
{
#ifdef __LITTLE_ENDIAN__
	*dest = (((uint64_t)stream->data[stream->offset++] << 56) | // Swaps byte order (only works in little-endian systems)
			 ((uint64_t)stream->data[stream->offset++] << 48) |
			 ((uint64_t)stream->data[stream->offset++] << 40) |
			 ((uint64_t)stream->data[stream->offset++] << 32) |
		     ((uint64_t)stream->data[stream->offset++] << 24) | 
             ((uint64_t)stream->data[stream->offset++] << 16) | 
             ((uint64_t)stream->data[stream->offset++] << 8)  | 
             ((uint64_t)stream->data[stream->offset++]));
#else
	*dest = (((uint64_t)stream->data[stream->offset++]) |
             ((uint64_t)stream->data[stream->offset++] << 8) | 
             ((uint64_t)stream->data[stream->offset++] << 16) | 
             ((uint64_t)stream->data[stream->offset++] << 24) |
			 ((uint64_t)stream->data[stream->offset++] << 32) |
			 ((uint64_t)stream->data[stream->offset++] << 40) |
			 ((uint64_t)stream->data[stream->offset++] << 48) |
			 ((uint64_t)stream->data[stream->offset++] << 56));
#endif
}

void bitstream_write_float32(bitstream_t * stream, float src)
{
	uint32_t p;
	uint32_t sign;

	if (src < 0) { sign = 1; src = -src; }
	else { sign = 0; }

	p = ((((uint32_t)src)&0x7fff)<<16) | (sign<<31); // whole part and sign
	p |= (uint32_t)(((src - (int)src) * 65537.0f))&0xffff; // fraction

	bitstream_write_uint32(stream, p);
}

void bitstream_read_float32(bitstream_t * stream, float * dest)
{
	uint32_t p;
	
	bitstream_read_uint32(stream, &p);
	
	float f = ((p>>16)&0x7fff); // whole part
	f += (p&0xffff) / 65536.0f; // fraction

	if (((p>>31)&0x1) == 0x1) { f = -f; } // sign bit set
	
	*dest = f; // copy
}

int bitstream_write_str(bitstream_t * stream, const char * src)
{
	size_t strlength = strlen(src);
	
	if(!mBitstreamBytesDoFit(stream, strlength+1)) // Either write entire <length,string> or nothing if doesn't fit
		return 0;
					
	bitstream_write_uint8(stream, (unsigned int)strlength); // string length
	bitstream_write_bytes(stream, (uint8_t *)src, strlength); // string bytes
	
	return strlength;
}

int bitstream_read_str(bitstream_t * stream, char * dest, const size_t maxlength)
{
	size_t strlength = 0;
	
	bitstream_read_uint8(stream, (unsigned int *)&strlength); // string length
	
	if(strlength+1 > maxlength) // Either read entire string or nothing if doesn't fit destination
		return 0;
	
	bitstream_read_bytes(stream, (uint8_t *)dest, strlength); // string bytes
	dest[strlength] = '\0'; // Null terminating character
	
	return strlength;
}