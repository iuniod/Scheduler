default: build

build: so_scheduler.o
	gcc -Wall -g -shared so_scheduler.o -o libscheduler.so

so_scheduler.o: so_scheduler.c utils.h pqueue.h
	gcc -Wall -g -fPIC -c so_scheduler.c

clean:
	rm *.o
	rm libscheduler.so
