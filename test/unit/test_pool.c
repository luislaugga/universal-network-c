/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_pool.c
* universal-network-c
*/

#include "test.h"
#include "pool.h"

static void test_pool_alloc_free()
{
	LOG_TEST_START;
	
	size_t testObjectSize = sizeof(int);
	
	pool_t test_pool = pool_create(testObjectSize, kPoolDefaultCapacity);
	
	size_t capacity = pool_capacity(test_pool);
	size_t halfCapacity = capacity/2;
	
	int * testObjects[capacity];
	int i = 0;
	
	assert(debug_pool_free_count(test_pool) == capacity);
	assert(debug_pool_alloc_count(test_pool) == 0);
		
	for(; i<halfCapacity; ++i)
	{
		testObjects[i] = (int *)pool_alloc(test_pool);
	}
	
	assert(debug_pool_free_count(test_pool) == capacity-halfCapacity);
	assert(debug_pool_alloc_count(test_pool) == halfCapacity);
	
	for(; i<capacity; ++i)
	{
		testObjects[i] = (int *)pool_alloc(test_pool);
	}
	
	assert(debug_pool_free_count(test_pool) == 0);
	assert(debug_pool_alloc_count(test_pool) == capacity);
	
	for(i=capacity-1; i>=0; --i)
	{
		pool_free(test_pool, testObjects[i]);
	}	
	
	assert(debug_pool_free_count(test_pool) == capacity);
	assert(debug_pool_alloc_count(test_pool) == 0);

	pool_destroy(test_pool);
	
	LOG_TEST_END;
}

static void test_pool_retain_release()
{
	LOG_TEST_START;
	
	size_t testObjectSize = sizeof(int);
	
	pool_t test_pool = pool_create(testObjectSize, kPoolDefaultCapacity);
	
	size_t capacity = pool_capacity(test_pool);
	size_t halfCapacity = capacity/2;
	
	int * testObjects[capacity];
	int i = 0;
	
	assert(debug_pool_free_count(test_pool) == capacity);
	assert(debug_pool_alloc_count(test_pool) == 0);
			
	for(; i<capacity; ++i)
	{
		testObjects[i] = (int *)pool_alloc(test_pool);
		pool_retain(test_pool, testObjects[i]);
	}
	
	assert(debug_pool_free_count(test_pool) == 0);
	assert(debug_pool_alloc_count(test_pool) == capacity);
	
	for(i=capacity-1; i>=0; --i)
	{
		pool_release(test_pool, testObjects[i]);
		pool_release(test_pool, testObjects[i]);
	}	
	
	assert(debug_pool_free_count(test_pool) == capacity);
	assert(debug_pool_alloc_count(test_pool) == 0);

	pool_destroy(test_pool);
	
	LOG_TEST_END;
}

int main(void)
{
	LOG_SUITE_START("pool");

	test_pool_alloc_free();
	test_pool_retain_release();
	
	return 0;
}
