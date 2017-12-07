#include "../interface.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct transmitter {
	int id;
};

bool hardware_interface_init(void)
{
	return true;
}

void hardware_interface_close(void)
{
}

struct transmitter * transmitter_open(int id)
{
	struct transmitter *t;

	// check if identifiers is valid.
	if (id != TRANSMITTER_WIFI1 && id != TRANSMITTER_WIFI2 && id != TRANSMITTER_WIFI3 &&
		id != TRANSMITTER_GPS && id != TRANSMITTER_RC1 && id != TRANSMITTER_RC2) {
		fprintf(stderr, "Failed to open a connection to transmitter: Unknown transmitter identifier %d.\n", id);
		return NULL;
	}

	// create transmitter interface.
	t = calloc(1, sizeof(struct transmitter));
	if (!t) {
		fprintf(stderr, "Failed to open a connection to %s transmitter: Insufficient memory.\n", transmitter_names[id]);
		return NULL;
	}

	t->id = id;

	return t;
}

const char * transmitter_utilization_set(struct transmitter *t, int util)
{
	if (util < 0 || util > 100) {
		return "Invalid utilization value";
	}

	return NULL;
}

void transmitter_close(struct transmitter *t)
{
	free(t);
}
