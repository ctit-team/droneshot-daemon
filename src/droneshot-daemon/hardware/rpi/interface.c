#include "../interface.h"
#include "../../logging.h"
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
	int id;
};

bool hardware_interface_init(void)
{
	const struct setting_main *s;
	uint32_t pins, states;
	int i;

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

	for (i = 0; i < MAX_TRANSMITTER; i++) {
		bcm2835_gpio_fsel(s->transmitters[i].ctlpin, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_fsel(s->transmitters[i].pwrpin, BCM2835_GPIO_FSEL_OUTP);
	}

	// set all control pins and clear all power pins.
	pins = 0;
	states = 0;

	for (i = 0; i < MAX_TRANSMITTER; i++) {
		pins |= bit(s->transmitters[i].ctlpin) | bit(s->transmitters[i].pwrpin);
		states |= bit(s->transmitters[i].ctlpin);
	}

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
	int i;

	// turn off transmitters first to prevent unexpected behavior when clearing
	// control pins.
	pins = 0;

	for (i = 0; i < MAX_TRANSMITTER; i++) {
		pins |= bit(s->transmitters[i].pwrpin);
	}

	bcm2835_gpio_clr_multi(pins);

	// we need to shutdown SPI while control pins are high to prevent executing
	// unexpected command.
	bcm2835_spi_end();

	// now it safe to clear all control pins.
	pins = 0;

	for (i = 0; i < MAX_TRANSMITTER; i++) {
		pins |= bit(s->transmitters[i].ctlpin);
	}

	bcm2835_gpio_clr_multi(pins);

	// clean up library.
	if (!bcm2835_close()) {
		fprintf(stderr, "Failed to clean up " SOCLIB ".\n");
	}
}

struct transmitter * transmitter_open(int id)
{
	struct transmitter *t;

	if (id <= 0 || id > MAX_TRANSMITTER) {
		fprintf(stderr, "Failed to open a connection to transmitter: Unknown transmitter identifier %d.\n", id);
		return NULL;
	}

	// setup transmitter specific data.
	t = malloc(sizeof(t[0]));
	if (!t) {
		fprintf(stderr, "Failed to open a connection to transmitter %d: Insufficient memory.\n", id);
		return NULL;
	}

	memset(t, 0, sizeof(t[0]));
	t->id = id;

	return t;
}

enum utilization_result transmitter_utilization_set(struct transmitter *t, int util)
{
	const struct transmitter_settings *s;
	uint8_t data[2];

	if (util < 0 || util > 100) {
		return utilization_invalid;
	}

	// adjust utilization.
	s = &setting_main_current()->transmitters[t->id - 1];

	data[0] = 0x10 | 0x03;
	data[1] = s->start + value_from_percent(s->end - s->start, util);

	bcm2835_gpio_clr(s->ctlpin);

	logging_write(logging_hardware, "Transmitting SPI data: %s.\n", logging_printable_raw(data, sizeof(data)));
	bcm2835_spi_writenb((char *)data, sizeof(data));

	bcm2835_gpio_set(s->ctlpin);

	// toggle master switch.
	if (util) {
		if (bcm2835_gpio_lev(s->pwrpin) != HIGH) {
			bcm2835_gpio_set(s->pwrpin);
		}
	} else {
		if (bcm2835_gpio_lev(s->pwrpin) != LOW) {
			bcm2835_gpio_clr(s->pwrpin);
		}
	}

	return utilization_success;
}

void transmitter_close(struct transmitter *t)
{
	free(t);
}
