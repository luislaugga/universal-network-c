/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_queue.c
* universal-network-c
*/

#include "test.h"
#include "queue.h"

static void test_queue()
{
	LOG_TEST_START;

	queue_t test_queue = queue_create();

	int numberOne = 1;
	int numberTwo = 2;

	queue_object_t test_obj_1 = &numberOne;
	queue_object_t test_obj_2 = &numberTwo;

	assert(queue_is_empty(test_queue) == true);
	assert(debug_queue_enqueued_count(test_queue) == 0);
	assert(debug_queue_reserved_count(test_queue) == 0);

	queue_push(test_queue, test_obj_1);

	assert(queue_is_empty(test_queue) == false);
	assert(debug_queue_enqueued_count(test_queue) == 1);
	assert(debug_queue_reserved_count(test_queue) == 0);

	queue_push(test_queue, test_obj_2);

	assert(queue_is_empty(test_queue) == false);
	assert(debug_queue_enqueued_count(test_queue) == 2);
	assert(debug_queue_reserved_count(test_queue) == 0);

	queue_object_t test_obj;

	test_obj = queue_pop(test_queue);

	assert(test_obj == test_obj_1);
	assert(queue_is_empty(test_queue) == false);
	assert(debug_queue_enqueued_count(test_queue) == 1);
	assert(debug_queue_reserved_count(test_queue) == 1);

	test_obj = queue_pop(test_queue);

	assert(test_obj == test_obj_2);
	assert(queue_is_empty(test_queue) == true);
	assert(debug_queue_enqueued_count(test_queue) == 0);
	assert(debug_queue_reserved_count(test_queue) == 2);

	queue_push(test_queue, test_obj_2);

	assert(queue_is_empty(test_queue) == false);
	assert(debug_queue_enqueued_count(test_queue) == 1);
	assert(debug_queue_reserved_count(test_queue) == 1);

	queue_push(test_queue, test_obj_2);

	assert(queue_is_empty(test_queue) == false);
	assert(debug_queue_enqueued_count(test_queue) == 2);
	assert(debug_queue_reserved_count(test_queue) == 0);

	test_obj = queue_pop(test_queue);

	assert(test_obj == test_obj_2);
	assert(queue_is_empty(test_queue) == false);
	assert(debug_queue_enqueued_count(test_queue) == 1);
	assert(debug_queue_reserved_count(test_queue) == 1);

	test_obj = queue_pop(test_queue);

	assert(test_obj == test_obj_2);
	assert(queue_is_empty(test_queue) == true);
	assert(debug_queue_enqueued_count(test_queue) == 0);
	assert(debug_queue_reserved_count(test_queue) == 2);

	queue_destroy(test_queue);

	LOG_TEST_END;
}

int main(void)
{
	LOG_SUITE_START("queue");

	test_queue();
	
	return 0;
}
