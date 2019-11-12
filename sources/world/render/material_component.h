#pragma once

#include "core/resource/texture.h"

#include <string>

namespace hg {

/** `MaterialComponent` contains material data for an entity. */
struct MaterialComponent final {
    std::string material;

    // These are initialized from `material` in `ResourceSystem`.
    // In order to update these fields `material` must be updated via `replace` world method.
    const Texture* color_roughness = nullptr;
    const Texture* normal_metal_ao = nullptr;
};

} // namespace hg
