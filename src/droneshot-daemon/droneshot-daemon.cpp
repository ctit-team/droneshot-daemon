#include "exception/formatter.h"
#include "rpc/rpc_server.h"

#include <cstdlib>
#include <exception>
#include <iostream>

static rpc::rpc_server start_rpc_server()
{
	rpc::rpc_server s;

	try {
		s.start();
	} catch (...) {
		std::throw_with_nested(std::runtime_error("Failed to start RPC server."));
	}

	return s;
}

static void run()
{
	rpc::rpc_server serv;

	try {
		serv = start_rpc_server();
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
