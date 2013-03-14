/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* net_addr.c
* universal-network-c
*/

#include "net_addr.h"
#include "universal_network_c.h"

void net_addr_zero(net_addr_t * addr)
{
	net_addr_set(addr, 0, 0, true); // this will convert 0 to Network Byte Order
}

void net_addr_set(net_addr_t * addr, const unsigned int host, const unsigned short port, bool doHostToNetwork)
{
	memset(addr, 0, sizeof(net_addr_t));
   	addr->sin_family = AF_INET;
    addr->sin_port = doHostToNetwork ? htons(port) : port;
    addr->sin_addr.s_addr = doHostToNetwork ? htonl(host) : host;
}

void net_addr_copy(net_addr_t * dest, const net_addr_t * src)
{
	// zero and copy the host sockaddr
	bzero(dest, sizeof(net_addr_t));
	memcpy(dest, src, sizeof(net_addr_t));
}

unsigned int net_addr_get_host(net_addr_t * addr)
{
	return ntohl(addr->sin_addr.s_addr);
}

unsigned short net_addr_get_port(net_addr_t * addr)
{
	return ntohs(addr->sin_port);
}

bool net_addr_is_equal(const net_addr_t * addrA, const net_addr_t * addrB)
{
	return (addrA->sin_family == addrB->sin_family &&
			addrA->sin_port == addrB->sin_port &&
			addrA->sin_addr.s_addr == addrB->sin_addr.s_addr);
}

bool net_addr_is_valid(net_addr_t * addr)
{
	net_addr_t zeroAddr;
	bzero(&zeroAddr, sizeof(net_addr_t));
	return net_addr_is_equal(addr, &zeroAddr) == false;
}

NetError net_addr_resolve(net_addr_t * addr, const char * name, const unsigned short port)
{
    struct addrinfo *result;
    struct addrinfo *res;
    PosixError posixError;
    
    // getaddrinfo hints
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4 only for know
    hints.ai_flags = 0; // No wildcard IP address
    hints.ai_socktype = 0; // Any type
    hints.ai_protocol = 0; // Any protocol
    
    // resolve the domain name into a list of addresses
    posixError = getaddrinfo(name, NULL, &hints, &result);

    if (posixError)
		return netErrorPosix(posixError);
 
    // loop over all returned results and do inverse lookup
    for (res = result; res != NULL; res = res->ai_next)
    {   
        char hostaddr[kNetHostMaxLen] = "";
 
        posixError = getnameinfo(res->ai_addr, res->ai_addrlen, hostaddr, kNetHostMaxLen, NULL, 0, 0); 
        if (posixError)
 			return netErrorPosix(posixError);

        if (*hostaddr != '\0')
		{
            net_addr_copy(addr, (net_addr_t *)res->ai_addr);
			break;
		}
    } 

    freeaddrinfo(result);

	addr->sin_port = htons(port);
	
	mNetworkLog("%s:%d resolves to %s:%d", name, ntohs(addr->sin_port), inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));

    return NetNoError;
}

NetError net_addr_local(net_addr_t * addr)
{
	struct ifaddrs * myaddrs, * ifa;
	void * in_addr;
	PosixError posixError;

	posixError = getifaddrs(&myaddrs);
	if(posixError)
		return netErrorPosix(posixError);

	for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
			continue;
		if (!(ifa->ifa_flags & IFF_UP) || (ifa->ifa_flags & IFF_LOOPBACK)) // Skip: not running, loopback
			continue;

		switch (ifa->ifa_addr->sa_family)
		{
			case AF_INET:
			{
				struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
				in_addr = &s4->sin_addr;
				net_addr_copy(addr, s4);
				break;
			}

			// IPv6 not supported yet
			// case AF_INET6:
			// {
			// 	struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
			// 	in_addr = &s6->sin6_addr;
			// 	break;
			// }

			default:
			continue;
		}
	}
	
	freeifaddrs(myaddrs);
	return NetNoError;
}

void net_addr_log(net_addr_t * addr)
{
	mNetworkLog("addr %s:%d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
}