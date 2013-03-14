/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* net_error.h
* universal-network-c
*/

#ifndef __universal_network_net_error_h__
#define __universal_network_net_error_h__

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <fcntl.h>

typedef enum {
	NetNoError = 0,
	NetUnreachableError, // Address is unreachable
	NetNotKnownError, // DNS or other lookup related tools
	NetCouldNotResolveError,
	NetBindError,
	NetSockError,
	NetInvalidError, // Invalid parameters, combination, data
	NetOtherError,  
} NetError;

typedef int PosixError;

NetError netErrorPosix(PosixError posixError);

void netErrorSetPosix(NetError * error, PosixError posixError);
void netErrorSet(NetError * error, NetError value);

void netErrorLog(NetError error);

#endif