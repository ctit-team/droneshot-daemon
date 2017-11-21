#include "formatter.h"
#include "../string/operation.h"

namespace exception {

void messages(std::exception const & e, std::vector<std::string> & v)
{
	v.push_back(e.what());

	try {
		std::rethrow_if_nested(e);
	} catch (std::exception & e) {
		messages(e, v);
	}
}

std::vector<std::string> messages(std::exception const & e)
{
	std::vector<std::string> v;
	messages(e, v);
	return v;
}

std::string to_string(std::exception const & e, std::string const & sep)
{
	auto v = messages(e);
	return string::join(v, sep);
}

}