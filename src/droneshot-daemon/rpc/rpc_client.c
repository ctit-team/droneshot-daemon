#include "rpc_client.h"
#include "../uv.h"

#include <uv.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MESSAGE 256

struct client {
	uv_pipe_t h; // must be the first member.
};

static void cleanup(uv_handle_t *handle);

static const struct uv_type type = {
	.cleanup = cleanup
};

static void cleanup(uv_handle_t *handle)
{
	struct client *c = (struct client *)handle;
	free(c);
}

static void alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = malloc(MAX_MESSAGE);
	buf->len = MAX_MESSAGE;
}

static void read_data(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
	// check error.
	if (nread < 0) {
		if (nread != UV_EOF) {
			fprintf(stderr, "Failed to read data from RPC client: %s.\n", uv_strerror(nread));
		}
		uv_close((uv_handle_t *)stream, cleanup);
		return;
	}

	// TODO: parse message.

	// clean up.
	free(buf->base);
}

bool rpc_client_start(const uv_pipe_t *h)
{
	struct client *c;
	int err;

	// setup data.
	c = malloc(sizeof(c[0]));
	if (!c) {
		fprintf(stderr, "Insufficient memory for a new RPC client.\n");
		return false;
	}

	memset(c, 0, sizeof(c[0]));
	memcpy(&c->h, h, sizeof(c->h));

	c->h.data = (void *)&type;

	// start receiving data.
	err = uv_read_start((uv_stream_t *)&c->h, alloc_buf, read_data);
	if (err < 0) {
		fprintf(stderr, "Failed to start receiving data from RPC client: %s.\n", uv_strerror(err));
		free(c);
		return false;
	}

	return true;
}
