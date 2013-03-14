/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* queue.c
* universal-network-c
*/

#include "queue.h"

struct queue_node_s {
	queue_object_t object;
	struct queue_node_s * next;
};

typedef struct queue_node_s * queue_node_t;

struct queue_s {
	queue_node_t head;
	queue_node_t tail;
	queue_node_t reserved; // allocated, not used
};

queue_t queue_create()
{
	queue_t new_queue;
	new_queue = (queue_t)malloc(sizeof(struct queue_s));
	if(new_queue)
	{
		new_queue->head = NULL;
		new_queue->tail = NULL;
		new_queue->reserved = NULL;
	}
	
	return new_queue;
}

void queue_destroy(queue_t q)
{
	queue_node_t iter, tmp;
	
	iter = q->head; // enqueued
	while(iter != NULL) 
	{
		tmp = iter;
		iter = iter->next;
		free(tmp);
	}
	
	iter = q->reserved; // reserved
	while(iter != NULL) 
	{
		tmp = iter;
		iter = iter->next;
		free(tmp);
	}
	
	free(q);
}

bool queue_is_empty(queue_t q)
{
	return (q->head == NULL && q->tail == NULL);
}

void queue_push(queue_t q, queue_object_t o)
{
	queue_node_t push_queue_node;
	
	if(q->reserved)
	{
		push_queue_node = q->reserved;
		q->reserved = q->reserved->next;
	}
	else
	{
		push_queue_node = (queue_node_t)malloc(sizeof(struct queue_node_s));
	}
	
	if(push_queue_node)
	{
		push_queue_node->object = o;
		push_queue_node->next = NULL;
		
		if(q->head == NULL && q->tail == NULL)
			q->head = push_queue_node;
		else
			q->tail->next = push_queue_node;
			
		q->tail = push_queue_node;
	}
}

queue_object_t queue_pop(queue_t q)
{
	queue_object_t pop_queue_object = NULL;
	
	if((q->head == NULL && q->tail == NULL) == false) // not empty
	{
		queue_node_t pop_queue_node = q->head;
	 	pop_queue_object = pop_queue_node->object;
		
		if(q->head == q->tail) // 1 node
		{
			q->head = NULL;
			q->tail = NULL;
		}
		else
		{
			q->head = q->head->next;
		}
		
		pop_queue_node->next = q->reserved; // reserved node
		q->reserved = pop_queue_node;
	}
	
	return pop_queue_object;
}

int debug_queue_enqueued_count(queue_t q)
{
	int enqueued_count = 0;
	queue_node_t iter;
	
	iter = q->head; // enqueued
	while(iter != NULL) 
	{
		iter = iter->next;
		++enqueued_count;
	}
	
	return enqueued_count;
}

int debug_queue_reserved_count(queue_t q)
{
	int reserved_count = 0;
	queue_node_t iter;
	
	iter = q->reserved; // reserved
	while(iter != NULL) 
	{
		iter = iter->next;
		++reserved_count;
	}
	
	return reserved_count;	
}
