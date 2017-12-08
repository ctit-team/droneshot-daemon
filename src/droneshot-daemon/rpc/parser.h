#pragma once

#include "rpc_client.h"

#include <stdbool.h>
#include <stddef.h>

bool rpc_parser_parse(struct rpc_client *c, const void *data, size_t len);
