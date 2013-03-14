/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stream.h
* universal-network-c
*/

#ifndef __universal_network_stream_h__
#define __universal_network_stream_h__

#include "stream_protocol.h"

#include "net.h"
#include "bitstream.h"
#include "timeout.h"
#include "list.h"

/*!
 * @header
 *
 * Stream is a high-level abstraction for active connections between peers. After the negotiation
 * phase (which takes place in transactions), the peer sets a virtual channel or connection
 * using the stream for application specific data exchange.
 *
 * The stream works on a dedicated socket, using UDP. The stream also adds reliability and 
 * flow control on top of UDP.
 */

typedef void (*StreamUpdateCallback)(void *, bitstream_t *); // Update - Send data
typedef void (*StreamReceiveCallback)(void *, net_addr_t *, bitstream_t *); // Receive data
typedef void (*StreamTimeoutCallback)(void *, net_addr_t *); // Timeout
typedef void (*StreamSuspendCallback)(void *); // Suspend

/*!
 * @typedef StreamConfiguration
 * @abstract Keeps track of multiple stream connections, associated socket and related configuration data
 */
typedef struct {
	net_socket_t socket; // Associated socket
    net_addr_t address; // Local socket address 
	
	dispatch_queue_t streamDispatchQueue; // Dispatch queue used to synchronize access to streams
    dispatch_source_t streamDispatchTimer; // Dispatch timer used to update connected streams with data
    float logAccumulator; // Time accumulator before next status log (Debug only)
	bool active; // Suspend/Resume with change active state
	
	StreamReceiveCallback receiveCallback; // Receive data callback (called on incoming data)
	StreamUpdateCallback updateCallback; // Update data callback (called by local update timer)
	StreamTimeoutCallback timeoutCallback; // Stream connected timeout callback (called when a stream becomes irresponsive)
    StreamSuspendCallback suspendCallback; // Stream suspend callback (called when there are no streams left and update timer is suspended)
	void * context; // Context callback object
	
	list_t streams; // List of active streams
} StreamConfiguration;

NetError streamSetup(StreamConfiguration *, const unsigned int, void *, StreamUpdateCallback, StreamReceiveCallback, StreamTimeoutCallback, StreamSuspendCallback); // StreamUpdateCallback is mandatory
void streamTeardown(StreamConfiguration *);

void streamSuspend(StreamConfiguration *);
void streamResume(StreamConfiguration *);

void streamAdd(StreamConfiguration *, const net_addr_t *);
void streamRemove(StreamConfiguration *, const net_addr_t *);
bool streamDoesExist(StreamConfiguration *, const net_addr_t *);

bool streamListIsEmpty(StreamConfiguration *);

#endif
