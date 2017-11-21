#pragma once

#include "../hardware_interface.h"

namespace hardware {
namespace emulation {

class hardware_interface final : public hardware::hardware_interface {
public:
    hardware_interface(hardware_interface const &) = delete;
    ~hardware_interface() override;

	hardware_interface & operator=(hardware_interface const &) = delete;

private:
	hardware_interface();

	static hardware_interface instance;
};

}
}
