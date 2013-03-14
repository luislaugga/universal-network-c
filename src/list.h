/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* list.h
* universal-network-c
*/

#ifndef __universal_network_list_h__
#define __universal_network_list_h__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*!
 * @header
 *
 * list_t is a generic linked list data type.
 *
 * It can be used to store any dynamic allocated object (pointer). It keeps a 
 * block of reserved nodes used to add new object to the list. The initial capacity of
 * this reserved block can be specified during creation. Whenever more space is needed,
 * the list will re-allocate a new block of reserved nodes with kListDefaultCapacity more 
 * nodes capacity than previously.
 *
 * IMPORTANT: The initial capacity of the listmay be increased. Realloc is used and all
 * memory contents may be copied as a result. So, the linked list uses indices to store
 * next/previous nodes for each node. NO memory pointers are stored as they may become
 * invalid during copying (if the entire memory block needs to be reassigned). A handy
 * macro called mListNode(list, index) will retrieve the node pointer.
 */

#define kListDefaultCapacity 10 // Reserves space for 10 elements by default

typedef void * list_object_t;
typedef struct list_s * list_t;

list_t list_create(const unsigned int);
void list_destroy(list_t);

bool list_is_empty(list_t); // Is empty?

void list_add(list_t, list_object_t); // Add object
void list_remove(list_t, list_object_t); // Remove object

typedef bool (^list_compare_block_t)(list_object_t); // Find using block to compare
list_object_t list_find(list_t, list_compare_block_t);

typedef void (^list_iterate_block_t)(list_object_t); // Loop using block for each node object
void list_iterate(list_t, list_iterate_block_t);

int debug_list_reserved_count(list_t);
int debug_list_added_count(list_t);

#endif