#pragma once

#include <stdbool.h>

#define TRANSMITTER_WIFI1	1 // 2.4 lower
#define TRANSMITTER_WIFI2	2 // 2.4 upper
#define TRANSMITTER_WIFI3	3 // 5.8
#define TRANSMITTER_GPS		4
#define TRANSMITTER_RC1		5
#define TRANSMITTER_RC2		6

enum utilization_result {
	utilization_success,
	utilization_invalid
};

struct transmitter;

bool hardware_interface_init(void);
void hardware_interface_close(void);

struct transmitter * transmitter_open(int id);
enum utilization_result transmitter_utilization_set(struct transmitter *t, int util);
void transmitter_close(struct transmitter *t);
