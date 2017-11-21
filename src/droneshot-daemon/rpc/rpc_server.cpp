#include "rpc_server.h"
#include "../exception/formatter.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <system_error>

#include <cerrno>

#include <unistd.h>
#include <sys/socket.h>

namespace rpc {

rpc_server::rpc_server() :
	fd(-1)
{
}

rpc_server::rpc_server(rpc_server && src) :
	fd(src.fd)
{
	src.fd = -1;
}

rpc_server::~rpc_server()
{
	if (is_running()) {
		try {
			stop();
		} catch (std::exception & e) {
			std::cerr << exception::to_string(e) << std::endl;
		}
	}
}

rpc_server & rpc_server::operator=(rpc_server && src)
{
	fd = src.fd;
	src.fd = -1;

	return *this;
}

void rpc_server::start()
{
	if (is_running()) {
		throw std::logic_error("Server is already running.");
	}

	try {
		create_socket();
	} catch (...) {
		std::throw_with_nested(std::runtime_error("Failed to create server socket."));
	}
}

void rpc_server::stop()
{
	if (!is_running()) {
		throw std::logic_error("Server is not running.");
	}

	try {
		close_socket();
	} catch (...) {
		std::stringstream s;
		s << "Failed to close RPC server socket #" << fd << ".";
		std::throw_with_nested(std::runtime_error(s.str()));
	}
}

void rpc_server::create_socket()
{
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
		throw std::system_error(errno, std::system_category());
	}
}

void rpc_server::close_socket()
{
	if (close(fd) == -1) {
		throw std::system_error(errno, std::system_category());
	}

	fd = -1;
}

}
