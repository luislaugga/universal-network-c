/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* net_socket.h
* universal-network-c
*/

#ifndef __universal_network_socket_internal_h__
#define __universal_network_socket_internal_h__

static NetError net_socket_set_nonblock(NetError * error, net_socket_t s);
static NetError net_socket_set_dispatch_sources(NetError * error, net_socket_t s);

static void net_socket_suspend_write(net_socket_t s);
static void net_socket_resume_write(net_socket_t s);

static void net_socket_destroy_async(net_socket_t s); // Asynchronous method called when both read+write dispatch sources cancel handlers are done

#endif