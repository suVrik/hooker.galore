#pragma once

#include "core/ecs/world.h"

#include <entt/meta/meta.hpp>
#include <string>

namespace hg {

struct PropertyEditorContext {
    World& world;
    entt::meta_handle component;
    std::string property_name;
    entt::meta_handle property_value;
};

} // namespace hg
