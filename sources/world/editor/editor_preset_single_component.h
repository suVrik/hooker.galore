#pragma once

#include <entt/meta/factory.hpp>
#include <entt/entity/registry.hpp>
#include <map>

namespace hg {

/** `EditorPresetSingleComponent` contains editor presets. */
struct EditorPresetSingleComponent final {
    std::map<std::string, std::vector<entt::meta_any>> presets;
    entt::entity placed_entity = entt::null;
};

} // namespace hg
