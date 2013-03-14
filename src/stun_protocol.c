/*
 
 stun_protocol.c
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

#include "stun_protocol.h"

void stunProtocolPackHeader(bitstream_t * bitstream, StunMsgType msgType, StunMsgClass msgClass, StunTransactionId transactionId, unsigned int length)
{
	// Most significant 2 bits are zeroes (00) + Message type 
	uint16_t type = 0;
	type = (msgType & 0x0f80) << 2; // Merge the msgType and msgClass, and the leading zero bits into a 16-bit field
	type |= (msgType & 0x0070) << 1;
	type |= (msgType & 0x000f);
	type |= (msgClass & 0x02) << 7;
	type |= (msgClass & 0x01) << 4;
	
	bitstream_write_uint16(bitstream, type);   // 2 bit zeroes + type
	bitstream_write_uint16(bitstream, length); // length
	bitstream_write_uint32(bitstream, kStunProtocolMagicCookie); // magic-cookie
	bitstream_write_bytes(bitstream, transactionId.id, sizeof(StunTransactionId)); // transaction id
}

int stunProtocolUnpackHeader(bitstream_t * bitstream, StunMsgType * msgType, StunMsgClass * msgClass, StunTransactionId * transactionId, unsigned int * length)
{
	// Type = 2 bit zeroes + Message type and class
	unsigned int type = 0;
	bitstream_read_uint16(bitstream, &type); // 2 bit zeroes + type
	
	// Message type
	*msgType = ((type & 0x000f) | ((type & 0x00e0) >> 1) | ((type & 0x3E00) >> 2));
	
	// Message class
	if (mStunIsRequest(type))
        *msgClass = StunMsgClassRequest;
    else if (mStunIsIndication(type))
        *msgClass = StunMsgClassIndication;
    else if (mStunIsSuccessResponse(type))
    	*msgClass = StunMsgClassSuccessResponse;
    else if (mStunIsErrorResponse(type))
        *msgClass = StunMsgClassFailureResponse;
    else
    {
	    *msgClass = StunMsgClassInvalidMessageClass; // Couldn't possibly happen, because StunMsgClass is only two bits
		return kStunInvalid;
    }

	// Message length
	bitstream_read_uint16(bitstream, length);

	// Message magic-cookie
	uint32_t magic_cookie;
	bitstream_read_uint32(bitstream, &magic_cookie);
	if(!mStunProtocolIsValidMagicCookie(magic_cookie))
		return kStunInvalid;
		
	// Message transaction id
	bitstream_read_bytes(bitstream, transactionId->id, sizeof(StunTransactionId));
		
	return kStunValid;
}

void stunProtocolPackAttributeChangeAddr(bitstream_t * bitstream, bool doChangeHost, bool doChangePort)
{
	uint32_t changeData = 0;
	
	if (doChangeHost)
    	changeData |= 0x04;

    if (doChangePort)
		changeData |= 0x02;
		
	bitstream_write_uint16(bitstream, StunAttributeChangeRequest);
	bitstream_write_uint16(bitstream, 0x0004);
	bitstream_write_uint32(bitstream, changeData);
}

int stunProtocolUnpackAttributeAddr(bitstream_t * bitstream, net_addr_t * addr)
{
	StunAddrFamilyType addrFamily;
	bitstream_read_uint16(bitstream, &addrFamily);
	
	if(addrFamily != StunAddrFamilyIPv4) // IPv6 not supported yet
		return kStunUnsupported;
		
	unsigned int port; // Port number
	bitstream_read_uint16(bitstream, &port);
	unsigned int host; // Host address ip
	bitstream_read_uint32(bitstream, &host);
	
	net_addr_set(addr, host, port, true); // Set addr
	
	return kStunValid;
}

int stunProtocolUnpackAttributeXorAddr(bitstream_t * bitstream, net_addr_t * addr)
{
	StunAddrFamilyType addrFamily;
	bitstream_read_uint16(bitstream, &addrFamily);
	
	if(addrFamily != StunAddrFamilyIPv4) // IPv6 not supported yet
		return kStunUnsupported;
		
	unsigned int xorPort; // Port number
	bitstream_read_uint16(bitstream, &xorPort);
	unsigned int  xorHost; // Host address ip
	bitstream_read_uint32(bitstream, &xorHost);
	
	uint16_t port = (xorPort ^ ((kStunProtocolMagicCookie >> 16) & 0xFFFF)); // XOR-Port is computed by XOR Port with 2 MSB of magic cookie
	uint32_t host = (xorHost ^ kStunProtocolMagicCookie); // XOR-Address is computed by XOR with magic cookie
	
	net_addr_set(addr, host, port, true); // Set addr
	
	return kStunValid;
}

int stunProtocolUnpackAttributeError(bitstream_t * bitstream, int * value)
{
	uint32_t n;
	bitstream_read_uint32(bitstream, &n);

    // first 21 bits of error-code attribute must be zero.
    // followed by 3 bits of "class" and 8 bits for the "error" number modulo 100
	n = (n >> 21);
    uint32_t class = n & 0x07;
	uint32_t error = (n >> 3) & 0xFF;
   
	*value = class * 100 + error;
	
	return kStunValid;
}

int stunProtocolUnpackBindingResponseBody(bitstream_t * bitstream, unsigned int length, StunBindingResponse * bindingResponse)
{
	// Clear binding response
	memset(bindingResponse, 0, sizeof(StunBindingResponse));
	
	// Remaining bytes in body
	int remainingBytes = length;
	
	// Loop over all Type-Length-Value attributes and fill in binding response
	while(remainingBytes > 0)
	{
		// Attribute Type
		StunAttributeType attributeType;
		bitstream_read_uint16(bitstream, &attributeType);
			
		// Attribute Length
		unsigned int attributeLength;
		bitstream_read_uint16(bitstream, &attributeLength);
		
		// Handle only: Addresses, XOR-Addresses and Errors
		switch(attributeType)
		{
			case StunAttributeMappedAddress:
			{
				if(stunProtocolUnpackAttributeAddr(bitstream, &bindingResponse->mappedAddressAddr) != kStunValid)
					return kStunInvalid;
					
				bindingResponse->hasMappedAddress = true;
			}
			break;
			case StunAttributeXorMappedAddress:
			{
				if(stunProtocolUnpackAttributeXorAddr(bitstream, &bindingResponse->xorMappedAddressAddr) != kStunValid)
					return kStunInvalid;
					
				bindingResponse->hasXorMappedAddress = true;
			}
			break;
			case StunAttributeAlternateServer:
			case StunAttributeOtherAddress:
			{
				if(stunProtocolUnpackAttributeAddr(bitstream, &bindingResponse->alternateServerAddr) != kStunValid)
					return kStunInvalid;
				
				bindingResponse->hasAlternateServer = true;
			}
			break;
			case StunAttributeOriginServer:
			{
				if(stunProtocolUnpackAttributeAddr(bitstream, &bindingResponse->responseOriginAddr) != kStunValid)
					return kStunInvalid;	
					
				bindingResponse->hasResponseOrigin = true;
			}
			break;
			case StunAttributeErrorCode:
			{
				if(stunProtocolUnpackAttributeError(bitstream, &bindingResponse->errorCodeValue) != kStunValid)
					return kStunInvalid;	
					
				bindingResponse->hasErrorCode = true;
			}
			break;
			default: // Ignore other attributes
			{
				bitstream_skip_bytes(bitstream, attributeLength);	
			}
			break;
		}
		
		remainingBytes = remainingBytes - attributeLength - 4; // Type (2B) Length (2B) Value (attributeLength)
	}
	
	return kStunValid;
}

void stunProtocolPackBindingRequest(bitstream_t * bitstream, StunTransactionId transactionId)
{
	// Pack header (empty body)
	stunProtocolPackHeader(bitstream, StunMsgTypeBinding, StunMsgClassRequest, transactionId, 0);
}

void stunProtocolPackBindingChangeRequest(bitstream_t * bitstream, StunTransactionId transactionId, bool doChangeHost, bool doChangePort)
{
	// Pack header (change request attribute body)
	stunProtocolPackHeader(bitstream, StunMsgTypeBinding, StunMsgClassRequest, transactionId, 8); // CHANGE-REQUEST has 8 bytes
	stunProtocolPackAttributeChangeAddr(bitstream, doChangeHost, doChangePort);
}

int stunProtocolUnpackBindingResponse(bitstream_t * bitstream, StunTransactionId * transactionId, StunBindingResponse * bindingResponse)
{
	StunMsgType msgType;
	StunMsgClass msgClass;
	unsigned int bodyLength;
	
	// Unpack header
	if(stunProtocolUnpackHeader(bitstream, &msgType, &msgClass, transactionId, &bodyLength) == kStunInvalid)
		return kStunInvalid;
	
	// Check message type and class
	if(msgType != StunMsgTypeBinding || msgClass == StunMsgClassRequest)
		return kStunUnexpected;
		
	// if(msgClass != StunMsgClassSuccessResponse)
	// 	return kStunFailure;
	
	// Check if body is not empty	
	if(bodyLength == 0)
		return kStunInvalid; // Expecting response values, body can't be empty
	
	// Unpack body
	if(stunProtocolUnpackBindingResponseBody(bitstream, bodyLength, bindingResponse) == kStunInvalid)
		return kStunInvalid;
		
	return kStunValid;
}
