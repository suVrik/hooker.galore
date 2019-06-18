#pragma once

#include "core/resource/material.h"

#include <string>

namespace hg {

/** `MaterialComponent` contains material for an entity. */
struct MaterialComponent final {
    std::string path;
    const Material* material = nullptr;
};

} // namespace hg
