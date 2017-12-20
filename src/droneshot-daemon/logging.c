#include "argument.h"
#include "logging.h"

#include <stdarg.h>
#include <stdio.h>

bool logging_init(void)
{
	return true;
}

void logging_term(void)
{
}

void logging_write(enum logging_category cat, const char *format, ...)
{
	va_list args;

	if (!(argument_current()->logging_enabled & cat)) {
		return;
	}

	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
}
