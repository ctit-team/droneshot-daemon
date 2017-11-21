#pragma once

#include <exception>
#include <string>
#include <vector>

namespace exception {

void messages(std::exception const & e, std::vector<std::string> & v);
std::vector<std::string> messages(std::exception const & e);
std::string to_string(std::exception const & e, std::string const & sep = " -> ");

}
