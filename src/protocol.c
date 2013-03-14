/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* protocol.c
* universal-network-c
*/

#include "protocol.h"

void protocolPackHeader(bitstream_t * bitstream, ProtocolId protocolId, ProtocolVersion protocolVersion, ProtocolType protocolType)
{
	bitstream_write_uint8(bitstream, protocolId); // unique id
	bitstream_write_uint8(bitstream, protocolVersion); // current version
	bitstream_write_uint8(bitstream, protocolType); // type
}

UnpackResult protocolUnpackHeader(bitstream_t * bitstream, ProtocolId protocolId, ProtocolVersion protocolVersion, ProtocolType protocolType)
{
	unsigned int readProtocolId, readProtocolVersion, readProtocolType;
	
	bitstream_read_uint8(bitstream, &readProtocolId); // unique id
	if(protocolId != readProtocolId)
		return UnpackInvalid;

	bitstream_read_uint8(bitstream, &readProtocolVersion); // current version
	if(protocolVersion != readProtocolVersion)
		return UnpackInvalid;
		
	bitstream_read_uint8(bitstream, &readProtocolType); // type
	if(protocolType != readProtocolType)
		return UnpackInvalid;
	
	return UnpackValid;
}