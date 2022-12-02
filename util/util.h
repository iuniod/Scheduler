#ifndef UTIL_H_
#define UTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "priority_queue.h"
#include "so_scheduler.h"

#define ERROR -1

typedef struct thread {
    pthread_t thread_id;
    int thread_priority;
    int time_quantum_left;
    enum thread_state {
        NEW,
        READY,
        RUNNING,
        WAITING,
        FINISHED
    } thread_state;
    so_handler *thread_handler;

    sem_t running_state;
    sem_t initialize_state;
} thread_t;

typedef struct scheduler {
    int time_quantum;
    int io_devices;;
    int no_threads;
    priority_queue_t *ready;
    priority_queue_t **waiting;
    thread_t *running;

    sem_t running_state;

} scheduler_t;

thread_t *thread_create(so_handler *handler, int priority, scheduler_t *scheduler);

void *thread_handler(void *arg);

void thread_free(void *arg);

#endif /* UTIL_H_ */