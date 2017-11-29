#include "../interface.h"

#include <bcm2835.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

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

struct transmitter {
	uint8_t downpin;
};

bool hardware_interface_init(void)
{
	uint32_t pins, states;

	// initialize library.
	if (!bcm2835_init()) {
		return false;
	}

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

void hardware_interface_close(void)
{
	// reset all pins.
	bcm2835_gpio_clr_multi(PWRPIN_MASK); // turn off first.
	bcm2835_gpio_clr_multi(CTRLPIN_MASK);

	// clean up library.
	if (!bcm2835_close()) {
		fprintf(stderr, "Failed to clean up C library for Broadcom BCM 2835.\n");
	}
}

struct transmitter * transmitter_open(int id)
{
	uint8_t pin;
	struct transmitter *t;

	// select control pin.
	switch (id) {
	case TRANSMITTER_WIFI1:
		pin = CTRLPIN_WIFI1;
		break;
	case TRANSMITTER_WIFI2:
		pin = CTRLPIN_WIFI2;
		break;
	case TRANSMITTER_WIFI3:
		pin = CTRLPIN_WIFI3;
		break;
	case TRANSMITTER_GPS:
		pin = CTRLPIN_GPS;
		break;
	case TRANSMITTER_RC1:
		pin = CTRLPIN_RC1;
		break;
	case TRANSMITTER_RC2:
		pin = CTRLPIN_RC2;
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

	t->downpin = pin;

	return t;
}

void transmitter_close(struct transmitter *t)
{
	free(t);
}
