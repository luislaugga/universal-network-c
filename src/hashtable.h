/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* hashtable.h
* universal-network-c
*/

#ifndef __universal_network_hashtable_h__
#define __universal_network_hashtable_h__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned int hashtable_key_t;
typedef void * hashtable_object_t;
typedef struct hashtable_s * hashtable_t;

hashtable_t hashtable_create(hashtable_key_t max_key);
void hashtable_destroy(hashtable_t ht);

hashtable_object_t hashtable_search(hashtable_t ht, hashtable_key_t key);
void hashtable_insert(hashtable_t ht, hashtable_key_t key, hashtable_object_t object);
void hashtable_delete(hashtable_t ht, hashtable_key_t key);

typedef void (^hashtable_iterate_block_t)(hashtable_key_t, hashtable_object_t); // Loop using block for each {key, object} entry
void hashtable_iterate(hashtable_t, hashtable_iterate_block_t);

#endif