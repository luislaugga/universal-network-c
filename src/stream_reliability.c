/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stream_reliability.c
* universal-network-c
*/

#include "stream_reliability.h"

void streamReliabilityClear(StreamReliabilityRef ref)
{
    assert(ref != NULL);
    
    ref->frontSent = 0;
    ref->frontAcked = 0;
    ref->back = 0;
    
    ref->sequence = 0;
    ref->ack = 0;
    ref->ackBits = 0;
    
    ref->totalSentPackets = 0;
    ref->totalReceivedPackets = 0;
    ref->totalAckedPackets = 0;
    ref->totalLostPackets = 0;
    
    ref->sentBytes = 0;
    ref->sentBytesPerSecond = 0;
    ref->ackedPackets = 0;
    ref->ackedPacketsPerSecond = 0;
    ref->ackedBytes = 0;
    ref->ackedBytesPerSecond = 0;
    
    ref->sentBandwidth = 0;
    ref->ackedBandwidth = 0;
    
    ref->rtt = 0.0f;
}

void streamReliabilityPacketSent(StreamReliabilityRef ref, size_t size)
{
    assert(ref != NULL);
    
    unsigned int pivotBack = ref->back;
    unsigned int pivotAcked = ref->frontAcked;
    
    if(pivotBack != pivotAcked)
        assert(pivotBack != pivotAcked);
    
    StreamPacketMetadata metadata;
    
    metadata.acked = 0;
    metadata.sequence = ref->sequence;
    metadata.time = 0.0f;
    metadata.size = size;
    
    ref->packets[ref->back] = metadata;
    increase(&ref->back, kStreamReliabilityBufferCapacity);
    
    increase(&ref->sequence, kStreamReliabilityMaxSequence);

    ref->totalSentPackets += 1;
    ref->sentBytes += size;
    
    if(ref->back == ref->frontAcked)
        increase(&ref->frontAcked, kStreamReliabilityBufferCapacity); // Overwrite, discard acked packets
    
    if(ref->back == ref->frontSent)
        increase(&ref->frontSent, kStreamReliabilityBufferCapacity); // Overwrite, discard sent packets 
    
    assert(ref->frontSent != ref->back);
    assert(ref->frontAcked != ref->back);
}

void streamReliabilityPacketReceived(StreamReliabilityRef ref, Sequence sequence, Ack ack, AckBitField ackBits)
{
    assert(ref != NULL);
    
    streamReliabilityProcessSequence(ref, sequence);
    streamReliabilityProcessAck(ref, ack, ackBits);
    
    ref->totalReceivedPackets += 1;
}

void streamReliabilityProcessSequence(StreamReliabilityRef ref, Sequence sequence)
{
    assert(ref != NULL);
    
    if (isSequenceMoreRecent(sequence, ref->ack, kStreamReliabilityMaxSequence))
    {
        unsigned int shifts = (sequence > ref->ack) ? (sequence-ref->ack) : (sequence-ref->ack+kStreamReliabilityMaxSequence+1); // Non and wrap-around cases
        
        if(shifts < sizeof(unsigned int)*8)
        {
            ref->ackBits <<= shifts; // Shift bit field
            ref->ackBits |= 1; // Set LSB bit
        }
        else 
        {
            ref->ackBits = 0; // Shift was > 31 bits, zero bit field
        }		
        
        ref->ack = sequence; // Update ack
    }
    else
    {
        unsigned int shifts = (ref->ack >= sequence) ? (ref->ack-sequence) : (ref->ack-sequence+kStreamReliabilityMaxSequence+1); // Non and wrap-around cases
        
        if(shifts < sizeof(unsigned int)*8)
        {
            ref->ackBits |= (1 << shifts);  // Not more recent, set bit that is |sequence - ack| bits to left
        }
    }
    
    ref->ackBits &= 0xFFFFFFFF; // Make sure only 4 LSB Bytes are filled in	
}

void streamReliabilityProcessAck(StreamReliabilityRef ref, Ack ack, AckBitField ackBits)
{	
    assert(ref != NULL);
   
    if(ref->frontSent == ref->back) // Assume it is empty
        return;
    
    int pivot, count; // Pivot and Count (delta between current sequence and received ack)
    
    pivot = ref->back; // Start at pivot = back, move back to ack pack and check last 32 sent against ackBits and acked flag

    if(ref->sequence > ack)
        count = ref->sequence-ack;
    else
        count = ref->sequence-ack+kStreamReliabilityMaxSequence+1; // Wrap around case
    
    if(count < kStreamReliabilityBufferCapacity) // Only check if count doesn't exceed buffer's capacity
    {
        decrease(&pivot, count, kStreamReliabilityBufferCapacity); // Move pivot back
        
        assert(pivot >= 0 && pivot < kStreamReliabilityBufferCapacity);
        
        for(int p=0; p<31; ++p) // Check 32 bits in ackBits field
        {
            if(!ref->packets[pivot].acked && (ackBits & 1)) // First time acked
            {
                ref->packets[pivot].acked = true;
                
                ref->totalAckedPackets += 1;
                ref->rtt += (ref->packets[pivot].time - ref->rtt) * 0.1f; // Low-pass filtered
            }
            
            ackBits >>= 1; // Move left
            decrease(&pivot, 1, kStreamReliabilityBufferCapacity); // Advance to previous
        }
    }
}

void streamReliabilityUpdate(StreamReliabilityRef ref, float deltaTime)
{
    assert(ref != NULL);
    
    if(ref->frontSent == ref->back) // Assume it is empty
        return;

    // Update time
    for(unsigned int i=0; i<kStreamReliabilityBufferCapacity; ++i)
        ref->packets[i].time += deltaTime;
    
    // RTT epsilon
    const float epsilon = 0.0001f;
    
    // Sent Front    
    while(ref->frontSent != ref->back && ref->packets[ref->frontSent].time > (kStreamReliabilityMaxRtt + epsilon))
    {
        ref->sentBytes -= ref->packets[ref->frontSent].size; // Remove from sent count
        
        if(ref->packets[ref->frontSent].acked == false)
            ++ref->totalLostPackets; // Consider it is lost if kStreamReliabilityMaxRtt+epsilon elapsed without ack
        
        increase(&ref->frontSent, kStreamReliabilityBufferCapacity); // Move sent head to next
    }
    
    // Acked Front
    unsigned int c = 0;
    while(ref->frontAcked != ref->back && ref->packets[ref->frontAcked].time > (kStreamReliabilityMaxRtt * 2.0f - epsilon))
    {
        ++c;
        increase(&ref->frontAcked, kStreamReliabilityBufferCapacity); // Move acked head to next
    }
    
    // Clear acked packets/bytes (to be recalculated)
    ref->ackedPackets = 0;
    ref->ackedBytes = 0;
    
    if(ref->frontAcked != ref->back) // Proceed in case there are acked packets
    {
        unsigned int pivot = ref->frontAcked;
        
        while(pivot != ref->back)
        {
            if(ref->packets[pivot].acked)
            {
                if(ref->packets[pivot].time >= kStreamReliabilityMaxRtt) // For stats. count only acked packets after kStreamReliabilityMaxRtt has elapsed
                {
                    ref->ackedPackets += 1;
                    ref->ackedBytes += ref->packets[pivot].size;
                }	
            }
            
            increase(&pivot, kStreamReliabilityBufferCapacity);
        }
    }
    
    // Stats
    ref->sentBytesPerSecond = ref->sentBytes / kStreamReliabilityMaxRtt;
    ref->ackedPacketsPerSecond = ref->ackedPackets / kStreamReliabilityMaxRtt;
    ref->ackedBytesPerSecond = ref->ackedBytes / kStreamReliabilityMaxRtt;
    ref->sentBandwidth = ref->sentBytesPerSecond * 0.008f; // ( 8 / 1000.0f );
    ref->ackedBandwidth = ref->ackedBytesPerSecond * 0.008f; // ( 8 / 1000.0f );
}
