/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* transaction.h
* universal-network-c
*/

#ifndef __universal_network_transaction_h__
#define __universal_network_transaction_h__

#include "transaction_protocol.h"

#include "net.h"
#include "bitstream.h"
#include "hashtable.h"
#include "timeout.h"

/*!
 * @header
 *
 * Transaction is an abstract model for UDP requests sent between server agent and
 * client agent. It provides reliability mechanism with acknowledgement from the
 * receiving party.
 *
 * All transactions have a fixed number that is unique for the pair client-server
 * agents. It starts with a request and doesn't finish until acknowledge is received.
 * If the acknowledge is not received within the specified timout, a retransmission
 * is done (retry). It will fail (and notify) to send a request after a specified number of
 * retries. 
 */

typedef void (*TransactionReceiveCallback)(void *, net_addr_t *, TransactionObject *);
typedef void (*TransactionErrorCallback)(void *, net_addr_t *);

/*!
 * @typedef TransactionConfiguration
 * @abstract Keeps track of existing transactions associated with a given socket and protocol
 */
typedef struct {
	dispatch_queue_t transactionsDispatchQueue; // Serial queue, used to synchronize access to transactions	
	hashtable_t transactions; // Active, sent transactions hashtable <key = trans. id, object = transaction struct>
	uint8_t cseq; 		 	  // Sequence number used in Trans. ID, 0-255, WEAK needs to be > 2 Bytes
	net_socket_t socket; 	  // Associated socket, can't be null
    bool isEnabled;           // Enable/Disable (valid for timeouts and pending transactions)
	
	TransactionReceiveCallback receiveCallback;
	TransactionErrorCallback errorCallback;
	void * context;
} TransactionConfiguration;

NetError transactionSetup(TransactionConfiguration *, const int port, void *, TransactionReceiveCallback, TransactionErrorCallback);
void transactionTeardown(TransactionConfiguration *);

void transactionEnable(TransactionConfiguration *);
void transactionDisable(TransactionConfiguration *);

void transactionRequest(TransactionConfiguration *, net_addr_t *, TransactionObject *);

#endif