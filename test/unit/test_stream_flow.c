/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_stream_flow.c
* universal-network-c
*/

#include "test.h"
#include "protocol.h"
#include "stream_flow.h"

static void test_stream_flow_mode()
{
	LOG_TEST_START;
	
	const float small_dt = 0.1; 
	const float big_dt = 4.0; 
	const float delta = 0.001;
	
	StreamFlow test_stream_flow;
	StreamFlowRef test_stream_flow_ref = &test_stream_flow;
	
	streamFlowClear(test_stream_flow_ref);
	
	assert(test_stream_flow.mode == StreamFlowModeBad);
	
	streamFlowUpdate(test_stream_flow_ref, kStreamFlowRttThreshold-delta, small_dt);
	
	assert(test_stream_flow.mode == StreamFlowModeBad);
	
	streamFlowUpdate(test_stream_flow_ref, kStreamFlowRttThreshold-delta, big_dt);
	
	assert(test_stream_flow.mode == StreamFlowModeGood);
	
	streamFlowUpdate(test_stream_flow_ref, kStreamFlowRttThreshold+delta, small_dt);
	
	assert(test_stream_flow.mode == StreamFlowModeBad);
	
	LOG_TEST_END;
}

int main(void)
{	
	LOG_SUITE_START("stream_flow");

	test_stream_flow_mode();
	
	return 0;
}
