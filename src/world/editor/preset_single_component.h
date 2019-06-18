#pragma once

#include <entt/meta/factory.hpp>
#include <string>
#include <map>

namespace hg {

/** `PresetSingleComponent` contains editor presets. */
class PresetSingleComponent final {
public:
    std::map<std::string, entt::meta_any> presets;
};

} // namespace hg
