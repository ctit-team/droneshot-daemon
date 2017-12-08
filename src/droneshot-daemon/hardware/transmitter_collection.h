#pragma once

#include "interface.h"

#include <stdbool.h>

extern struct transmitter *transmitter_wifi1;
extern struct transmitter *transmitter_wifi2;
extern struct transmitter *transmitter_wifi3;
extern struct transmitter *transmitter_gps;
extern struct transmitter *transmitter_rc1;
extern struct transmitter *transmitter_rc2;

bool transmitter_collection_init(void);
struct transmitter * transmitter_collection_get(int id);
void transmitter_collection_close(void);
