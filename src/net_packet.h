/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* net_packet.h
* universal-network-c
*/

#ifndef __universal_network_net_packet_h__
#define __universal_network_net_packet_h__

#include <dispatch/dispatch.h>

#include "bitstream.h"
#include "net_error.h"
#include "net_addr.h"

#define kNetPacketMaxLen 256

struct net_packet_s {
	struct sockaddr_in addr;
	uint8_t data[kNetPacketMaxLen];
	size_t length; // Length <= kNetPacketMaxLen, set when sent or received
	bitstream_t bitstream;
};

typedef struct net_packet_s * net_packet_t;

void net_packet_init(net_packet_t p);
size_t net_packet_len(net_packet_t p);

void net_packet_set_data(net_packet_t, uint8_t *, size_t);

void net_packet_set(net_packet_t p, const char * host, const int port);
void net_packet_addr(net_packet_t p, net_addr_t * addr);

#endif