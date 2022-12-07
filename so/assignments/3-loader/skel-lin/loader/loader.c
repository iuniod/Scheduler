/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "exec_parser.h"

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while (0)

static so_exec_t *exec;
static int fd;

/*handler pentru semnalul SIGSEGV*/
void handler(int signum, siginfo_t *info, void *ucontext)
{
	int i;
	int rc;
	int page_size;
	int page_index;
	int found_seg;
	void *p;
	so_seg_t segment;

	/*seteaza found_seg pe 1 daca pagina se gaseste intr-un segment*/
	found_seg = 0;
	for (i = 0; i < exec->segments_no; i++) {
		segment = exec->segments[i];
		if ((int) info->si_addr <= segment.vaddr + segment.mem_size &&
			(int) info->si_addr >= segment.vaddr) {
			found_seg = 1;
			break;
		}
	}

	/*exit(139) este modul in care ar functiona handler-ul default*
	 *daca nu s-a gasit pagina*/
	if (!found_seg)
		exit(139);

	/*daca se incearca accesarea cu permisiuni insuficiente*/
	if (info->si_code == SEGV_ACCERR)
		exit(139);

	page_size = getpagesize();
	page_index = ((int) info->si_addr - segment.vaddr) / page_size;

	/*se mapeaza adresa respectiva cu persmisiunile indicate*/
	p = mmap((void *) (segment.vaddr + page_index * page_size), page_size, PERM_W,
		MAP_SHARED | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
	DIE(p == MAP_FAILED, "mmap error");

	if (page_index * page_size < segment.file_size) {
		rc = lseek(fd, segment.offset + page_size * page_index, SEEK_SET);
		DIE(rc < 0, "lseek error");

		if (page_size * (page_index + 1) > segment.file_size) {
			rc = read(fd, p, segment.file_size - page_size * page_index);
			DIE(rc < 0, "read error");
		} else {
			rc = read(fd, p, page_size);
			DIE(rc < 0, "read error");
		}
	}

	rc = mprotect(p, page_size, segment.perm);
	DIE(rc == -1, "mprotect error");
}

int so_init_loader(void)
{
	/* TODO: initialize on-demand loader */
	int rc;
	struct sigaction signals;

	memset(&signals, 0, sizeof(struct sigaction));
	signals.sa_flags = SA_SIGINFO;
	signals.sa_sigaction = handler;

	rc = sigaction(SIGSEGV, &signals, NULL);
	DIE(rc == -1, "sigaction error");

	return -1;
}

int so_execute(char *path, char *argv[])
{
	int rc;

	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	fd = open(path, O_RDONLY);
	DIE(fd < 0, "open error");

	so_start_exec(exec, argv);

	rc = close(fd);
	DIE(rc < 0, "close error");

	return -1;
}
