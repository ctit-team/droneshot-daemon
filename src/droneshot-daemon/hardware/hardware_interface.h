#pragma once

#include <cassert>

namespace hardware {

class hardware_interface {
public:
    hardware_interface(hardware_interface const &) = delete;
    virtual ~hardware_interface();

	hardware_interface & operator=(hardware_interface const &) = delete;

	static hardware_interface * instance();

protected:
	hardware_interface();

private:
	static hardware_interface * current;
};

inline hardware_interface * hardware_interface::instance()
{
	assert(current);
	return current;
}

}
