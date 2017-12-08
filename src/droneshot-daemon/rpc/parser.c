#include "client.h"
#include "messages.h"
#include "parser.h"
#include "../hardware/transmitter_collection.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

struct parser {
	size_t min_payload;
	bool (*parser) (struct rpc_client *c, uint8_t method, const void *req);
};

static bool parse_set_transmitter_util(struct rpc_client *c, uint8_t method, const void *req);

static const struct parser parsers[] = {
	[RPC_SET_TRANSMITTER_UTIL] = {
		.min_payload = sizeof(struct rpc_transmitter_util),
		.parser = parse_set_transmitter_util
	}
};

static bool success(struct rpc_client *c, uint8_t method, const void *payload, size_t len)
{
	return rpc_client_send(c, method, RPC_SUCCESS, payload, len);
}

static bool internal_error(struct rpc_client *c, uint8_t method)
{
	return rpc_client_send(c, method, RPC_INTERNAL_ERROR, NULL, 0);
}

static bool invalid_argument(struct rpc_client *c, uint8_t method, const char *name)
{
	return rpc_client_send(c, method, RPC_INVALID_ARGUMENT, name, strlen(name) + 1);
}

static bool parse_set_transmitter_util(struct rpc_client *c, uint8_t method, const void *req)
{
	const struct rpc_transmitter_util *r = req;
	struct transmitter *t;

	// get transmitter to adjust.
	t = transmitter_collection_get(r->transmitter_id);
	if (!t) {
		return invalid_argument(c, method, "transmitter_id");
	}

	// adjust transmitter power.
	switch (transmitter_utilization_set(t, r->utilization)) {
	case utilization_success:
		break;
	case utilization_invalid:
		return invalid_argument(c, method, "utilization");
	default:
		fprintf(stderr, "transmitter_utilization_set() return unexpected result.\n");
		return internal_error(c, method);
	}

	return success(c, method, NULL, 0);
}

bool rpc_parser_parse(struct rpc_client *c, const void *data, size_t len)
{
	const struct rpc_message *m = (const struct rpc_message *)data;
	const struct parser *p;

	// check message.
	if (len < sizeof(m[0]) || m->version != RPC_VERSION || m->type != RPC_INVOKE) {
		return false;
	}

	if (m->method_id > RPC_LAST_METHOD || !(p = &parsers[m->method_id])->parser) {
		return false;
	}

	if ((len - sizeof(m[0])) < p->min_payload) {
		return false;
	}

	// parse.
	return p->parser(c, m->method_id, m->payload);
}
