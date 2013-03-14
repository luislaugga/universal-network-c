/*
 
 net_socket.h
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

#ifndef __universal_network_net_socket_h__
#define __universal_network_net_socket_h__

#include <dispatch/dispatch.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <Block.h> // Required for socket receive "callback" (read source events)

#include "queue.h"
#include "pool.h"
#include "net_error.h"
#include "net_addr.h"
#include "net_packet.h"

#define kNetPacketPoolCapacity 64

typedef void (^net_socket_receive_block_t)(net_packet_t);
typedef void (*net_socket_receive_callback_t)(void *, void *);
typedef void * net_socket_receive_callback_context_t;

struct StunStruct;

typedef struct StunStruct * stun_t;

struct net_socket_s {
	int fd;
	net_addr_t sockaddr;
	
	dispatch_source_t readDispatchSource; // DISPATCH_SOURCE_TYPE_READ for socket file descriptor
    dispatch_source_t writeDispatchSource; // DISPATCH_SOURCE_TYPE_WRITE for socket file descriptor
	dispatch_queue_t socketDispatchQueue;
	
	int isSending;
	
	queue_t pendingPackets;
	pool_t poolPackets;
	
	net_socket_receive_callback_t receiveCallback;
	net_socket_receive_callback_context_t receiveCallbackContext;
	net_socket_receive_block_t receiveBlock;
};

typedef struct net_socket_s * net_socket_t;

net_socket_t net_socket_create(NetError * error, int domain, const char * host, const int port);
void net_socket_destroy(net_socket_t s);

net_packet_t net_packet_alloc(net_socket_t s); // Caller assumes ownership for allocated net_packet_t
void net_packet_free(net_socket_t s, net_packet_t p); // Should be used with CAUTION, will free net_packet_t
void net_packet_retain(net_socket_t s, net_packet_t p); // Retain packet
void net_packet_release(net_socket_t s, net_packet_t p); // Preferred way of releasing a net_packet_t when no longer needed

void net_socket_set_receive_callback(net_socket_t s, void *, void (*receiveCallback)(void *, net_packet_t));
void net_socket_set_receive_block(net_socket_t, net_socket_receive_block_t);

void net_socket_send(net_socket_t s, net_packet_t packet);
void net_socket_local_addr(net_socket_t s, net_addr_t * addr);

typedef struct {
	net_socket_t socket;
	net_socket_receive_callback_t receiveCallback;
	net_socket_receive_callback_context_t receiveCallbackContext;
	net_socket_receive_block_t receiveBlock;
} net_socket_info_s;

void net_socket_copy_info(net_socket_t socket, net_socket_info_s * info); // Copies net_socket callback info to net_socket_info_s
void net_socket_set_info(net_socket_t socket, net_socket_info_s * info); // Sets back net_socket_info_s callback info to net_socket

#endif