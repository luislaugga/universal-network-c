/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* transaction.c
* universal-network-c
*/

#include "transaction.h"
#include "transaction_internal.h"
#include "transaction_protocol.h"
#include "universal_network_c.h"

#pragma mark -
#pragma mark Global setup
	
NetError transactionSetup(TransactionConfiguration * config, const int port, void * context, TransactionReceiveCallback receiveCallback, TransactionErrorCallback errorCallback)
{
	config->transactionsDispatchQueue = dispatch_queue_create("com.laugga.transactionsDispatchQueue", NULL); // Create transactions dispatch queue
	
	config->cseq = 0;
	config->transactions = hashtable_create(kTransactionMaxId); // Max trans. id shouldn't exceed 256 WEAK
    config->isEnabled = true; // active by default
	
	// Socket
	NetError netError;
	static const char * localhost = "0.0.0.0"; // Listening on all network interfaces
	config->socket = net_socket_create(&netError, AF_INET, localhost, port);
	
	// Check error
	if(netError)
	{
		mNetworkLog("Error creating socket (NetError %d)", netError);
		return netError;
	}
	
	net_socket_set_receive_callback(config->socket, config, transactionSocketReceiveCallback); // Set socket callback
	
	config->receiveCallback = receiveCallback;
	config->errorCallback = errorCallback;
	config->context = context;
	
	return NetNoError;
}

void transactionTeardown(TransactionConfiguration * config)
{
	transactionRemoveAll(config); // Remove all pending transactions
	hashtable_destroy(config->transactions); // Release hashtable
	dispatch_release(config->transactionsDispatchQueue); // Release dispatch queue
	net_socket_destroy(config->socket);	// Close and Release the listening socket
}

void transactionRemoveAll(TransactionConfiguration * config)
{
	// Remove all pending transactions
	hashtable_iterate(config->transactions, ^(hashtable_key_t transactionId, hashtable_object_t transaction) {
		hashtable_delete(config->transactions, transactionId); // remove from hashtable
		transactionDestroyTimeout(transaction); // destroy timeout
		transactionFree(config, transaction); // release
	});
}

#pragma mark -
#pragma mark Enable/Disable

void transactionEnable(TransactionConfiguration * config)
{
    dispatch_async(config->transactionsDispatchQueue, ^{
		mNetworkPrettyLog;
        
		if(config->isEnabled == false)
            config->isEnabled = true;
	});
}

void transactionDisable(TransactionConfiguration * config)
{
	// Disable + Remove all pending transactions
	dispatch_async(config->transactionsDispatchQueue, ^{
		mNetworkPrettyLog;
    
    	if(config->isEnabled == true)
		{
            config->isEnabled = false;
			transactionRemoveAll(config);
		}
	});
}

#pragma mark -
#pragma mark Alloc/Free

Transaction * transactionAlloc(TransactionConfiguration * config)
{
	Transaction * transaction = NULL;
	net_packet_t transactionPacket = net_packet_alloc(config->socket); // Transaction owns packet. Responsible for releasing it.
	
	if(transactionPacket) // Only proceed if packet was allocated successfuly
	{
		transaction = calloc(1, sizeof(Transaction));

		transaction->state = TransactionIdle;
		transaction->id = config->cseq++; config->cseq %= kTransactionMaxId; // WEAK
		transaction->packet = transactionPacket;
	}
	
	return transaction;
}

void transactionFree(TransactionConfiguration * config, Transaction * transaction)
{
	if(transaction)
	{
		net_packet_release(config->socket, transaction->packet); // Release packet
		free(transaction);
	}
}

#pragma mark -
#pragma mark Response

void transactionResponse(TransactionConfiguration * config, net_addr_t * addr, TransactionId transactionId, TransactionResponseType transactionResponseType)
{
	mNetworkLog("Transaction Response ID %d", transactionId);
	
	// Get new packet
	net_packet_t packet = net_packet_alloc(config->socket);
	
	// Pack Header
	transactionProtocolPackHeader(&packet->bitstream, transactionId, TransactionTypeResponse);
	
	// Pack response
	transactionProtocolPackResponse(&packet->bitstream, transactionResponseType);
	
	// Set packet destination addresss
	net_packet_addr(packet, addr); 
	
	// Send transaction packet
	net_socket_send(config->socket, packet);
		
	// Release packet
	net_packet_release(config->socket, packet);
}

#pragma mark -
#pragma mark Request

void transactionRequest(TransactionConfiguration * config, net_addr_t * addr, TransactionObject * object)
{
	// Allocate transaction
	Transaction * transaction = transactionAlloc(config);
	
	if(transaction)
	{
		bitstream_t * bitstream = &transaction->packet->bitstream;
	
		// Pack Header
		transactionProtocolPackHeader(bitstream, transaction->id, TransactionTypeRequest);
	
		// Pack request
		transactionProtocolPackRequest(bitstream, object);
	
		dispatch_async(config->transactionsDispatchQueue, ^{
			// Insert transaction in the hash table
			hashtable_insert(config->transactions, transaction->id, transaction);
		});
		
		// Set transaction packet destination addresss
		net_packet_addr(transaction->packet, addr); 
		
		// Define timeout block (simplifies access to config and results)
		transaction->timeoutBlock = Block_copy(^(void) {
			transactionRetryOrFail(config, transaction->id); // Retry or fail, after max. retries
		});
	
		// Send transaction packet
		net_socket_send(config->socket, transaction->packet);
	
		// Set state
		transaction->state = TransactionWaiting;
	
		// Set timeout
		timeout_create_block(&transaction->timeout, transaction->timeoutBlock, kTransactionInitialRto);
	
		mNetworkLog("Transaction Request ID %d", transaction->id);
	}
}

#pragma mark -
#pragma mark Socket callback

void transactionSocketReceiveCallback(void * context, net_packet_t packet)
{	
	mNetworkPrettyLog;
	
	TransactionConfiguration * config = (TransactionConfiguration *)context;
	
	if(config->isEnabled) // make socket go down/refuse/close in the future
	{
		mNetworkLog("Unpacking");
		
		bitstream_t * bitstream = &packet->bitstream;

		TransactionId transactionId; // Transaction id
		TransactionType transactionType; // Transaction type (request or response)

		if(transactionProtocolUnpackHeader(bitstream, &transactionId, &transactionType) == UnpackValid) // Only proceed if valid
		{
			mNetworkLog("Transaction id %d", transactionId);
			
			if(transactionType == TransactionTypeResponse) // Response
			{
				transactionDidReceiveResponse(config, packet, transactionId);
			}
			else if(transactionType == TransactionTypeRequest) // Request
			{
				transactionDidReceiveRequest(config, packet, transactionId);
			}
		}
		else
		{
			mNetworkLog("Unpack failed");
		}	
	}
			
	net_packet_release(config->socket, packet); // release packet from socket (socket will never release callback packets, so we have to do it)
}

void transactionDidReceiveResponse(TransactionConfiguration * config, net_packet_t packet, TransactionId transactionId)
{
	net_packet_retain(config->socket, packet); // retain packet
	
	dispatch_async(config->transactionsDispatchQueue, ^{
		Transaction * transaction = hashtable_search(config->transactions, transactionId); // Concurrency
		if(transaction)
		{
			// Cancel timeout
			transactionDestroyTimeout(transaction);

			// Unpack response
			TransactionResponseType transactionResponseType;
			transactionProtocolUnpackResponse(&packet->bitstream, &transactionResponseType);

			// Process response
			if(transactionResponseType == TransactionResponseTypeSuccess)
			{
				mNetworkLog( "Response Transaction ID %u success", transactionId);
				transaction->state = TransactionAcknowledged;
			}
			else
			{
				mNetworkLog( "Response Transaction ID %u error", transactionId);
				transaction->state = TransactionError;
				// ...
			}

			// Set complete
			transactionSetComplete(config, transaction);
			
			net_packet_release(config->socket, packet); // release packet
		}
	});
}

void transactionDidReceiveRequest(TransactionConfiguration * config, net_packet_t packet, TransactionId transactionId)
{
	net_packet_retain(config->socket, packet); // retain packet
	
	dispatch_async(config->transactionsDispatchQueue, ^{	
		mNetworkLog("Request Transaction ID %u (%lu)", transactionId, packet->length);

		// Send response
		transactionResponse(config, &packet->addr, transactionId, TransactionResponseTypeSuccess);
	
		// Unpack request
		TransactionObject * transactionObject = calloc(1, sizeof(TransactionObject)); // heap is safer for objects
		UnpackResult unpackResult = transactionProtocolUnpackRequest(&packet->bitstream, transactionObject);

		// Forward object
		if(config->receiveCallback && unpackResult == UnpackValid)
			config->receiveCallback(config->context, &packet->addr, transactionObject);
		else
			mNetworkLog("Unpack result: %d", unpackResult);
				
		net_packet_release(config->socket, packet); // release packet
		free(transactionObject); // free object
	});
}

void transactionDestroyTimeout(Transaction * transaction)
{
	timeout_destroy(&transaction->timeout); // Destroy timeout
}

void transactionSetComplete(TransactionConfiguration * config, Transaction * transaction)
{
	// Notify about state 
	if(transaction->state != TransactionAcknowledged && config->errorCallback)
		config->errorCallback(config->context, &transaction->packet->addr);
	
	// Release
	hashtable_delete(config->transactions, transaction->id);
	transactionFree(config, transaction);
}

void transactionRetryOrFail(TransactionConfiguration * config, TransactionId transactionId)
{
	// NOTE: Using transactionId is safer than using a pointer to transaction (Transaction *)
	// because timeout can be triggered after associated transaction has been removed and released
	
	dispatch_async(config->transactionsDispatchQueue, ^{
		Transaction * transaction = hashtable_search(config->transactions, transactionId); // Concurrency
		if(transaction) // always check if transaction still exists
		{
			if(transaction->state == TransactionWaiting) // If transaction has not been completed
			{
			    transactionDestroyTimeout(transaction);
			    if(transaction->retries < kTransactionMaxRetries) // Retry
			    {		
			        transaction->retries++; // Update retries
					long timeoutMilliseconds = kTransactionInitialRto * kTransactionRtoIncreaseFactor * transaction->retries; // Calculate rto timeout
					net_socket_send(config->socket, transaction->packet); // Retransmit
			        timeout_create_block(&transaction->timeout, transaction->timeoutBlock, timeoutMilliseconds); // Set timeout
					mNetworkLog("Timeout Transaction ID %d retry %ld", transaction->id, timeoutMilliseconds);
			    }
			    else // Fail
			    {	
			        mNetworkLog("Fail Transaction ID %d timeout", transaction->id);
            
			        transaction->state = TransactionTimeout; // Set transaction timeout
			        transactionSetComplete(config, transaction);
			    }
			}
		}
	});
}