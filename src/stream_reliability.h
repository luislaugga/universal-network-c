/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stream_reliability.h
* universal-network-c
*/

#ifndef __universal_network_stream_reliability_h__
#define __universal_network_stream_reliability_h__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "stream_flow.h"
#include "stream_protocol.h"

#define kStreamReliabilityMaxRtt 1.0
#define kStreamReliabilityMaxSequence 0xFFFF // 16 bits

#define kStreamReliabilityBufferCapacity ((int)(2.35f * kStreamReliabilityMaxRtt * kStreamFlowMaxRate + 0.5))

typedef struct 
{
    unsigned int sequence;  // Packet sequence number
    float time;             // Time offset since packet was sent or received (depending on context)
    size_t size;            // Packet total size in bytes (header and payload)
    bool acked;             // Confirms if packets has been received on the remote side
    
} StreamPacketMetadata;

typedef StreamPacketMetadata * StreamPacketMetadataRef;

typedef struct
{    
    StreamPacketMetadata packets[kStreamReliabilityBufferCapacity]; // Ring buffer, used to store all sent packets metadata
    
    unsigned int frontSent;             // Oldest index in buffer that is in "sent list"
    unsigned int frontAcked;            // Oldest index in buffer that is in "acked list"
    unsigned int back;                  // Next available index
    
    Sequence sequence;              // Current local sequence, increased every streamReliabilityPacketSent
    Ack ack;                   		// Last remote sequence received
    AckBitField ackBits;            // Last remote sequence bit field received
    
    unsigned int totalSentPackets;      // Total sent packets recorded
    unsigned int totalReceivedPackets;  // Total received packets recorded
    unsigned int totalAckedPackets;     // Total acked packets recorded
    unsigned int totalLostPackets;      // Total lost packets recorded

    unsigned int sentBytes;
    unsigned int sentBytesPerSecond;    // Sent bytes, used for sentBandwidth
    unsigned int ackedPackets;
    unsigned int ackedPacketsPerSecond; // Acked packets 
    unsigned int ackedBytes;
    unsigned int ackedBytesPerSecond;   // Acked bytes, used for ackedBandwidth

    float sentBandwidth;				// Approximate sent bandwidth over the last second in kbps
    float ackedBandwidth;				// Approximate acked bandwidth over the last second in kbps
   
    float rtt;							// Estimated round trip time based on acked packets
    
} StreamReliability;

typedef StreamReliability * StreamReliabilityRef;

void streamReliabilityClear(StreamReliabilityRef);

void streamReliabilityPacketSent(StreamReliabilityRef, size_t);
void streamReliabilityPacketReceived(StreamReliabilityRef, Sequence, Ack, AckBitField);

void streamReliabilityProcessSequence(StreamReliabilityRef, Sequence); // Remote sequence, changes ack and ackBits
void streamReliabilityProcessAck(StreamReliabilityRef, Ack, AckBitField); // Local sequence, changes ackedPackets

void streamReliabilityUpdate(StreamReliabilityRef, float);

static inline void increase(unsigned int * value, unsigned max)
{
    ++(*value); (*value) %= max;
}

static inline void decrease(int * value, unsigned int count, unsigned int max)
{
    (*value) = (*value) >= count ? (*value)-count : ((*value)+max)-count;
}

static inline int isSequenceMoreRecent(Sequence sequence, Sequence otherSequence, Sequence maxSequence)
{
    return (( sequence > otherSequence ) && ( sequence - otherSequence <= maxSequence/2 )) || (( otherSequence > sequence ) && ( otherSequence - sequence > maxSequence/2 ));
}

#endif