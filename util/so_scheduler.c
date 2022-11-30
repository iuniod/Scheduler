#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "so_scheduler.h"
#include "priority_queue.h"

#define ERROR -1

typedef struct thread {
    pthread_t id_thread;
    int priority;
    int time_quantum_left;
    enum thread_state {
        NEW,
        READY,
        RUNNING,
        WAITING,
        FINISHED
    } thread_state;
    so_handler *handler;
    sem_t is_running;
} thread_t;

typedef struct scheduler {
    int time_quantum;
    int io_devices;;
    int no_threads;
    priority_queue_t *ready;
    priority_queue_t **waiting;
    thread_t *running;

} scheduler_t;


scheduler_t *scheduler;

void *thread_handler(void *arg) {
    thread_t *thread = (thread_t *) arg;

    thread->handler(thread->id_thread);

    thread->thread_state = FINISHED;
    so_exec();

    return NULL;
}

DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io) {
    /* check if scheduler is already initialized of
    if the number of IOs is bigger than the maximum number of IOs*/
    if (io > SO_MAX_NUM_EVENTS || scheduler) {
        return ERROR;
    }

    // allocate memory for scheduler and check if allocation was successful
    scheduler = calloc(1, sizeof(scheduler_t));
    if (!scheduler) {
        return ERROR;
    }

    // initialize scheduler
    scheduler->time_quantum = time_quantum;
    scheduler->io_devices = io;

    // allocate memory for the ready threads queue and check if allocation was successful
    scheduler->ready = priority_queue_create();
    if (!scheduler->ready) {
        return INVALID_TID;
    }

    // allocate memory for the waiting threads queues and check if allocation was successful
    scheduler->waiting = calloc(io, sizeof(priority_queue_t *));
    if (!scheduler->waiting) {
        return INVALID_TID;
    }
    // initialize the waiting threads queues for each IO device and check if initialization was successful
    for (unsigned int i = 0; i < io; i++) {
        scheduler->waiting[i] = priority_queue_create();
        if (!scheduler->waiting[i]) {
            return INVALID_TID;
        }
    }

    return 0;
}

DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority) {
    if (!scheduler || !func || priority > SO_MAX_PRIO) {
        return ERROR;
    }

    // allocate memory for the new thread and check if allocation was successful
    thread_t *new_thread = calloc(1, sizeof(thread_t));
    if (!new_thread) {
        return ERROR;
    }

    // initialize the new thread
    new_thread->id_thread = scheduler->no_threads;
    new_thread->priority = priority;
    new_thread->time_quantum_left = scheduler->time_quantum;
    new_thread->thread_state = NEW;
    new_thread->handler = func;
    scheduler->no_threads++;

    // pthread_create(&new_thread->id_thread, NULL, (void *)func, new_thread);
    // sem_init(&new_thread->is_running, 0, 0);

    // push the new thread in the ready queue
    new_thread->thread_state = READY;
    push(&scheduler->ready, new_thread, priority);
    int ret = new_thread->id_thread;
    free(new_thread);
    so_exec();

    return ret;
}

DECL_PREFIX int so_wait(unsigned int io) {
    if (!scheduler || io >= scheduler->io_devices) {
        return ERROR;
    }

    return 0;
}

DECL_PREFIX int so_signal(unsigned int io) {
    if (!scheduler || io >= scheduler->io_devices) {
        return ERROR;
    }

    return 0;
}

DECL_PREFIX void so_exec(void) {
    if (!scheduler->running) {
        return;
    }
}

DECL_PREFIX void so_end(void) {
    if (scheduler) {
        // free the waiting threads queues
        if (scheduler->waiting) {
            for (unsigned int i = 0; i < scheduler->io_devices; i++) {
                if (scheduler->waiting[i]) {
                    priority_queue_destroy(&(scheduler->waiting[i]));
                }
            }
            free(scheduler->waiting);
        }

        // free the ready threads queue
        if (scheduler->ready) {
            priority_queue_destroy(&(scheduler->ready));
        }

        if (scheduler->running) {
            free(scheduler->running);
        }

        // free the scheduler
        free(scheduler);
    }

    scheduler = NULL;
}
