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

void linked_list_add(linked_list_t **list, void *data) {
    if (*list == NULL) {
        return;
    }
    
    // allocate memory for the new node
    node_t *new_node = node_create(data, 0);
    if (new_node == NULL) {
        return;
    }

    // if the list is empty, set the new node as head
    if ((*list)->head == NULL) {
        (*list)->head = new_node;
        return;
    }

    // insert the new node
    new_node->next = (*list)->head;
    (*list)->head = new_node;
}
