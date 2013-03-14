/*
 
 timeout.c
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

#include "timeout.h"

#define kNSEC_PER_MILLISEC (1000000ull) // 1 ms = 10^6 ns = 1 000 000 ns
#define kLEEWAY (10ull * kNSEC_PER_MILLISEC) // 10 ms

void timeout_create(timeout_t * timeout, void (*callback)(void *), void * context, long milliseconds)
{
	if(timeout->valid)
		timeout_destroy(timeout); // Destroy any valid timeout before creating a new one
		
	dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	
	timeout->timerDispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
	
	if(timeout->timerDispatchSource)
	{		
		dispatch_source_set_timer(timeout->timerDispatchSource, 
								  dispatch_time(DISPATCH_TIME_NOW, milliseconds * kNSEC_PER_MILLISEC),
								  DISPATCH_TIME_FOREVER, 
								  kLEEWAY); // Fire once
		
		dispatch_set_context(timeout->timerDispatchSource, context);
		dispatch_source_set_event_handler(timeout->timerDispatchSource, ^{
			callback(dispatch_get_context(timeout->timerDispatchSource)); // Callback
			timeout->valid = false;
			dispatch_release(timeout->timerDispatchSource);
		});
		
        timeout->valid = true; // Mark as valid timeout
		dispatch_resume(timeout->timerDispatchSource); // Resume
	}
}

void timeout_create_block(timeout_t * timeout, timeout_block_t block, long milliseconds)
{
	if(timeout->valid)
		timeout_destroy(timeout); // Destroy any valid timeout before creating a new one
		
	dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	
	timeout->timerDispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
	
	if(timeout->timerDispatchSource)
	{
		dispatch_source_set_timer(timeout->timerDispatchSource, 
								  dispatch_time(DISPATCH_TIME_NOW, milliseconds * kNSEC_PER_MILLISEC),
								  DISPATCH_TIME_FOREVER, 
								  kLEEWAY); // Fire once
								
		dispatch_source_set_event_handler(timeout->timerDispatchSource, ^{
			block();
			timeout->valid = false;
			dispatch_release(timeout->timerDispatchSource);
		});	
		
        timeout->valid = true; // Mark as valid timeout
		dispatch_resume(timeout->timerDispatchSource); // Resume
	}
}

void timeout_create_queue(timeout_t * timeout, dispatch_queue_t queue, long milliseconds, timeout_block_t block)
{
	if(timeout->valid)
		timeout_destroy(timeout); // Destroy any valid timeout before creating a new one
    
	timeout->timerDispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
	
	if(timeout->timerDispatchSource)
	{
		dispatch_source_set_timer(timeout->timerDispatchSource,
								  dispatch_time(DISPATCH_TIME_NOW, milliseconds * kNSEC_PER_MILLISEC),
								  DISPATCH_TIME_FOREVER,
								  kLEEWAY); // Fire once
        
		dispatch_source_set_event_handler(timeout->timerDispatchSource, ^{
			block();
			timeout->valid = false;
			dispatch_release(timeout->timerDispatchSource);
		});
		
        timeout->valid = true; // Mark as valid timeout
		dispatch_resume(timeout->timerDispatchSource); // Resume
	}
}

void timeout_destroy(timeout_t * timeout)
{
	if(timeout->valid)
	{
        timeout->valid = false; // Mark as invalid
		dispatch_source_cancel(timeout->timerDispatchSource);
		dispatch_release(timeout->timerDispatchSource);
	}
}