#include "so_scheduler.h"
#include "priority_queue.h"
#include "util.h"
#include <errno.h>

#define SO_VERBOSE_ERROR

DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io)
{
	// check if scheduler is already initialized of
	// if the number of IOs is bigger than the maximum number of IOs
	if (io > SO_MAX_NUM_EVENTS || scheduler || time_quantum <= 0)
		return ERROR;

	// allocate memory for scheduler
	scheduler = calloc(1, sizeof(scheduler_t));
	if (!scheduler)
		return ERROR;

	// initialize scheduler
	scheduler->time_quantum = time_quantum;
	scheduler->io_devices = io;

	// allocate memory for the ready threads queue
	scheduler->ready = priority_queue_create();
	if (!scheduler->ready)
		return ERROR;

	// allocate memory for the finished threads list
	scheduler->finished = linked_list_create();
	if (!scheduler->finished)
		return ERROR;

	// allocate memory for the waiting threads queues
	scheduler->waiting = calloc(io, sizeof(linked_list_t *));
	if (!scheduler->waiting)
		return ERROR;

	// initialize the waiting threads queues for each IO device
	for (unsigned int i = 0; i < io; i++) {
		scheduler->waiting[i] = linked_list_create();
		if (!scheduler->waiting[i])
			return ERROR;
	}

	// increase the semaphore value to 1
	sem_init(&scheduler->running_state, 0, 1);

	return 0;
}

DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority)
{
	if (!scheduler || !func || priority > SO_MAX_PRIO)
		return INVALID_TID;

	if (scheduler->no_threads == 0)
		sem_wait(&(scheduler->running_state));

	// create a new thread and initialize it
	thread_t *new_thread = thread_create(func, priority, scheduler);

	// count the number of threads
	scheduler->no_threads += 1;
	// change the state of the new thread to ready and add it to the ready queue
	if (new_thread->thread_state == NEW) {
		new_thread->thread_state = READY;
		push(&(scheduler->ready), new_thread, new_thread->thread_priority);
	}

	// schedule the thread according to the running thread
	if (scheduler->running == NULL)
		thread_schedule();
	else
		so_exec();

	// return the thread id
	return new_thread->thread_id;
}

DECL_PREFIX int so_wait(unsigned int io)
{
	// particular cases of error
	if (!scheduler || io >= scheduler->io_devices || io < 0)
		return ERROR;

	// if there is no running thread, schedule a new one
	if (scheduler->running == NULL) {
		thread_schedule();
		return 0;
	}

	// change the state of the running thread to waiting
	scheduler->running->thread_state = WAITING;
	// add the running thread to the waiting list of the IO device
	linked_list_add(&(scheduler->waiting[io]), scheduler->running);

	// execute the next thread
	so_exec();

	return 0;
}

DECL_PREFIX int so_signal(unsigned int io)
{
	// particular cases of error
	if (!scheduler || io >= scheduler->io_devices || io < 0)
		return ERROR;

	int cnt_threads = 0;
	// pop the first thread from the waiting queue of the IO device
	thread_t *thread = linked_list_remove(&(scheduler->waiting[io]), 0);

	// for each thread in the waiting queue of the IO device
	while (thread != NULL) {
		// change the state of the thread to ready
		thread->thread_state = READY;
		// add the thread to the ready queue
		push(&(scheduler->ready), thread, thread->thread_priority);
		// count the number of threads and pop the next one
		thread = linked_list_remove(&(scheduler->waiting[io]), 0);
		cnt_threads += 1;
	}

	// execute the running thread
	so_exec();

	// return the number of threads that were in the waiting queue
	return cnt_threads;
}

DECL_PREFIX void so_exec(void)
{
	// check if there is a running thread
	if (!scheduler->running)
		return;

	// find the running thread
	thread_t *running_thread = scheduler->running;

	// decrement the remaining time of the running thread
	running_thread->time_quantum_left -= 1;

	// schedule a new thread if it is time to do so
	thread_schedule();

	// wait for the running thread to finish
	sem_wait(&(running_thread->running_state));
}

DECL_PREFIX void so_end(void)
{
	// check if we allocated memory for the scheduler
	if (scheduler) {
		// wait for all threads to finish if there are any
		if (scheduler->no_threads != 0)
			sem_wait(&scheduler->running_state);

		// free the waiting threads queues
		if (scheduler->waiting) {
			for (unsigned int i = 0; i < scheduler->io_devices; i++) {
				if (scheduler->waiting[i])
					linked_list_destroy(&(scheduler->waiting[i]), thread_free);
			}
			free(scheduler->waiting);
		}

		// free the ready threads queue
		if (scheduler->ready)
			priority_queue_destroy(&(scheduler->ready), thread_free);

		// free the finished threads list
		if (scheduler->finished)
			linked_list_destroy(&(scheduler->finished), thread_free);

		if (scheduler->running)
			thread_free(scheduler->running);

		// free the scheduler
		sem_destroy(&scheduler->running_state);
		free(scheduler);
	}

	scheduler = NULL;
}
