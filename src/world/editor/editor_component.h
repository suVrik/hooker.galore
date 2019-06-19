#pragma once

#include <string>

namespace hg {

/** `EditorComponent` is a component required for all entities manageable by editor. */
struct EditorComponent final {
    std::string name;
    uint32_t guid;
};

} // namespace hg
