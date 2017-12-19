#pragma once

#include <inttypes.h>
#include <stdbool.h>

struct setting_main {
	uint8_t transmitter1_control_pin;
	uint8_t transmitter1_power_pin;

	uint8_t transmitter2_control_pin;
	uint8_t transmitter2_power_pin;

	uint8_t transmitter3_control_pin;
	uint8_t transmitter3_power_pin;

	uint8_t transmitter4_control_pin;
	uint8_t transmitter4_power_pin;

	uint8_t transmitter5_control_pin;
	uint8_t transmitter5_power_pin;

	uint8_t transmitter6_control_pin;
	uint8_t transmitter6_power_pin;
};

bool setting_main_init(void);
const struct setting_main * setting_main_current(void);
