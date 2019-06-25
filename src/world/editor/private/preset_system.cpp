#include "core/base/split.h"
#include "core/ecs/world.h"
#include "world/editor/editor_component.h"
#include "world/editor/guid_single_component.h"
#include "world/editor/history_single_component.h"
#include "world/editor/preset_single_component.h"
#include "world/editor/preset_system.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/name_single_component.h"
#include "world/shared/render/outline_component.h"

#include <algorithm>
#include <fmt/format.h>
#include <ghc/filesystem.hpp>
#include <imgui.h>
#include <SDL2/SDL_timer.h>

namespace hg {

PresetSystem::PresetSystem(World& world) noexcept
        : NormalSystem(world) {
    world.set<PresetSingleComponent>();
}

void PresetSystem::update(float /*elapsed_time*/) {
    auto& guid_single_component = world.ctx<GuidSingleComponent>();
    auto& history_single_component = world.ctx<HistorySingleComponent>();
    auto& name_single_component = world.ctx<NameSingleComponent>();
    auto& preset_single_component = world.ctx<PresetSingleComponent>();
    auto& selected_entity_single_component = world.ctx<SelectedEntitySingleComponent>();

    ImGui::SetNextWindowDockID(ImGui::GetID("Main"), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Presets")) {
        char buffer[255] = { '\0' };
        ImGui::InputText("Filter", buffer, sizeof(buffer));
        ImGui::Separator();

        std::string lower_case_filter = buffer;
        std::transform(lower_case_filter.begin(), lower_case_filter.end(), lower_case_filter.begin(), ::tolower);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::BeginChildFrame(ImGui::GetID("level-frame"), ImVec2(0.f, 0.f));
        ImGui::PopStyleColor();

        std::vector<std::string> directories;
        bool is_last_item_open = true;

        intptr_t idx = 0;
        for (auto& [preset_name, preset] : preset_single_component.presets) {
            std::string lower_case_name = preset_name;
            std::transform(lower_case_name.begin(), lower_case_name.end(), lower_case_name.begin(), ::tolower);

            if (lower_case_filter.empty() || lower_case_name.find(lower_case_filter) != std::string::npos) {
                std::vector<std::string> preset_path = split(preset_name, ghc::filesystem::path::preferred_separator);

                auto directories_it = directories.begin();
                for (auto preset_path_it = preset_path.begin(); preset_path_it != preset_path.end(); ++preset_path_it) {
                    if (directories_it != directories.end()) {
                        if (*directories_it != *preset_path_it) {
                            for (auto it = directories_it; it != directories.end(); ++it) {
                                if (std::next(it) != directories.end() || is_last_item_open) {
                                    ImGui::TreePop();
                                }
                            }
                            directories.erase(directories_it, directories.end());
                            directories_it = directories.end();
                            is_last_item_open = true;
                        } else {
                            ++directories_it;
                            continue;
                        }
                    }
                    if (is_last_item_open) {
                        if (std::next(preset_path_it) != preset_path.end()) {
                            directories.push_back(*preset_path_it);
                            directories_it = directories.end();

                            is_last_item_open = ImGui::TreeNode(preset_path_it->c_str());
                            if (!is_last_item_open) {
                                break;
                            }
                        } else {
                            ImGui::TreeNodeEx(reinterpret_cast<void *>(++idx), ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf, "%s", preset_path_it->c_str());
                            if (ImGui::IsItemClicked()) {
                                if (ImGui::IsMouseDoubleClicked(0)) {
                                    auto *change = history_single_component.begin(world, "");
                                    if (change != nullptr) {
                                        entt::entity entity = change->create_entity(world, ghc::filesystem::path(*preset_path_it).replace_extension("").string());

                                        assert(world.has<EditorComponent>(entity));
                                        auto &editor_component = world.get<EditorComponent>(entity);
                                        change->description = fmt::format("Create entity \"{}\"", editor_component.name);

                                        for (entt::meta_any &component_prototype : preset) {
                                            change->assign_component(world, entity, component_prototype);
                                        }

                                        selected_entity_single_component.select_entity(world, entity);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        for (auto it = directories.begin(); it != directories.end(); ++it) {
            if (std::next(it) != directories.end() || is_last_item_open) {
                ImGui::TreePop();
            }
        }

        ImGui::EndChildFrame();
    }
    ImGui::End();
}

} // namespace hg
