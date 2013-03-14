/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* list.c
* universal-network-c
*/

#include "list.h"
#include "network.h"

#define kListNullNode 0
#define mListNode(list, index) ((list_node_t)(list->reserved_mem + index*sizeof(struct list_node_s))) // Retrieves node using index within reserved memory
#define mListHead(list) ((list_node_t)(list->reserved_mem + list->head*sizeof(struct list_node_s))) // Retrieves node using index within reserved memory
#define mListTail(list) ((list_node_t)(list->reserved_mem + list->tail*sizeof(struct list_node_s))) // Retrieves node using index within reserved memory


struct list_node_s {
	list_object_t object;
	unsigned int next; // next node index
	unsigned int previous; // previous node index
};

typedef struct list_node_s * list_node_t;

unsigned int list_reserve(list_t, const unsigned int);
unsigned int list_find_node_index(list_t, list_object_t);

struct list_s {
	unsigned int head; // previous points to index 0 (can't use pointers because it may be copied to another memory block)
	unsigned int tail; // next point to index 0 (can't use pointers because it may be copied to another memory block)
	unsigned int reserved; // reserved, next free node to be used
	uint8_t * reserved_mem; // reserved allocated memory
	unsigned int capacity; // current capacity, equivalent to sizeof(reserved_mem)/sizeof(struct queue_node_s)
};

list_t list_create(const unsigned int initialCapacity)
{
	list_t new_list;
	new_list = malloc(sizeof(struct list_s));
	if(new_list)
	{
		new_list->head = kListNullNode; // index 0 is equivalent to null, not used
		new_list->tail = kListNullNode; // index 0 is equivalent to null, not used
		new_list->reserved = kListNullNode; // index 0 is equivalent to null, not used
		new_list->reserved_mem = NULL;
		new_list->capacity = 0; 
		list_reserve(new_list, initialCapacity); // allocate first reserved nodes
	}
	
	return new_list;
}

unsigned int list_reserve(list_t l, const unsigned int capacity)
{
	unsigned int newCapacity = l->capacity + capacity; // calculate capacity
	l->reserved_mem = realloc(l->reserved_mem, (1+newCapacity)*sizeof(struct list_node_s)); // (re)allocate reserved_mem for new capacity
	if(l->reserved_mem)
	{
		unsigned int next_weave_node_index = kListNullNode; // weave backwards
		for(int i=capacity; i>0; --i) // weave backwards (index 0 not used, equivalent to null)
		{
			unsigned int weave_node_index = l->capacity + i;
			list_node_t weave_node = mListNode(l, weave_node_index);
			weave_node->object = NULL;
			weave_node->next = next_weave_node_index;
			weave_node->previous = kListNullNode; // not used
			next_weave_node_index = weave_node_index;
		}
		l->reserved = next_weave_node_index; // link to reserved node pointer
		l->capacity = newCapacity; // Increase capacity
		return newCapacity;
	}
	
	return 0; // not successful 
}

void list_destroy(list_t l)
{
	free(l->reserved_mem); // reserved block
	free(l); // list
}

bool list_is_empty(list_t l)
{
	return (l->head == kListNullNode && l->tail == kListNullNode);
}

void list_add(list_t l, list_object_t o)
{	
	// Increase capacity if necessary
	if(!l->reserved)
	{
		if(!list_reserve(l, kListDefaultCapacity)) // increase capacity
			return; // return if can't reserve more memory
	}	
	
	// Get node
	unsigned int add_list_node_index = l->reserved;
	list_node_t add_list_node = mListNode(l, add_list_node_index);
	l->reserved = add_list_node->next;

	// Reset node
	add_list_node->object = o;
	add_list_node->next = add_list_node->previous = kListNullNode;

    // Add node
    if(l->tail)
    { 
		list_node_t tail = mListTail(l);
        tail->next = add_list_node_index; 
        add_list_node->previous = l->tail;
        l->tail = add_list_node_index; 
    } 
    else
    { 
        l->head = l->tail = add_list_node_index; 
    }
}

void list_remove(list_t l, list_object_t o)
{
	unsigned int remove_list_node_index = list_find_node_index(l, o); // Find node to remove
	list_node_t remove_list_node = mListNode(l, remove_list_node_index);
	
	if(remove_list_node->previous == kListNullNode) // Remove from head
    {
        if(remove_list_node->next == kListNullNode) // head = tail = null
        {
            l->head = l->tail = kListNullNode;
        }
        else // head = next
        {
			mListNode(l, remove_list_node->next)->previous = kListNullNode; //remove_list_node->next->previous = kListNullNode; 
            l->head = remove_list_node->next;
        }
    }
    else if(remove_list_node->next == kListNullNode) // Remove from tail
    {
        l->tail = remove_list_node->previous; // tail = previous
		list_node_t tail = mListTail(l);
        tail->next = kListNullNode;
    }
    else // Remove from middle
    {
		mListNode(l, remove_list_node->previous)->next = remove_list_node->next; // remove_list_node->previous->next = remove_list_node->next;
		mListNode(l, remove_list_node->next)->previous = remove_list_node->previous; // remove_list_node->next->previous = remove_list_node->previous;
    }

	remove_list_node->next = l->reserved; // reserved node
	remove_list_node->previous = kListNullNode;
	l->reserved = remove_list_node_index;
}

unsigned int list_find_node_index(list_t l, list_object_t o)
{
	list_node_t node;
	unsigned int index = l->head; // head first
	while(index != kListNullNode) 
	{
	 	node = mListNode(l, index);
		if(node->object == o)
			return index;
			
		index = node->next;
	}
	
	return kListNullNode;
}

list_object_t list_find(list_t l, list_compare_block_t compare_block)
{
	list_node_t node;
	unsigned int index = l->head; // head first
	while(index != kListNullNode) 
	{
		node = mListNode(l, index);
		if(compare_block(node->object))	// Use provided block to compare
	 		return node->object;
			
		index = node->next;
	}	

	return NULL;
}

void list_iterate(list_t l, list_iterate_block_t iterate_block)
{
	list_node_t node;
	unsigned int index = l->head; // head first
	while(index != kListNullNode) 
	{
		node = mListNode(l, index);
		iterate_block(node->object); // Execute block for each object
			
		index = node->next;
	}
}

int debug_list_added_count(list_t l)
{
	int added_count = 0;
	
	list_node_t node;
	unsigned int index = l->head; // head first
	while(index != kListNullNode) 
	{
		node = mListNode(l, index);
		++added_count;
		
		index = node->next;
	}
	
	return added_count;
}

int debug_list_reserved_count(list_t l)
{
	int reserved_count = 0;
	
	list_node_t node;
	unsigned int index = l->reserved; // reserved first
	while(index != kListNullNode) 
	{
		node = mListNode(l, index);
		++reserved_count;
		
		index = node->next;
	}
	
	return reserved_count;	
}
