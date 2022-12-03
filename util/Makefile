CC = gcc
CFLAGS = -Wall -g -ggdb
# TEST?=default_value_if_not_set_in_environment
.PHONY: build
build: libscheduler.so
 
libscheduler.so: so_scheduler.o linked_list.o priority_queue.o util.o
	$(CC) $(CFLAGS) -shared so_scheduler.o linked_list.o priority_queue.o util.o -o libscheduler.so
	cp libscheduler.so ../checker-lin/

so_scheduler.o: so_scheduler.c
	$(CC) $(CFLAGS) -fPIC so_scheduler.c -c -o so_scheduler.o

linked_list.o: linked_list.c
	$(CC) $(CFLAGS) -fPIC linked_list.c -c -o linked_list.o

priority_queue.o: priority_queue.c
	$(CC) $(CFLAGS) -fPIC priority_queue.c -c -o priority_queue.o

util.o: util.c
	$(CC) $(CFLAGS) -fPIC util.c -c -o util.o

pack:
	zip -r 322CA_Caramida_Iustina_Andreea_Tema2.zip *.c *.h Makefile ../README

test:
	$(CC) -std=gnu99 -g main.c linked_list.c priority_queue.c -o test

test_run:
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./test

.PHONY: clean
clean:
	-rm -f so_scheduler.o libscheduler.so ../checker-lin/libscheduler.so *.o test