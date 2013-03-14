/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stun_protocol.h
* Universal
*/

#ifndef __universal_network_stun_protocol_h__
#define __universal_network_stun_protocol_h__

#include "bitstream.h"
#include "net_packet.h"

typedef enum {
    StunMsgClassRequest = 0x00,
    StunMsgClassIndication = 0x01,
    StunMsgClassSuccessResponse = 0x02,
    StunMsgClassFailureResponse = 0x03,
    StunMsgClassInvalidMessageClass = 0xff
} StunMsgClass;

typedef enum {
    StunMsgTypeBinding = 0x0001,
    StunMsgTypeInvalid = 0xffff
} StunMsgType;

typedef enum {
	StunAttributeMappedAddress = 0x0001,
	StunAttributeXorMappedAddress = 0x0020,
	StunAttributeAlternateServer = 0x8023, 
	StunAttributeOtherAddress = 0x802c, // Equivalent to StunAttributeAlternateServer
	StunAttributeOriginServer = 0x802b,
	StunAttributeErrorCode = 0x0009,
	StunAttributeChangeRequest = 0x0003 // Typically sent inside the request body (legacy)
} StunAttributeType;

#define StunAddrFamilyIPv4 0x01
#define StunAddrFamilyIPv6 0x02
typedef uint32_t StunAddrFamilyType;

#define kStunInvalid -1    // Invalid packet
#define kStunUnexpected 1  // Valid STUN packet, not expected type or class
#define kStunUnsupported 3 // Valid STUN packet, invalid required feature (ie. IPv6)
#define kStunFailure 2	   // Valid STUN packet, failure from server WEAK ???
#define kStunValid 0	   // Valid STUN packet

#define mStunIsRequest(type)         (((type) & 0x0110) == 0x0000)
#define mStunIsIndication(type)      (((type) & 0x0110) == 0x0010)
#define mStunIsSuccessResponse(type) (((type) & 0x0110) == 0x0100)
#define mStunIsErrorResponse(type)   (((type) & 0x0110) == 0x0110)

#define kStunProtocolMagicCookie 0x2112A442 // Constant, check RFC 5389
#define mStunProtocolIsValidMagicCookie(magic_cookie) (magic_cookie == kStunProtocolMagicCookie)

#define kStunHeaderLen 20 // 20 bytes
#define kStunTransactionIdLen 12 // 12 Bytes = 96 bits

typedef struct {
	uint8_t id[kStunTransactionIdLen]; // MUST be uniformly and SHOULD be cryptographically random
} StunTransactionId;
	
void stunProtocolPackHeader(bitstream_t * bitstream, StunMsgType, StunMsgClass, StunTransactionId, unsigned int);
int stunProtocolUnpackHeader(bitstream_t * bitstream, StunMsgType *, StunMsgClass *, StunTransactionId *, unsigned int *);

typedef struct {
	bool hasResponseOrigin; // RESPONSE-ORIGIN
	net_addr_t responseOriginAddr; 
	bool hasMappedAddress; // MAPPED-ADDRESS
	net_addr_t mappedAddressAddr;
	bool hasXorMappedAddress; // XOR-MAPPED-ADDRESS
	net_addr_t xorMappedAddressAddr;
	bool hasAlternateServer; // ALTERNATE-SERVER
	net_addr_t alternateServerAddr;
	bool hasErrorCode; // ERROR-CODE
	int errorCodeValue;
} StunBindingResponse;

void stunProtocolPackAttributeChangeAddr(bitstream_t *, bool doChangeHost, bool doChangePort);
int stunProtocolUnpackAttributeAddr(bitstream_t *, net_addr_t *);
int stunProtocolUnpackAttributeXorAddr(bitstream_t *, net_addr_t *);
int stunProtocolUnpackAttributeError(bitstream_t *, int *);
int stunProtocolUnpackBindingResponseBody(bitstream_t *, unsigned int length, StunBindingResponse *);

void stunProtocolPackBindingRequest(bitstream_t *, StunTransactionId);
void stunProtocolPackBindingChangeRequest(bitstream_t *, StunTransactionId, bool doChangeHost, bool doChangePort);
int stunProtocolUnpackBindingResponse(bitstream_t *, StunTransactionId *, StunBindingResponse *);

#endif