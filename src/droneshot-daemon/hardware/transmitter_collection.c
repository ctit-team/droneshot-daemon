#include "transmitter_collection.h"

#include <stddef.h>

#define DESCRIPTOR_COUNT (sizeof(descriptors) / sizeof(descriptors[0]))

struct transmitter_descriptor {
	int id;
	struct transmitter **interface;
};

struct transmitter *transmitter_wifi1;
struct transmitter *transmitter_wifi2;
struct transmitter *transmitter_wifi3;
struct transmitter *transmitter_gps;
struct transmitter *transmitter_rc1;
struct transmitter *transmitter_rc2;

static const struct transmitter_descriptor descriptors[] = {
	{ .interface = &transmitter_wifi1, .id = TRANSMITTER_WIFI1 },
	{ .interface = &transmitter_wifi2, .id = TRANSMITTER_WIFI2 },
	{ .interface = &transmitter_wifi3, .id = TRANSMITTER_WIFI3 },
	{ .interface = &transmitter_gps, .id = TRANSMITTER_GPS },
	{ .interface = &transmitter_rc1, .id = TRANSMITTER_RC1 },
	{ .interface = &transmitter_rc2, .id = TRANSMITTER_RC2 }
};

bool transmitter_collection_init(void)
{
	int i;

	for (i = 0; i < DESCRIPTOR_COUNT; i++) {
		const struct transmitter_descriptor *desc = &descriptors[i];
		struct transmitter *t;

		if (*desc->interface) {
			continue;
		}

		t = transmitter_open(desc->id);
		if (!t) {
			for (i--; i >= 0; i--) {
				transmitter_close(*desc->interface);
				*desc->interface = NULL;
			}
			return false;
		}

		*desc->interface = t;
	}

	return true;
}

struct transmitter * transmitter_collection_get(int id)
{
	int i;

	for (i = 0; i < DESCRIPTOR_COUNT; i++) {
		const struct transmitter_descriptor *desc = &descriptors[i];
		if (desc->id == id) {
			return *desc->interface;
		}
	}

	return NULL;
}

void transmitter_collection_close(void)
{
	int i;

	for (i = DESCRIPTOR_COUNT - 1; i >= 0; i--) {
		const struct transmitter_descriptor *desc = &descriptors[i];

		if (*desc->interface) {
			transmitter_close(*desc->interface);
			*desc->interface = NULL;
		}
	}
}
