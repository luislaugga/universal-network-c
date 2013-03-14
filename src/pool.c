/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* pool.c
* universal-network-c
*/

#include "pool.h"

struct pool_node_s {
	void * object;
	unsigned int retainCount; // 0 if free, > 0 if allocated/retained
	struct pool_node_s * next;
};

typedef struct pool_node_s * pool_node_t;

void pool_free_node(pool_t p, pool_node_t node); // Internal

struct pool_s {
	uint8_t * memblock;
	uint8_t * membound;

	size_t sizeObject;
	size_t sizeObjectNode;
	size_t capacity;
	
	pool_node_t freeList;
	unsigned int allocCount;
};

#define mAddressDoesNotBelongsToPool(Address, Pool) ((Address < Pool->memblock) || (Address >= Pool->membound))
	
static void pool_weave_freelist(pool_t p)
{
	 // Memory layout: Object, [Object *, Retain Count, Next *]
	const size_t sizeObjectNode = p->sizeObjectNode; // Object+Node = Object size + Node size
	const size_t sizeObject = p->sizeObject;
	
	uint8_t * addrNode = p->memblock + sizeObject; // Memory address: end of Object+Node

	pool_node_t prevNode = NULL; // Temporary variable to store last weaved Node
	
	for(int i=0; i<p->capacity; ++i)
	{
		pool_node_t tmp = (pool_node_t)addrNode;
		tmp->object = (void *)(addrNode - sizeObject);
		tmp->retainCount = 0; // Reset to 0
		tmp->next = NULL;
		
		if(p->freeList == NULL)
			p->freeList = tmp; // First node
		else
			prevNode->next = tmp;
		
		prevNode = tmp;
		addrNode += sizeObjectNode;
	}
}

pool_t pool_create(size_t sizeObject, size_t capacity)
{
	pool_t p = (pool_t)malloc(sizeof(struct pool_s));
	if(p)
	{
		p->freeList = NULL;
		p->allocCount = 0;
		
		p->sizeObject = sizeObject; // Object
		p->sizeObjectNode = sizeObject + sizeof(struct pool_node_s); // Object + Node
		p->capacity = capacity;
		
		const size_t memsize = capacity * p->sizeObjectNode; // Total size of pool's memory block returned from malloc
		p->memblock = (uint8_t *)malloc(memsize);
		
		if(!p->memblock)  // Not enough space to allocate memblock
		{
			free(p);
			return NULL;
		};
			
		p->membound = p->memblock + memsize; // End memory address bound (does not belong to pool)
		
		pool_weave_freelist(p); // Weave nodes in list
	}

	return p;
}

void pool_destroy(pool_t p)
{
	if(p)
	{
		free(p->memblock);
		free(p);
	}
}

size_t pool_capacity(pool_t p)
{
	return p->capacity;
}

void * pool_alloc(pool_t p)
{
	if(p->freeList == NULL)
		return NULL; // Pool is out of space
	
	pool_node_t allocNode = p->freeList; // Remove from free list
	p->freeList = allocNode->next; // Update free list
	allocNode->next = NULL; // Reset node
	allocNode->retainCount = 1; // Set retain count

	++p->allocCount; // Increase alloc count
	
	return allocNode->object;
}

void pool_retain(pool_t p, void * object)
{
	if(p->allocCount == 0)
		return; // Pool doesn't have any allocated nodes. Object doesn't belong to this pool...

	// Convert object's address for arithmetic manipulation
	uint8_t * addrObject = (uint8_t *)object;
	
	if(mAddressDoesNotBelongsToPool(addrObject, p))
		return; // Object's memory address doesn't belong to pool
		
	pool_node_t retainNode = (pool_node_t)(addrObject + p->sizeObject); // Retrieve node
	++retainNode->retainCount; // Increase retain count
}

void pool_free_node(pool_t p, pool_node_t node) // Internal
{
	node->next = p->freeList; // Add to free list
	p->freeList = node; // Update free list
	node->retainCount = 0; // Set retain count
	
	--p->allocCount; // Decrease alloc count
}

void pool_free(pool_t p, void * object)
{
	if(p->allocCount == 0)
		return; // Pool doesn't have any allocated nodes. Object doesn't belong to this pool...

	// Convert object's address for arithmetic manipulation
	uint8_t * addrObject = (uint8_t *)object;
	
	if(mAddressDoesNotBelongsToPool(addrObject, p))
		return; // Object's memory address doesn't belong to pool
		
	pool_node_t freeNode = (pool_node_t)(addrObject + p->sizeObject); // Retrieve node
	pool_free_node(p, freeNode);
}

void pool_release(pool_t p, void * object)
{
	if(p->allocCount == 0)
		return; // Pool doesn't have any allocated nodes. Object doesn't belong to this pool...

	// Convert object's address for arithmetic manipulation
	uint8_t * addrObject = (uint8_t *)object;
	
	if(mAddressDoesNotBelongsToPool(addrObject, p))
		return; // Object's memory address doesn't belong to pool
		
	pool_node_t releaseNode = (pool_node_t)(addrObject + p->sizeObject); // Retrieve node
	
	if((--releaseNode->retainCount) == 0) // Decrease retain count
		pool_free_node(p, releaseNode); // Free if retain count reaches 0
}

int debug_pool_free_count(pool_t p)
{
	int freeCount = 0;
	pool_node_t iter;
	
	iter = p->freeList; // free
	while(iter != NULL) 
	{
		iter = iter->next;
		++freeCount;
	}
	
	return freeCount;	
}

int debug_pool_alloc_count(pool_t p)
{	
	return p->allocCount;
}
