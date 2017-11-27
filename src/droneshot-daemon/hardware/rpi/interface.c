#include "../interface.h"

#include <bcm2835.h>

#include <inttypes.h>
#include <stdio.h>

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
	initialize_pin(RPI_V2_GPIO_P1_11);
	initialize_pin(RPI_V2_GPIO_P1_13);
	initialize_pin(RPI_V2_GPIO_P1_15);
	initialize_pin(RPI_V2_GPIO_P1_33);
	initialize_pin(RPI_V2_GPIO_P1_35);
	initialize_pin(RPI_V2_GPIO_P1_37);

	return true;
}

void hardware_interface_close(void)
{
	// set all selector pin to low.
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_11);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_13);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_15);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_33);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_35);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_37);

	// clean up library.
	if (!bcm2835_close()) {
		fprintf(stderr, "Failed to clean up C library for Broadcom BCM 2835.\n");
	}
}
