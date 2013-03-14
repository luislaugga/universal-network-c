/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* transaction_protocol.c
* universal-network-c
*/

#include "transaction_protocol.h"

#pragma mark -
#pragma mark Header

void transactionProtocolPackHeader(bitstream_t * bitstream, TransactionId transactionId, TransactionType transactionType)
{
	// Pack protocol header
	protocolPackHeader(bitstream, kProtocolDefaultId, kProtocolDefaultVersion, ProtocolTypeTransaction);
	
	// Pack transaction protocol header
	bitstream_write_uint8(bitstream, transactionId); // id
	bitstream_write_uint8(bitstream, transactionType); // type
}

UnpackResult transactionProtocolUnpackHeader(bitstream_t * bitstream, TransactionId * transactionId, TransactionType * transactionType)
{
	// Unpack protocol header
	if(protocolUnpackHeader(bitstream, kProtocolDefaultId, kProtocolDefaultVersion, ProtocolTypeTransaction) != UnpackValid)
		return UnpackInvalid;
	
	// Unpack transaction protocol header
	bitstream_read_uint8(bitstream, transactionId); // id
	bitstream_read_uint8(bitstream, transactionType); // type
	
	return UnpackValid;
}

#pragma mark -
#pragma mark Response

void transactionProtocolPackResponse(bitstream_t * bitstream, TransactionResponseType responseType)
{
	bitstream_write_uint8(bitstream, responseType); // Response type	
}

UnpackResult transactionProtocolUnpackResponse(bitstream_t * bitstream, TransactionResponseType * responseType)
{
	bitstream_read_uint8(bitstream, responseType); // Response type	
	
	return UnpackValid;
}

#pragma mark -
#pragma mark Request

void transactionProtocolPackRequest(bitstream_t * bitstream, TransactionObject * transactionObject)
{
	bitstream_write_uint8(bitstream, transactionObject->type); // Request type
	
	switch(transactionObject->type) {
		case TransactionUserCreate: // user create
			transactionProtocolPackObjectUserCreate(bitstream, (TransactionObjectUserCreate *)transactionObject);
			break;
		case TransactionResourceCreate: // resource create 
			transactionProtocolPackObjectResourceCreate(bitstream, (TransactionObjectResourceCreate *)transactionObject);
			break;
		case TransactionOnline: // online 
			transactionProtocolPackObjectOnline(bitstream, (TransactionObjectOnline *)transactionObject);
			break;
		case TransactionOffline: // offline 
			transactionProtocolPackObjectOffline(bitstream, (TransactionObjectOffline *)transactionObject);
			break;
		case TransactionPeerList: // peerlist 
			transactionProtocolPackObjectPeerList(bitstream, (TransactionObjectPeerList *)transactionObject);
			break;
		case TransactionConnect: // connect 
			transactionProtocolPackObjectConnect(bitstream, (TransactionObjectConnect *)transactionObject);
			break;
		case TransactionConnectAccept: // connect accept 
			transactionProtocolPackObjectConnectAccept(bitstream, (TransactionObjectConnectAccept *)transactionObject);
			break;
        case TransactionConnectRefuse: // connect refuse 
			transactionProtocolPackObjectConnectRefuse(bitstream, (TransactionObjectConnectRefuse *)transactionObject);
			break;
		case TransactionDisconnect: // disconnect
			transactionProtocolPackObjectDisconnect(bitstream, (TransactionObjectDisconnect *)transactionObject);
			break;
		default: // empty
		break;
	}
}

UnpackResult transactionProtocolUnpackRequest(bitstream_t * bitstream, TransactionObject * transactionObject)
{
	UnpackResult unpackResult;	
	
	bitstream_read_uint8(bitstream, &transactionObject->type); // Request type
	
	switch(transactionObject->type) {
		case TransactionUserCreate: // user create 
			unpackResult = transactionProtocolUnpackObjectUserCreate(bitstream, (TransactionObjectUserCreate *)transactionObject);
			break;
		case TransactionResourceCreate: // resource create 
			unpackResult = transactionProtocolUnpackObjectResourceCreate(bitstream, (TransactionObjectResourceCreate *)transactionObject);
			break;
		case TransactionOnline: // online 
			unpackResult = transactionProtocolUnpackObjectOnline(bitstream, (TransactionObjectOnline *)transactionObject);
			break;
		case TransactionOffline: // offline 
			unpackResult = transactionProtocolUnpackObjectOffline(bitstream, (TransactionObjectOffline *)transactionObject);
			break;
		case TransactionPeerList: // peerlist 
			unpackResult = transactionProtocolUnpackObjectPeerList(bitstream, (TransactionObjectPeerList *)transactionObject);
			break;
		case TransactionConnect: // connect 
			unpackResult = transactionProtocolUnpackObjectConnect(bitstream, (TransactionObjectConnect *)transactionObject);
			break;
		case TransactionConnectAccept: // connect accept
			unpackResult = transactionProtocolUnpackObjectConnectAccept(bitstream, (TransactionObjectConnectAccept *)transactionObject);
			break;
        case TransactionConnectRefuse: // connect refuse
			unpackResult = transactionProtocolUnpackObjectConnectRefuse(bitstream, (TransactionObjectConnectRefuse *)transactionObject);
			break;
		case TransactionDisconnect: // disconnect
			unpackResult = transactionProtocolUnpackObjectDisconnect(bitstream, (TransactionObjectDisconnect *)transactionObject);
			break;
		case TransactionEmpty: // test purposes only
			unpackResult = UnpackValid;
			break;
		default:
			unpackResult = UnpackUnexpected;
			break;
	}
	
	return unpackResult;
}

#pragma mark -
#pragma mark Attributes

void transactionProtocolPackAttributeString(bitstream_t * bitstream, char * string)
{
	bitstream_write_uint8(bitstream, TransactionAttributeTypeString); // string attribute type
	bitstream_write_uint8(bitstream, mTransactionAttributeLengthString(string)); // string length
	bitstream_write_str(bitstream, string); // string
}

UnpackResult transactionProtocolUnpackAttributeString(bitstream_t * bitstream, char * string, size_t max)
{
	TransactionAttributeType attributeType;
	bitstream_read_uint8(bitstream, &attributeType);
	
	if(attributeType != TransactionAttributeTypeString)
		return UnpackInvalid;
	
	TransactionAttributeLength attributeLength;
	bitstream_read_uint8(bitstream, &attributeLength);
	
	bitstream_read_str(bitstream, string, max);
	
	return UnpackValid;
}

void transactionProtocolPackAttributeAddress(bitstream_t * bitstream, net_addr_t * address)
{
	bitstream_write_uint8(bitstream, TransactionAttributeTypeAddress); // Address attribute type
	bitstream_write_uint8(bitstream, kTransactionAttributeLengthAddress); // Address attribute length = fixed
	unsigned int port = address->sin_port;
	bitstream_write_uint16_endian(bitstream, port); // Address port
	bitstream_write_uint32_endian(bitstream, address->sin_addr.s_addr); // Address host
}

UnpackResult transactionProtocolUnpackAttributeAddress(bitstream_t * bitstream, net_addr_t * address)
{		
	TransactionAttributeType attributeType;
	bitstream_read_uint8(bitstream, &attributeType);
	
	if(attributeType != TransactionAttributeTypeAddress)
		return UnpackInvalid;
	
	TransactionAttributeLength attributeLength;
	bitstream_read_uint8(bitstream, &attributeLength);
	
	if(attributeLength != kTransactionAttributeLengthAddress)
		return UnpackInvalid;
	
	unsigned int port;
	bitstream_read_uint16_endian(bitstream, &port); // Address port
	unsigned int host;
	bitstream_read_uint32_endian(bitstream, &host); // Address host
	
	net_addr_set(address, host, port, false);
		
	return UnpackValid;
}

void transactionProtocolPackAttributeSeconds(bitstream_t * bitstream, time_t seconds)
{
	bitstream_write_uint8(bitstream, TransactionAttributeTypeSeconds); // Expire time attribute type
	bitstream_write_uint8(bitstream, kTransactionAttributeLengthSeconds); // Expire time attribute length = 4 Bytes
	bitstream_write_uint32(bitstream, seconds);
}

UnpackResult transactionProtocolUnpackAttributeSeconds(bitstream_t * bitstream, time_t * seconds)
{
	TransactionAttributeType attributeType;
	bitstream_read_uint8(bitstream, &attributeType);
	
	if(!(attributeType == TransactionAttributeTypeSeconds))
		return UnpackInvalid;
	
	unsigned int attributeLength;
	bitstream_read_uint8(bitstream, &attributeLength);
	
	if(attributeLength != kTransactionAttributeLengthSeconds)
		return UnpackInvalid;
	
	unsigned int value;
	bitstream_read_uint32(bitstream, &value);
	*seconds = value;
	
	return UnpackValid;

}

void transactionProtocolPackAttributeCount(bitstream_t * bitstream, unsigned int count)
{
	bitstream_write_uint8(bitstream, TransactionAttributeTypeCount); // Count attribute type
	bitstream_write_uint8(bitstream, kTransactionAttributeLengthCount); // Count attribute length = 2 Bytes
	bitstream_write_uint16(bitstream, count);
}

UnpackResult transactionProtocolUnpackAttributeCount(bitstream_t * bitstream, unsigned int * count)
{
	TransactionAttributeType attributeType;
	bitstream_read_uint8(bitstream, &attributeType);
	
	if(!(attributeType == TransactionAttributeTypeCount))
		return UnpackInvalid;
	
	unsigned int attributeLength;
	bitstream_read_uint8(bitstream, &attributeLength);
	
	if(attributeLength != kTransactionAttributeLengthCount)
		return UnpackInvalid;
	
	bitstream_read_uint16(bitstream, count);

	return UnpackValid;
}

#pragma mark -
#pragma mark User

void transactionProtocolPackObjectUserCreate(bitstream_t * bitstream, TransactionObjectUserCreate * objectUserCreate)
{
	bitstream_write_uint64(bitstream, objectUserCreate->uid); // huid_t
	transactionProtocolPackAttributeString(bitstream, objectUserCreate->name); // name
	transactionProtocolPackAttributeString(bitstream, objectUserCreate->email); // email
}

UnpackResult transactionProtocolUnpackObjectUserCreate(bitstream_t * bitstream, TransactionObjectUserCreate * objectUserCreate)
{	
	UnpackResult unpackResult = UnpackValid;
	
	bitstream_read_uint64(bitstream, &objectUserCreate->uid); // huid_t
	unpackResult = transactionProtocolUnpackAttributeString(bitstream, (char *)&objectUserCreate->name, kUniversalUserNameMaxLength); // name
	unpackResult = transactionProtocolUnpackAttributeString(bitstream, (char *)&objectUserCreate->email, kUniversalUserEmailMaxLength); // name
	
	return unpackResult;
}

#pragma mark -
#pragma mark Resource

void transactionProtocolPackObjectResourceCreate(bitstream_t * bitstream, TransactionObjectResourceCreate * resourceObject)
{
	bitstream_write_uint64(bitstream, resourceObject->rid); // huid_t
	bitstream_write_uint64(bitstream, resourceObject->uid); // huid_t
	bitstream_write_uint8(bitstream, resourceObject->length); // data length
	bitstream_write_bytes(bitstream, resourceObject->data, resourceObject->length); // data bytes
}

UnpackResult transactionProtocolUnpackObjectResourceCreate(bitstream_t * bitstream, TransactionObjectResourceCreate * resourceObject)
{
	UnpackResult unpackResult = UnpackValid;
	
	bitstream_read_uint64(bitstream, &resourceObject->rid); // huid_t
	bitstream_read_uint64(bitstream, &resourceObject->uid); // huid_t
	bitstream_read_uint8(bitstream, &resourceObject->length); // data length
	int read_length = bitstream_read_bytes(bitstream, resourceObject->data, resourceObject->length); // data bytes
	
	if(read_length != resourceObject->length)
		unpackResult = UnpackInvalid;
	
	return unpackResult;
}

#pragma mark -
#pragma mark Online

void transactionProtocolPackObjectOnline(bitstream_t * bitstream, TransactionObjectOnline * objectOnline)
{
	bitstream_write_uint64(bitstream, objectOnline->uid); // huid_t
	transactionProtocolPackAttributeAddress(bitstream, &objectOnline->localAddress); // local address
	transactionProtocolPackAttributeAddress(bitstream, &objectOnline->mappedAddress); // mapped address	
	transactionProtocolPackAttributeSeconds(bitstream, objectOnline->expireSeconds); // expire Seconds
}

UnpackResult transactionProtocolUnpackObjectOnline(bitstream_t * bitstream, TransactionObjectOnline * objectOnline)
{	
	UnpackResult unpackResult = UnpackValid;
	
	bitstream_read_uint64(bitstream, &objectOnline->uid); // huid_t
	//unpackResult = transactionProtocolUnpackAttributeString(bitstream, (char *)&objectOnline->peerId, kUniversalPeerIdMaxLength); // id 
	unpackResult = transactionProtocolUnpackAttributeAddress(bitstream, &objectOnline->localAddress); // local address
	unpackResult = transactionProtocolUnpackAttributeAddress(bitstream, &objectOnline->mappedAddress); // mapped address	
	unpackResult = transactionProtocolUnpackAttributeSeconds(bitstream, &objectOnline->expireSeconds); // expire Seconds

	return unpackResult;
}

#pragma mark -
#pragma mark Offline

void transactionProtocolPackObjectOffline(bitstream_t * bitstream, TransactionObjectOffline * objectOffline)
{
	transactionProtocolPackAttributeString(bitstream, objectOffline->peerId); // id 
}

UnpackResult transactionProtocolUnpackObjectOffline(bitstream_t * bitstream, TransactionObjectOffline * objectOffline)
{
	UnpackResult unpackResult = UnpackValid;
	
	unpackResult = transactionProtocolUnpackAttributeString(bitstream, (char *)&objectOffline->peerId, kUniversalPeerIdMaxLength); // id 

	return unpackResult;
}

#pragma mark -
#pragma mark PeerList

void transactionProtocolInitializeObjectPeerList(TransactionObjectPeerList * objectPeerList)
{
	objectPeerList->count = 0;
}

void transactionProtocolAddPeerToObjectPeerList(TransactionObjectPeerList * objectPeerList, TransactionObjectPeer * objectPeer)
{
	memcpy(&objectPeerList->list[objectPeerList->count], objectPeer, sizeof(TransactionObjectPeer));
	++objectPeerList->count;
}

void transactionProtocolPackObjectPeerList(bitstream_t * bitstream, TransactionObjectPeerList * objectPeerList)
{
	transactionProtocolPackAttributeCount(bitstream, objectPeerList->count); // count 	
	for(int i=0; i<objectPeerList->count; ++i)
	{
		transactionProtocolPackAttributeString(bitstream, objectPeerList->list[i].peerId); // id 
		transactionProtocolPackAttributeAddress(bitstream, &objectPeerList->list[i].localAddress); // local address
		transactionProtocolPackAttributeAddress(bitstream, &objectPeerList->list[i].mappedAddress); // mapped address	
	}
}

UnpackResult transactionProtocolUnpackObjectPeerList(bitstream_t * bitstream, TransactionObjectPeerList * objectPeerList)
{
	UnpackResult unpackResult = UnpackValid;
	
	unpackResult = transactionProtocolUnpackAttributeCount(bitstream, &objectPeerList->count); // count 
	for(int i=0; i<objectPeerList->count; ++i)
	{
		unpackResult = transactionProtocolUnpackAttributeString(bitstream, objectPeerList->list[i].peerId, kUniversalPeerIdMaxLength); // id 
		unpackResult = transactionProtocolUnpackAttributeAddress(bitstream, &objectPeerList->list[i].localAddress); // local address
		unpackResult = transactionProtocolUnpackAttributeAddress(bitstream, &objectPeerList->list[i].mappedAddress); // mapped address	
	}
	
	return unpackResult;
}

#pragma mark -
#pragma mark Connect

void transactionProtocolPackObjectConnect(bitstream_t * bitstream, TransactionObjectConnect * objectConnect)
{
	transactionProtocolPackAttributeString(bitstream, objectConnect->peerId); // id 
    transactionProtocolPackAttributeAddress(bitstream, &objectConnect->streamAddress); // stream address
}

UnpackResult transactionProtocolUnpackObjectConnect(bitstream_t * bitstream, TransactionObjectConnect * objectConnect)
{
	UnpackResult unpackResult = UnpackValid;
	
	unpackResult = transactionProtocolUnpackAttributeString(bitstream, (char *)&objectConnect->peerId, kUniversalPeerIdMaxLength); // id 
 	unpackResult = transactionProtocolUnpackAttributeAddress(bitstream, &objectConnect->streamAddress); // stream address
	
	return unpackResult;
}

#pragma mark -
#pragma mark ConnectAccept

void transactionProtocolPackObjectConnectAccept(bitstream_t * bitstream, TransactionObjectConnectAccept * objectConnectAccept)
{
	transactionProtocolPackAttributeString(bitstream, objectConnectAccept->peerId); // id 
    transactionProtocolPackAttributeAddress(bitstream, &objectConnectAccept->streamAddress); // stream address
}

UnpackResult transactionProtocolUnpackObjectConnectAccept(bitstream_t * bitstream, TransactionObjectConnectAccept * objectConnectAccept)
{
	UnpackResult unpackResult = UnpackValid;
	
	unpackResult = transactionProtocolUnpackAttributeString(bitstream, (char *)&objectConnectAccept->peerId, kUniversalPeerIdMaxLength); // id 
    unpackResult = transactionProtocolUnpackAttributeAddress(bitstream, &objectConnectAccept->streamAddress); // stream address

	return unpackResult;
}

#pragma mark -
#pragma mark ConnectRefuse

void transactionProtocolPackObjectConnectRefuse(bitstream_t * bitstream, TransactionObjectConnectRefuse * objectConnectRefuse)
{
	transactionProtocolPackAttributeString(bitstream, objectConnectRefuse->peerId); // id 
}

UnpackResult transactionProtocolUnpackObjectConnectRefuse(bitstream_t * bitstream, TransactionObjectConnectRefuse * objectConnectRefuse)
{
	UnpackResult unpackResult = UnpackValid;
	
	unpackResult = transactionProtocolUnpackAttributeString(bitstream, (char *)&objectConnectRefuse->peerId, kUniversalPeerIdMaxLength); // id 
    
	return unpackResult;
}

#pragma mark -
#pragma mark Disconnect

void transactionProtocolPackObjectDisconnect(bitstream_t * bitstream, TransactionObjectDisconnect * objectDisconnect)
{
	transactionProtocolPackAttributeString(bitstream, objectDisconnect->peerId); // id 
}

UnpackResult transactionProtocolUnpackObjectDisconnect(bitstream_t * bitstream, TransactionObjectDisconnect * objectDisconnect)
{
	UnpackResult unpackResult = UnpackValid;
	
	unpackResult = transactionProtocolUnpackAttributeString(bitstream, (char *)&objectDisconnect->peerId, kUniversalPeerIdMaxLength); // id 
    
	return unpackResult;
}
