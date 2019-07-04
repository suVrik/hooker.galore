#pragma once

#include <entt/meta/factory.hpp>
#include <map>
#include <string>

namespace hg {

/** `PresetSingleComponent` contains editor presets. */
class PresetSingleComponent final {
public:
    std::map<std::string, std::vector<entt::meta_any>> presets;
};

} // namespace hg
