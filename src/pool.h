/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* pool.h
* universal-network-c
*/

#ifndef __universal_network_pool_h__
#define __universal_network_pool_h__

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define kPoolDefaultCapacity 32

typedef struct pool_s * pool_t;

pool_t pool_create(size_t sizeObject, size_t capacity);
void pool_destroy(pool_t p);

size_t pool_capacity(pool_t p);

void * pool_alloc(pool_t p);
void pool_free(pool_t p, void * object);

void pool_retain(pool_t p, void * object);
void pool_release(pool_t p, void * object);

int debug_pool_free_count(pool_t p);
int debug_pool_alloc_count(pool_t p);

#endif