#include "linked_list.h"

node_t *node_create(void *data) {
    // allocate memory for the node
    node_t *node = malloc(sizeof(node_t));
    node->data = data;
    node->next = NULL;

    return node;
}

void node_destroy(node_t *node) {
    free(node->data);
    free(node);
}

linked_list_t *linked_list_create() {
    // allocate memory for the list - calloc initializes the memory to 0
    linked_list_t *list = calloc(1, sizeof(linked_list_t));

    return list;
}

void linked_list_destroy(linked_list_t *list) {
    node_t *current = list->head;

    // iterate through the list and free all nodes
    while (current != NULL) {
        node_t *next = current->next;
        free(current);
        current = next;
    }

    free(list);
}

void linked_list_add(linked_list_t **list, void *data, int position) {
    if (position < 0 || position > (*list)->size) {
        // invalid position
        fprintf(stderr, "Invalid position: %d\n", position);
        return;
    }

    // allocate memory for the new node
    node_t *new_node = node_create(data);

    if (position == 0) {
        // add the node at the beginning of the list
        new_node->next = (*list)->head;
        (*list)->head = new_node;
    } else {
        // find the node before the position
        node_t *current = (*list)->head;
        for (int i = 0; i < position - 1; i++) {
            current = current->next;
        }

        // add the node after the current node
        new_node->next = current->next;
        current->next = new_node;
    }

    // update the tail if the node was added at the end of the list
    if (position == (*list)->size) {
        (*list)->tail = new_node;
    }

    // update the size of the list
    (*list)->size++;
}

void linked_list_remove(linked_list_t **list, int position) {
    if (position < 0 || position >= (*list)->size) {
        // invalid position
        fprintf(stderr, "Invalid position: %d\n", position);
        return;
    }

    if (position == 0) {
        // remove the first node
        node_t *new_head = (*list)->head->next;
        node_destroy((*list)->head);
        (*list)->head = new_head;
    } else {
        // find the node before the position
        node_t *current = (*list)->head;
        for (int i = 0; i < position - 1; i++) {
            current = current->next;
        }

        // remove the node after the current node
        node_t *new_next = current->next->next;
        node_destroy((*list)->head);
        current->next = new_next;
    }

    // update the tail if the node was removed from the end of the list
    if (position == (*list)->size - 1) {
        // find the new tail
        node_t *new_tail = (*list)->head;
        for (int i = 0; i < position - 1; i++) {
            new_tail = new_tail->next;
        }
        (*list)->tail = new_tail;
    }

    // update the size of the list
    (*list)->size--;
}
