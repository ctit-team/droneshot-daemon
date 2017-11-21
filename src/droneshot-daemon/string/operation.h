#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace string {

template<class CharT, class Traits, class Alloc>
std::basic_string<CharT, Traits, Alloc> join(std::vector<std::basic_string<CharT, Traits, Alloc>> const & v, std::basic_string<CharT, Traits, Alloc> const & sep);


template<class CharT, class Traits, class Alloc>
inline std::basic_string<CharT, Traits, Alloc> join(std::vector<std::basic_string<CharT, Traits, Alloc>> const & v, std::basic_string<CharT, Traits, Alloc> const & sep)
{
	std::basic_stringstream<CharT, Traits, Alloc> s;

	for (auto it = v.begin();;) {
		if (it == v.end()) {
			break;
		}

		s << *it;

		it++;
		if (it != v.end()) {
			s << sep;
		}
	}

	return s.str();
}

}
