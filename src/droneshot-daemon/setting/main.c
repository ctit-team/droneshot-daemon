#include "main.h"
#include "parser.h"

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static struct setting_main current;

static bool parse_transmitter_utilization(const struct setting_parsing *data, uint8_t *res)
{
	unsigned long value;

	errno = 0;
	value = strtoul(data->value, NULL, 0);

	if (errno || value > 0xFF) {
		return setting_parsing_error(data, "'%s' is not a valid utilization value.\n", data->value);
	}

	res[0] = (uint8_t)value;
	return true;
}

static bool parse_transmitter_setting(const struct setting_parsing *data, const char *suffix, struct transmitter_settings *s)
{
	if (!strcmp(suffix, "_control_pin")) {
		return setting_parse_pin(data, &s->ctlpin);
	} else if (!strcmp(suffix, "_power_pin")) {
		return setting_parse_pin(data, &s->pwrpin);
	} else if (!strcmp(suffix, "_utilization_start")) {
		return parse_transmitter_utilization(data, &s->start);
	} else if (!strcmp(suffix, "_utilization_end")) {
		return parse_transmitter_utilization(data, &s->end);
	}

	return setting_parsing_unknown(data);
}

static bool parse(const struct setting_parsing *data)
{
	struct setting_main *s = (struct setting_main *)data->ctx;
	const char *suffix;
	int num;

	num = setting_parse_prefix(data->name, "transmitter", &suffix);
	if (num > 0 && num <= MAX_TRANSMITTER) {
		return parse_transmitter_setting(data, suffix, &s->transmitters[num - 1]);
	}

	return setting_parsing_unknown(data);
}

bool setting_main_init(void)
{
	return setting_parse("etc/droneshot-daemon.conf", parse, &current);
}

const struct setting_main * setting_main_current(void)
{
	return &current;
}
