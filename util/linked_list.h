/*
 * Threads scheduler header
 *
 * 2017, Operating Systems
 */

#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Linked list node
 * data: pointer to the data stored in the node
 * next: pointer to the next node in the list
 */
typedef struct node {
    void *data;
    struct node *next;
    int priority;
} node_t;

/**
 * @brief Linked list
 * head: pointer to the first node in the list
 * tail: pointer to the last node in the list
 * size: number of nodes in the list
 */
typedef struct linked_list {
    node_t *head;
    int size;
} linked_list_t;

/**
 * @brief Creates a new node
 *
 * @param data pointer to the data to be stored in the node
 * @return node_t* pointer to the new node
 */
node_t *node_create(void *data, int priority);

/**
 * @brief Destroys a node - frees node memory
 *
 * @param node pointer to the node to be destroyed
 */
void node_destroy(node_t *node, void (*free_data)(void *));

/**
 * @brief Creates a new linked list
 *
 * @return linked_list_t* pointer to the new list
 */
linked_list_t *linked_list_create();

/**
 * @brief Add a new node to the list - the new node is added
 * at the beginning of the list
 *
 * @param list the list to which the node is added
 * @param data pointer to the node data
 */
void linked_list_add(linked_list_t **list, void *data);

/**
 * @brief Destroys a linked list - frees all memory
 *
 * @param list pointer to the list to be destroyed
 */
void linked_list_destroy(linked_list_t **list, void (*free_data)(void *));

#endif /* LINKED_LIST_H_ */
