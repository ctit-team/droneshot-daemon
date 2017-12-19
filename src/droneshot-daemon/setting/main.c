#include "main.h"
#include "parser.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static struct setting_main current;

static bool parse(const char *file, size_t ln, const char *name, const char *value, void *context)
{
	struct setting_main *s = (struct setting_main *)context;

	if (!strcmp(name, "transmitter1_control_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter1_control_pin);
	} else if (!strcmp(name, "transmitter1_power_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter1_power_pin);
	} else if (!strcmp(name, "transmitter2_control_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter2_control_pin);
	} else if (!strcmp(name, "transmitter2_power_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter2_power_pin);
	} else if (!strcmp(name, "transmitter3_control_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter3_control_pin);
	} else if (!strcmp(name, "transmitter3_power_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter3_power_pin);
	} else if (!strcmp(name, "transmitter4_control_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter4_control_pin);
	} else if (!strcmp(name, "transmitter4_power_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter4_power_pin);
	} else if (!strcmp(name, "transmitter5_control_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter5_control_pin);
	} else if (!strcmp(name, "transmitter5_power_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter5_power_pin);
	} else if (!strcmp(name, "transmitter6_control_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter6_control_pin);
	} else if (!strcmp(name, "transmitter6_power_pin")) {
		return setting_parse_pin(file, ln, value, &s->transmitter6_power_pin);
	}

	fprintf(stderr, "%s:%zu: Unknown setting '%s'.\n", file, ln, name);
	return false;
}

bool setting_main_init(void)
{
	return setting_parse("etc/droneshot-daemon.conf", parse, &current);
}

const struct setting_main * setting_main_current(void)
{
	return &current;
}
