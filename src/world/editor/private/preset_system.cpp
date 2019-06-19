#include "core/ecs/world.h"
#include "world/editor/preset_single_component.h"
#include "world/editor/preset_system.h"

#include <imgui.h>

namespace hg {

PresetSystem::PresetSystem(World& world) noexcept
        : NormalSystem(world) {
    world.set<PresetSingleComponent>();
}

void PresetSystem::update(float /*elapsed_time*/) {
    auto split = [](const std::string& string, char delimiter) {
        std::vector<std::string> result;

        size_t last_position = 0;
        for (size_t i = 0; i < string.size(); i++) {
            if (string[i] == delimiter) {
                if (last_position < i) {
                    result.push_back(string.substr(last_position, i - last_position));
                }
                last_position = i + 1;
            }
        }

        if (last_position < string.size()) {
            result.push_back(string.substr(last_position, string.size() - last_position));
        }

        return result;
    };

    auto& preset_single_component = world.ctx<PresetSingleComponent>();
    if (ImGui::Begin("Presets")) {
#ifdef __APPLE__
        constexpr char PATH_DELIMITER = '/';
#else
        constexpr char PATH_DELIMITER = '\\';
#endif

        std::vector<std::string> directories;

        for (const auto& [preset_name, preset] : preset_single_component.presets) {
            if (std::find(preset_name.begin(), preset_name.end(), PATH_DELIMITER) != preset_name.end()) {
                std::vector<std::string> preset_path = split(preset_name, PATH_DELIMITER);
            }
        }

        for (const auto& [preset_name, preset] : preset_single_component.presets) {
            if (std::find(preset_name.begin(), preset_name.end(), PATH_DELIMITER) == preset_name.end()) {

            }
        }
    }
    ImGui::End();
}

} // namespace hg
