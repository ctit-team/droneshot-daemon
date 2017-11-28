#include "hardware/interface.h"
#include "hardware/transmitter_collection.h"
#include "rpc/rpc_client.h"
#include "rpc/rpc_server.h"

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>

static bool terminating;

static void interrupt_handler(int sig)
{
	terminating = true;
}

static bool setup_signal_handlers(void)
{
	struct sigaction sigact;

	memset(&sigact, 0, sizeof(sigact));
	sigact.sa_handler = interrupt_handler;
	sigact.sa_flags = SA_RESETHAND; // we want one-shot only.

	if (sigaction(SIGINT, &sigact, NULL) == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to install interrupt handler: %s.\n", reason);
		return false;
	}

	return true;
}

static bool run_rpc_server(int server_fd)
{
	struct pollfd fds[2];
	int ui_fd;

	if (!setup_signal_handlers()) {
		return false;
	}

	// watch server file descriptor.
	memset(fds, 0, sizeof(fds));

	fds[0].fd = server_fd;
	fds[0].events = POLLIN;

	// wait for events.
	ui_fd = -1;

	for (; !terminating;) {
		int nev, i;

		nev = poll(fds, ui_fd != -1 ? 2 : 1, -1);
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

			if (fds[i].fd == server_fd) {
				int fd = rpc_server_accept(server_fd);

				if (fd != -1) {
					if (ui_fd != -1) {
						printf("Maximum connections is already reached, rejecting connection.\n");
						rpc_client_close(fd);
					} else {
						memset(&fds[1], 0, sizeof(fds[1]));
						fds[1].fd = fd;
						fds[1].events = POLLIN;
						ui_fd = fd;
					}
				}
			} else if (fds[i].fd == ui_fd) {
				char data[256];
				ssize_t n;
				const char *reason;

				n = recv(ui_fd, data, sizeof(data), 0);

				switch (n) {
				case -1:
					reason = strerror(errno);
					fprintf(stderr, "Failed to read data from RPC client on file descriptor #%d: %s.\n", ui_fd, reason);
					break;
				case 0:
					rpc_client_close(ui_fd);
					memset(&fds[1], 0, sizeof(fds[1]));
					ui_fd = -1;
					break;
				default:
					rpc_client_parse(data, n);
				}
			} else {
				fprintf(stderr, "Events occurred on an unknown file descriptor #%d.\n", fds[i].fd);
			}

			nev--;
		}
	}

	// clean up.
	if (ui_fd != -1 && close(ui_fd) == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to close a connection to RPC client: %s.\n", reason);
	}

	return true;
}

static bool start_rpc_server(void)
{
	bool res;
	int server_fd;

	server_fd = rpc_server_start();
	if (server_fd == -1) {
		return false;
	}

	res = run_rpc_server(server_fd);
	rpc_server_stop(server_fd);

	return res;
}

static bool init_hardware(void)
{
	if (hardware_interface_init()) {
		if (transmitter_collection_init()) {
			return true;
		}

		hardware_interface_close();
	}

	return false;
}

static void close_hardware()
{
	transmitter_collection_close();
	hardware_interface_close();
}

int main(int argc, char *argv[])
{
	int res = EXIT_FAILURE;

	if (init_hardware()) {
		if (start_rpc_server()) {
			res = EXIT_SUCCESS;
		}

		close_hardware();
	}

	return res;
}
