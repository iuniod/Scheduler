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
} thread_t;

typedef struct scheduler {
	int time_quantum;
	int io_devices;
	int no_threads;
	priority_queue_t *ready;
	linked_list_t **waiting;
	thread_t *running;
	linked_list_t *finished;
	sem_t running_state;

} scheduler_t;

scheduler_t *scheduler;

/**
 * @brief Make the thread ready and add it to the ready queue
 * 
 * @param thread the thread to be added
 */
void thread_set(thread_t *thread);

/**
 * @brief Schedule the threads
 * 
 */
void thread_schedule(void);

/**
 * @brief Creates a new thread
 * 
 * @param handler handler function
 * @param priority priority of the thread
 * @param scheduler scheduler
 * @return thread_t* pointer to the new thread
 */
thread_t *thread_create(so_handler *handler, int priority, scheduler_t *scheduler);

/**
 * @brief Free the thread and its resources and join it
 * 
 * @param arg the thread to be freed
 */
void thread_free(void *arg);

#endif /* UTIL_H_ */
