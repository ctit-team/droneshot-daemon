#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

struct setting_parsing {
	size_t line_num;
	const char *file;
	const char *name;
	const char *value;
	void *ctx;
};

typedef bool (*setting_parser_t) (const struct setting_parsing *data);

bool setting_parse(const char *file, setting_parser_t parser, void *context);

bool setting_parsing_error(const struct setting_parsing *data, const char *reason, ...);
bool setting_parsing_unknown(const struct setting_parsing *data);

bool setting_parse_pin(const struct setting_parsing *data, uint8_t *res);
int setting_parse_prefix(const char *name, const char *prefix, const char **suffix);
