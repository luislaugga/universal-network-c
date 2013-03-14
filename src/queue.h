/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* queue.h
* universal-network-c
*/

#ifndef __universal_network_queue_h__
#define __universal_network_queue_h__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef void * queue_object_t;
typedef struct queue_s * queue_t;

queue_t queue_create();
void queue_destroy(queue_t q);

bool queue_is_empty(queue_t q);

void queue_push(queue_t q, queue_object_t o);
queue_object_t queue_pop(queue_t q);

int debug_queue_enqueued_count(queue_t q);
int debug_queue_reserved_count(queue_t q);

#endif