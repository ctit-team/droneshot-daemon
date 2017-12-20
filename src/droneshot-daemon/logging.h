#pragma once

#include <stdbool.h>

enum logging_category {
	logging_none		= 0x00,
	logging_hardware	= 0x01
};

bool logging_init(void);
void logging_term(void);

void logging_write(enum logging_category cat, const char *format, ...);
