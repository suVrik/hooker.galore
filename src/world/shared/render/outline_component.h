#pragma once

namespace hg {

/** `OutlineComponent` forces the renderer to draw outline around the model. Color of outline is specified in
    `OutlinePassSingleComponent`. */
struct OutlineComponent final {
    uint32_t group_index = 1;
};

} // namespace hg
