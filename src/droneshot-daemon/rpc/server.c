#include "client.h"
#include "server.h"
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
#include <sys/types.h>
#include <sys/un.h>

struct rpc_server {
	uv_pipe_t h; // must be the first member.
	char *sock;
};

static void cleanup(uv_handle_t *handle);

static const struct uv_type type = {
	.cleanup = cleanup
};

static void cleanup(uv_handle_t *handle)
{
	struct rpc_server *s = (struct rpc_server *)handle;

	if (unlink(s->sock) == -1 && errno != ENOENT) {
		const char *r = strerror(errno);
		fprintf(stderr, "Failed to remove %s: %s.\n", s->sock, r);
	}

	free(s->sock);
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
	struct rpc_client *client;

	// check waiting status.
	if (status < 0) {
		fprintf(stderr, "Failed to waiting connections from RPC client: %s.\n", uv_strerror(status));
		return;
	}

	// accept incoming connection.
	client = rpc_client_new(server->loop);
	if (!client) {
		return;
	}

	if (!rpc_client_start(client, server)) {
		rpc_client_free(client);
		return;
	}
}

static bool start_server(struct rpc_server *s)
{
	int err;

	// change socket permission to world writable.
	if (chmod(s->sock, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
		const char *r = strerror(errno);
		fprintf(stderr, "Failed to allow world writable to %s: %s.\n", s->sock, r);
		return false;
	}

	// listen for connections
	err = uv_listen((uv_stream_t *)s, 5, accept_connection);
	if (err < 0) {
		fprintf(stderr, "Failed to listen for connection: %s.\n", uv_strerror(err));
		return false;
	}

	return true;
}

struct rpc_server * rpc_server_new(uv_loop_t *uv, const char *sock)
{
	struct rpc_server *s;
	int err;

	// allocate data.
	s = malloc(sizeof(s[0]));
	if (!s) {
		fprintf(stderr, "Insufficient memory for server data.\n");
		return NULL;
	}

	memset(s, 0, sizeof(s[0]));

	// initialize data.
	s->sock = strdup(sock);
	if (!s->sock) {
		fprintf(stderr, "Insufficient memory for server socket name.\n");
		free(s);
		return NULL;
	}

	err = uv_pipe_init(uv, (uv_pipe_t *)s, true);
	if (err < 0) {
		fprintf(stderr, "Failed to initialize server handle: %s.\n", uv_strerror(err));
		free(s->sock);
		free(s);
		return NULL;
	}

	s->h.data = (void *)&type;

	return s;
}

void rpc_server_free(struct rpc_server *s)
{
	uv_close((uv_handle_t *)s, cleanup);
}

bool rpc_server_start(struct rpc_server *s)
{
	int fd, err;

	// create socket.
	fd = create_socket(s->sock);
	if (fd == -1) {
		return false;
	}

	// initialize handle.
	err = uv_pipe_open((uv_pipe_t *)s, fd);
	if (err < 0) {
		fprintf(stderr, "Failed to associate server descriptor to handle: %s.\n", uv_strerror(err));
		close_socket(fd);
		return false;
	}

	// start server.
	return start_server(s);
}
