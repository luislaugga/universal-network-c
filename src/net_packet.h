/*
 
 net_packet.h
 universal-network-c
 
 Copyright (cc) 2012 Luis Laugga.
 Some rights reserved, all wrongs deserved.
 
 Licensed under a Creative Commons Attribution-ShareAlike 3.0 License;
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://creativecommons.org/licenses/by-sa/3.0/
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" basis,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 
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