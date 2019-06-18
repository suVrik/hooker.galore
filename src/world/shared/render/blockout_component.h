#pragma once

namespace hg {

/** `BlockoutComponent` makes the renderer to use `geometry_blockout_pass` shader instead of a regular `geometry_pass`
    shader, which makes object's UV coordinates scale-dependent without changing vertex buffers. */
struct BlockoutComponent final {
    bool dummy = false;
};

} // namespace hg
