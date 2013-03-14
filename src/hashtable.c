/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* hashtable.c
* universal-network-c
*/

#include "hashtable.h"

struct hashtable_s {
	hashtable_key_t max_key;
	hashtable_object_t * objects;
};

#define mKeyBelongsToHashtable(ht, key) (key >= 0 && key <= ht->max_key)

hashtable_t hashtable_create(hashtable_key_t max_key)
{
	hashtable_t new_hashtable;
	new_hashtable = (hashtable_t)malloc(sizeof(struct hashtable_s));
	if(new_hashtable)
	{
		new_hashtable->max_key = max_key; // Max nr. used for key
		new_hashtable->objects = (hashtable_object_t *)calloc(max_key+1, sizeof(hashtable_object_t)); // Allocate dyn. array of objects filled with 0 (NULL)
	}
	
	return new_hashtable;
}

void hashtable_destroy(hashtable_t ht)
{
	free(ht->objects);
	free(ht);
}

hashtable_object_t hashtable_search(hashtable_t ht, hashtable_key_t key)
{
	if(!mKeyBelongsToHashtable(ht, key))
		return NULL;
		
	return ht->objects[key];
}

void hashtable_insert(hashtable_t ht, hashtable_key_t key, hashtable_object_t object)
{
	if(!mKeyBelongsToHashtable(ht, key))
		return;
		
	ht->objects[key] = object;
}

void hashtable_delete(hashtable_t ht, hashtable_key_t key)
{
	if(!mKeyBelongsToHashtable(ht, key))
		return;
		
	ht->objects[key] = NULL;	
}

void hashtable_iterate(hashtable_t ht, hashtable_iterate_block_t iterate_block)
{
	for(hashtable_key_t key = 0; key <= ht->max_key; ++key) // loop through entire hashtable key set
	{
		if(ht->objects[key] != NULL) // if object exists for key
		{
			iterate_block(key, ht->objects[key]); // call iterate block for {key, object[key]}
		}
	}
}
