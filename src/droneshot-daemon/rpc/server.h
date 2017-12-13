#pragma once

#include <uv.h>

#include <stdbool.h>

struct rpc_server;

struct rpc_server * rpc_server_new(uv_loop_t *uv, const char *sock);
void rpc_server_free(struct rpc_server *s);

bool rpc_server_start(struct rpc_server *s);
