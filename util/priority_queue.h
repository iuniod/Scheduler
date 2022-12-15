#ifndef PRIORITY_QUEUE_H_
#define PRIORITY_QUEUE_H_

#include <stdlib.h>
#include <stdio.h>

#include "linked_list.h"

/**
 * @brief A priority queue data structure.
 * size: number of elements in the queue
 * queue: pointer to the linked list used to store the elements
 */
typedef struct {
	int size;
	linked_list_t *queue;
} priority_queue_t;

/**
 * @brief Creates a new priority queue
 * 
 * @return priority_queue_t* the new queue
 */
priority_queue_t *priority_queue_create(void);

/**
 * @brief Destroys a priority queue - frees queue memory
 * 
 * @param queue queue to be destroyed
 * @param free_data function used to free the data stored in the queue
 */
void priority_queue_destroy(priority_queue_t **queue, void (*free_data)(void *));

/**
 * @brief Adds a new element to the queue
 * 
 * @param queue queue to which the element is added
 * @param data element data
 * @param priority element priority
 */
void push(priority_queue_t **queue, void *data, int priority);

/**
 * @brief Removes the element with the highest priority from the queue
 * 
 * @param queue queue from which the element is removed
 * @return void* pointer to the data stored in the removed element
 */
void *pop(priority_queue_t **queue);

/**
 * @brief Returns the element with the highest priority from the queue
 * 
 * @param queue queue from which the element is returned
 * @return void* pointer to the data stored in the returned element
 */
void *peek(priority_queue_t *queue);

#endif /* PRIORITY_QUEUE_H_ */
