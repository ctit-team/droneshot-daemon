#include "argument.h"
#include "logging.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERS_MAX 8
#define BUFFER_SIZE 128

static const char hex_chars[] = "0123456789abcdef";

static char buffers[BUFFERS_MAX][BUFFER_SIZE];
static size_t next_buf;

static char * getbuf()
{
	size_t i = next_buf++;

	if (next_buf >= BUFFERS_MAX) {
		next_buf = 0;
	}

	return buffers[i];
}

bool logging_init(void)
{
	return true;
}

void logging_term(void)
{
	memset(buffers, 0, sizeof(buffers));
	next_buf = 0;
}

char * logging_printable_raw(const void *data, size_t len)
{
	char *buf;
	size_t i, ch;

	buf = getbuf();
	buf[0] = 0;

	// each iteration guarantee there is at least 7 bytes remaining in the
	// buffer.
	for (i = 0, ch = 0; i < len; i++) {
		uint8_t byte = ((const uint8_t *)data)[i];

		if (i) {
			buf[ch++] = ':';
		}

		buf[ch++] = hex_chars[byte & 0x0F];
		buf[ch++] = hex_chars[byte >> 4];
		buf[ch] = 0;

		// stop if the remaining buffer for next iteration is less than 7 bytes.
		if ((BUFFER_SIZE - ch - 1) < 7) {
			buf[ch++] = '.';
			buf[ch++] = '.';
			buf[ch++] = '.';
			buf[ch] = 0;
			break;
		}
	}

	return buf;
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
