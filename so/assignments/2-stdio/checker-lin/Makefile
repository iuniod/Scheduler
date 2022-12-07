default: build

build: tema2so.o
	gcc -shared tema2so.o -o libso_stdio.so

tema2so.o: tema2so.c
	gcc -fPIC -c tema2so.c

clean:
	rm *.o
	rm libso_stdio.so
