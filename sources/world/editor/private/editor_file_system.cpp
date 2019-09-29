#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/editor/editor_file_single_component.h"
#include "world/editor/editor_file_system.h"
#include "world/editor/editor_history_single_component.h"
#include "world/editor/editor_menu_single_component.h"
#include "world/editor/editor_selection_single_component.h"
#include "world/shared/level_single_component.h"
#include "world/shared/name_component.h"
#include "world/shared/name_single_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/resource_utils.h"
#include "world/shared/window_single_component.h"

#include <fstream>
#include <ghc/filesystem.hpp>
#include <imgui.h>
#include <yaml-cpp/yaml.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(EditorFileSystem),
    REQUIRE("editor"),
    BEFORE("ImguiPassSystem", "GeometryPassSystem"),
    AFTER("EditorMenuSystem", "WindowSystem", "ImguiFetchSystem", "ResourceSystem")
)

EditorFileSystem::EditorFileSystem(World& world) noexcept
    : NormalSystem(world) {
    auto& editor_file_single_component = world.set<EditorFileSingleComponent>();
    editor_file_single_component.new_level     = std::make_shared<bool>(false);
    editor_file_single_component.open_level    = std::make_shared<bool>(false);
    editor_file_single_component.save_level    = std::make_shared<bool>(false);
    editor_file_single_component.save_level_as = std::make_shared<bool>(false);

    auto& editor_menu_single_component = world.ctx<EditorMenuSingleComponent>();
    editor_menu_single_component.add_item("0File/New Level",        editor_file_single_component.new_level,     "Ctrl+N");
    editor_menu_single_component.add_item("0File/Open Level...",    editor_file_single_component.open_level,    "Ctrl+O");
    editor_menu_single_component.add_item("0File/Save Level",       editor_file_single_component.save_level,    "Ctrl+S");
    editor_menu_single_component.add_item("0File/Save Level As...", editor_file_single_component.save_level_as, "Ctrl+Shift+S");
}

void EditorFileSystem::update(float /*elapsed_time*/) {
    auto& editor_file_single_component = world.ctx<EditorFileSingleComponent>();
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();
    handle_new_level_action(editor_file_single_component, normal_input_single_component);
    handle_open_level_action(editor_file_single_component, normal_input_single_component);
    handle_save_level_action(editor_file_single_component, normal_input_single_component);
    handle_save_level_as_action(editor_file_single_component, normal_input_single_component);
    handle_save_level_popup(editor_file_single_component);
    handle_save_level_as_popup(editor_file_single_component);
    handle_open_level_popup(editor_file_single_component);
    handle_level_error_popup(editor_file_single_component);
    update_window_title();
}

void EditorFileSystem::handle_new_level_action(EditorFileSingleComponent& editor_file_single_component, 
                                               NormalInputSingleComponent& normal_input_single_component) noexcept {
    if (*editor_file_single_component.new_level || (normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_pressed(Control::KEY_N))) {
        *editor_file_single_component.new_level = false;

        if (!editor_file_single_component.ignore_changed_level && is_level_changed()) {
            editor_file_single_component.save_level_action = editor_file_single_component.new_level;
            ImGui::OpenPopup("Save Level");
        } else {
            editor_file_single_component.save_level_as_action = EditorFileSingleComponent::SaveLevelAsAction::NEW_LEVEL;
            editor_file_single_component.save_level_as_path[0] = '\0';
            editor_file_single_component.ignore_changed_level = false;
            ImGui::OpenPopup("Save Level As");
        }
    }
}

void EditorFileSystem::handle_open_level_action(EditorFileSingleComponent& editor_file_single_component, 
                                                NormalInputSingleComponent& normal_input_single_component) noexcept {
    if (*editor_file_single_component.open_level || (normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_pressed(Control::KEY_O))) {
        *editor_file_single_component.open_level = false;

        if (!editor_file_single_component.ignore_changed_level && is_level_changed()) {
            editor_file_single_component.save_level_action = editor_file_single_component.open_level;
            ImGui::OpenPopup("Save Level");
        } else {
            editor_file_single_component.ignore_changed_level = false;
            editor_file_single_component.selected_level = "";
            ImGui::OpenPopup("Open Level");
        }
    }
}

void EditorFileSystem::handle_save_level_action(EditorFileSingleComponent& editor_file_single_component, 
                                                NormalInputSingleComponent& normal_input_single_component) noexcept {
    if (*editor_file_single_component.save_level || (normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_pressed(Control::KEY_S))) {
        *editor_file_single_component.save_level = false;

        if (is_level_changed()) {
            if (!save_level()) {
                ImGui::OpenPopup("Level Error");
            }
        }
    }
}

void EditorFileSystem::handle_save_level_as_action(EditorFileSingleComponent& editor_file_single_component, 
                                                   NormalInputSingleComponent& normal_input_single_component) noexcept {
    if (*editor_file_single_component.save_level_as || (normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_down(Control::KEY_SHIFT) && normal_input_single_component.is_pressed(Control::KEY_S))) {
        *editor_file_single_component.save_level_as = false;

        editor_file_single_component.save_level_as_action = EditorFileSingleComponent::SaveLevelAsAction::NONE;
        editor_file_single_component.save_level_as_path[0] = '\0';
        ImGui::OpenPopup("Save Level As");
    }
}

void EditorFileSystem::handle_save_level_popup(EditorFileSingleComponent& editor_file_single_component) noexcept {
    bool show_error = false;

    if (ImGui::BeginPopupModal("Save Level", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto& level_single_component = world.ctx<LevelSingleComponent>();
        
        ImGui::Text("Save changes to \"%s\"?", level_single_component.level_name.c_str());

        ImGui::Spacing();
        ImGui::Indent(100.f);

        if (ImGui::Button("Cancel")) {
            editor_file_single_component.save_level_action = nullptr;

            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();

        if (ImGui::Button("Don't Save")) {
            editor_file_single_component.ignore_changed_level = true;

            assert(editor_file_single_component.save_level_action);
            *editor_file_single_component.save_level_action = true;
            editor_file_single_component.save_level_action = nullptr;

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Save Level")) {
            if (save_level()) {
                assert(editor_file_single_component.save_level_action);
                *editor_file_single_component.save_level_action = true;
            } else {
                show_error = true;
            }

            editor_file_single_component.save_level_action = nullptr;

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (show_error) {
        ImGui::OpenPopup("Level Error");
    }
}

void EditorFileSystem::handle_save_level_as_popup(EditorFileSingleComponent& editor_file_single_component) noexcept {
    bool show_error = false;

    if (ImGui::BeginPopupModal("Save Level As", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Level Name", editor_file_single_component.save_level_as_path.data(), editor_file_single_component.save_level_as_path.size());

        ImGui::Spacing();
        ImGui::Indent(150.f);

        if (ImGui::Button("Cancel")) {
            editor_file_single_component.save_level_as_action = EditorFileSingleComponent::SaveLevelAsAction::NONE;

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Save Level")) {
            auto& level_single_component = world.ctx<LevelSingleComponent>();

            const std::string old_level_name = level_single_component.level_name;

            level_single_component.level_name = editor_file_single_component.save_level_as_path.data();
            if (level_single_component.level_name.empty()) {
                level_single_component.level_name = "untitled";
            }
            if (level_single_component.level_name.size() < 5 || level_single_component.level_name.substr(level_single_component.level_name.size() - 5, 5) != ".yaml") {
                level_single_component.level_name += ".yaml";
            }

            if (editor_file_single_component.save_level_as_action == EditorFileSingleComponent::SaveLevelAsAction::NEW_LEVEL) {
                if (!new_level()) {
                    level_single_component.level_name = old_level_name;
                    show_error = true;
                }
            } else {
                if (!save_level()) {
                    level_single_component.level_name = old_level_name;
                    show_error = true;
                }
            }

            editor_file_single_component.save_level_as_action = EditorFileSingleComponent::SaveLevelAsAction::NONE;

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (show_error) {
        ImGui::OpenPopup("Level Error");
    }
}

void EditorFileSystem::handle_open_level_popup(EditorFileSingleComponent& editor_file_single_component) noexcept {
    bool show_error = false;

    if (ImGui::BeginPopupModal("Open Level", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        const ghc::filesystem::path directory = ghc::filesystem::path(ResourceUtils::get_resource_directory()) / "levels";
        if (ghc::filesystem::exists(directory)) {
            for (const auto& directory_entry : ghc::filesystem::recursive_directory_iterator(directory)) {
                const ghc::filesystem::path& file_path = directory_entry.path();
                const std::string relative_file_path = file_path.lexically_relative(directory).lexically_normal().string();
                if (ImGui::Selectable(relative_file_path.c_str(), editor_file_single_component.selected_level == relative_file_path, ImGuiSelectableFlags_DontClosePopups)) {
                    editor_file_single_component.selected_level = relative_file_path;
                }
            }
        }

        ImGui::Spacing();
        ImGui::Indent(100.f);

        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Open Level")) {
            if (!editor_file_single_component.selected_level.empty()) {
                auto& level_single_component = world.ctx<LevelSingleComponent>();
                const std::string old_level_name = level_single_component.level_name;
                level_single_component.level_name = editor_file_single_component.selected_level;

                if (!open_level()) {
                    level_single_component.level_name = old_level_name;
                    show_error = true;
                }
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    if (show_error) {
        ImGui::OpenPopup("Level Error");
    }
}

void EditorFileSystem::handle_level_error_popup(EditorFileSingleComponent& editor_file_single_component) noexcept {
    if (ImGui::BeginPopupModal("Level Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Failed to save level.");

        ImGui::Spacing();
        ImGui::Indent(130.f);

        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void EditorFileSystem::update_window_title() noexcept {
    auto& level_single_component = world.ctx<LevelSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    window_single_component.title = level_single_component.level_name;
    if (is_level_changed()) {
        window_single_component.title += "*";
    }
    window_single_component.title += u8"  \u2014  Hooker Galore";
}

bool EditorFileSystem::is_level_changed() noexcept {
    return world.ctx<EditorHistorySingleComponent>().is_level_changed;
}

bool EditorFileSystem::new_level() noexcept {
    auto& level_single_component = world.ctx<LevelSingleComponent>();

    YAML::Node root_node(YAML::NodeType::Map);
    root_node["entities"] = YAML::Node(YAML::NodeType::Sequence);

    const ghc::filesystem::path level_path = ghc::filesystem::path(ResourceUtils::get_resource_directory()) / "levels" / level_single_component.level_name;

    std::ofstream stream(level_path.string());
    if (!stream.is_open()) {
        return false;
    }

    if (!(stream << root_node)) {
        return false;
    }

    clear_level();

    return true;
}

void EditorFileSystem::clear_level() noexcept {
    auto& editor_history_single_component = world.ctx<EditorHistorySingleComponent>();
    auto& editor_selection_single_component = world.ctx<EditorSelectionSingleComponent>();
    auto& name_single_component = world.ctx<NameSingleComponent>();

    editor_selection_single_component.clear_selection(world);

    std::vector<entt::entity> entities_to_delete;
    for (entt::entity entity : world.view<NameComponent>()) {
        entities_to_delete.push_back(entity);
    }

    for (entt::entity entity : entities_to_delete) {
        auto& name_component = world.get<NameComponent>(entity);

        assert(name_single_component.name_to_entity.count(name_component.name) == 1);
        assert(name_single_component.name_to_entity[name_component.name] == entity);
        name_single_component.name_to_entity.erase(name_component.name);

        world.destroy(entity);
    }

    // TODO: Reset camera as well.

    assert(name_single_component.name_to_entity.empty());

    std::fill(editor_history_single_component.undo.begin(), editor_history_single_component.undo.end(), EditorHistorySingleComponent::HistoryChange());
    editor_history_single_component.undo_position = 0;
    editor_history_single_component.redo.clear();
    editor_history_single_component.is_level_changed = false;
}

bool EditorFileSystem::save_level() noexcept {
    if (ResourceUtils::serialize_level(world, true)) {
        world.ctx<EditorHistorySingleComponent>().is_level_changed = false;
        return true;
    }
    return false;
}

bool EditorFileSystem::open_level() noexcept {
    auto& level_single_component = world.ctx<LevelSingleComponent>();
    auto& name_single_component = world.ctx<NameSingleComponent>();

    const ghc::filesystem::path level_path = ghc::filesystem::path(ResourceUtils::get_resource_directory()) / "levels" / level_single_component.level_name;
    if (!ghc::filesystem::exists(level_path)) {
        return false;
    }

    std::ifstream stream(level_path.string());
    if (!stream.is_open()) {
        return false;
    }

    const YAML::Node node = YAML::Load(stream);
    if (!node.IsMap()) {
        return false;
    }

    const YAML::Node entities = node["entities"];
    if (!entities.IsSequence()) {
        return false;
    }

    clear_level();

    ResourceUtils::deserialize_level(world, entities, &name_single_component);

    return true;
}

} // namespace hg
