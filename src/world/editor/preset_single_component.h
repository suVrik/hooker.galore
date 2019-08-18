#pragma once

#include <entt/meta/factory.hpp>
#include <map>
#include <string>

namespace hg {

/** `PresetSingleComponent` contains editor presets. */
struct PresetSingleComponent final {
    std::map<std::string, std::vector<entt::meta_any>> presets;
    entt::entity placed_entity = entt::null;
};

} // namespace hg
