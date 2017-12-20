#pragma once

#include <stdbool.h>
#include <stddef.h>

enum logging_category {
	logging_none		= 0x00,
	logging_hardware	= 0x01
};

bool logging_init(void);
void logging_term(void);

char * logging_printable_raw(const void *data, size_t len);

void logging_write(enum logging_category cat, const char *format, ...);
