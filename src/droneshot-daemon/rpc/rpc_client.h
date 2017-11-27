#pragma once

#include <stdbool.h>
#include <stdlib.h>

bool rpc_client_parse(const void *data, size_t len);
void rpc_client_close(int fd);
