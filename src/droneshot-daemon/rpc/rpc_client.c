#include "rpc_client.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

bool rpc_client_parse(const void *data, size_t len)
{
	// TODO: parse client data.
}

void rpc_client_close(int fd)
{
	if (close(fd) == -1) {
		const char *reason = strerror(errno);
		fprintf(stderr, "Failed to close a connection to RPC client: %s.\n", reason);
	}
}
