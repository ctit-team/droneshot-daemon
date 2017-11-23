#pragma once

#include <droneshot-lib/os/file_descriptor.h>

namespace rpc {

class rpc_server final {
public:
	rpc_server();
	rpc_server(rpc_server && src);
	rpc_server(rpc_server const &) = delete;
	~rpc_server();

	rpc_server & operator=(rpc_server const &) = delete;

	bool is_running() const;

	void start();
	void stop();

private:
	droneshot::os::file_descriptor fd;
};

}
