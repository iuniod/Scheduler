#include "so_stdio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

#define BUF_SIZE 4096

struct _so_file {
	int fileno; /*descriptorul de fisier*/
	unsigned char buf[BUF_SIZE]; /*buffer*/
	int buf_point; /*pozitia in buffer*/
	int buf_len; /*lungimea in in buffer*/
	int last_op; /*ultima operatie*/
	long cursor; /*pozitia cursorului in fisier*/
	int err; /*setata pe 1 daca a fost eroare*/
	int eof; /*setata pe 1 daca s-a ajuns la finalul fisierului*/
	pid_t pid; /*salveaza id-ul procesului creat de fork*/
};

#define min(a, b) ((a) < (b) ? (a) : (b))

typedef struct _so_file SO_FILE;

/*deschide fisierul in modul corespunzator si creeaza structura SO_FILE*/
/*daca esuaza returneaza NULL*/
SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	int flags = 0;
	mode_t create_mode = 0644;
	SO_FILE *fp;

	if (strcmp(mode, "r") == 0)
		flags = O_RDONLY;
	else if (strcmp(mode, "r+") == 0)
		flags = O_RDWR;
	else if (strcmp(mode, "w") == 0)
		flags = O_WRONLY | O_CREAT | O_TRUNC;
	else if (strcmp(mode, "w+") == 0)
		flags = O_RDWR | O_CREAT | O_TRUNC;
	else if (strcmp(mode, "a") == 0)
		flags = O_WRONLY | O_APPEND | O_CREAT;
	else if (strcmp(mode, "a+") == 0)
		flags = O_RDWR | O_APPEND | O_CREAT;
	else
		return NULL;

	fp = (SO_FILE *) calloc(1, sizeof(SO_FILE));
	if (fp == NULL)
		return NULL;
	fp->buf_point = 0;
	fp->buf_len = 0;
	fp->last_op = -1;
	fp->cursor = 0;
	fp->err = 0;
	fp->eof = 0;
	memset(fp->buf, '\0', BUF_SIZE);
	fp->fileno = open(pathname, flags, create_mode);
	if (fp->fileno < 0) {
		free(fp);
		return NULL;
	}

	return fp;
}

/*inchide file_descriptorul primit pentru fisier returnand 0*/
/*inainte sa inchida da flush, daca esueaza returneaza SO_EOF*/
int so_fclose(SO_FILE *stream)
{
	int rc = 0;

	rc = so_fflush(stream);
	if (rc != 0)
		stream->err = SO_EOF;

	rc = close(stream->fileno);
	if (rc != 0)
		stream->err = SO_EOF;

	if (stream->err == SO_EOF)
		rc = SO_EOF;

	free(stream);
	return rc;
}

/*returneaza file descriptorul*/
int so_fileno(SO_FILE *stream)
{
	return stream->fileno;
}

/*scrie in fisier ce a ramas in buffer daca ultima operatie a fost write*/
/*returneaza SO_EOF daca esueaza*/
int so_fflush(SO_FILE *stream)
{
	int rc;

	if (stream->last_op == 1) {
		rc = write(stream->fileno, stream->buf, stream->buf_len);
		if (rc <= 0) {
			stream->err = 1;
			return SO_EOF;
		}

		if (rc != stream->buf_len) {
			stream->buf_point = rc;
			while (stream->buf_point < stream->buf_len) {
				rc = write(stream->fileno, &stream->buf[stream->buf_point],
					stream->buf_len - stream->buf_point);
				if (rc <= 0) {
					stream->err = 1;
					return SO_EOF;
				}

				stream->buf_point += rc;
			}
		}
		memset(stream->buf, '\0', BUF_SIZE);
		stream->buf_point = 0;
		stream->last_op = -1;
		stream->buf_len = 0;
	}

	return 0;
}

/*seteaza cursorul in fisier la pozitia specificata prin offset si whence*/
/*returneaza -1 daca esueaza, altfel 0*/
int so_fseek(SO_FILE *stream, long offset, int whence)
{
	int rc;

	if (stream->last_op == 0) {
		memset(stream->buf, '\0', BUF_SIZE);
		stream->buf_point = 0;
		stream->last_op = -1;
		stream->buf_len = 0;
	} else if (stream->last_op == 1)
		so_fflush(stream);

	rc = lseek(stream->fileno, offset, whence);
	if (rc < 0) {
		stream->err = 1;
		return -1;
	}

	stream->cursor = rc;

	return 0;
}

/*returneaza pozitia cursorului*/
long so_ftell(SO_FILE *stream)
{
	return stream->cursor;
}

/*citeste numarul de elemente de dimensiune size folosing fgetc si returneaza*/
/*numarul de elemente citite*/
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int rc;
	size_t elem = 0;
	size_t k = 0;
	int chr;

	while (elem < nmemb) {
		k = 0;
		while (k < size) {
			chr = so_fgetc(stream);
			if (chr == SO_EOF) {
				stream->err = 1;
				return elem;
			}

			memcpy(ptr, &chr, 1);
			ptr++;
			k++;
		}
		elem++;
	}
	return nmemb;
}

/*scrie numarul de elemente de dimensiune size folosind fputc si returneaza*/
/*numarul de elemente scrise sau SO_EOF daca a fost eroare*/
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int rc;
	size_t elem = 0;
	size_t k = 0;
	unsigned char chr;

	while (elem < nmemb) {
		k = 0;
		while (k < size) {
			memcpy(&chr, ptr, 1);
			chr = so_fputc(chr, stream);
			if (chr == -1) {
				stream->err = 1;
				return SO_EOF;
			} else if (chr == SO_EOF) {
				return elem;
			}
			ptr++;
			k++;
		}
		elem++;
	}
	return nmemb;
}

/*intoarce un caracter, daca buffer-ul este gol/plin citeste din fisier*/
/*pana umple buffer-ul, returneaza -1 daca esueaza*/
int so_fgetc(SO_FILE *stream)
{
	int rc;
	unsigned char chr;

	rc = so_fflush(stream);
	if (rc != 0)
		return SO_EOF;

	if (stream->last_op == -1 || stream->buf_point == stream->buf_len) {
		rc = read(stream->fileno, stream->buf, BUF_SIZE);
		if (rc < 0) {
			stream->err = 1;
			return -1;
		} else if (rc == 0) {
			stream->eof = 1;
			return SO_EOF;
		}

		stream->buf_len = rc;
		stream->buf_point = 0;
	}

	stream->last_op = 0;
	chr = stream->buf[stream->buf_point];
	stream->buf_point += 1;
	stream->cursor += 1;
	return (int) chr;
}

/*intoarce caracterul scris, daca buffer-ul este plin scrie in fisier*/
/*pana goleste buffer-ul, returneaza -1 daca esueaza*/
int so_fputc(int c, SO_FILE *stream)
{
	int rc;

	if (stream->last_op == 0) {
		memset(stream->buf, '\0', BUF_SIZE);
		stream->buf_len = 0;
		stream->buf_point = 0;
		stream->last_op = -1;
	}

	if (stream->buf_point == BUF_SIZE) {
		rc = write(stream->fileno, stream->buf, BUF_SIZE);
		if (rc <= 0) {
			stream->err = 1;
			return SO_EOF;
		}
		if (rc != BUF_SIZE) {
			stream->buf_point = rc;
			while (stream->buf_point < BUF_SIZE) {
				rc = write(stream->fileno, &stream->buf[stream->buf_point],
					BUF_SIZE - stream->buf_point);
				if (rc <= 0) {
					stream->err = 1;
					return SO_EOF;
				}
				stream->buf_point += rc;
			}
		}

		memset(stream->buf, '\0', BUF_SIZE);
		stream->buf_point = 0;
		stream->buf_len = 0;
	}
	stream->last_op = 1;
	stream->buf[stream->buf_point] = c;
	stream->buf_point += 1;
	stream->buf_len += 1;
	stream->cursor += 1;
	return c;
}

/*returneaza daca s-a ajuns la finalul fisierului, 0-nu, 1-da*/
int so_feof(SO_FILE *stream)
{
	return stream->eof;
}

/*returneaza daca a fost setata o eroare, 0-nu, 1-da*/
int so_ferror(SO_FILE *stream)
{
	return stream->err;
}

/*creeaza un pipe intre procesul parinte si cel copil in functie de type*/
/*daca se afla in procesul parinte returneaza SO_FILE, daca se afla in*/
/*procesul copil executa comanda si apoi returneaza NULL*/
SO_FILE *so_popen(const char *command, const char *type)
{
	int rc;
	int pipefds[2];
	SO_FILE *pipefp;
	pid_t pid;

	rc = pipe(pipefds);
	if (rc != 0)
		return NULL;

	pipefp = (SO_FILE *) calloc(1, sizeof(SO_FILE));
	if (pipefp == NULL)
		return NULL;
	pipefp->buf_point = 0;
	pipefp->buf_len = 0;
	pipefp->last_op = -1;
	pipefp->cursor = 0;
	pipefp->err = 0;
	pipefp->eof = 0;
	memset(pipefp->buf, '\0', BUF_SIZE);

	pid = fork();
	pipefp->pid = pid;
	switch (pid) {
	case -1:
		close(pipefds[0]);
		close(pipefds[1]);
		free(pipefp);
		return NULL;
	case 0:
		if (strcmp(type, "r") == 0) {
			close(pipefds[0]);
			dup2(pipefds[1], STDOUT_FILENO);
		} else if (strcmp(type, "w") == 0) {
			close(pipefds[1]);
			dup2(pipefds[0], STDIN_FILENO);
		}
		rc = execl("/bin/sh", "/bin/sh", "-c", command, NULL);
		if (rc == -1)
			return NULL;

		if (strcmp(type, "r") == 0)
			close(pipefds[1]);
		else if (strcmp(type, "w") == 0)
			close(pipefds[0]);

		return NULL;

	default:
		if (strcmp(type, "r") == 0) {
			close(pipefds[1]);
			pipefp->fileno = pipefds[0];
		} else if (strcmp(type, "w") == 0) {
			close(pipefds[0]);
			pipefp->fileno = pipefds[1];
		}

		return pipefp;
	}
}

/*inchide file_descriptorul pipe-ului*/
/*returneaza 0 daca reuseste, -1 daca esueaza*/
int so_pclose(SO_FILE *stream)
{
	int rc;

	rc = so_fflush(stream);
	if (rc != 0)
		stream->err = SO_EOF;

	rc = close(stream->fileno);
	if (rc != 0)
		return -1;

	rc = waitpid(stream->pid, NULL, 0);
	if (rc < 0) {
		free(stream);
		return -1;
	}
	free(stream);
	return 0;
}
