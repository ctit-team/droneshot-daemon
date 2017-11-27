#include "rpc_server.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#define SOCKET_NAME "/tmp/droneshot"

static bool listen_connections(int fd)
{
	// change socket permission to world writable.
	if (chmod(SOCKET_NAME, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to allow world writable to %s: %s.\n", SOCKET_NAME, reason);
		return false;
	}

	// listen for connections
	if (listen(fd, 5) == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to listen for connection: %s.\n", reason);
		return false;
	}

	return true;
}

static bool start_server(int fd)
{
	struct sockaddr_un addr;

	// set socket name.
	memset(&addr, 0, sizeof(addr));

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCKET_NAME);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to bind server socket to %s: %s.\n", SOCKET_NAME, reason);
		return false;
	}

	if (!listen_connections(fd)) {
		if (unlink(SOCKET_NAME) == -1) {
			const char *reason = strerror(errno);
			fprintf(stderr, "Failed to remove %s: %s.\n", SOCKET_NAME, reason);
		}
		return false;
	}

	return true;
}

int rpc_server_start(void)
{
	int fd;

	// create socket.
	fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (fd == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to create server socket: %s.\n", reason);
		return -1;
	}

	if (!start_server(fd)) {
		if (close(fd) == -1) {
			const char *reason = strerror(errno);
			fprintf(stderr, "Failed to clean up file descriptor #%d: %s.\n", fd, reason);
		}
		return -1;
	}

	return fd;
}

int rpc_server_accept(int fd)
{
	fd = accept(fd, NULL, 0);
	if (fd == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to accept a connection from RPC client: %s.\n", reason);
		return -1;
	}

	return fd;
}

void rpc_server_stop(int fd)
{
	// close socket
	if (close(fd) == -1) {
		// we still can remove socket from file system, so don't return when we
		// fail to close socket.
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to close server socket: %s.\n", reason);
	}

	// remove socket
	if (unlink(SOCKET_NAME) == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to remove %s: %s.\n", SOCKET_NAME, reason);
	}
}
