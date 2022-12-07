CC = cl.exe
LINK=link.exe
CLFAGS = /MD /nologo /W4
default: build

build: so-cpp.obj hashtable.obj
	$(CC) /Zi so-cpp.obj hashtable.obj /Feso-cpp.exe

so-cpp.obj: so-cpp.c
	$(CC) $(CFLAGS) /c so-cpp.c /Foso-cpp.obj

hashtable.obj: hashtable.c
	$(CC) $(CFLAGS) /c hashtable.c /Fohashtable.obj

clean:
	del *.obj
	del so-cpp.exe
