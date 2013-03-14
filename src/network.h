/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* network.h
* universal-network-c
*/

#ifndef __universal_network_network_h__
#define __universal_network_network_h__

#include <Block.h>
#include <dispatch/dispatch.h>

#include <signal.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if (DEBUG | TEST)
#define mNetworkLog(format, ...) printf(("Network: " format "\n"), ## __VA_ARGS__)
#define mNetworkPrettyLog printf("Network: %s\n", __PRETTY_FUNCTION__) 
#else
#define mNetworkLog(format, ...)
#define mNetworkPrettyLog
#endif

#endif