#include "rpc_client.h"
#include "rpc_server.h"
#include "../uv.h"

#include <uv.h>

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#define SOCKET_NAME "/tmp/droneshot"

struct rpc_server {
	uv_pipe_t h; // must be the first member.
};

static void cleanup(uv_handle_t *handle);

static const struct uv_type type = {
	.cleanup = cleanup
};

static void cleanup(uv_handle_t *handle)
{
	struct rpc_server *s = (struct rpc_server *)handle;

	if (unlink(SOCKET_NAME) == -1) {
		const char *r = strerror(errno);
		fprintf(stderr, "Failed to remove %s: %s.\n", SOCKET_NAME, r);
	}

	free(s);
}

static void close_socket(int fd)
{
	if (close(fd) == -1) {
		const char *r = strerror(errno);
		fprintf(stderr, "Failed to clean up file descriptor #%d: %s.\n", fd, r);
	}
}

static bool set_socket_name(int fd, const char *name)
{
	struct sockaddr_un addr;

	// set socket name.
	memset(&addr, 0, sizeof(addr));

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, name);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		const char *r = strerror(errno);
		fprintf(stderr, "Failed to bind server socket to %s: %s.\n", name, r);
		return false;
	}

	return true;
}

static int create_socket(const char *name)
{
	int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (fd == -1) {
		fprintf(stderr, "Failed to create server socket: %s.\n", strerror(errno));
		return -1;
	}

	if (!set_socket_name(fd, name)) {
		close_socket(fd);
		return -1;
	}

	return fd;
}

static void accept_connection(uv_stream_t *server, int status)
{
	uv_pipe_t client;

	// check waiting status.
	if (status < 0) {
		fprintf(stderr, "Failed to waiting connections from RPC client: %s.\n", uv_strerror(status));
		return;
	}

	// accept incoming connection.
	status = uv_pipe_init(server->loop, &client, true);
	if (status < 0) {
		fprintf(stderr, "Failed to initialize handle for RPC client: %s.\n", uv_strerror(status));
		return;
	}

	status = uv_accept(server, (uv_stream_t *)&client);
	if (status < 0) {
		fprintf(stderr, "Failed to accept a connection from RPC client: %s.\n", uv_strerror(status));
		uv_close((uv_handle_t *)&client, NULL);
		return;
	}

	// start client.
	if (!rpc_client_start(&client)) {
		uv_close((uv_handle_t *)&client, NULL);
	}
}

static bool start_server(uv_pipe_t *h)
{
	int err;

	// change socket permission to world writable.
	err = uv_pipe_chmod(h, UV_WRITABLE | UV_READABLE);
	if (err < 0) {
		fprintf(stderr, "Failed to allow world writable to server socket: %s.\n", uv_strerror(err));
		return false;
	}

	// listen for connections
	err = uv_listen((uv_stream_t *)h, 5, accept_connection);
	if (err < 0) {
		fprintf(stderr, "Failed to listen for connection: %s.\n", uv_strerror(err));
		return false;
	}

	return true;
}

bool rpc_server_start(uv_loop_t *uv)
{
	struct rpc_server *s;
	int fd, err;

	// allocate data.
	s = malloc(sizeof(s[0]));
	if (!s) {
		fprintf(stderr, "Insufficient memory for server data.\n");
		return false;
	}

	memset(s, 0, sizeof(s[0]));

	// create socket.
	fd = create_socket(SOCKET_NAME);
	if (fd == -1) {
		free(s);
		return false;
	}

	// initialize handle.
	err = uv_pipe_init(uv, &s->h, true);
	if (err < 0) {
		fprintf(stderr, "Failed to initialize server handle: %s.\n", uv_strerror(err));
		close_socket(fd);
		cleanup((uv_handle_t *)s);
		return false;
	}

	err = uv_pipe_open(&s->h, fd);
	if (err < 0) {
		fprintf(stderr, "Failed to associate server descriptor to handle: %s.\n", uv_strerror(err));
		close_socket(fd);
		uv_close((uv_handle_t *)&s->h, cleanup);
		return false;
	}

	s->h.data = (void *)&type;

	// start server.
	if (!start_server(&s->h)) {
		uv_close((uv_handle_t *)&s->h, cleanup);
		return false;
	}

	return true;
}
