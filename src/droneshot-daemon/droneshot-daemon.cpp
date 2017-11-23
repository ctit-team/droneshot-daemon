#include "exception/formatter.h"
#include "rpc/rpc_server.h"

#include <cstdlib>
#include <exception>
#include <iostream>

static void run()
{
	rpc::rpc_server serv;

	try {
		serv.start();
	} catch (...) {
		std::throw_with_nested(std::runtime_error("Failed to start RPC server."));
	}
}

int main(int argc, char *argv[])
{
	try {
		run();
	} catch (std::exception & e) {
		std::cerr << exception::to_string(e) << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
