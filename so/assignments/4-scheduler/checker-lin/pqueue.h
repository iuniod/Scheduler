#ifndef PQUEUE_H_
#define PQUEUE_H_

#include <stdio.h>
#include <stdlib.h>

typedef struct PQNode {
	unsigned long data;
	unsigned int priority;
	struct PQNode *next;
} PQNode;

typedef struct PQueue {
	PQNode *head;
	long size;
} PQueue;

PQueue *createPQueue(void)
{
	PQueue *pq = (PQueue *) malloc(sizeof(PQueue));

	pq->head = NULL;
	pq->size = 0;
	return pq;
}

int isPQEmpty(PQueue *pq)
{
	return pq->head == NULL;
}

void printQ(PQueue *pq)
{
	PQNode *aux = pq->head;

	while (aux != NULL) {
		printf("id:%u\n", aux->priority);
		aux = aux->next;
	}
	printf("\n");
}

void push(PQueue *pq, unsigned long data, unsigned int priority)
{
	PQNode *newNode = (PQNode *) malloc(sizeof(PQNode));

	newNode->data = data;
	newNode->priority = priority;
	newNode->next = NULL;

	if (isPQEmpty(pq)) {
		pq->head = newNode;
		pq->size += 1;
	} else {
		PQNode *start = pq->head;

		if (start->priority < priority) {
			newNode->next = start;
			pq->head = newNode;
			pq->size += 1;
		} else {
			while (start->next != NULL && start->next->priority >= priority)
				start = start->next;

			newNode->next = start->next;
			start->next = newNode;
			pq->size += 1;
		}
	}
}

unsigned long peek(PQueue *pq)
{
	return pq->head->data;
}

void pop(PQueue *pq)
{
	if (isPQEmpty(pq))
		return;

	PQNode *aux = pq->head;

	pq->head = pq->head->next;
	free(aux);
	pq->size -= 1;
}

void destroyPQ(PQueue *pq)
{
	while (!(isPQEmpty(pq)))
		pop(pq);

	free(pq);
}

#endif
