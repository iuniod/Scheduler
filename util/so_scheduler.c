#include "so_scheduler.h"
#include "priority_queue.h"
#include "util.h"
#include <errno.h>

scheduler_t *scheduler;

DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io) {
    /* check if scheduler is already initialized of
    if the number of IOs is bigger than the maximum number of IOs*/
    if (io > SO_MAX_NUM_EVENTS || scheduler || time_quantum <= 0) {
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
        return ERROR;
    }

    // allocate memory for the waiting threads queues and check if allocation was successful
    scheduler->waiting = calloc(io, sizeof(priority_queue_t *));
    if (!scheduler->waiting) {
        return ERROR;
    }
    // initialize the waiting threads queues for each IO device and check if initialization was successful
    for (unsigned int i = 0; i < io; i++) {
        scheduler->waiting[i] = priority_queue_create();
        if (!scheduler->waiting[i]) {
            return ERROR;
        }
    }

    // increase the semaphore value to 1
    sem_init(&scheduler->running_state, 0, 1);

    return 0;
}

DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority) {
    if (!scheduler || !func || priority > SO_MAX_PRIO) {
        return INVALID_TID;
    }

    // create a new thread and initialize it
    thread_t *new_thread = thread_create(func, priority, scheduler);
    
    // wait for the new thread to be initialized
    sem_wait(&new_thread->initialize_state);

    // IDK if this is necessary
    so_exec();

    return new_thread->thread_id;
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
     // check if there is a running thread
    if (!scheduler->running) {
        return;
    }

    /*thread_t *running_thread = scheduler->running;
    running_thread->time_quantum_left--;

    thread_t *top_thread = peek(scheduler->ready);

    // check if the running thread has finished its time quantum
    if (running_thread->time_quantum_left == 0) {
        running_thread->time_quantum_left = scheduler->time_quantum;
        running_thread->thread_state = READY;
        push(&(scheduler->ready), running_thread, running_thread->thread_priority);
        scheduler->running = top_thread;
        scheduler->running->thread_state = RUNNING;
        sem_post(&scheduler->running->running_state);
        pop(&(scheduler->ready), thread_free);
    } else if (top_thread && running_thread->thread_priority > top_thread->thread_priority) {
        running_thread->thread_state = READY;
        push(&(scheduler->ready), running_thread, running_thread->thread_priority);
        scheduler->running = top_thread;
        scheduler->running->thread_state = RUNNING;
        sem_post(&scheduler->running->running_state);
        pop(&(scheduler->ready), thread_free);
    }

    if (scheduler->running != running_thread) {
        sem_wait(&scheduler->running->running_state);
    }*/
}

DECL_PREFIX void so_end(void) {
    if (scheduler) {
        if (scheduler->no_threads != 0) {
            sem_wait(&scheduler->running_state);
        }
        // free the waiting threads queues
        if (scheduler->waiting) {
            for (unsigned int i = 0; i < scheduler->io_devices; i++) {
                if (scheduler->waiting[i]) {
                    priority_queue_destroy(&(scheduler->waiting[i]), thread_free);
                }
            }
            free(scheduler->waiting);
        }

        // free the ready threads queue
        if (scheduler->ready) {
            priority_queue_destroy(&(scheduler->ready), thread_free);
        }

        if (scheduler->running) {
            thread_free(scheduler->running);
        }

        // free the scheduler
        free(scheduler);
        sem_destroy(&scheduler->running_state);
    }

    scheduler = NULL;
}
