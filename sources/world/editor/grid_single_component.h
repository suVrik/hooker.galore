#pragma once

#include <memory>

namespace hg {

/** `GridSingleComponent` describes look of editor grid. */
struct GridSingleComponent final {
    // Menu items.
    std::shared_ptr<bool> is_shown;
};

} // namespace hg
