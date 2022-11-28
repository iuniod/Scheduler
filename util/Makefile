CC = gcc
CFLAGS = -Wall -g -ggdb
# TEST?=default_value_if_not_set_in_environment
.PHONY: build
build: libscheduler.so

libscheduler.so: so_scheduler.o
	$(CC) $(CFLAGS) -shared so_scheduler.o -o libscheduler.so
	cp libscheduler.so ../checker-lin/

so_scheduler.o: so_scheduler.c
	$(CC) $(CFLAGS) -fPIC so_scheduler.c -c -o so_scheduler.o

pack:
	zip -r 322CA_Caramida_Iustina_Andreea_Tema2.zip util/*

.PHONY: clean
clean:
	-rm -f so_scheduler.o libscheduler.so ../checker-lin/libscheduler.so