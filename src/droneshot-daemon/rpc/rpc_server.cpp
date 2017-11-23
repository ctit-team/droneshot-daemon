#include "rpc_server.h"
#include "../exception/formatter.h"

#include <droneshot-api/socket.h>

#include <cerrno>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <utility>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

namespace rpc {

rpc_server::rpc_server()
{
}

rpc_server::rpc_server(rpc_server && src) :
	fd(std::move(src.fd))
{
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

bool rpc_server::is_running() const
{
	return fd || access(droneshot::api::socket_path.c_str(), F_OK) == 0;
}

void rpc_server::start()
{
	// check if we already running.
	if (is_running()) {
		throw std::logic_error("Server is already running");
	}

	// create socket.
	droneshot::os::file_descriptor fd;

	try {
		auto res = socket(AF_UNIX, SOCK_STREAM, 0);
		if (res == -1) {
			throw std::system_error(errno, std::system_category());
		}
		fd.reset(res);
	} catch (...) {
		std::throw_with_nested(std::runtime_error("Failed to create server socket"));
	}

	// bind local address
	try {
		sockaddr_un addr = { 0 };

		addr.sun_family = AF_UNIX;
		std::strcpy(addr.sun_path, droneshot::api::socket_path.c_str());

		if (bind(fd.get(), reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1) {
			throw std::system_error(errno, std::system_category());
		}
	} catch (...) {
		std::throw_with_nested(std::runtime_error("Failed to bind socket to local address"));
	}

	// change socket permission to world writable.
	try {
		if (chmod(droneshot::api::socket_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
			throw std::system_error(errno, std::system_category());
		}
	} catch (...) {
		if (unlink(droneshot::api::socket_path.c_str()) == -1) {
			auto err = errno;
			std::cerr << "Failed to remove " << droneshot::api::socket_path << ": "
					  << std::system_category().message(err) << std::endl;
		}

		std::throw_with_nested(std::runtime_error("Failed to allow world writable to server socket"));
	}

	this->fd = std::move(fd);
}

void rpc_server::stop()
{
	if (!is_running()) {
		throw std::logic_error("Server is not running.");
	}

	// close socket
	if (fd) {
		try {
			fd.close();
		} catch (...) {
			std::throw_with_nested(std::runtime_error("Failed to close server socket"));
		}
	}

	// remove socket
	if (access(droneshot::api::socket_path.c_str(), F_OK) == 0) {
		try {
			if (unlink(droneshot::api::socket_path.c_str()) == -1) {
				throw std::system_error(errno, std::system_category());
			}
		} catch (...) {
			std::throw_with_nested(std::runtime_error("Failed to remove server socket"));
		}
	}
}

}
