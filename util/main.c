#include <stdio.h>
#include <stdlib.h>

#include "linked_list.h"
#include "priority_queue.h"

typedef struct {
    int a;
    int *b;

} test_t;

void free_t(void *arg) {
    test_t *t = (test_t *) arg;
    free(t->b);
    free(t);
}

int main() {
    priority_queue_t *q = priority_queue_create();
    test_t *t = malloc(sizeof(test_t));
    t->a = 1;
    t->b = malloc(5 * sizeof(int));
    t->b[0] = 1;
    t->b[1] = 2;
    t->b[2] = 3;
    t->b[3] = 4;
    t->b[4] = 5;
    push(&q, t, 1);

    test_t *t2 = malloc(sizeof(test_t));
    t2->a = 2;
    t2->b = malloc(5 * sizeof(int));
    t2->b[0] = 1;
    t2->b[1] = 2;
    t2->b[2] = 3;
    t2->b[3] = 4;
    t2->b[4] = 5;
    push(&q, t2, 4);

    priority_queue_destroy(&q, free_t);
}