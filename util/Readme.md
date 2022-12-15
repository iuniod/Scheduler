# Threads Scheduler in C

This is a project for the Operating Systems course at the University of Bucharest.

## DESCRIPTION
This project is a scheduler for threads. It uses a structure called `scheduler` and a structure called `thread` to manage the threads.

## FUNCTIONS
- `so_init` - initializes the scheduler: verifies the arguments, allocates memory for the scheduler and its fields, verifies the allocation and initializes the fields (time quantum, io devices, 0 and null)
- `so_end` - ends the scheduler: wait until all threads have finished running, free the memory allocated for the scheduler and its fields
- `so_exec` - executes a thread: verifies if there is a thread running, decreases the time quantum left of the running thread, schedules the threads and wait for the running thread to finish
- `so_fork` - creates a new thread: verifies if the arguments are valid, creates the thread, counts the number of threads, adds the thread to the ready queue, schedules the threads and returns the id of the new thread
- `so_wait` - waits for an io device: verifies if the arguments are valid, schedules the threads if there is no thread running, otherwise changes the state of the running thread to waiting, stores the thread in the waiting list and executes the next thread
- `so_signal` - signals an io device: verifies if the arguments are valid, frees the waiting list of the io device, counting the number of threads and moved them to the ready queue, and executes the next thread

## MORE EXPLANATIONS ABOUT THE IMPLEMENTATION
Each thread has a running semaphore, which is used to wait for the thread to finish running. I used a running semaphore for the scheduler too, which is used to wait for all threads to finish running.


I chose to use semaphores instead of mutexes because I wanted to use the `sem_wait` function to wait for the threads to finish running and it seemed easier to use semaphores instead of mutexes (personal opinion).

I used a linked list for finished threads because it was easier to free the memory allocated for the threads and join them.

I used a priority queue for ready threads because it was easier to access the thread with the highest priority.

I used an array of linked list for waiting threads because it was easier to access the waiting list of a specific io device.

`thread_scheduler()` is in charge of scheduling the threads. It checks if the are no threads left to run and changes the state of the scheduler semaphore. It verifies if the running thread does not exist, is finished of is waiting for an IO device - in this case, the running thread is changed with the next priority thread. It the running thread finished its time, compare its priority with the next priority thread - if the priority of the next thread is bigger or equal with the priority of the running thread, the running thread is changed with the next priority thread. Otherwise, the time quantum left of the running thread is reseted. In the end, it the priority of the runnging thread is not bigger than the priority of the next thread, the running thread is changed with the next priority thread. *It is very important to verify the conditions in this order, otherwise the program will not work correctly.*

In `util.h` there are some functions that are used in `so_scheduler.c` to make the code more readable. Moreover, because `so_scheduler.h` is forbidden to be modified, I had to put the functions and the structures in `util.h`.

`linked_list.c` and `priority_queue.c` are used to implement the linked list and the priority queue with only the functions that are needed for this project.




## HOW TO RUN
- util/
    - `make` to compile the project and move the shared object to the checker folder
    - `make clean` to clean the project
    - `pack` to pack the project
    - `test` to test the project in *main.c* and `test_run` to run the test with valgrind
- checker-lin/
    - `make -f Makefile.checker` to compile and run the checker
    - `make -f Makefile.checker clean` to clean the checker project
    - `checkpatch.pl --no-tree -f [file_path]` to check the coding style of a file


# TODO LIST ABOUT HOMEWORK

- [x] add the skel and the checker
- [x] create linked_list + test it
- [x] create priority_queue + test it
- [x] do the structs
- [x] do so_init
- [x] do so_end
- [x] do so_exec
- [x] do so_fork
- [x] do so_wait
- [x] do so_signal
- [x] write README.md
- [ ] upload to vmchecker
