#include <stdio.h>
#include <stdlib.h>

#include "linked_list.h"

int main() {
    linked_list_t *list = linked_list_create();
    linked_list_add(&list, "Hello", 0);
    linked_list_add(&list, "World", 1);
    linked_list_add(&list, "!", 2);
    
    node_t *current = list->head;
    for (int i = 0; i < list->size; i++) {
        printf("%s ", current->data);
        current = current->next;
    }

    linked_list_destroy(list);
}