#include "../interface.h"
#include "../../setting/main.h"

#include <bcm2835.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCLIB "C library for Broadcom BCM 2835"

#define bit(v) (1 << (v))
#define value_from_percent(m, p) ((m) * (p) / 100)

struct transmitter {
	uint8_t ctrlpin;
	uint8_t pwrpin;
};

bool hardware_interface_init(void)
{
	const struct setting_main *s;
	uint32_t pins, states;

	if (!bcm2835_init()) {
		return false;
	}

	if (!bcm2835_spi_begin()) {
		fprintf(stderr, "Failed to initializes SPI interface.\n");
		goto fail;
	}

	// disable SPI automatic chip selector since we control it manually.
	bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);

	// adjust SPI options. see MCP41050 data sheet for more information.
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128); // 3.125MHz on RPI3.
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

	// set all control/power pins to output mode.
	s = setting_main_current();

	bcm2835_gpio_fsel(s->transmitter1_control_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(s->transmitter2_control_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(s->transmitter3_control_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(s->transmitter4_control_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(s->transmitter5_control_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(s->transmitter6_control_pin, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_fsel(s->transmitter1_power_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(s->transmitter2_power_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(s->transmitter3_power_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(s->transmitter4_power_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(s->transmitter5_power_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(s->transmitter6_power_pin, BCM2835_GPIO_FSEL_OUTP);

	// set all control pins and clear all power pins.
	pins   = bit(s->transmitter1_control_pin) | bit(s->transmitter2_control_pin)
		   | bit(s->transmitter3_control_pin) | bit(s->transmitter4_control_pin)
		   | bit(s->transmitter5_control_pin) | bit(s->transmitter6_control_pin)
		   | bit(s->transmitter1_power_pin) | bit(s->transmitter2_power_pin)
		   | bit(s->transmitter3_power_pin) | bit(s->transmitter4_power_pin)
		   | bit(s->transmitter5_power_pin) | bit(s->transmitter6_power_pin);
	states = bit(s->transmitter1_control_pin) | bit(s->transmitter2_control_pin)
		   | bit(s->transmitter3_control_pin) | bit(s->transmitter4_control_pin)
		   | bit(s->transmitter5_control_pin) | bit(s->transmitter6_control_pin);

	bcm2835_gpio_write_mask(states, pins);

	return true;

	// error handlers.
	fail:

	if (!bcm2835_close()) {
		fprintf(stderr, "Failed to clean up " SOCLIB ".\n");
	}

	return false;
}

void hardware_interface_close(void)
{
	const struct setting_main *s = setting_main_current();
	uint32_t pins;

	// turn off transmitters first to prevent unexpected behavior when clearing
	// control pins.
	pins = bit(s->transmitter1_power_pin) | bit(s->transmitter2_power_pin)
		 | bit(s->transmitter3_power_pin) | bit(s->transmitter4_power_pin)
		 | bit(s->transmitter5_power_pin) | bit(s->transmitter6_power_pin);

	bcm2835_gpio_clr_multi(pins);

	// we need to shutdown SPI while control pins are high to prevent executing
	// unexpected command.
	bcm2835_spi_end();

	// now it safe to clear all control pins.
	pins = bit(s->transmitter1_control_pin) | bit(s->transmitter2_control_pin)
		 | bit(s->transmitter3_control_pin) | bit(s->transmitter4_control_pin)
		 | bit(s->transmitter5_control_pin) | bit(s->transmitter6_control_pin);

	bcm2835_gpio_clr_multi(pins);

	// clean up library.
	if (!bcm2835_close()) {
		fprintf(stderr, "Failed to clean up " SOCLIB ".\n");
	}
}

struct transmitter * transmitter_open(int id)
{
	const struct setting_main *s = setting_main_current();
	uint8_t ctl, pwr;
	struct transmitter *t;

	// select control pin.
	switch (id) {
	case TRANSMITTER_WIFI1:
		ctl = s->transmitter1_control_pin;
		pwr = s->transmitter1_power_pin;
		break;
	case TRANSMITTER_WIFI2:
		ctl = s->transmitter2_control_pin;
		pwr = s->transmitter2_power_pin;
		break;
	case TRANSMITTER_WIFI3:
		ctl = s->transmitter3_control_pin;
		pwr = s->transmitter3_power_pin;
		break;
	case TRANSMITTER_GPS:
		ctl = s->transmitter4_control_pin;
		pwr = s->transmitter4_power_pin;
		break;
	case TRANSMITTER_RC1:
		ctl = s->transmitter5_control_pin;
		pwr = s->transmitter5_power_pin;
		break;
	case TRANSMITTER_RC2:
		ctl = s->transmitter6_control_pin;
		pwr = s->transmitter6_power_pin;
		break;
	default:
		fprintf(stderr, "Failed to open a connection to transmitter: Unknown transmitter identifier %d.\n", id);
		return NULL;
	}

	// create transmitter interface.
	t = malloc(sizeof(t[0]));
	if (!t) {
		fprintf(stderr, "Failed to open a connection to %s transmitter: Insufficient memory.\n", transmitter_names[id]);
		return NULL;
	}

	memset(t, 0, sizeof(t[0]));
	t->ctrlpin = ctl;
	t->pwrpin = pwr;

	return t;
}

enum utilization_result transmitter_utilization_set(struct transmitter *t, int util)
{
	const uint8_t start = value_from_percent(0xFF, 80);
	uint8_t data[2];

	if (util < 0 || util > 100) {
		return utilization_invalid;
	}

	// adjust utilization.
	data[0] = 0x10 | 0x03;
	data[1] = start + value_from_percent(0xFF - start, util);

	bcm2835_gpio_clr(t->ctrlpin);
	bcm2835_spi_writenb((char *)data, sizeof(data));
	bcm2835_gpio_set(t->ctrlpin);

	// toggle master switch.
	if (util) {
		if (bcm2835_gpio_lev(t->pwrpin) != HIGH) {
			bcm2835_gpio_set(t->pwrpin);
		}
	} else {
		if (bcm2835_gpio_lev(t->pwrpin) != LOW) {
			bcm2835_gpio_clr(t->pwrpin);
		}
	}

	return utilization_success;
}

void transmitter_close(struct transmitter *t)
{
	free(t);
}
