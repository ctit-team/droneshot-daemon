#include "hardware_interface.h"

#include <iostream>
#include <stdexcept>

#include <bcm2835.h>

namespace hardware {
namespace rpi {

hardware_interface hardware_interface::instance;

hardware_interface::hardware_interface()
{
	if (!bcm2835_init()) {
		throw std::runtime_error("Failed to initializes C library for Broadcom BCM 2835.");
	}
}

hardware_interface::~hardware_interface()
{
	if (!bcm2835_close()) {
		std::cerr << "Failed to clean up C library for Broadcom BCM 2835." << std::endl;
	}
}

}
}
