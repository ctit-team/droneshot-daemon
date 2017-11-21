#include "hardware_interface.h"

#include <stdexcept>

namespace hardware {

hardware_interface * hardware_interface::current;

hardware_interface::hardware_interface()
{
	if (current) {
		throw std::logic_error("There is multiple hardware interfaces included "
				"in this build. Only one interface is supported at a time.");
	}

	current = this;
}

hardware_interface::~hardware_interface()
{
	current = nullptr;
}

}
