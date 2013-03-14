/*
 
 timeout.h
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
void timeout_create_queue(timeout_t * timeout, dispatch_queue_t queue, long milliseconds, timeout_block_t block);
void timeout_destroy(timeout_t * timeout);

#endif