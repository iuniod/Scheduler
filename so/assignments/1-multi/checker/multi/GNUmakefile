default: build

build: so-cpp.o hashtable.o
	gcc -Wall -g so-cpp.o hashtable.o -o so-cpp	

so-cpp.o: so-cpp.c hashtable.h utils.h
	gcc -c so-cpp.c

hashtable.o: hashtable.c
	gcc -c hashtable.c

clean:
	rm hashtable.o
	rm so-cpp.o
	rm so-cpp
