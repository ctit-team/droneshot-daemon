#pragma once

#include <uv.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

struct rpc_client;

struct rpc_client * rpc_client_new(uv_loop_t *uv);
void rpc_client_free(struct rpc_client *c);

bool rpc_client_start(struct rpc_client *c, uv_stream_t *s);
bool rpc_client_send(struct rpc_client *c, uint8_t method, uint8_t type, const void *payload, size_t len);
