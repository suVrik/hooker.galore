#pragma once

#include "core/resource/model.h"

#include <memory>
#include <string>

namespace hg {

/** `ModelComponent` contains hierarchy of geometry. */
struct ModelComponent final {
    std::string path;
    Model model;
};

} // namespace hg
