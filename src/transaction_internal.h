/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* transaction_internal.h
* universal-network-c
*/

#ifndef __universal_network_transaction_internal_h__
#define __universal_network_transaction_internal_h__

#include "transaction_protocol.h"
#include "net.h"
#include "bitstream.h"

/*!
 * @header
 *
 * Internal, reliability transaction protocol structures, enumerations and 
 * related definitions
 *
 * The transaction is responsible for low level interaction with socket, packet and
 * memory pool of packets.
 */

/*!
 * @typedef TransactionState
 * @abstract Possible states of a transaction: Idle, Waiting, Processing, Acknowledged, Error, Timeout
 */
typedef enum {
    TransactionIdle,		 // Transaction not sent, may or may not contain valid data
   	TransactionWaiting,		 // Transaction sent, waiting response from destination agent
	TransactionProcessing, 	 // Transaction sent and got Provisional reponse, being processed on destination agent
	TransactionAcknowledged, // Transaction sent and successfully acknowledged from destination agent
	TransactionError,		 // Transaction has an error
	TransactionTimeout		 // Transaction sent up to specified nr. retries but no response from destination agent
} TransactionState;

/*!
 * @typedef Transaction
 * @abstract Contains all the data associated with a single transaction: ID, Packet, Packet bitstream_t, Timeout and State
 * @discussion
 * The transaction can be discarded on TransactionAcknowledged or on TransactionError/TransactionTimeout after notifying
 * with callback about error/timeout
 */
typedef struct {
	TransactionState state; // State
	TransactionId id; 	 	// Unique nr. used to identify transaction from responses
	net_packet_t packet; 	// Packet with data and destination address for last sent request
	unsigned int retries;	// Nr. of retransmissions
	timeout_t timeout; 		// Timer for timeout while waiting for response 
	timeout_block_t timeoutBlock; // timeoutBlock needs to be released (Block_Copy on start)
} Transaction;

#define kTransactionInitialRto 1000ull // 1000 ms before first retransmission
#define kTransactionRtoIncreaseFactor 2ull // Rto = retry count x initial rto x 2
#define kTransactionMaxRetries 2  // Retransmit transaction packet up to max. retries
#define kTransactionMaxId 0xFF 	  // Max Id number used

Transaction * transactionAlloc(TransactionConfiguration *); // Will return a newly allocated transaction, with a unique sequential id
void transactionFree(TransactionConfiguration *, Transaction *);

void transactionRemoveAll(TransactionConfiguration *);

void transactionResponse(TransactionConfiguration *, net_addr_t * addr, TransactionId, TransactionResponseType);

void transactionSocketReceiveCallback(void *, net_packet_t);
void transactionDidReceiveResponse(TransactionConfiguration *, net_packet_t, TransactionId);
void transactionDidReceiveRequest(TransactionConfiguration *, net_packet_t, TransactionId);

void transactionDestroyTimeout(Transaction * transaction);
void transactionSetComplete(TransactionConfiguration *, Transaction * transaction);
void transactionRetryOrFail(TransactionConfiguration *, TransactionId transactionId);

#endif