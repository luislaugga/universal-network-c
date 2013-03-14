/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_net_socket.c
* universal-network-c
*/

#include "test.h"
#include "queue.h"
#include "pool.h"
#include "net.h"

static void test_net_socket_info()
{
	// net_socket_t test_socket = net_socket_create(&netError, AF_INET, 0, 0);
	// 
	// assert(test_socket);
	// assert(test_sendSocket);
	// 
	// test_net_socket_info
	// 
	// net_socket_set_receive_callback(test_receiveSocket, NULL, test_net_socket_info_callback);
}


int main(void)
{
	LOG_SUITE_START("net_socket");

	test_net_socket_info();
	
	return 0;
}
