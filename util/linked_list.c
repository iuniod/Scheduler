#include "linked_list.h"

node_t *node_create(void *data, int priority) {
    // allocate memory for the node
    node_t *node = malloc(sizeof(node_t));
    node->data = data;
    node->priority = priority;
    node->next = NULL;

    return node;
}

void node_destroy(node_t *node, void (*free_data)(void *)) {
    free_data(node->data);
    free(node);
}

linked_list_t *linked_list_create() {
    // allocate memory for the list - calloc initializes the memory to 0
    linked_list_t *list = calloc(1, sizeof(linked_list_t));

    return list;
}

void linked_list_destroy(linked_list_t **list, void (*free_data)(void *)) {
    node_t *current = (*list)->head;

    // iterate through the list and free all nodes
    while (current != NULL) {
        node_t *next = current->next;
        node_destroy(current, free_data);
        current = next;
    }

    free(*list);
    *list = NULL;
}
