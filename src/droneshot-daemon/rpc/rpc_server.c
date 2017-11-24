#include "rpc_server.h"

#include <droneshot-api/socket.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

static int server_fd = -1;

static bool setup_server_socket(int fd)
{
	struct sockaddr_un addr;

	// set socket name.
	memset(&addr, 0, sizeof(addr));

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, DRONESHOT_SOCKET_NAME);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to bind server socket to %s: %s.\n", DRONESHOT_SOCKET_NAME, reason);
		return false;
	}

	// change socket permission to world writable.
	if (chmod(DRONESHOT_SOCKET_NAME, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to allow world writable to %s: %s.\n", DRONESHOT_SOCKET_NAME, reason);

		if (unlink(DRONESHOT_SOCKET_NAME) == -1) {
			const char *reason = strerror(errno);
			fprintf(stderr, "Failed to remove %s: %s.\n", DRONESHOT_SOCKET_NAME, reason);
		}

		return false;
	}

	return true;
}

bool rpc_server_start()
{
	int fd;

	// check if we already running.
	if (server_fd != -1) {
		fprintf(stderr, "Failed to start RPC server: Server is already running.\n");
		return false;
	}

	// create socket.
	fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (fd == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to create server socket: %s.\n", reason);
		return false;
	}

	if (!setup_server_socket(fd)) {
		if (close(fd) == -1) {
			const char *reason = strerror(errno);
			fprintf(stderr, "Failed to clean up file descriptor #%d: %s.\n", fd, reason);
		}
		return false;
	}

	server_fd = fd;

	return true;
}

void rpc_server_stop()
{
	if (server_fd == -1) {
		fprintf(stderr, "Failed to stop RPC server: Server is not running.\n");
		return;
	}

	// close socket
	if (close(server_fd) == -1) {
		// we still can remove socket from file system, so don't return when we
		// fail to close socket.
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to close server socket: %s.\n", reason);
	}

	server_fd = -1;

	// remove socket
	if (unlink(DRONESHOT_SOCKET_NAME) == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to remove %s: %s.\n", DRONESHOT_SOCKET_NAME, reason);
	}
}
