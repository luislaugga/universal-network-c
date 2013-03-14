/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stream_internal.h
* universal-network-c
*/

#ifndef __lUniversal_network_stream_internal_h__
#define __lUniversal_network_stream_internal_h__

#include "net.h"
#include "network.h"
#include "hashtable.h"
#include "bitstream.h"

#include "stream_protocol.h"
#include "stream_reliability.h"
#include "stream_flow.h"

/*!
 * @header
 *
 * Internal stream structs and functions
 *
 * The stream is responsible for handling multiple connections (to multiple peers) in the
 * data exchange process (sending and receiving application data updates)
 */

#define kStreamTimeout 5.0 // 5 secs.
#define kStreamTimerUpdateInterval (1.0f/kStreamFlowMaxRate)
#define kStreamLogStatusInterval 5.0 // 5 secs.

void streamTimerCallback(void *);
void streamSocketReceiveCallback(void *, net_packet_t);

/*!
 * @typedef StreamState
 * @abstract Possible states of a transaction: Waiting, Connected, Disconnected, Timeout
 */
typedef enum {
	StreamWaiting,		// Stream was just added, waiting first ack from remote peer
    StreamConnected,	// Stream is active and exchanging data between peers
   	StreamDisconnected, // Stream was deactivated
	StreamTimeout, 	 	// Stream didn't receive any ack from remote peer for the specified timeout
} StreamState;

/*!
 * @typedef Stream
 * @abstract Contains all the data associated with a single transaction: ID, Packet, Packet bitstream_t, Timeout and State
 * @discussion
 * The transaction can be discarded on TransactionAcknowledged or on TransactionError/TransactionTimeout after notifying
 * with callback about error/timeout
 */
typedef struct StreamStruct {
	StreamState state; 				// State
	StreamReliability reliability;  // Reliability
    StreamFlow flow;				// Flow control
	net_addr_t address; 			// Remote side address
	float timeoutAccumulator;		// Time accumulator before timeout
    float updateAccumulator;		// Time accumulator before next update
   	struct StreamStruct * next;
	struct StreamStruct * previous;
} Stream;

Stream * streamCreate(const net_addr_t *);
void streamDestroy(Stream **);

bool streamUpdate(StreamConfiguration *, Stream *); // Returns true if it's time to update data
void streamSend(StreamConfiguration *, Stream *, StreamObject *);
void streamReceive(StreamConfiguration *, Stream *, Sequence, Ack, AckBitField, StreamObject *);
void streamTimeout(StreamConfiguration *, Stream *);
void streamLog(Stream *);

#endif