/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stream_protocol.h
* universal-network-c
*/

#ifndef __universal_network_stream_protocol_h__
#define __universal_network_stream_protocol_h__

#include "protocol.h"
#include "universal_network_c.h"
#include "bitstream.h"
#include "net.h"

/*!
 * @header
 *
 * Stream Protocol is a higher-level interface used by clients to exchange
 * application specific data.
 * 
 * It has a dedicated channel (socket) that is previously negotiated between peers
 *
 * Data-Exchange Format:
 * 0            15 16          31 32                         63 
 * +--------------+--------------+--------------+--------------+
 * | Sequence Nr.                | Ack Nr.                     | 4+4 Bytes
 * +--------------+--------------+--------------+--------------+ 
 * | Ack Bit Field 		           |                               4 Bytes
 * +-----------------------------------------------------------+
 * | Tag                                                       | 8 Bytes
 * +--------------+--------------+--------------+--------------+
 * | Body (length)|                                              2 Bytes
 * +--------------+--------------+--------------+--------------+
 * | Body (data)                                               | ? Bytes
 * | ...                                                       | 
 * +--------------+--------------+--------------+--------------+
 *
 * Data: Application specific, length must conform to packet's max. size 
 */

#define kStreamObjectDataMaxLength (kProtocolMaxLength-22)

/*!
 * @typedef SequenceNr
 * @abstract Current local sequence, increased every time a new packet is sent
 */
typedef unsigned int Sequence;

/*!
 * @typedef SequenceNr
 * @abstract Last acked sequence received
 */
typedef unsigned int Ack;

/*!
 * @typedef SequenceNr
 * @abstract Last acked sequence bit field received
 */
typedef unsigned int AckBitField; 

void streamProtocolPackHeader(bitstream_t * bitstream, Sequence, Ack, AckBitField);
UnpackResult streamProtocolUnpackHeader(bitstream_t * bitstream, Sequence *, Ack *, AckBitField *);

/*!
 * @typedef StreamObject
 * @abstract Generic object containing data associated with any stream exchange
 * @discussion
 */
typedef struct {
	uint8_t data[kStreamObjectDataMaxLength];
    unsigned int length;
    uint64_t tag;
} StreamObject;

void streamObjectSetup(StreamObject *);
void streamObjectCopyData(StreamObject *, uint8_t *, unsigned int);

void streamProtocolPackData(bitstream_t * bitstream, StreamObject *);
UnpackResult streamProtocolUnpackData(bitstream_t * bitstream, StreamObject *);

#endif