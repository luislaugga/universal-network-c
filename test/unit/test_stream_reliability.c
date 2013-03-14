/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_stream_reliability.c
* universal-network-c
*/

#include "test.h"
#include "stream_reliability.h"

static void test_stream_reliability_clear()
{
	LOG_TEST_START;
	
	StreamReliability test_stream_reliability;
	StreamReliabilityRef test_stream_reliability_ref = &test_stream_reliability;
	
	streamReliabilityClear(test_stream_reliability_ref);
	
	assert(test_stream_reliability_ref->rtt == 0);
	assert(test_stream_reliability_ref->sentBytes == 0);
	assert(test_stream_reliability_ref->totalSentPackets == 0);
	assert(test_stream_reliability_ref->sequence == 0);
	
	LOG_TEST_END;
}

static void test_stream_reliability_no_loss()
{
	LOG_TEST_START;
	
	// Test 1 sending fixed packet length (100 bytes), with 0.0% loss from A to B and 0.0% loss from B to A
	// Simulation over a period of 10 seconds, with 1/10th second delta
	const float dt = 0.1;
	const size_t packet_size = 100;
	
	StreamReliability test_stream_reliability_A;
	StreamReliabilityRef test_stream_reliability_ref_A = &test_stream_reliability_A;
	streamReliabilityClear(test_stream_reliability_ref_A);
	
	StreamReliability test_stream_reliability_B;
	StreamReliabilityRef test_stream_reliability_ref_B = &test_stream_reliability_B;
	streamReliabilityClear(test_stream_reliability_ref_B);
	
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
	
	// Test 1
	for(int s=1; s<=10; ++s)
	{
		for(int ds=1; ds<=10; ++ds)
		{
			// A to B
			streamReliabilityPacketReceived(test_stream_reliability_ref_B, test_stream_reliability_ref_A->sequence, test_stream_reliability_ref_A->ack, test_stream_reliability_ref_A->ackBits);
			streamReliabilityPacketSent(test_stream_reliability_ref_A, packet_size);
			
			// B to A
			streamReliabilityPacketReceived(test_stream_reliability_ref_A, test_stream_reliability_ref_B->sequence, test_stream_reliability_ref_B->ack, test_stream_reliability_ref_B->ackBits);
			streamReliabilityPacketSent(test_stream_reliability_ref_B, packet_size);
			
			// Update
			streamReliabilityUpdate(test_stream_reliability_ref_A, dt);
			streamReliabilityUpdate(test_stream_reliability_ref_B, dt);
		}
		
		// Check
		assert(test_stream_reliability_ref_A->totalSentPackets == s*10);
		assert(test_stream_reliability_ref_A->totalReceivedPackets == s*10);
		assert(test_stream_reliability_ref_A->totalAckedPackets == s*10);
		assert(test_stream_reliability_ref_A->totalLostPackets == 0);
		assert(test_stream_reliability_ref_A->sentBytes == 10*packet_size);
		assert(test_stream_reliability_ref_A->sentBytesPerSecond == 10*packet_size);
		assert(test_stream_reliability_ref_A->ackedBytes <= 10*s*packet_size);
		assert(test_stream_reliability_ref_A->ackedBytesPerSecond <= 10*packet_size);
		assert(test_stream_reliability_ref_A->sentBandwidth <= 10*packet_size*0.008);
		assert(test_stream_reliability_ref_A->ackedBandwidth <= 10*packet_size*0.008);
		assert(test_stream_reliability_ref_A->rtt <= 0.1);
		
		assert(test_stream_reliability_ref_B->totalSentPackets == s*10);
		assert(test_stream_reliability_ref_B->totalReceivedPackets == s*10);
		assert(test_stream_reliability_ref_B->totalAckedPackets = s*10-1);
		assert(test_stream_reliability_ref_B->totalLostPackets == 0);
		assert(test_stream_reliability_ref_B->sentBytes <= 10*packet_size);
		assert(test_stream_reliability_ref_B->sentBytesPerSecond <= 10*packet_size);
		assert(test_stream_reliability_ref_B->ackedBytes <= 10*s*packet_size);
		assert(test_stream_reliability_ref_B->ackedBytesPerSecond <= 10*packet_size);
		assert(test_stream_reliability_ref_B->sentBandwidth <= 10*packet_size*0.008);
		assert(test_stream_reliability_ref_B->ackedBandwidth <= 10*packet_size*0.008);
		assert(test_stream_reliability_ref_B->rtt <= 0.1);
	}
	
	LOG_TEST_END;
}

static void test_stream_reliability_with_loss()
{
	LOG_TEST_START;
	
	// Test 2 sending fixed packet length (100 bytes), with 10.0% loss (1 in 10) from A to B and 0.0% loss from B to A
	// Simulation over a period of 10 seconds, with 1/10th second delta
	const float dt = 0.1;
	const size_t packet_size = 100;
	
	StreamReliability test_stream_reliability_A;
	StreamReliabilityRef test_stream_reliability_ref_A = &test_stream_reliability_A;
	streamReliabilityClear(test_stream_reliability_ref_A);
	
	StreamReliability test_stream_reliability_B;
	StreamReliabilityRef test_stream_reliability_ref_B = &test_stream_reliability_B;
	streamReliabilityClear(test_stream_reliability_ref_B);
	
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
	
	// Test 1
	for(int s=1; s<=10; ++s)
	{
		for(int ds=1; ds<=10; ++ds)
		{
			// A to B
			if(ds != 10) // every 10th, loose 1 packet from A to B
				streamReliabilityPacketReceived(test_stream_reliability_ref_B, test_stream_reliability_ref_A->sequence, test_stream_reliability_ref_A->ack, test_stream_reliability_ref_A->ackBits);	
			streamReliabilityPacketSent(test_stream_reliability_ref_A, packet_size);
			
			// B to A
			streamReliabilityPacketReceived(test_stream_reliability_ref_A, test_stream_reliability_ref_B->sequence, test_stream_reliability_ref_B->ack, test_stream_reliability_ref_B->ackBits);
			streamReliabilityPacketSent(test_stream_reliability_ref_B, packet_size);
			
			// Update
			streamReliabilityUpdate(test_stream_reliability_ref_A, dt);
			streamReliabilityUpdate(test_stream_reliability_ref_B, dt);
		}
		
		// Check
		assert(test_stream_reliability_ref_A->totalSentPackets == s*10);
		assert(test_stream_reliability_ref_A->totalReceivedPackets == s*10);
		assert(test_stream_reliability_ref_A->totalAckedPackets < s*10);
		assert(test_stream_reliability_ref_A->sentBytes == 10*packet_size);
		assert(test_stream_reliability_ref_A->sentBytesPerSecond == 10*packet_size);
		assert(test_stream_reliability_ref_A->ackedBytes <= 10*s*packet_size);
		assert(test_stream_reliability_ref_A->ackedBytesPerSecond <= 10*packet_size);
		assert(test_stream_reliability_ref_A->sentBandwidth <= 10*packet_size*0.008);
		assert(test_stream_reliability_ref_A->ackedBandwidth <= 10*packet_size*0.008);
		assert(test_stream_reliability_ref_A->rtt <= 0.1);
		
		assert(test_stream_reliability_ref_B->totalSentPackets == s*10);
		assert(test_stream_reliability_ref_B->totalReceivedPackets < s*10);
		assert(test_stream_reliability_ref_B->totalAckedPackets = s*10-1);
		assert(test_stream_reliability_ref_B->sentBytes <= 10*packet_size);
		assert(test_stream_reliability_ref_B->sentBytesPerSecond <= 10*packet_size);
		assert(test_stream_reliability_ref_B->ackedBytes <= 10*s*packet_size);
		assert(test_stream_reliability_ref_B->ackedBytesPerSecond <= 10*packet_size);
		assert(test_stream_reliability_ref_B->sentBandwidth <= 10*packet_size*0.008);
		assert(test_stream_reliability_ref_B->ackedBandwidth <= 10*packet_size*0.008);
		assert(test_stream_reliability_ref_B->rtt <= 0.2);
	}
	
	assert(test_stream_reliability_ref_A->totalLostPackets == 9); // 9 packet lost, the last one is only check 1 seconds after being sent
	assert(test_stream_reliability_ref_B->totalLostPackets == 0);
	
	LOG_TEST_END;
}



int main(void)
{	
	LOG_SUITE_START("stream_reliability");

	test_stream_reliability_clear();
	test_stream_reliability_no_loss();
	test_stream_reliability_with_loss();
	
	return 0;
}
