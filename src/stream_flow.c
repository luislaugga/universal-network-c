/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stream_flow.c
* universal-network-c
*/

#include "stream_flow.h"

void streamFlowClear(StreamFlowRef ref)
{ 
    ref->mode = StreamFlowModeBad;
    ref->updateInterval = 1.0f/kStreamFlowModeBadRate;
    ref->penalty = 4.0f;
    ref->goodConditions = 0.0f;
    ref->penaltyReductionAccumulator = 0.0f;
}

void streamFlowUpdate(StreamFlowRef ref, float rtt, float deltaTime)
{  
    if (ref->mode == StreamFlowModeGood)
    {
        if (rtt > kStreamFlowRttThreshold)
        {
            ref->mode = StreamFlowModeBad;
            ref->updateInterval = 1.0f/kStreamFlowModeBadRate;
            
            if(ref->goodConditions < 10.0f && ref->penalty < 60.0f)
            {
                ref->penalty *= 2.0f;
                
                if (ref->penalty > 60.0f)
                    ref->penalty = 60.0f;
            }
            
            ref->goodConditions = 0.0f;
            ref->penaltyReductionAccumulator = 0.0f;
        }
        else
        {
            ref->goodConditions += deltaTime;
            ref->penaltyReductionAccumulator += deltaTime;
            
            if (ref->penaltyReductionAccumulator > 10.0f && ref->penalty > 1.0f )
            {
                ref->penalty /= 2.0f;
                
                if (ref->penalty < 1.0f )
                    ref->penalty = 1.0f;
                
                ref->penaltyReductionAccumulator = 0.0f;
            }
        }
    }
    
    if (ref->mode == StreamFlowModeBad)
    {
        if (rtt <= kStreamFlowRttThreshold)
            ref->goodConditions += deltaTime;
        else
            ref->goodConditions = 0.0f;
        
        if (ref->goodConditions > ref->penalty)
        {
            ref->goodConditions = 0.0f;
            ref->penaltyReductionAccumulator = 0.0f;
            
            ref->mode = StreamFlowModeGood;
            ref->updateInterval = 1.0f/kStreamFlowModeGoodRate;
        }
    }
}