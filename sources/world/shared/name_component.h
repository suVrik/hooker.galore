#pragma once

#include <string>

namespace hg {

/** `NameComponent` is a component required for all entities manageable by editor. */
struct NameComponent final {
    std::string name;
};

} // namespace hg
