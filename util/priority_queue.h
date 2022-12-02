/*
 * Threads scheduler header
 *
 * 2017, Operating Systems
 */

#ifndef PRIORITY_QUEUE_H_
#define PRIORITY_QUEUE_H_

#include <stdlib.h>
#include <stdio.h>

#include "linked_list.h"

typedef struct priority_queue {
    int size;
    linked_list_t *queue;
} priority_queue_t;

priority_queue_t *priority_queue_create(void);

void priority_queue_destroy(priority_queue_t **queue, void (*free_data)(void *));

void push(priority_queue_t **queue, void *data, int priority);

void *pop(priority_queue_t **queue, void (*free_data)(void *));

void *peek(priority_queue_t *queue);

#endif /* PRIORITY_QUEUE_H_ */