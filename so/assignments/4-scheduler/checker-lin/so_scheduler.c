#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "utils.h"
#include "_test/so_scheduler.h"
#include "pqueue.h"

#define MAX_THREADS 1024

typedef enum status { NEW, READY, RUNNING, WAITING, TERMINATED } t_status;

typedef struct Thread {
	unsigned int priority;
	unsigned int quantum_left;
	unsigned int event;
	tid_t id;
	t_status status;
	so_handler *handler;
	sem_t thread_sem;
} Thread;

typedef struct Scheduler {
	unsigned int no_events;
	unsigned int quantum_size;
	unsigned int no_threads;
	Thread *curr_run_thread;
	Thread **threads;
	PQueue *pqueue;
	sem_t sch_sem;
} Scheduler;

static Scheduler *scheduler;

static void reschedule(void);

static void *thread_function(void *args)
{
	int rc;
	Thread *thread;

	thread = (Thread *) args;
	rc = sem_wait(&(thread->thread_sem));
	DIE(rc != 0, "wait semaphore thread");

	thread->handler(thread->priority);
	thread->status = TERMINATED;

	reschedule();

	return NULL;
}

static void run_thread(Thread *thread)
{
	int rc;

	pop(scheduler->pqueue);
	thread->status = RUNNING;
	thread->quantum_left = scheduler->quantum_size;
	rc = sem_post(&(thread->thread_sem));
	DIE(rc != 0, "run thread semaphore post");
}

static void reschedule(void)
{
	int rc;
	int i;
	unsigned long prio_id;
	Thread *prio_thread;
	Thread *curr_thread;

	curr_thread = scheduler->curr_run_thread;
	if (scheduler->pqueue->size == 0) {
		if (curr_thread->status == TERMINATED) {
			rc = sem_post(&(scheduler->sch_sem));
			DIE(rc != 0, "resch sch sem post");
		}
		rc = sem_post(&(curr_thread->thread_sem));
		DIE(rc != 0, "resch thread sem post");
		return;
	}

	prio_id = peek(scheduler->pqueue);
	for (i = 0; i < scheduler->no_threads; i++)
		if (scheduler->threads[i]->id == prio_id) {
			prio_thread = scheduler->threads[i];
			break;
		}


	if (curr_thread == NULL || curr_thread->status == TERMINATED ||
		curr_thread->status == WAITING) {
		scheduler->curr_run_thread = prio_thread;
		run_thread(prio_thread);
		return;
	}

	if (prio_thread->priority > curr_thread->priority) {
		push(scheduler->pqueue, curr_thread->id, curr_thread->priority);
		scheduler->curr_run_thread = prio_thread;
		run_thread(prio_thread);
		return;
	}

	if (curr_thread->quantum_left < 1) {
		if (prio_thread->priority == curr_thread->priority) {
			push(scheduler->pqueue, curr_thread->id, curr_thread->priority);
			scheduler->curr_run_thread = prio_thread;
			run_thread(prio_thread);
			return;
		}
		curr_thread->quantum_left = scheduler->quantum_size;
	}

	rc = sem_post(&(curr_thread->thread_sem));
	DIE(rc != 0, "resch thread sem post");
}

int so_init(unsigned int time_quantum, unsigned int no_io)
{
	int rc;

	if (time_quantum <= 0 || no_io > SO_MAX_NUM_EVENTS || scheduler != NULL)
		return -1;

	scheduler = (Scheduler *) calloc(1, sizeof(Scheduler));
	scheduler->no_events = no_io;
	scheduler->quantum_size = time_quantum;
	scheduler->no_threads = 0;
	scheduler->curr_run_thread = NULL;

	scheduler->threads = (Thread **) malloc(MAX_THREADS * sizeof(Thread *));
	scheduler->pqueue = createPQueue();


	rc = sem_init(&(scheduler->sch_sem), 0, 1);
	DIE(rc != 0, "scheduler semaphore init");

	return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	int rc;
	Thread *thread;

	if (func == NULL || priority > SO_MAX_PRIO)
		return INVALID_TID;

	if (scheduler->no_threads == 0) {
		rc = sem_wait(&(scheduler->sch_sem));
		DIE(rc != 0, "fork semaphore wait");
	}

	/*init thread struct*/
	thread = (Thread *) malloc(sizeof(Thread));
	DIE(thread == NULL, "thread malloc");
	thread->priority = priority;
	thread->quantum_left = scheduler->quantum_size;
	thread->event = SO_MAX_NUM_EVENTS;
	thread->id = INVALID_TID;
	thread->status = NEW;
	thread->handler = func;

	rc = sem_init(&(thread->thread_sem), 0, 0);
	DIE(rc != 0, "fork thread semaphore init");

	rc = pthread_create(&(thread->id), NULL, &thread_function, (void *)thread);
	DIE(rc != 0, "pthread create");

	scheduler->threads[scheduler->no_threads] = thread;
	scheduler->no_threads += 1;
	push(scheduler->pqueue, thread->id, thread->priority);

	if (scheduler->curr_run_thread == NULL)
		reschedule();
	else
		so_exec();

	return thread->id;
}

int so_wait(unsigned int io)
{
	if (io < 0 || io >= scheduler->no_events)
		return -1;

	scheduler->curr_run_thread->status = WAITING;
	scheduler->curr_run_thread->event = io;
	so_exec();
	return 0;
}

int so_signal(unsigned int io)
{
	int i;
	int no_unlocked = 0;

	if (io < 0 || io >= scheduler->no_events)
		return -1;

	for (i = 0; i < scheduler->no_threads; i++)
		if (scheduler->threads[i]->status == WAITING &&
			scheduler->threads[i]->event == io) {
			scheduler->threads[i]->status = READY;
			scheduler->threads[i]->event = SO_MAX_NUM_EVENTS;
			no_unlocked++;
			push(scheduler->pqueue, scheduler->threads[i]->id,
				scheduler->threads[i]->priority);
		}

	so_exec();

	return no_unlocked;
}

void so_exec(void)
{
	int rc;
	Thread *curr_thread;

	curr_thread = scheduler->curr_run_thread;
	curr_thread->quantum_left -= 1;

	reschedule();
	rc = sem_wait(&(curr_thread->thread_sem));
	DIE(rc != 0, "exec thread semaphore wait");
}

void so_end(void)
{
	int rc;
	long i;

	if (scheduler == NULL)
		return;

	rc = sem_wait(&(scheduler->sch_sem));
	DIE(rc != 0, "scheduler semaphore wait");

	for (i = 0; i < scheduler->no_threads; i++) {
		rc = pthread_join(scheduler->threads[i]->id, NULL);
		DIE(rc != 0, "thread join");
	}

	/*Destroy threads*/
	for (i = 0; i < scheduler->no_threads; i++) {
		rc = sem_destroy(&(scheduler->threads[i]->thread_sem));
		DIE(rc != 0, "thread semaphore destroy");
		free(scheduler->threads[i]);
	}

	/*Destroy scheduler*/
	destroyPQ(scheduler->pqueue);
	free(scheduler->threads);
	rc = sem_destroy(&(scheduler->sch_sem));
	DIE(rc != 0, "scheduler semaphore destroy");
	free(scheduler);
	scheduler = NULL;
}
