/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* net_socket.c
* universal-network-c
*/

#include "net_socket.h"
#include "net_socket_internal.h"
#include "universal_network_c.h"

#include <dispatch/dispatch.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>

#pragma mark -
#pragma mark Initialization

net_socket_t net_socket_create(NetError * error, int domain, const char * host, const int port)
{	
	net_socket_t s = (net_socket_t)calloc(1, sizeof(struct net_socket_s));	
	
	if(s) 
	{
        s->socketDispatchQueue = dispatch_queue_create("com.laugga.socketDispatchQueue", NULL); // Create send dispatch queue
		s->pendingPackets = queue_create();
		s->poolPackets = pool_create(sizeof(struct net_packet_s), kNetPacketPoolCapacity);
		
		if ((s->fd = socket(domain, SOCK_DGRAM, 0)) == -1) {
	        netErrorSetPosix(error, errno);
			net_socket_destroy(s);
	        return NULL;
	    }

	    // Set reuse address
		int reuseaddr = 1;
	    if (setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) == -1) {
	        netErrorSetPosix(error, errno);
			net_socket_destroy(s);
	        return NULL;
	    }

		// Set bind address
		struct sockaddr_in bindaddr;
		memset(&bindaddr, 0, sizeof(bindaddr));
	    bindaddr.sin_family = domain;
	    bindaddr.sin_port = htons(port);
	    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	    if (host && inet_aton(host, &bindaddr.sin_addr) == 0) {
	        netErrorSet(error, NetInvalidError);
	        net_socket_destroy(s);
	        return NULL;
	    }

		// Bind address
		if(bind(s->fd, (const struct sockaddr *)&bindaddr, sizeof(struct sockaddr_in)) == -1) {
			netErrorSetPosix(error, errno);
			net_socket_destroy(s);
	        return NULL;
		}

		// Get Sockname
		memset(&s->sockaddr, 0, sizeof(s->sockaddr));
		socklen_t sockaddrlen = sizeof(s->sockaddr);
		if(getsockname(s->fd, (struct sockaddr *)&s->sockaddr, &sockaddrlen) == -1) {
			netErrorSetPosix(error, errno);
			net_socket_destroy(s);
	        return NULL;
		}
			
		// Check sockaddr against bindaddr
		if((s->sockaddr.sin_port != bindaddr.sin_port && s->sockaddr.sin_addr.s_addr != bindaddr.sin_addr.s_addr)) {
			netErrorSet(error, NetInvalidError);
			net_socket_destroy(s);
	        return NULL;
		}
		
		// Set nonblocking
		if (net_socket_set_nonblock(error, s) != NetNoError) {
			net_socket_destroy(s);
	        return NULL;
		}

		// Set dispatch sources
		if(net_socket_set_dispatch_sources(error, s) != NetNoError) {
			net_socket_destroy(s);
	        return NULL;
		}
		
		mNetworkLog("Created socket %s:%u", inet_ntoa(s->sockaddr.sin_addr), ntohs(s->sockaddr.sin_port));		
	}
	
	*error = NetNoError; // Successfully created socket
	return s;
}

void net_socket_destroy(net_socket_t s)
{
	mNetworkLog("Closing socket");
	
    // Cancel read dispatch_source
    if(s->readDispatchSource)
    {
        dispatch_source_cancel(s->readDispatchSource);
    }
    
    // Cancel write dispatch_source
    if(s->writeDispatchSource)
    {
		net_socket_resume_write(s); // MUST resume write dispatch source before, so cancel handler is called
        dispatch_source_cancel(s->writeDispatchSource);
    }
}

void net_socket_destroy_async(net_socket_t s)
{
	// Release read dispatch_source
    if(s->readDispatchSource)
        dispatch_release(s->readDispatchSource);
    
    // Release write dispatch_source
    if(s->writeDispatchSource)
		dispatch_release(s->writeDispatchSource);
	
	// Release send dispatch queue
	dispatch_release(s->socketDispatchQueue);

	// Free pendingPackets 
	queue_destroy(s->pendingPackets);

	// Free packets pool
	pool_destroy(s->poolPackets);

	// Free socket
	free(s);
	
	mNetworkLog("Closed socket");
}

#pragma mark -
#pragma mark Packet

net_packet_t net_packet_alloc(net_socket_t s)
{
	net_packet_t packet = (net_packet_t)pool_alloc(s->poolPackets); // Alloc from pool
	net_packet_init(packet); // Set or Reset packet bitstream

	return packet;
}

void net_packet_release(net_socket_t s, net_packet_t p)
{
	pool_release(s->poolPackets, p);
}

void net_packet_retain(net_socket_t s, net_packet_t p)
{
	pool_retain(s->poolPackets, p);
}

void net_packet_free(net_socket_t s, net_packet_t p)
{
	pool_free(s->poolPackets, p);
}

#pragma mark -
#pragma mark Callback/Block

void net_socket_set_receive_callback(net_socket_t s, void * context, void (*receiveCallback)(void *, net_packet_t))
{
	s->receiveCallback = (void (*)(void *, void *))receiveCallback;
	s->receiveCallbackContext = context;
}

void net_socket_set_receive_block(net_socket_t s, net_socket_receive_block_t receiveBlock)
{
	s->receiveBlock = receiveBlock;
}

#pragma mark -
#pragma mark Send

void net_socket_send(net_socket_t s, net_packet_t packet)
{
	packet->length = packet->bitstream.offset; // Set packet's length from bitstream current offset

	if(packet->length > 0) // Don't queue up empty packets
	{
		pool_retain(s->poolPackets, packet); // Retain packet until sendto

		dispatch_async(s->socketDispatchQueue, ^{
		    queue_push(s->pendingPackets, packet); // Queue packet
			net_socket_resume_write(s); // Resume. Will not work if already resumed... WEAK
	    });
	}
}

void net_socket_local_addr(net_socket_t s, net_addr_t * addr)
{
	net_addr_local(addr);
	addr->sin_port = s->sockaddr.sin_port;
}

#pragma mark -
#pragma mark Socket Info

void net_socket_copy_info(net_socket_t socket, net_socket_info_s * info)
{
	info->socket = socket;
	info->receiveCallback = socket->receiveCallback;
	info->receiveCallbackContext = socket->receiveCallbackContext;
	info->receiveBlock = socket->receiveBlock;
}

void net_socket_set_info(net_socket_t socket, net_socket_info_s * info)
{
	socket->receiveCallback = info->receiveCallback;
	socket->receiveCallbackContext = info->receiveCallbackContext;
	socket->receiveBlock = info->receiveBlock;
}

#pragma mark -
#pragma mark Internal

NetError net_socket_set_nonblock(NetError * error, net_socket_t s)
{
    int flags;

	// Getting socket configuration
 	// Note that fcntl(2) for F_GETFL and F_SETFL can't be interrupted by a signal
    if ((flags = fcntl(s->fd, F_GETFL)) == -1) {
        netErrorSetPosix(error, errno);
        return netErrorPosix(errno);
    }

    // Set the socket nonblocking
   	if (fcntl(s->fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		netErrorSetPosix(error, errno);
        return netErrorPosix(errno);
    }

    return NetNoError;
}

NetError net_socket_set_dispatch_sources(NetError * error, net_socket_t s)
{
	dispatch_queue_t globalQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	
	// Create socket's read event source and attach to globalQueue (events are coalesced, so it can be run in a concurrent queue)
    s->readDispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, s->fd, 0, globalQueue);
    
    // Failed to create read dispatch source
    if(!s->readDispatchSource)
    {
		netErrorSet(error, NetSockError);
        return NetSockError;
    }
    
    // Create socket's write event source and attach to socketDispatchQueue
    s->writeDispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE, s->fd, 0, s->socketDispatchQueue);
    
    // Failed to create write dispatch source
    if(!s->writeDispatchSource)
    {
      	netErrorSet(error, NetSockError);
        return NetSockError;
    }

 	// Install the read event handler
    dispatch_source_set_event_handler(s->readDispatchSource, ^{
		// Allocate packet
		net_packet_t packet = net_packet_alloc(s);
		
		if(packet)
		{
	        // Datagram source address length
	        socklen_t addr_len = sizeof(packet->addr);

	        // Read into data 
			int read_flags = MSG_TRUNC; // Not standard, won't work in FreeBSD, Mac OS X and other unix systems
	        ssize_t read_bytes = recvfrom(s->fd, (uint8_t *)packet->data, kNetPacketMaxLen, read_flags, (struct sockaddr *)&packet->addr, &addr_len);
	        //mNetworkLog("recvfrom %lu", read_bytes);
        
	        if(read_bytes > 0) // Successfull
	        {
				if(read_bytes <= kNetPacketMaxLen) // ALWAYS CHECK: read_bytes will return the size of datagram received, even when truncated (MSG_TRUNC)
				{
					packet->length = read_bytes;

					if(s->receiveBlock) // block
					{
						dispatch_async(globalQueue, ^{
							s->receiveBlock(packet);
						});
					}
					else if(s->receiveCallback) // alternative callback
					{
						dispatch_async(globalQueue, ^{
	                        s->receiveCallback(s->receiveCallbackContext, packet);
						});
					}
				}
				else
				{
					mNetworkLog("Error read_bytes (%lu) doesn't fit kNetPacketMaxLen", read_bytes);
					net_packet_free(s, packet); // Release packet
				}
	        }
	        else if(read_bytes < 0) // Error
	        {
				mNetworkLog("Error reading from socket");
				net_packet_free(s, packet); // Release packet
	        }
		}
    });
    
    // Install the write event handler
    dispatch_source_set_event_handler(s->writeDispatchSource, ^{
		// Pending packet
        net_packet_t packet = NULL;	
		// Loop over pendingPackets
        while ((packet = (net_packet_t)queue_pop(s->pendingPackets))) 
		{	
            ssize_t write_bytes = sendto(s->fd, packet->data, packet->length, 0, (struct sockaddr *)&packet->addr, sizeof(packet->addr));
            
            if(write_bytes < 0) // Error
            {
		        mNetworkLog("Error writing to socket");
            }
            
			pool_release(s->poolPackets, packet); // Release, not free. Ownership belongs to outside scope
        }

		net_socket_suspend_write(s); // Suspend until there's something more to send WEAK
    });
    
    // Socket retain count by read and write dispatch sources
    // Socket's file descriptor socketIPv4 can be closed when socketIPv4RetainCount = 0
    __block int socketCloseRetainCount = 2;
	
    // Use dispatch_source cancel handler to close socket file descriptor
    void (^dispatchSourceCancelHandlerBlock)() = ^{ 
        if(--socketCloseRetainCount == 0)
        {	
			net_socket_destroy_async(s);
            close(s->fd);
            s->fd = 0;
        }
    };
    
    // Install the read cancellation handler
    dispatch_source_set_cancel_handler(s->readDispatchSource, dispatchSourceCancelHandlerBlock);
    
    // Install the write cancellation handler
    dispatch_source_set_cancel_handler(s->writeDispatchSource, dispatchSourceCancelHandlerBlock);
    
    // Start reading the socket.
    dispatch_resume(s->readDispatchSource);
    
    // Start writing the socket.
    //dispatch_resume(s->writeDispatchSource); Don't resume until there's something queued up to send WEAK
	s->isSending = 0;
	
	return NetNoError;
}

void net_socket_suspend_write(net_socket_t s)
{
	if(s->isSending)
	{
		dispatch_suspend(s->writeDispatchSource); // Suspend until there's something more to send WEAK
		s->isSending = 0;
	}
}

void net_socket_resume_write(net_socket_t s)
{
	if(s->isSending == 0)
	{
		s->isSending = 1;
		dispatch_resume(s->writeDispatchSource); 
	}
}