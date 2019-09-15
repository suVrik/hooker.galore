#pragma once

#include <memory>

namespace hg {

/** `EditorGridSingleComponent` describes look of editor grid. */
struct EditorGridSingleComponent final {
    // Menu items.
    std::shared_ptr<bool> is_shown;
};

} // namespace hg
