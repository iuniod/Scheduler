#include "priority_queue.h"

priority_queue_t *priority_queue_create(void) {
    priority_queue_t *queue = calloc(1, sizeof(priority_queue_t));
    if (queue == NULL) {
        return NULL;
    }

    queue->queue = linked_list_create(); 
    if (queue->queue == NULL) {
        free(queue);
        return NULL;
    }

    return queue;
}

void priority_queue_destroy(priority_queue_t **queue) {
    if (*queue == NULL) {
        return;
    }

    linked_list_destroy(&((*queue)->queue));
    free(*queue);
    *queue = NULL;
}

void push(priority_queue_t **queue, void *data, int priority) {
    if (*queue == NULL) {
        return;
    }

    // allocate memory for the new node
    node_t *new_node = node_create(data, priority);
    if (new_node == NULL) {
        return;
    }

    // find the node before the position
    node_t *current = (*queue)->queue->head;
    if (current == NULL) {
        (*queue)->queue->head = new_node;
        (*queue)->size++;
        return;
    }

    while (current != NULL && current->priority < priority) {
        current = current->next;
    }

    if (current == NULL) {
        current->next = new_node;
    } else {
        new_node->next = current->next;
        current->next = new_node;
    }

    // update the size of the queue
    (*queue)->size++;
}

void *pop(priority_queue_t **queue) {
    if (*queue == NULL) {
        return NULL;
    }

    // get the first node
    node_t *first = (*queue)->queue->head;
    if (first == NULL) {
        return NULL;
    }

    // remove the first node
    (*queue)->queue->head = first->next;
    
    // get the data from the first node
    void *data = first->data;

    // free the first node
    node_destroy(first);

    // update the size of the queue
    (*queue)->size--;

    return data;
}

void *peek(priority_queue_t *queue) {
    if (queue == NULL) {
        return NULL;
    }

    // get the first node
    node_t *first = queue->queue->head;
    if (first == NULL) {
        return NULL;
    }

    // get the data from the first node
    void *data = first->data;

    return data;
}