#pragma once

#include <string>
#include <vector>

namespace hg {

/** Split the specified `string` with the given `delimiter` and return a bunch of tokens. */
std::vector<std::string> split(const std::string& string, char delimiter);

} // namespace hg
