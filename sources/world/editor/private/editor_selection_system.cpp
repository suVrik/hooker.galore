#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/editor/editor_history_single_component.h"
#include "world/editor/editor_menu_single_component.h"
#include "world/editor/editor_selection_single_component.h"
#include "world/editor/editor_selection_system.h"
#include "world/shared/name_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/render/outline_component.h"
#include "world/shared/render/picking_pass_single_component.h"
#include "world/shared/render/render_single_component.h"
#include "world/shared/window_single_component.h"

#include <ImGuizmo.h>
#include <SDL2/SDL_timer.h>
#include <algorithm>
#include <fmt/format.h>
#include <glm/common.hpp>
#include <imgui.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(EditorSelectionSystem),
    REQUIRE("editor"),
    BEFORE("ImguiPassSystem", "PickingPassSystem"),
    AFTER("EditorMenuSystem", "WindowSystem", "ImguiFetchSystem")
)

EditorSelectionSystem::EditorSelectionSystem(World& world) noexcept
        : NormalSystem(world) {
    auto& editor_selection_single_component = world.set<EditorSelectionSingleComponent>();
    editor_selection_single_component.select_all_entities = std::make_shared<bool>(false);
    editor_selection_single_component.clear_selected_entities = std::make_shared<bool>(false);
    editor_selection_single_component.delete_selected_entities = std::make_shared<bool>(false);

    auto& editor_menu_single_component = world.ctx<EditorMenuSingleComponent>();
    editor_menu_single_component.add_item("1Edit/2Select all entities",      editor_selection_single_component.select_all_entities,      "Ctrl+A");
    editor_menu_single_component.add_item("1Edit/3Clear selected entities",  editor_selection_single_component.clear_selected_entities,  "Ctrl+D");
    editor_menu_single_component.add_item("1Edit/4Delete selected entities", editor_selection_single_component.delete_selected_entities, "Del");
}

void EditorSelectionSystem::update(float /*elapsed_time*/) {
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();
    auto& editor_selection_single_component = world.ctx<EditorSelectionSingleComponent>();

    world.sort<NameComponent>([](const NameComponent& a, const NameComponent& b) {
        return a.name < b.name;
    });

    show_level_window(editor_selection_single_component, normal_input_single_component);
    perform_picking(editor_selection_single_component, normal_input_single_component);
    delete_selected(editor_selection_single_component, normal_input_single_component);
    select_all(editor_selection_single_component, normal_input_single_component);
    clear_selection(editor_selection_single_component, normal_input_single_component);
}

void EditorSelectionSystem::show_level_window(EditorSelectionSingleComponent& editor_selection_single_component, 
                                              NormalInputSingleComponent& normal_input_single_component) const noexcept {
    if (ImGui::Begin("Level", nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
        char buffer[255] = { '\0' };
        ImGui::InputText("Filter", buffer, sizeof(buffer));
        ImGui::Separator();

        std::string lower_case_filter = buffer;
        std::transform(lower_case_filter.begin(), lower_case_filter.end(), lower_case_filter.begin(), ::tolower);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::BeginChildFrame(ImGui::GetID("level-frame"), ImVec2(0.f, 0.f));
        ImGui::PopStyleColor();

        size_t index = 0;
        world.view<NameComponent>().each([&](entt::entity entity, NameComponent& name_component) {
            std::string lower_case_name = name_component.name;
            std::transform(lower_case_name.begin(), lower_case_name.end(), lower_case_name.begin(), ::tolower);
            if (lower_case_filter.empty() || lower_case_name.find(lower_case_filter) != std::string::npos) {
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf;
                if (std::find(editor_selection_single_component.selected_entities.begin(), editor_selection_single_component.selected_entities.end(), entity) != editor_selection_single_component.selected_entities.end()) {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }
                ImGui::TreeNodeEx(reinterpret_cast<void*>(++index), flags, "%s", name_component.name.c_str());
                if (ImGui::IsItemClicked()) {
                    if (normal_input_single_component.is_down(Control::KEY_SHIFT)) {
                        editor_selection_single_component.add_to_selection(world, entity);
                    } else {
                        editor_selection_single_component.select_entity(world, entity);
                    }
                }
            }
        });

        ImGui::EndChildFrame();
    }
    ImGui::End();
}

void EditorSelectionSystem::perform_picking(EditorSelectionSingleComponent& editor_selection_single_component,
                                            NormalInputSingleComponent& normal_input_single_component) const noexcept {
    auto& picking_pass_single_component = world.ctx<PickingPassSingleComponent>();
    auto& render_single_component = world.ctx<RenderSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (editor_selection_single_component.waiting_for_pick) {
        if (render_single_component.current_frame >= picking_pass_single_component.target_frame) {
            editor_selection_single_component.waiting_for_pick = false;

            const bgfx::RendererType::Enum renderer_type = bgfx::getRendererType();
            if (renderer_type == bgfx::RendererType::OpenGL || renderer_type == bgfx::RendererType::OpenGLES) {
                // OpenGL coordinate system starts at lower-left corner.
                editor_selection_single_component.selection_y = window_single_component.height - editor_selection_single_component.selection_y;
            }

            assert(window_single_component.width != 0);
            assert(window_single_component.height != 0);

            const auto selection_x = static_cast<uint32_t>(glm::clamp(editor_selection_single_component.selection_x, 0, static_cast<int32_t>(window_single_component.width) - 1));
            const auto selection_y = static_cast<uint32_t>(glm::clamp(editor_selection_single_component.selection_y, 0, static_cast<int32_t>(window_single_component.height) - 1));

            entt::entity selected_entity = entt::null;
            const uint32_t offset = (window_single_component.width * selection_y + selection_x) * 4;
            if (offset + sizeof(uint32_t) <= picking_pass_single_component.target_data.size()) {
                selected_entity = *reinterpret_cast<entt::entity*>(picking_pass_single_component.target_data.data() + offset);
            }

            if (!normal_input_single_component.is_down(Control::KEY_SHIFT) && !normal_input_single_component.is_down(Control::KEY_ALT)) {
                editor_selection_single_component.clear_selection(world);
            }

            if (world.valid(selected_entity)) {
                if (normal_input_single_component.is_down(Control::KEY_ALT)) {
                    editor_selection_single_component.remove_from_selection(world, selected_entity);
                } else {
                    editor_selection_single_component.add_to_selection(world, selected_entity);
                }
            }
        }
    } else {
        const int32_t mouse_x = normal_input_single_component.get_mouse_x();
        const int32_t mouse_y = normal_input_single_component.get_mouse_y();
        if (normal_input_single_component.is_pressed(Control::BUTTON_LEFT)) {
            editor_selection_single_component.selection_x = mouse_x;
            editor_selection_single_component.selection_y = mouse_y;
            editor_selection_single_component.selection_time = SDL_GetTicks();
        } else if (normal_input_single_component.is_released(Control::BUTTON_LEFT)) {
            if (SDL_GetTicks() - editor_selection_single_component.selection_time < 150) {
                picking_pass_single_component.perform_picking = true;
                editor_selection_single_component.waiting_for_pick = true;
            }
        }
    }
}

void EditorSelectionSystem::delete_selected(EditorSelectionSingleComponent& editor_selection_single_component,
                                            NormalInputSingleComponent& normal_input_single_component) const noexcept {
    auto& editor_history_single_component = world.ctx<EditorHistorySingleComponent>();

    if ((*editor_selection_single_component.delete_selected_entities || normal_input_single_component.is_pressed(Control::KEY_DELETE)) && !editor_selection_single_component.selected_entities.empty()) {
        *editor_selection_single_component.delete_selected_entities = false;

        std::string description;
        if (editor_selection_single_component.selected_entities.size() == 1) {
            assert(world.has<NameComponent>(editor_selection_single_component.selected_entities[0]));
            auto& name_component = world.get<NameComponent>(editor_selection_single_component.selected_entities[0]);
            description = fmt::format("Delete entity \"{}\"", name_component.name);
        } else {
            description = "Delete entities";
        }

        auto* change = editor_history_single_component.begin(world, description);
        if (change != nullptr) {
            std::vector<entt::entity> entities_to_delete = editor_selection_single_component.selected_entities;

            // Remove outline components from these entities first.
            editor_selection_single_component.clear_selection(world);

            for (entt::entity entity : entities_to_delete) {
                assert(world.valid(entity));
                change->delete_entity(world, entity);
            }
        }
    }
}

void EditorSelectionSystem::select_all(EditorSelectionSingleComponent& editor_selection_single_component,
                                       NormalInputSingleComponent& normal_input_single_component) const noexcept {
    if (*editor_selection_single_component.select_all_entities || (normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_pressed(Control::KEY_A))) {
        *editor_selection_single_component.select_all_entities = false;

        editor_selection_single_component.clear_selection(world);

        world.view<NameComponent>().each([&](entt::entity entity, NameComponent& name_component) {
            editor_selection_single_component.add_to_selection(world, entity);
        });
    }
}

void EditorSelectionSystem::clear_selection(EditorSelectionSingleComponent& editor_selection_single_component, 
                                            NormalInputSingleComponent& normal_input_single_component) const noexcept {
    if (*editor_selection_single_component.clear_selected_entities || (normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_pressed(Control::KEY_D))) {
        *editor_selection_single_component.clear_selected_entities = false;

        editor_selection_single_component.clear_selection(world);
    }
}

} // namespace hg
