#include "../interface.h"

#include <bcm2835.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SOCLIB "C library for Broadcom BCM 2835"

#define CTRLPIN_WIFI1	RPI_V2_GPIO_P1_03
#define CTRLPIN_WIFI2	RPI_V2_GPIO_P1_05
#define CTRLPIN_WIFI3	RPI_V2_GPIO_P1_07
#define CTRLPIN_GPS		RPI_V2_GPIO_P1_22
#define CTRLPIN_RC1		RPI_V2_GPIO_P1_29
#define CTRLPIN_RC2		RPI_V2_GPIO_P1_31

#define PWRPIN_WIFI1	RPI_V2_GPIO_P1_40
#define PWRPIN_WIFI2	RPI_V2_GPIO_P1_38
#define PWRPIN_WIFI3	RPI_V2_GPIO_P1_36
#define PWRPIN_GPS		RPI_V2_GPIO_P1_37
#define PWRPIN_RC1		RPI_V2_GPIO_P1_35
#define PWRPIN_RC2		RPI_V2_GPIO_P1_33

#define CTRLPIN_MASK	((1 << CTRLPIN_WIFI1) | \
						 (1 << CTRLPIN_WIFI2) | \
						 (1 << CTRLPIN_WIFI3) | \
						 (1 << CTRLPIN_GPS) | \
						 (1 << CTRLPIN_RC1) | \
						 (1 << CTRLPIN_RC2))

#define PWRPIN_MASK		((1 << PWRPIN_WIFI1) | \
						 (1 << PWRPIN_WIFI2) | \
						 (1 << PWRPIN_WIFI3) | \
						 (1 << PWRPIN_GPS) | \
						 (1 << PWRPIN_RC1) | \
						 (1 << PWRPIN_RC2))

#define value_from_percent(m, p) ((m) * (p) / 100)

struct transmitter {
	uint8_t ctrlpin;
	uint8_t pwrpin;
};

bool hardware_interface_init(void)
{
	if (bcm2835_init()) {
		if (bcm2835_spi_begin()) {
			uint32_t pins, states;

			// disable SPI automatic chip selector since we control it manually.
			bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);

			// adjust SPI options. see MCP41050 data sheet for more information.
			bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
			bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128); // 3.125MHz on RPI3.
			bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

			// set all control/power pins to output mode.
			bcm2835_gpio_fsel(CTRLPIN_WIFI1, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(CTRLPIN_WIFI2, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(CTRLPIN_WIFI3, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(CTRLPIN_GPS, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(CTRLPIN_RC1, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(CTRLPIN_RC2, BCM2835_GPIO_FSEL_OUTP);

			bcm2835_gpio_fsel(PWRPIN_WIFI1, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(PWRPIN_WIFI2, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(PWRPIN_WIFI3, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(PWRPIN_GPS, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(PWRPIN_RC1, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(PWRPIN_RC2, BCM2835_GPIO_FSEL_OUTP);

			// initialize pins state.
			pins = 0; states = 0;

			pins   |= CTRLPIN_MASK | PWRPIN_MASK;
			states |= CTRLPIN_MASK; // set all control pins and clear all power pins.

			bcm2835_gpio_write_mask(states, pins);

			return true;
		}

		fprintf(stderr, "Failed to initializes SPI interface.\n");

		if (!bcm2835_close()) {
			fprintf(stderr, "Failed to clean up " SOCLIB ".\n");
		}
	}

	return false;
}

void hardware_interface_close(void)
{
	// turn off transmitters first to prevent unexpected behavior when clearing control pins.
	bcm2835_gpio_clr_multi(PWRPIN_MASK);

	// we need to shutdown SPI while control pins are high to prevent executing unexpected command.
	bcm2835_spi_end();

	// now it safe to clear all control pins.
	bcm2835_gpio_clr_multi(CTRLPIN_MASK);

	// clean up library.
	if (!bcm2835_close()) {
		fprintf(stderr, "Failed to clean up " SOCLIB ".\n");
	}
}

struct transmitter * transmitter_open(int id)
{
	uint8_t ctl, pwr;
	struct transmitter *t;

	// select control pin.
	switch (id) {
	case TRANSMITTER_WIFI1:
		ctl = CTRLPIN_WIFI1;
		pwr = PWRPIN_WIFI1;
		break;
	case TRANSMITTER_WIFI2:
		ctl = CTRLPIN_WIFI2;
		pwr = PWRPIN_WIFI2;
		break;
	case TRANSMITTER_WIFI3:
		ctl = CTRLPIN_WIFI3;
		pwr = PWRPIN_WIFI3;
		break;
	case TRANSMITTER_GPS:
		ctl = CTRLPIN_GPS;
		pwr = PWRPIN_GPS;
		break;
	case TRANSMITTER_RC1:
		ctl = CTRLPIN_RC1;
		pwr = PWRPIN_RC1;
		break;
	case TRANSMITTER_RC2:
		ctl = CTRLPIN_RC2;
		pwr = PWRPIN_RC2;
		break;
	default:
		fprintf(stderr, "Failed to open a connection to transmitter: Unknown transmitter identifier %d.\n", id);
		return NULL;
	}

	// create transmitter interface.
	t = calloc(1, sizeof(struct transmitter));
	if (!t) {
		fprintf(stderr, "Failed to open a connection to %s transmitter: Insufficient memory.\n", transmitter_names[id]);
		return NULL;
	}

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
