#pragma once

#include <inttypes.h>
#include <stdbool.h>

#define MAX_TRANSMITTER 6

struct transmitter_settings {
	uint8_t ctlpin;
	uint8_t pwrpin;
	uint8_t start;
	uint8_t end;
};

struct setting_main {
	struct transmitter_settings transmitters[MAX_TRANSMITTER];
};

bool setting_main_init(void);
const struct setting_main * setting_main_current(void);
