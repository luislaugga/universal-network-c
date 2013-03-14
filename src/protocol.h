/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* protocol.h
* universal-network-c
*/

#ifndef __universal_network_protocol_h__
#define __universal_network_protocol_h__

#include "universal_network_c.h"
#include "bitstream.h"
#include "net.h"

/*!
 * @header
 *
 * Protocol is a generic-level interface used by server and client to exchange
 * data in the context of a transaction or existing stream between peers.
 * 
 * It provides a common set of field that identify transaction or stream packets and
 * an protocol version+version associated with a specific application
 *
 * Format:
 * 0            7 8          15 16         23 
 * +-------------+-------------+-------------+
 * | Id          | Version     | Type        | 0 - 2
 * +-------------+-------------+-------------+ 
 * |	Transaction/Stream Body (...)        | 3 - 255
 * +-------------+-------------+-------------+
 *
 */

#define kProtocolMaxLength kNetPacketMaxLen

typedef enum {
	UnpackValid = 0, // Valid protocol
	UnpackUnexpected = 1, // Unexpected value
	UnpackInvalid = -1, // Invalid protocol
} UnpackResult;

#define kProtocolDefaultId 0xa2 // Default protocol id
#define kProtocolDefaultVersion 0x01 // Default protocol version

/*!
 * @typedef ProtocolId
 * @abstract Unique code used to identify a specific application
 */
typedef unsigned int ProtocolId;

/*!
 * @typedef ProtocolVersion
 * @abstract Version number associated with application
 */
typedef unsigned int ProtocolVersion;

/*!
 * @typedef ProtocolType
 *
 * @abstract Transaction or Stream
 * @discussion 
 * Transaction is a 2-way communication, a request is followed by a response (ie. ACK)
 * Stream is a connection between 2 peers, previously negotiated over a channel
 * 
 * Pack/Unpack: ProtocolId, ProtocolVersion and ProtocolType
 */

typedef enum {	
    ProtocolTypeTransaction = 0x01,
	ProtocolTypeStream = 0x05,
} ProtocolType;

void protocolPackHeader(bitstream_t * bitstream, ProtocolId, ProtocolVersion, ProtocolType);
UnpackResult protocolUnpackHeader(bitstream_t * bitstream, ProtocolId, ProtocolVersion, ProtocolType);

#endif