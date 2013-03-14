/*
 
 net_packet.c
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

#include "net_packet.h"

void net_packet_init(net_packet_t p)
{
	if(p)
		p->bitstream = bitstream_create(p->data, kNetPacketMaxLen);
}

size_t net_packet_len(net_packet_t p)
{
	return p->length; // Set before sendto or after recvfrom
}

void net_packet_set_data(net_packet_t packet, uint8_t * data, size_t length)
{
	if(length <= kNetPacketMaxLen)
	{	
		memcpy(packet->data, data, length);
		packet->length = length;
		packet->bitstream.offset = packet->length;
	}
}

void net_packet_set(net_packet_t p, const char * host, const int port)
{
	memset(&p->addr, 0, sizeof(p->addr));
    p->addr.sin_family = AF_INET;
    p->addr.sin_port = htons(port);
    p->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_aton(host, &p->addr.sin_addr);
}

void net_packet_addr(net_packet_t p, net_addr_t * addr)
{
	memcpy(&p->addr, addr, sizeof(net_addr_t));
}