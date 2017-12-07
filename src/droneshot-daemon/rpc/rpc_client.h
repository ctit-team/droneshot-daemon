#pragma once

#include <uv.h>

#include <stdbool.h>

bool rpc_client_start(const uv_pipe_t *h);
