/*
 
 net_error.c
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

#include "net_error.h"
#include "universal_network_c.h"

NetError netErrorPosix(PosixError posixError)
{
	NetError error;
		
	switch (posixError) {
        case ENETDOWN:
        case ENETUNREACH:
            error = NetUnreachableError;
			break;
		case EAI_NONAME:
			error = NetNotKnownError;
			break;
		case EAI_AGAIN:
		case EAI_FAIL:
			error = NetCouldNotResolveError;
			break;
        case EMSGSIZE:
            error = NetInvalidError;
			break;
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case EAFNOSUPPORT:
            error = NetBindError;
			break;
        case EIO:
        case EAGAIN:
            error = NetSockError;
            break;
        case EBADF:
        case ENOTSOCK:
        case EINVAL:
			error = NetInvalidError;
		case EAI_SYSTEM:
        default:
            error = NetOtherError;
            break;
    }

	return error;
}

void netErrorSetPosix(NetError * error, PosixError posixError)
{
	*error = netErrorPosix(posixError);
}

void netErrorSet(NetError * error, NetError value)
{
	*error = value;
}

void netErrorLog(NetError error)
{
    switch (error) {
        case NetNoError:
            mNetworkLog("NetNoError (%d)", error);
            break;
        case NetUnreachableError:
            mNetworkLog("NetUnreachableError (%d)", error);
            break;
        case NetNotKnownError:
            mNetworkLog("NetNotKnownError (%d)", error);
            break;
        case NetCouldNotResolveError:
            mNetworkLog("NetCouldNotResolveError (%d)", error);
            break;
        case NetBindError:
            mNetworkLog("NetBindError (%d)", error);
            break;
        case NetSockError:
            mNetworkLog("NetSockError (%d)", error);
            break;
        case NetInvalidError:
            mNetworkLog("NetInvalidError (%d)", error);
            break;
        case NetOtherError:
            mNetworkLog("NetOtherError (%d)", error);
            break;
    }
}