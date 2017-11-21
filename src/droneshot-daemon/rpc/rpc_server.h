#pragma once

namespace rpc {

class rpc_server final {
public:
	rpc_server();
	rpc_server(rpc_server && src);
	rpc_server(rpc_server const &) = delete;
	~rpc_server();

	rpc_server & operator=(rpc_server && src);
	rpc_server & operator=(rpc_server const &) = delete;

	bool is_running() const { return fd != -1; }

	void start();
	void stop();

private:
	void create_socket();
	void close_socket();

	int fd;
};

}
