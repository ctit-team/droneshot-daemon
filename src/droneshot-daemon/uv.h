#pragma once

#include <uv.h>

typedef void (*uv_destroy_t) (uv_handle_t *h);

struct uv_type {
	uv_destroy_t destroy;
};
