#include "client.h"
#include "messages.h"
#include "parser.h"
#include "../uv.h"

#include <uv.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct rpc_client {
	uv_pipe_t h; // must be the first member.
};

struct write_request {
	uv_write_t req; // must be the first member.
	uv_buf_t buf;
};

static void cleanup(uv_handle_t *handle);

static const struct uv_type type = {
	.cleanup = cleanup
};

static void cleanup(uv_handle_t *handle)
{
	struct rpc_client *c = (struct rpc_client *)handle;
	free(c);
}

static void write_completed(uv_write_t *req, int status)
{
	struct write_request *r = (struct write_request *)req;

	if (status < 0) {
		fprintf(stderr, "Failed to send data to client: %s.\n", uv_strerror(status));
	}

	free(r->buf.base);
	free(r);
}

static void alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = malloc(RPC_MAX_MESSAGE);
	buf->len = RPC_MAX_MESSAGE;
}

static void read_data(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
	struct rpc_client *client = (struct rpc_client *)stream;

	// check error.
	if (nread < 0) {
		if (nread != UV_EOF) {
			fprintf(stderr, "Failed to read data from RPC client: %s.\n", uv_strerror(nread));
		}
		uv_close((uv_handle_t *)client, cleanup);
		return;
	}

	if (!nread) {
		goto done;
	}

	// parse message.
	if (!rpc_parser_parse(client, buf->base, nread)) {
		uv_close((uv_handle_t *)client, cleanup);
		goto done;
	}

	// clean up.
	done:
	free(buf->base);
}

struct rpc_client * rpc_client_new(uv_loop_t *uv)
{
	struct rpc_client *c;
	int err;

	// allocate data.
	c = malloc(sizeof(c[0]));
	if (!c) {
		fprintf(stderr, "Insufficient memory for a new RPC client.\n");
		return NULL;
	}

	memset(c, 0, sizeof(c[0]));

	// setup data.
	err = uv_pipe_init(uv, &c->h, true);
	if (err < 0) {
		fprintf(stderr, "Failed to initialize handle for RPC client: %s.\n", uv_strerror(err));
		free(c);
		return NULL;
	}

	c->h.data = (void *)&type;

	return c;
}

void rpc_client_free(struct rpc_client *c)
{
	uv_close((uv_handle_t *)c, cleanup);
}

bool rpc_client_start(struct rpc_client *c, uv_stream_t *s)
{
	int err;

	// accept incoming connection.
	err = uv_accept(s, (uv_stream_t *)c);
	if (err < 0) {
		fprintf(stderr, "Failed to accept a connection from RPC client: %s.\n", uv_strerror(err));
		return false;
	}

	// start receiving data.
	err = uv_read_start((uv_stream_t *)c, alloc_buf, read_data);
	if (err < 0) {
		fprintf(stderr, "Failed to start receiving data from RPC client: %s.\n", uv_strerror(err));
		return false;
	}

	return true;
}

bool rpc_client_send(struct rpc_client *c, uint8_t method, uint8_t type, const void *payload, size_t len)
{
	struct rpc_message *msg;
	struct write_request *req;
	int err;

	// setup message.
	msg = malloc(sizeof(msg[0]) + len);
	if (!msg) {
		goto out_of_memory;
	}

	memset(msg, 0, sizeof(msg[0]));
	memcpy(msg->payload, payload, len);

	msg->version = RPC_VERSION;
	msg->method_id = method;
	msg->type = type;

	// setup libuv request.
	req = malloc(sizeof(req[0]));
	if (!req) {
		free(msg);
		goto out_of_memory;
	}

	memset(req, 0, sizeof(req[0]));
	req->buf.base = (char *)msg;
	req->buf.len = sizeof(msg[0]) + len;

	// send.
	err = uv_write((uv_write_t *)req, (uv_stream_t *)c, &req->buf, 1, write_completed);
	if (err < 0) {
		fprintf(stderr, "Failed to send data to client: %s.\n", uv_strerror(err));
		free(req);
		free(msg);
		return false;
	}

	return true;

	out_of_memory:
	fprintf(stderr, "Insufficient memory to send message to the client.\n");
	return false;
}
