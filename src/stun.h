/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stun.h
* Universal
*/

#ifndef __lNetwork_stun_h__
#define __lNetwork_stun_h__

#include "stun_tests.h"

#define kStunServerDefaultPort 3478

#define kStunDefaultRto 1000 // 1000 ms
#define kStunDefaultRetries 2 // Retry 2 times after initial request

/*!
 * @typedef stun_t
 *
 * @abstract Stun object associated with a specified address (host:port)
 * @discussion 
 * The Stun object can be used only in associated mode.
 * Associated: the stun object uses a previously created and valid net_socket
 * 
 * If an incoming packet is not STUN-valid, it is forwarded to socket's original callback
 *
 * Warning: StunTest will release any memory once it did complete. DO NOT attempt to access it out of the resolve sequence.
 */
struct StunStruct {
	bool didResolve; // True if resolved already for socket in config
	void (^stunDidResolve)(struct StunStruct *); // Did resolve block 
	void (^stunDidFailResolve)(struct StunStruct *); // Did fail block
	
	StunResults results; // Results are only valid if didResolve == true
	StunConfig config;   // Configuration
	
	StunTest bindingTest1; // Binding test 1
	StunTest behaviorTest1; // Behavior test 1 (Binding with alternate server address on primary port)
	StunTest behaviorTest2; // Behavior test 2 (Binding with alternate server address on alternate port)
	StunTest filteringTest1; // Filtering test 1
	StunTest filteringTest2; // Filtering test 2
};

typedef void (^StunDidResolve)(stun_t);
typedef void (^StunDidFailResolve)(stun_t);

stun_t stunCreate(net_socket_t, const char *, StunDidResolve, StunDidFailResolve);
void stunDestroy(stun_t ); // shouldn't be called from any of its blocks or callbacks

void stunResolve(stun_t);
void stunLog(stun_t );

#endif