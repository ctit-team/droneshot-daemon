#include "rpc/rpc_server.h"

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <poll.h>

static bool terminating;

static void interrupt_handler(int sig)
{
	terminating = true;
}

static bool run_rpc_server(int server_fd)
{
	struct sigaction sigact;
	struct pollfd fds[2];
	nfds_t nfd;

	// install interrupt handler.
	memset(&sigact, 0, sizeof(sigact));
	sigact.sa_handler = interrupt_handler;
	sigact.sa_flags = SA_RESETHAND; // we want one-shot only.

	if (sigaction(SIGINT, &sigact, NULL) == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to install interrupt handler: %s.\n", reason);
		return false;
	}

	// watch server file descriptor.
	memset(fds, 0, sizeof(fds));
	nfd = 0;

	fds[nfd].fd = server_fd;
	fds[nfd].events = POLLIN;
	nfd++;

	// wait for events.
	for (; !terminating;) {
		int nev, i;

		nev = poll(fds, nfd, -1);
		if (nev == -1) {
			if (errno != EINTR) {
				const char *reason = strerror(errno);
				fprintf(stderr, "Failed to polling events: %s.\n", reason);
				break;
			}
			continue;
		}

		// process events.
		for (i = 0; nev > 0; i++) {
			if (!fds[i].revents) {
				continue;
			}

			// TODO: process events.

			nev--;
		}
	}

	return true;
}

int main(int argc, char *argv[])
{
	int res;
	int server_fd;

	// start RPC server.
	server_fd = rpc_server_start();
	if (server_fd == -1) {
		return EXIT_FAILURE;
	}

	res = run_rpc_server(server_fd) ? EXIT_SUCCESS : EXIT_FAILURE;

	// clean up.
	rpc_server_stop(server_fd);

	return res;
}
