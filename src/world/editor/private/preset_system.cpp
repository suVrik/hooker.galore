#include "core/ecs/world.h"
#include "world/editor/preset_single_component.h"
#include "world/editor/preset_system.h"

#include <ghc/filesystem.hpp>
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
        std::vector<std::string> directories;

        intptr_t idx = 0;
        for (const auto& [preset_name, preset] : preset_single_component.presets) {
            std::vector<std::string> preset_path = split(preset_name, ghc::filesystem::path::preferred_separator);

            auto directories_it = directories.begin();
            for (auto preset_path_it = preset_path.begin(); preset_path_it != preset_path.end(); ++preset_path_it) {
                if (directories_it != directories.end()) {
                    if (*directories_it != *preset_path_it) {
                        for (auto it = directories_it; it != directories.end(); ++it) {
                            ImGui::TreePop();
                        }
                        directories.erase(directories_it, directories.end());
                        directories_it = directories.end();
                    } else {
                        ++directories_it;
                        continue;
                    }
                }
                if (std::next(preset_path_it) != preset_path.end()) {
                    if (ImGui::TreeNode(preset_path_it->c_str())) {
                        directories.push_back(*preset_path_it);
                        directories_it = directories.end();
                    } else {
                        break;
                    }
                } else {
                    ImGui::TreeNodeEx(reinterpret_cast<void*>(++idx), ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf, "%s", preset_path_it->c_str());
                    if (ImGui::IsItemClicked()) {
                        if (ImGui::IsMouseDoubleClicked(0)) {
                            entt::entity entity = world.create();
                            for (const entt::meta_any& component_prototype : preset) {
                                world.assign(entity, component_prototype);
                            }
                        }
                    }
                }
            }
        }
        for (auto it = directories.begin(); it != directories.end(); ++it) {
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

} // namespace hg
