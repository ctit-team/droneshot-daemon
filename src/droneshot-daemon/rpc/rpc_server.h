#pragma once

#include <stdbool.h>

int rpc_server_start(void);
int rpc_server_accept(int fd);
void rpc_server_stop(int fd);
