#include "../interface.h"

#include <bcm2835.h>

#include <stdio.h>

bool hardware_interface_init(void)
{
	if (!bcm2835_init()) {
		return false;
	}

	return true;
}

void hardware_interface_close(void)
{
	if (!bcm2835_close()) {
		fprintf(stderr, "Failed to clean up C library for Broadcom BCM 2835.\n");
	}
}
