/*
 
 net_error.h
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