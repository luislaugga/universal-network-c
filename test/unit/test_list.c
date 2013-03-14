/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_list.c
* universal-network-c
*/

#include "test.h"
#include "list.h"

static void test_list_create()
{
	LOG_TEST_START;
	
	list_t test_list = list_create(kListDefaultCapacity);
	
	assert(list_is_empty(test_list) == true);
	assert(debug_list_reserved_count(test_list) == kListDefaultCapacity);
	
	list_destroy(test_list);
	
	LOG_TEST_END;
}

static void test_list_add_remove()
{
	LOG_TEST_START;

	list_t test_list = list_create(kListDefaultCapacity);

	int numberOne = 1;
	int numberTwo = 2;

	list_object_t test_obj_1 = &numberOne;
	list_object_t test_obj_2 = &numberTwo;

	assert(list_is_empty(test_list) == true);
	assert(debug_list_added_count(test_list) == 0);
	assert(debug_list_reserved_count(test_list) == kListDefaultCapacity);

	list_add(test_list, test_obj_1);

	assert(list_is_empty(test_list) == false);
	assert(debug_list_added_count(test_list) == 1);
	assert(debug_list_reserved_count(test_list) == kListDefaultCapacity-1);

	list_add(test_list, test_obj_2);

	assert(list_is_empty(test_list) == false);
	assert(debug_list_added_count(test_list) == 2);
	assert(debug_list_reserved_count(test_list) == kListDefaultCapacity-2);

	list_remove(test_list, test_obj_1);

	assert(list_is_empty(test_list) == false);
	assert(debug_list_added_count(test_list) == 1);
	assert(debug_list_reserved_count(test_list) == kListDefaultCapacity-1);

	list_remove(test_list, test_obj_2);

	assert(list_is_empty(test_list) == true);
	assert(debug_list_added_count(test_list) == 0);
	assert(debug_list_reserved_count(test_list) == kListDefaultCapacity);

	list_add(test_list, test_obj_2);

	assert(list_is_empty(test_list) == false);
	assert(debug_list_added_count(test_list) == 1);
	assert(debug_list_reserved_count(test_list) == kListDefaultCapacity-1);

	list_add(test_list, test_obj_1);

	assert(list_is_empty(test_list) == false);
	assert(debug_list_added_count(test_list) == 2);
	assert(debug_list_reserved_count(test_list) == kListDefaultCapacity-2);

	list_remove(test_list, test_obj_1);

	assert(list_is_empty(test_list) == false);
	assert(debug_list_added_count(test_list) == 1);
	assert(debug_list_reserved_count(test_list) == kListDefaultCapacity-1);

	list_remove(test_list, test_obj_2);

	assert(list_is_empty(test_list) == true);
	assert(debug_list_added_count(test_list) == 0);
	assert(debug_list_reserved_count(test_list) == kListDefaultCapacity);

	list_destroy(test_list);

	LOG_TEST_END;
}

static void test_list_capacity()
{
	LOG_TEST_START;

	list_t test_list = list_create(kListDefaultCapacity);
	
	assert(list_is_empty(test_list) == true);
	assert(debug_list_added_count(test_list) == 0);
	assert(debug_list_reserved_count(test_list) == kListDefaultCapacity);
	
	int numbers[10*kListDefaultCapacity];
	
	unsigned int added_estimate;
	unsigned int reserved_estimate;
	
	// Add 
	for(int i=0; i<10*kListDefaultCapacity; ++i)
	{
		numbers[i] = i;
		
		list_add(test_list, &numbers[i]);
			
		added_estimate = i+1;
		reserved_estimate = ((i/kListDefaultCapacity+1)*kListDefaultCapacity)-added_estimate;
			
		assert(list_is_empty(test_list) == false);
		assert(debug_list_added_count(test_list) == added_estimate);
		assert(debug_list_reserved_count(test_list) == reserved_estimate);
	}
	
	// Remove and check (middle)
	for(int i=3*kListDefaultCapacity; i<6*kListDefaultCapacity; ++i)
	{
		list_remove(test_list, &numbers[i]);
			
		--added_estimate;
		++reserved_estimate;
		
		assert(list_is_empty(test_list) == false);
		assert(debug_list_added_count(test_list) == added_estimate);
		assert(debug_list_reserved_count(test_list) == reserved_estimate);
	}
	
	// Remove and check (last)
	for(int i=6*kListDefaultCapacity; i<10*kListDefaultCapacity; ++i)
	{
		list_remove(test_list, &numbers[i]);
			
		--added_estimate;
		++reserved_estimate;
		
		assert(list_is_empty(test_list) == false);
		assert(debug_list_added_count(test_list) == added_estimate);
		assert(debug_list_reserved_count(test_list) == reserved_estimate);
	}
	
	// Remove and check (first)
	for(int i=0; i<3*kListDefaultCapacity; ++i)
	{
		list_remove(test_list, &numbers[i]);
			
		--added_estimate;
		++reserved_estimate;
			
		assert(debug_list_added_count(test_list) == added_estimate);
		assert(debug_list_reserved_count(test_list) == reserved_estimate);
	}
	
	assert(list_is_empty(test_list) == true);

	list_destroy(test_list);

	LOG_TEST_END;
}

static void test_list_find()
{
	LOG_TEST_START;

	unsigned int capacity = 10;
	list_t test_list = list_create(capacity);
	
	assert(list_is_empty(test_list) == true);
	
	int numbers[capacity];
		
	// Add 
	for(int i=0; i<capacity; ++i)
	{
		numbers[i] = i;
		list_add(test_list, &numbers[i]);
		assert(list_is_empty(test_list) == false);
	}
	
	// Find + Remove
	for(int i=5; i<10; ++i)
	{
		int * compare_ptr = &numbers[i];
		int * found_ptr = list_find(test_list, ^(list_object_t object) {
			return (bool)(object == compare_ptr);
		});
			
		assert(found_ptr != NULL);
		assert(*found_ptr == numbers[i]);
			
		list_remove(test_list, found_ptr);
	
	}
	
	assert(list_is_empty(test_list) == false);

	list_destroy(test_list);

	LOG_TEST_END;
}

static void test_list_iterate()
{
	LOG_TEST_START;

	unsigned int capacity = 10;
	list_t test_list = list_create(capacity);
	
	assert(list_is_empty(test_list) == true);
	
	int numbers[capacity];
		
	// Add 
	for(int i=0; i<capacity; ++i)
	{
		numbers[i] = i;
		list_add(test_list, &numbers[i]);
		assert(list_is_empty(test_list) == false);
	}
	
	// Iterate
	for(int i=0; i<capacity; ++i)
	{
		numbers[i] *= 2; // Multiply by two
		
		list_iterate(test_list, ^(list_object_t object) {
			unsigned int * object_ptr = (unsigned int *)object;
			(*object_ptr) *= 2; // Multiply by two
		});
	}
	
	// Compare
	for(int i=0; i<capacity; ++i)
	{
		int * compare_ptr = &numbers[i];
		int * found_ptr = list_find(test_list, ^(list_object_t object) {
			return (bool)(object == compare_ptr);
		});
			
		assert(found_ptr != NULL);
		assert(*found_ptr == numbers[i]); // Check they match
	}

	list_destroy(test_list);

	LOG_TEST_END;
}

int main(void)
{
	LOG_SUITE_START("list");

	test_list_create();	
	test_list_add_remove();
	test_list_capacity();
	test_list_find();
	test_list_iterate();
	
	return 0;
}
