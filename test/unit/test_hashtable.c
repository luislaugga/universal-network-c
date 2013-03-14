/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_hashtable.c
* universal-network-c
*/

#include "test.h"
#include "hashtable.h"

static void test_hashtable()
{
	LOG_TEST_START;
	
	const hashtable_key_t max_key = 255;

	hashtable_t test_hashtable = hashtable_create(max_key);

	int number1 = 1;
	int number2 = 2;
	int number3 = 6;
	int number4 = 8;

	hashtable_object_t test_obj_1 = &number1;
	hashtable_object_t test_obj_2 = &number2;
	hashtable_object_t test_obj_3 = &number3;
	hashtable_object_t test_obj_4 = &number4;
	hashtable_key_t test_key_1 = 10;
	hashtable_key_t test_key_2 = max_key;
	hashtable_key_t test_key_3 = 0;
	hashtable_key_t test_key_4 = max_key+1; // Impossible

	assert(hashtable_search(test_hashtable, test_key_1) == NULL);	
	hashtable_insert(test_hashtable, test_key_1, test_obj_1);
	
	assert(hashtable_search(test_hashtable, test_key_2) == NULL);	
	hashtable_insert(test_hashtable, test_key_2, test_obj_2);
	
	assert(hashtable_search(test_hashtable, test_key_3) == NULL);	
	hashtable_insert(test_hashtable, test_key_3, test_obj_3);
	
	assert(hashtable_search(test_hashtable, test_key_4) == NULL);	
	hashtable_insert(test_hashtable, test_key_4, test_obj_4); // Impossible, will do nothing
	
	assert(hashtable_search(test_hashtable, test_key_1) != NULL);
	assert(hashtable_search(test_hashtable, test_key_2) != NULL);
	assert(hashtable_search(test_hashtable, test_key_3) != NULL);
	assert(hashtable_search(test_hashtable, test_key_4) == NULL); // Impossible, should return NULL
	
	assert(hashtable_search(test_hashtable, test_key_1) == test_obj_1);
	assert(hashtable_search(test_hashtable, test_key_2) == test_obj_2);
	assert(hashtable_search(test_hashtable, test_key_3) == test_obj_3);

	hashtable_delete(test_hashtable, test_key_1);
	assert(hashtable_search(test_hashtable, test_key_1) == NULL);
	
	hashtable_delete(test_hashtable, test_key_2);
	assert(hashtable_search(test_hashtable, test_key_2) == NULL);
	
	hashtable_delete(test_hashtable, test_key_3);
	assert(hashtable_search(test_hashtable, test_key_3) == NULL);
	
	hashtable_delete(test_hashtable, test_key_4); // Impossible, will do nothing
	
	assert(hashtable_search(test_hashtable, test_key_1) == NULL);
	assert(hashtable_search(test_hashtable, test_key_2) == NULL);
	assert(hashtable_search(test_hashtable, test_key_3) == NULL);
	
	hashtable_destroy(test_hashtable);

	LOG_TEST_END;
}

static void test_hashtable_iterate()
{
	LOG_TEST_START;
	
	const hashtable_key_t max_key = 255;

	hashtable_t test_hashtable = hashtable_create(max_key);

	int number1 = 1;
	int number2 = 2;
	int number3 = 6;
	int number4 = 8;

	hashtable_object_t test_obj_1 = &number1;
	hashtable_object_t test_obj_2 = &number2;
	hashtable_object_t test_obj_3 = &number3;
	hashtable_object_t test_obj_4 = &number4;
	hashtable_key_t test_key_1 = 10;
	hashtable_key_t test_key_2 = max_key;
	hashtable_key_t test_key_3 = 0;
	hashtable_key_t test_key_4 = max_key+1; // Impossible

	assert(hashtable_search(test_hashtable, test_key_1) == NULL);	
	assert(hashtable_search(test_hashtable, test_key_2) == NULL);	
	assert(hashtable_search(test_hashtable, test_key_3) == NULL);	
	assert(hashtable_search(test_hashtable, test_key_4) == NULL);	

	hashtable_insert(test_hashtable, test_key_1, test_obj_1);
	hashtable_insert(test_hashtable, test_key_2, test_obj_2);	
	hashtable_insert(test_hashtable, test_key_3, test_obj_3);
	hashtable_insert(test_hashtable, test_key_4, test_obj_4); // Impossible, will do nothing
	
	assert(hashtable_search(test_hashtable, test_key_1) != NULL);
	assert(hashtable_search(test_hashtable, test_key_2) != NULL);
	assert(hashtable_search(test_hashtable, test_key_3) != NULL);
	assert(hashtable_search(test_hashtable, test_key_4) == NULL); // Impossible, should return NULL
	
	assert(hashtable_search(test_hashtable, test_key_1) == test_obj_1);
	assert(hashtable_search(test_hashtable, test_key_2) == test_obj_2);
	assert(hashtable_search(test_hashtable, test_key_3) == test_obj_3);

	// Delete all using iterate block
	hashtable_iterate(test_hashtable, ^(hashtable_key_t key, hashtable_object_t object){
		assert(object != NULL);
		assert(hashtable_search(test_hashtable, key) != NULL);
		hashtable_delete(test_hashtable, key);
		assert(hashtable_search(test_hashtable, key) == NULL);
	});

	assert(hashtable_search(test_hashtable, test_key_1) == NULL);
	assert(hashtable_search(test_hashtable, test_key_2) == NULL);
	assert(hashtable_search(test_hashtable, test_key_3) == NULL);
	hashtable_delete(test_hashtable, test_key_4); // Impossible, will do nothing
	
	hashtable_destroy(test_hashtable);

	LOG_TEST_END;
}

int main(void)
{	
	LOG_SUITE_START("hashtable");

	test_hashtable();
	test_hashtable_iterate();
	
	return 0;
}
