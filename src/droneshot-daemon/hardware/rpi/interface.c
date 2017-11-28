#include "../interface.h"

#include <bcm2835.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define CTRLPIN_WIFI1	RPI_V2_GPIO_P1_11
#define CTRLPIN_WIFI2	RPI_V2_GPIO_P1_13
#define CTRLPIN_WIFI3	RPI_V2_GPIO_P1_15
#define CTRLPIN_GPS		RPI_V2_GPIO_P1_33
#define CTRLPIN_RC1		RPI_V2_GPIO_P1_35
#define CTRLPIN_RC2		RPI_V2_GPIO_P1_37

struct transmitter {
	uint8_t downpin;
};

static void initialize_pin(uint8_t pin)
{
	bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set(pin);
}

bool hardware_interface_init(void)
{
	// initialize library.
	if (!bcm2835_init()) {
		return false;
	}

	// set all selector pin to high.
	initialize_pin(CTRLPIN_WIFI1);
	initialize_pin(CTRLPIN_WIFI2);
	initialize_pin(CTRLPIN_WIFI3);
	initialize_pin(CTRLPIN_GPS);
	initialize_pin(CTRLPIN_RC1);
	initialize_pin(CTRLPIN_RC2);

	return true;
}

void hardware_interface_close(void)
{
	// set all selector pin to low.
	bcm2835_gpio_clr(CTRLPIN_WIFI1);
	bcm2835_gpio_clr(CTRLPIN_WIFI2);
	bcm2835_gpio_clr(CTRLPIN_WIFI3);
	bcm2835_gpio_clr(CTRLPIN_GPS);
	bcm2835_gpio_clr(CTRLPIN_RC1);
	bcm2835_gpio_clr(CTRLPIN_RC2);

	// clean up library.
	if (!bcm2835_close()) {
		fprintf(stderr, "Failed to clean up C library for Broadcom BCM 2835.\n");
	}
}

struct transmitter * hardware_interface_transmitter_open(int id)
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
		fprintf(stderr, "Unknown transmitter identifier %d.\n", id);
		return NULL;
	}

	// create transmitter interface.
	t = calloc(1, sizeof(struct transmitter));
	if (!t) {
		fprintf(stderr, "Failed to allocated memory for transmitter interface.\n");
		return NULL;
	}

	t->downpin = pin;

	return t;
}

void hardware_interface_transmitter_close(struct transmitter *t)
{
	free(t);
}
