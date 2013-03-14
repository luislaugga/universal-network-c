/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* transaction_protocol.h
* universal-network-c
*/

#ifndef __lUniversal_network_transaction_protocol_h__
#define __lUniversal_network_transaction_protocol_h__

#include "protocol.h"
#include "network.h"
#include "bitstream.h"
#include "huid.h"
#include "net.h"

/*!
 * @header
 *
 * Transaction Protocol is a higher-level interface used by server and client to exchange
 * data about session presence, initiation and negotiation.
 * 
 * It provides a set of functions and callbacks that translate requests and 
 * responses to and from the underlying binary network protocol. 
 *
 * Request Format:
 * 0            7 8          15 16         23 
 * +-------------+-------------+-------------+
 * | Trans. ID   | Trans. Type | Req. Type   | 0 - 2
 * +-------------+-------------+-------------+ 
 * |	Body: TLV Attributes (bits 40-2047)  | 3 - 255
 * +-------------+-------------+-------------+
 * 
 * Response Format:
 * 0            7 8          15 16         23 
 * +-------------+-------------+-------------+
 * | Trans. ID   | Trans. Type | Resp. Type  | 0 - 2
 * +-------------+-------------+-------------+ 
 *
 * TLV Attribute format: Type-Length-Value
 * Each Request type has a fixed set of attributes, that are always included in the packet
 */

/*!
 * @typedef TransactionId
 * @abstract Unique id used to identify a single transaction
 */
typedef unsigned int TransactionId;

/*!
 * @typedef TransactionType
 * @abstract Type of transaction: Request or Response to previous Request
 * @discussion
 * Pack/Unpack: TransactionId and  TransactionType
 */
typedef enum {
    TransactionTypeRequest = 0x01,
    TransactionTypeResponse = 0x02,
} TransactionType;

void transactionProtocolPackHeader(bitstream_t * bitstream, TransactionId, TransactionType);
UnpackResult transactionProtocolUnpackHeader(bitstream_t * bitstream, TransactionId *, TransactionType *);

/*!
 * @typedef ResponseType
 * @abstract Type of transaction: Request or Response to previous Request
 * @discussion
 * Pack/Unpack: TransactionResponseType 
 */
typedef enum {
    TransactionResponseTypeProvisional = 0x01,   // Request received and being processed
    TransactionResponseTypeSuccess = 0x02,		  // The action was successfully received, understood, and accepted
    TransactionResponseTypeClientError = 0x04,	  // The request contains bad syntax or cannot be fulfilled at the server
    TransactionResponseTypeServerError = 0x05,	  // The server failed to fulfill an apparently valid request
    TransactionResponseTypeGlobalFailure = 0x06, // The request cannot be fulfilled
} TransactionResponseType;

void transactionProtocolPackResponse(bitstream_t * bitstream, TransactionResponseType);
UnpackResult transactionProtocolUnpackResponse(bitstream_t * bitstream, TransactionResponseType *);

/*!
 * @typedef TransactionAttributeType
 *
 * @abstract
 * @discussion 
 * ...
 */
typedef enum {
	TransactionAttributeTypeString = 0x01, // string
	TransactionAttributeTypeAddress = 0x02, // address
    TransactionAttributeTypeSeconds = 0x03, // seconds
	TransactionAttributeTypeCount = 0x04, // count
} TransactionAttributeType;

/*!
 * @typedef TransactionAttributeLength
 *
 * @abstract
 * @discussion 
 * ...
 */
typedef unsigned int TransactionAttributeLength;

#define kTransactionAttributeLengthAddress (8)
#define mTransactionAttributeLengthString(id) (2+strlen(id))
#define kTransactionAttributeLengthSeconds (4)
#define kTransactionAttributeLengthCount (2)

void transactionProtocolPackAttributeString(bitstream_t *, char *);
UnpackResult transactionProtocolUnpackAttributeString(bitstream_t *, char *, size_t);
void transactionProtocolPackAttributeAddress(bitstream_t *, net_addr_t *);
UnpackResult transactionProtocolUnpackAttributeAddress(bitstream_t *, net_addr_t *);
void transactionProtocolPackAttributeSeconds(bitstream_t *, time_t);
UnpackResult transactionProtocolUnpackAttributeSeconds(bitstream_t *, time_t *);
void transactionProtocolPackAttributeCount(bitstream_t *, unsigned int);
UnpackResult transactionProtocolUnpackAttributeCount(bitstream_t *, unsigned int *);

/*!
 * @typedef RequestType
 * @abstract Request methods sent between agents
 * @discussion 
 */
typedef enum {	
	TransactionUserCreate = 0x50,
	TransactionUserRead = 0x51,
	TransactionUserUpdate = 0x52,
	TransactionUserDelete = 0x53,
	TransactionResourceCreate = 0x70,
	TransactionResourceRead = 0x71,
	TransactionResourceUpdate = 0x72,
	TransactionResourceDelete = 0x73,
    TransactionOnline = 0x91,
	TransactionOffline = 0x92,
	TransactionPeerList = 0x93,
	TransactionConnect = 0xc0,
	TransactionConnectAccept = 0xc1,
	TransactionConnectRefuse = 0xc2,
	TransactionDisconnect = 0xc3,
	TransactionPing = 0x11,
	TransactionPong = 0x12,
	TransactionEmpty = 0x01, // for testing purposes only
} TransactionRequestType;

/*!
 * @typedef TransactionObject
 * @abstract Generic object containing data associated with any transaction
 * @discussion
 * TransactionObject is an abstract data type used mainly as an opaque struct pointer handled by
 * request packing/unpacking. It should be treated as a "polymorphic struct", whose pointer can
 * be any of the derived TransactionObject structs bellow.
 *
 * Pack/Unpack: TransactionRequestType
 */
#define kTransactionProtocolObjectMaxSize (kProtocolMaxLength-4)

typedef struct {
	TransactionRequestType type;
	char data[kTransactionProtocolObjectMaxSize];
} TransactionObject;

void transactionProtocolPackRequest(bitstream_t * bitstream, TransactionObject *);
UnpackResult transactionProtocolUnpackRequest(bitstream_t * bitstream, TransactionObject *);

#define kUniversalUserNameMaxLength 40 // WEAK
#define kUniversalUserEmailMaxLength 40 // WEAK
#define kUniversalPeerIdMaxLength 32 // WEAK

/*!
 * @typedef TransactionObjectUserCreate
 * @abstract TransactionObject used to create new user
 * @discussion
 * Pack/Unpack: Attributes
 */
typedef struct {
	TransactionRequestType type;
	huid_t uid; // user id
	char name[kUniversalUserNameMaxLength]; // user <first, last> name
	char email[kUniversalUserEmailMaxLength]; // user email
} TransactionObjectUserCreate;

void transactionProtocolPackObjectUserCreate(bitstream_t *, TransactionObjectUserCreate *);
UnpackResult transactionProtocolUnpackObjectUserCreate(bitstream_t *, TransactionObjectUserCreate *);

#define kUniversalResourceDataMaxLength 128

/*!
 * @typedef TransactionObjectResourceCreate
 * @abstract TransactionObject used to create new resource
 * @discussion
 * Pack/Unpack: Attributes id, data, user creator id
 */
typedef struct {
	TransactionRequestType type;
	huid_t rid; // resource id
	uint8_t data[kUniversalResourceDataMaxLength]; // WEAK
	unsigned int length; // WEAK
	huid_t uid; // user creator id
} TransactionObjectResourceCreate;

void transactionProtocolPackObjectResourceCreate(bitstream_t *, TransactionObjectResourceCreate *);
UnpackResult transactionProtocolUnpackObjectResourceCreate(bitstream_t *, TransactionObjectResourceCreate *);

/*!
 * @typedef TransactionObjectOnline
 * @abstract TransactionObject used to store presence request data
 * @discussion
 * Pack/Unpack: Attributes peerId, localAddress, mappedAddress and expireSeconds
 */
typedef struct {
	TransactionRequestType type;
	huid_t uid; // user id
	net_addr_t localAddress; // address
	net_addr_t mappedAddress; // address
	time_t expireSeconds; // seconds
} TransactionObjectOnline;

void transactionProtocolPackObjectOnline(bitstream_t *, TransactionObjectOnline *);
UnpackResult transactionProtocolUnpackObjectOnline(bitstream_t *, TransactionObjectOnline *);

/*!
 * @typedef TransactionObjectOffline
 * @abstract
 * @discussion
 * Pack/Unpack: Attributes peerId
 */
typedef struct {
	TransactionRequestType type;
	char peerId[kUniversalPeerIdMaxLength]; // string
} TransactionObjectOffline;

void transactionProtocolPackObjectOffline(bitstream_t *, TransactionObjectOffline *);
UnpackResult transactionProtocolUnpackObjectOffline(bitstream_t *, TransactionObjectOffline *);

/*!
 * @typedef TransactionObjectPeer
 * @abstract Used as part of TransactionObjectPeerList
 * @discussion Node of linked list
 * Pack/Unpack: Attributes peerId, local address, mapped address
 */
typedef struct {
	char peerId[kUniversalPeerIdMaxLength]; // string
	net_addr_t localAddress; // local address
	net_addr_t mappedAddress; // mapped (external) address
} TransactionObjectPeer;

/*!
 * @typedef TransactionObjectPeerList
 * @abstract
 * @discussion
 * Pack/Unpack: Attributes peer count, peer 1, peer 2, ..., peer n
 */
typedef struct {
	TransactionRequestType type;
	unsigned int count; // count
    TransactionObjectPeer list[10]; // list
} TransactionObjectPeerList;

void transactionProtocolInitializeObjectPeerList(TransactionObjectPeerList *);
void transactionProtocolAddPeerToObjectPeerList(TransactionObjectPeerList *, TransactionObjectPeer *);

void transactionProtocolPackObjectPeerList(bitstream_t *, TransactionObjectPeerList *);
UnpackResult transactionProtocolUnpackObjectPeerList(bitstream_t *, TransactionObjectPeerList *);

/*!
 * @typedef TransactionObjectConnect
 * @abstract
 * @discussion
 * Pack/Unpack: Attributes peerId
 */
typedef struct {
	TransactionRequestType type;
	char peerId[kUniversalPeerIdMaxLength]; // string
    net_addr_t streamAddress; // stream address to be used
} TransactionObjectConnect;

void transactionProtocolPackObjectConnect(bitstream_t *, TransactionObjectConnect *);
UnpackResult transactionProtocolUnpackObjectConnect(bitstream_t *, TransactionObjectConnect *);

/*!
 * @typedef TransactionObjectConnectAccept
 * @abstract
 * @discussion
 * Pack/Unpack: Attributes peerId
 */
typedef struct {
	TransactionRequestType type;
	char peerId[kUniversalPeerIdMaxLength]; // string
    net_addr_t streamAddress; // stream address to be used
} TransactionObjectConnectAccept;

void transactionProtocolPackObjectConnectAccept(bitstream_t *, TransactionObjectConnectAccept *);
UnpackResult transactionProtocolUnpackObjectConnectAccept(bitstream_t *, TransactionObjectConnectAccept *);

/*!
 * @typedef TransactionObjectConnectRefuse
 * @abstract
 * @discussion
 * Pack/Unpack: Attributes peerId
 */
typedef struct {
	TransactionRequestType type;
	char peerId[kUniversalPeerIdMaxLength]; // string
} TransactionObjectConnectRefuse;

void transactionProtocolPackObjectConnectRefuse(bitstream_t *, TransactionObjectConnectRefuse *);
UnpackResult transactionProtocolUnpackObjectConnectRefuse(bitstream_t *, TransactionObjectConnectRefuse *);

/*!
 * @typedef TransactionObjectDisconnect
 * @abstract
 * @discussion
 * Pack/Unpack: Attributes peerId
 */
typedef struct {
	TransactionRequestType type;
	char peerId[kUniversalPeerIdMaxLength]; // string
} TransactionObjectDisconnect;

void transactionProtocolPackObjectDisconnect(bitstream_t *, TransactionObjectDisconnect *);
UnpackResult transactionProtocolUnpackObjectDisconnect(bitstream_t *, TransactionObjectDisconnect *);

#endif