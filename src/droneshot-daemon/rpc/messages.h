#pragma once

#include <inttypes.h>

#define RPC_VERSION		0
#define RPC_MAX_MESSAGE 256

#define RPC_INVOKE				0
#define RPC_SUCCESS				1
#define RPC_INVALID_ARGUMENT	2
#define RPC_INTERNAL_ERROR		3

#define RPC_SET_TRANSMITTER_UTIL	0x01
#define RPC_LAST_METHOD				RPC_SET_TRANSMITTER_UTIL

struct rpc_message {
	uint8_t version;
	uint8_t method_id;
	uint8_t type;
	uint8_t reserved; // padding to prevent misaligned access on payload.
	uint8_t payload[];
};

struct rpc_transmitter_util {
	uint8_t transmitter_id;
	uint8_t utilization;
	uint8_t reserved[2];
};
