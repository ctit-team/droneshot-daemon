#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

typedef bool (*setting_parser_t) (const char *file, size_t ln, const char *name, const char *value, void *context);

bool setting_parse(const char *file, setting_parser_t parser, void *ctx);

bool setting_parse_pin(const char *file, size_t ln, const char *name, uint8_t *res);
