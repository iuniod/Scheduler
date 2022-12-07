/**
  * SO, 2016
  * Lab #4
  *
  * Task #5, lin
  *
  * Avoid creating zombies using signals
  */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

#define TIMEOUT		20


/*
 * configure signal handler
 */
static void set_signals(void)
{
	struct sigaction sa;
	int rc;

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = SIG_IGN;
	/* TODO - ignore SIGCHLD */
	rc = sigaction(SIGCHLD, &sa, NULL);
	DIE(rc == -1, "sigaction");

}

int main(void)
{
	pid_t pid;

	/* TODO - create child process without waiting */
	pid = fork();
	if (pid == 0)
		exit(1);
	/* TODO - sleep */
	sleep(TIMEOUT);

	set_signals();
	return 0;
}
