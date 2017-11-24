#include "rpc/rpc_server.h"

#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (!rpc_server_start()) {
		return EXIT_FAILURE;
	}

	rpc_server_stop();

	return EXIT_SUCCESS;
}
