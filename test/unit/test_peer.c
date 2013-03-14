/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_peer.c
* universal-network-c
*/

#include "test.h"
#include "peer.h"
#include "log.h"

static void test_Peer()
{
	LOG_TEST_START;
	
	net_addr_t addr;
	net_addr_local(&addr); // example
	char * id = "RegisteredPeer";
	
	Peer * peer = peerCreate(id, &addr, &addr);
	
	assert(net_addr_is_equal(&addr, &peer->localAddr));
	assert(net_addr_is_equal(&addr, &peer->mappedAddr));	
	assert(strcmp(peer->id, id) == 0);
	
	peerDestroy(&peer);
	
	assert(peer == NULL);
	
	LOG_TEST_END;
}

static void test_PeerList()
{
	LOG_TEST_START;
	
	net_addr_t addr1, addr2;
	net_addr_local(&addr1);
	net_addr_local(&addr2);
	char * id1 = "Peer1";
	char * id2 = "Peer2";
	
	PeerList list = peerListCreate();
	
	assert(peerListIsEmpty(&list) == true);
	
	peerListAdd(&list, id1, &addr1, &addr1);
	
	assert(list.count == 1);
	assert(peerListIsEmpty(&list) == false);
	
	peerListAdd(&list, id2, &addr2, &addr2);
	
	assert(list.count == 2);
	assert(peerListIsEmpty(&list) == false);
	
	Peer * peer1 = peerListFind(&list, id1);
	Peer * peer2 = peerListFind(&list, id2);
		
	assert(peer1 != NULL);
	assert(peer1 != NULL);
	
	Peer * peerNot = peerListFind(&list, "Not");
	
	assert(peerNot == NULL);
	
	peerListRemove(&list, peer1);
	peerListRemove(&list, peer2);
		
	assert(peerListIsEmpty(&list) == true);

	peerListDestroy(&list);
	
	LOG_TEST_END;
}

int main(void)
{
	LOG_SUITE_START("peer");

	test_Peer();
	test_PeerList();
	
	return 0;
}
