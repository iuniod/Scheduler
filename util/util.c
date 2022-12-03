#include "util.h"

scheduler_t *scheduler;

void thread_plan(thread_t *thread) {
    if (scheduler->running == NULL) {
        thread->thread_state = RUNNING;
        scheduler->running = thread;
        sem_post(&scheduler->running->running_state);
    } else if (thread->thread_priority > scheduler->running->thread_priority) {
        thread->thread_state = RUNNING;
        scheduler->running->thread_state = READY;
        push(&(scheduler->ready), scheduler->running, scheduler->running->thread_priority);
        scheduler->running = thread;
        // sem_post(&scheduler->running->running_state);
    } else {
        thread->thread_state = READY;
        push(&(scheduler->ready), thread, thread->thread_priority);
    }

    sem_post(&thread->initialize_state);
}

void *thread_handler(void *arg) {
    thread_t *thread = (thread_t *) arg;

    // initialize the thread
    thread_plan(thread);

    // wait until thread has finished its run
    sem_wait(&thread->running_state);

    // call the handler
    thread->thread_handler(thread->thread_priority);

    // mark the thread as finished
    thread->thread_state = FINISHED;

    linked_list_add(&(scheduler->finished), thread);
	scheduler->running = peek(scheduler->ready);
    pop(&(scheduler->ready), thread_free);

    if (scheduler->running != NULL)
		sem_post(&scheduler->running->running_state);

    if (scheduler->running == NULL && peek(scheduler->ready) == NULL) {
			sem_post(&scheduler->running_state);
    }

    return NULL;
}

void thread_free(void *arg) {
    thread_t *thread = (thread_t *) arg;

    pthread_join(thread->thread_id, NULL);
    sem_destroy(&(thread->running_state));
    sem_destroy(&(thread->initialize_state));
    free(thread);
}

thread_t *thread_create(so_handler *handler, int priority, scheduler_t *sch) {
    scheduler = sch;
    
    // allocate memory for the new thread and check if allocation was successful
    thread_t *new_thread = calloc(1, sizeof(thread_t));
    if (!new_thread) {
        return NULL;
    }

    // initialize the new thread's structure
    new_thread->thread_priority = priority;
    new_thread->time_quantum_left = scheduler->time_quantum;
    new_thread->thread_state = NEW;
    new_thread->thread_handler = handler;
    scheduler->no_threads++;
    sem_init(&new_thread->running_state, 0, 0);
    sem_init(&new_thread->initialize_state, 0, 0);

    // create the new thread
    pthread_create(&new_thread->thread_id, NULL, thread_handler, new_thread);
    
    return new_thread;
}

thread_t *next_priority_thread(scheduler_t *scheduler) {
    thread_t *thread = peek(scheduler->ready);
    while (thread && thread->thread_state == FINISHED) {
        linked_list_add(&(scheduler->finished), thread);
        pop(&(scheduler->ready), thread_free);
        thread = peek(scheduler->ready);
    }

    return thread;
}