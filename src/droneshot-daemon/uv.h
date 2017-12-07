#pragma once

#include <uv.h>

struct uv_type {
	uv_close_cb cleanup;
};
