/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* timeout.h
* universal-network-c
*/

#ifndef __universal_network_timeout_h__
#define __universal_network_timeout_h__

#include <dispatch/dispatch.h>

/*!
 * @typedef timeout_t
 * @abstract Will create a timer that fires once and autoreleases itself
 * @discussion
 * The time will fire and release automatically
 * If needed, timeout_destroy will cancel the timer asynchronously and release it
 */
typedef struct {
	dispatch_source_t timerDispatchSource;
    bool valid;
} timeout_t;

typedef void (^timeout_block_t)(void);

void timeout_create(timeout_t * timeout, void (*callback)(void *), void * context, long milliseconds);
void timeout_create_block(timeout_t * timeout, timeout_block_t block, long milliseconds);
void timeout_destroy(timeout_t * timeout);

#endif