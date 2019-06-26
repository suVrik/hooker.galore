#include "core/base/split.h"

namespace hg {

std::vector<std::string> split(const std::string& string, char delimiter) noexcept {
    std::vector<std::string> result;

    size_t last_position = 0;
    for (size_t i = 0; i < string.size(); i++) {
        if (string[i] == delimiter) {
            if (last_position < i) {
                result.push_back(string.substr(last_position, i - last_position));
            }
            last_position = i + 1;
        }
    }

    if (last_position < string.size()) {
        result.push_back(string.substr(last_position, string.size() - last_position));
    }

    return result;
}

} // namespace hg
