#include "../interface.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct transmitter {
	int id;
};

bool hardware_interface_init(void)
{
}

void hardware_interface_close(void)
{
}

struct transmitter * hardware_interface_transmitter_open(int id)
{
	struct transmitter *t;

	// check if identifiers is valid.
	if (id != TRANSMITTER_WIFI1 && id != TRANSMITTER_WIFI2 && id != TRANSMITTER_WIFI3 &&
		id != TRANSMITTER_GPS && id != TRANSMITTER_RC1 && id != TRANSMITTER_RC2) {
		fprintf(stderr, "Unknown transmitter identifier %d.\n", id);
		return NULL;
	}

	// create transmitter interface.
	t = calloc(1, sizeof(struct transmitter));
	if (!t) {
		fprintf(stderr, "Failed to allocated memory for transmitter interface.\n");
		return NULL;
	}

	t->id = id;

	return t;
}

void hardware_interface_transmitter_close(struct transmitter *t)
{
	free(t);
}
