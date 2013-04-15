/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stream_protocol.c
* universal-network-c
*/

#include "stream_protocol.h"

#pragma mark -
#pragma mark Header

void streamProtocolPackHeader(bitstream_t * bitstream, Sequence sequence, Ack ack, AckBitField ackBitField)
{
	// Pack protocol header
	protocolPackHeader(bitstream, kProtocolDefaultId, kProtocolDefaultVersion, ProtocolTypeStream);
	
	// Pack stream protocol header
	bitstream_write_uint32(bitstream, sequence);
	bitstream_write_uint32(bitstream, ack);
	bitstream_write_uint32(bitstream, ackBitField);
}

UnpackResult streamProtocolUnpackHeader(bitstream_t * bitstream, Sequence * sequence, Ack * ack, AckBitField * ackBitField)
{
	// Unpack protocol header
	if(protocolUnpackHeader(bitstream, kProtocolDefaultId, kProtocolDefaultVersion, ProtocolTypeStream) != UnpackValid)
		return UnpackInvalid;
	
	// Unpack stream protocol header	
	bitstream_read_uint32(bitstream, sequence);
	bitstream_read_uint32(bitstream, ack);
	bitstream_read_uint32(bitstream, ackBitField);

	return UnpackValid;
}

#pragma mark -
#pragma mark Data

void streamObjectSetup(StreamObject * object)
{
  object->length = 0;
  object->tag = 0;
}

void streamObjectCopyData(StreamObject * object, uint8_t * data, unsigned int length)
{
  if((object->length + length) <= kStreamObjectDataMaxLength)
  {
    memcpy(object->data+object->length, data, length);
    object->length += length; 
  }
}

void streamProtocolPackData(bitstream_t * bitstream, StreamObject * object)
{
  bitstream_write_uint64(bitstream, object->tag); // tag (use bitstream)
	bitstream_write_uint16(bitstream, object->length); // length (use bitstream)
	bitstream_write_bytes(bitstream, object->data, object->length); // data
}

UnpackResult streamProtocolUnpackData(bitstream_t * bitstream, StreamObject * object)
{
	UnpackResult unpackResult = UnpackValid;
	
  bitstream_read_uint64(bitstream, &object->tag); // length (use bitstream)
  bitstream_read_uint16(bitstream, &object->length); // length (use bitstream)
	bitstream_read_bytes(bitstream, object->data, object->length); // data
    
  mNetworkLog("streamProtocolUnpackData %llu => %d", object->tag, object->length);
    
  return unpackResult;
}

