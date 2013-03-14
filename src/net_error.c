/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* net_error.c
* universal-network-c
*/

#include "net_error.h"
#include "network.h"

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