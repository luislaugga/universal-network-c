/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stream_flow.h
* universal-network-c
*/

#ifndef __lUniversal_network_stream_flow_h__
#define __lUniversal_network_stream_flow_h__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define kStreamFlowModeBadRate 5.0f // 5 updates per sec.
#define kStreamFlowModeGoodRate 15.0f // 15 updates per sec.
#define kStreamFlowMaxRate kStreamFlowModeGoodRate

#define kStreamFlowRttThreshold 0.25f // 250 ms

typedef enum {
    StreamFlowModeGood,
    StreamFlowModeBad
} StreamFlowMode;

typedef struct {    
    StreamFlowMode mode;               // Flow Control Mode, affects update frequency
    
    float updateInterval;               // Time in seconds between send update
    float goodConditions;               // Time in seconds of consecutive good rtt
    float penalty;                      // Time in seconds of consecutive good rtt before returning from bad to good flow mode
    float penaltyReductionAccumulator;  // Accumulated time in seconds of consecutive good rtt, used to reduce penalty
} StreamFlow;

typedef StreamFlow * StreamFlowRef;

void streamFlowClear(StreamFlowRef ref);

void streamFlowUpdate(StreamFlowRef ref, float rtt, float deltaTime);

#endif