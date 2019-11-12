#pragma once

namespace hg {

/** `RenderSingleComponent` contains renderer state. */
struct RenderSingleComponent final {
    uint32_t current_frame = 0;
    bool show_debug_info = false;
};

} // namespace hg
