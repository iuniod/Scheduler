#include "util.h"

void thread_free(void *arg)
{
	thread_t *thread = (thread_t *) arg;

	if (thread == NULL)
		return;

	pthread_join(thread->thread_id, NULL);
	sem_destroy(&(thread->running_state));

	free(thread);
}

void thread_set(thread_t *thread)
{
	thread->thread_state = READY;
	push(&(scheduler->ready), thread, thread->thread_priority);
}

/**
 * @brief Change the running thread and make its settings
 * 
 * @param thread thenext running thread
 */
void thread_run(thread_t *thread)
{
	scheduler->running = thread;
	// remove the thread from the ready queue
	pop(&(scheduler->ready));
	thread->thread_state = RUNNING;
	sem_post(&(thread->running_state));
}

/**
 * @brief Get the next thread with the highest priority that is ready
 * 
 * @param scheduler the scheduler
 * @return thread_t* the next thread
 */
thread_t *next_priority_thread(scheduler_t *scheduler)
{
	thread_t *thread = peek(scheduler->ready);

	while (thread && thread->thread_state != READY) {
		if (thread->thread_state == FINISHED)
			linked_list_add(&(scheduler->finished), thread);
		pop(&(scheduler->ready));
		thread = peek(scheduler->ready);
	}

	return thread;
}

void thread_schedule(void)
{
	thread_t *running = scheduler->running;
	thread_t *top = next_priority_thread(scheduler);

	// there are no threads to run
	if (top == NULL) {
		if (running->thread_state == FINISHED)
			sem_post(&(scheduler->running_state));

		sem_post(&(running->running_state));
		return;
	}

	// the running thread has finished, is waiting or it doesn't exist
	if (running == NULL || running->thread_state == FINISHED || running->thread_state == WAITING) {
		// if it exists, add it to the finished list
		if (running != NULL && running->thread_state == FINISHED)
			linked_list_add(&(scheduler->finished), running);
		thread_run(top);
		return;
	}

	// the running thread has finished its time quantum
	if (running->time_quantum_left <= 0) {
		running->time_quantum_left = scheduler->time_quantum;
		if (top->thread_priority >= running->thread_priority) {
			thread_set(running);
			thread_run(top);
			return;
		}
	}

	// the next thread has a higher priority than the running thread
	if (top->thread_priority > running->thread_priority) {
		thread_set(running);
		thread_run(top);
		return;
	}

	// if we get here, we have the same running thread 
	sem_post(&(running->running_state));
}

void *thread_handler(void *arg)
{
	thread_t *thread = (thread_t *) arg;

	// wait until thread has finished its run
	sem_wait(&thread->running_state);

	// call the handler
	thread->thread_handler(thread->thread_priority);

	// mark the thread as finished
	thread->thread_state = FINISHED;

	thread_schedule();

	return NULL;
}

thread_t *thread_create(so_handler *handler, int priority, scheduler_t *sch)
{
	scheduler = sch;

	// allocate memory for the new thread and check if allocation was successful
	thread_t *new_thread = calloc(1, sizeof(thread_t));

	if (!new_thread)
		return NULL;

	// initialize the new thread's structure
	new_thread->thread_priority = priority;
	new_thread->time_quantum_left = scheduler->time_quantum;
	new_thread->thread_state = NEW;
	new_thread->thread_handler = handler;
	sem_init(&new_thread->running_state, 0, 0);

	// create the new thread
	pthread_create(&new_thread->thread_id, NULL, thread_handler, new_thread);

	return new_thread;
}
