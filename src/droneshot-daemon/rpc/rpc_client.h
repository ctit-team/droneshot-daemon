#pragma once

#include <uv.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

struct rpc_client;

bool rpc_client_start(const uv_pipe_t *h);
bool rpc_client_send(struct rpc_client *c, uint8_t method, uint8_t type, const void *payload, size_t len);
