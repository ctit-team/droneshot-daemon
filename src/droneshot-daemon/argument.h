#pragma once

#include "logging.h"

#include <stdbool.h>

enum argument_init_result {
	argument_init_ok,
	argument_init_error,
	argument_init_shown_help
};

struct arguments {
	enum logging_category logging_enabled;
};

enum argument_init_result argument_init(int argc, char *argv[]);
void argument_term(void);

const struct arguments * argument_current(void);
