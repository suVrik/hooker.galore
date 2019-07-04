#include "core/ecs/world.h"
#include "world/editor/editor_component.h"
#include "world/editor/entity_selection_system.h"
#include "world/editor/guid_single_component.h"
#include "world/editor/history_single_component.h"
#include "world/editor/menu_single_component.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/render/outline_component.h"
#include "world/shared/render/picking_pass_single_component.h"
#include "world/shared/render/render_single_component.h"
#include "world/shared/window_single_component.h"

#include <algorithm>
#include <fmt/format.h>
#include <glm/common.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <set>

namespace hg {

EntitySelectionSystem::EntitySelectionSystem(World& world) noexcept
        : NormalSystem(world) {
    auto& selected_entity_single_component = world.set<SelectedEntitySingleComponent>();
    selected_entity_single_component.select_all_entities = std::make_shared<bool>(false);
    selected_entity_single_component.clear_selected_entities = std::make_shared<bool>(false);
    selected_entity_single_component.delete_selected_entities = std::make_shared<bool>(false);

    auto& menu_single_component = world.ctx<MenuSingleComponent>();
    menu_single_component.items.emplace("1Edit/2Select all entities", MenuSingleComponent::MenuItem(selected_entity_single_component.select_all_entities, "Ctrl+A"));
    menu_single_component.items.emplace("1Edit/3Clear selected entities", MenuSingleComponent::MenuItem(selected_entity_single_component.clear_selected_entities, "Ctrl+D"));
    menu_single_component.items.emplace("1Edit/4Delete selected entities", MenuSingleComponent::MenuItem(selected_entity_single_component.delete_selected_entities, "Del"));
}

void EntitySelectionSystem::update(float /*elapsed_time*/) {
    auto& guid_single_component = world.ctx<GuidSingleComponent>();
    auto& history_single_component = world.ctx<HistorySingleComponent>();
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();
    auto& picking_pass_single_component = world.ctx<PickingPassSingleComponent>();
    auto& render_single_component = world.ctx<RenderSingleComponent>();
    auto& selected_entity_single_component = world.ctx<SelectedEntitySingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    world.sort<EditorComponent>([](const EditorComponent& a, const EditorComponent& b) {
        return a.name < b.name;
    });

    ImGui::SetNextWindowDockID(ImGui::GetID("Main"), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Level")) {
        char buffer[255] = { '\0' };
        ImGui::InputText("Filter", buffer, sizeof(buffer));
        ImGui::Separator();

        std::string lower_case_filter = buffer;
        std::transform(lower_case_filter.begin(), lower_case_filter.end(), lower_case_filter.begin(), ::tolower);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::BeginChildFrame(ImGui::GetID("level-frame"), ImVec2(0.f, 0.f));
        ImGui::PopStyleColor();

        size_t index = 0;
        world.view<EditorComponent>().each([&](entt::entity entity, EditorComponent &editor_component) {
            std::string lower_case_name = editor_component.name;
            std::transform(lower_case_name.begin(), lower_case_name.end(), lower_case_name.begin(), ::tolower);
            if (lower_case_filter.empty() || lower_case_name.find(lower_case_filter) != std::string::npos) {
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf;
                if (std::find(selected_entity_single_component.selected_entities.begin(), selected_entity_single_component.selected_entities.end(), entity) != selected_entity_single_component.selected_entities.end()) {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }
                ImGui::TreeNodeEx(reinterpret_cast<void*>(++index), flags, "%s", editor_component.name.c_str());
                if (ImGui::IsItemClicked()) {
                    if (normal_input_single_component.is_down(Control::KEY_SHIFT)) {
                        selected_entity_single_component.add_to_selection(world, entity);
                    } else {
                        selected_entity_single_component.select_entity(world, entity);
                    }
                }
            }
        });

        ImGui::EndChildFrame();
    }
    ImGui::End();

    if (selected_entity_single_component.waiting_for_pick) {
        if (render_single_component.current_frame >= picking_pass_single_component.target_frame) {
            selected_entity_single_component.waiting_for_pick = false;

            const bgfx::RendererType::Enum renderer_type = bgfx::getRendererType();
            if (renderer_type == bgfx::RendererType::OpenGL || renderer_type == bgfx::RendererType::OpenGLES) {
                // OpenGL coordinate system starts at lower-left corner.
                selected_entity_single_component.selection_start_y = window_single_component.height - selected_entity_single_component.selection_start_y;
                selected_entity_single_component.selection_end_y = window_single_component.height - selected_entity_single_component.selection_end_y;
            }

            const int32_t selection_start_x = glm::clamp(std::min(selected_entity_single_component.selection_start_x, selected_entity_single_component.selection_end_x), 0, std::max(int32_t(window_single_component.width), 1) - 1);
            const int32_t selection_start_y = glm::clamp(std::min(selected_entity_single_component.selection_start_y, selected_entity_single_component.selection_end_y), 0, std::max(int32_t(window_single_component.height), 1) - 1);
            const int32_t selection_end_x = glm::clamp(std::max(selected_entity_single_component.selection_start_x, selected_entity_single_component.selection_end_x), 0, std::max(int32_t(window_single_component.width), 1) - 1);
            const int32_t selection_end_y = glm::clamp(std::max(selected_entity_single_component.selection_start_y, selected_entity_single_component.selection_end_y), 0, std::max(int32_t(window_single_component.height), 1) - 1);

            std::set<uint32_t> selected_entities;
            for (int32_t y = selection_start_y; y <= selection_end_y; y++) {
                for (int32_t x = selection_start_x; x <= selection_end_x; x++) {
                    const size_t offset = size_t(window_single_component.width * y + x) * 4;
                    if (offset + sizeof(uint32_t) <= picking_pass_single_component.target_data.size()) {
                        selected_entities.insert(*reinterpret_cast<uint32_t*>(picking_pass_single_component.target_data.data() + offset) & 0x00FFFFFF);
                    }
                }
            }

            if (!normal_input_single_component.is_down(Control::KEY_SHIFT) && !normal_input_single_component.is_down(Control::KEY_ALT)) {
                selected_entity_single_component.clear_selection(world);
            }

            for (uint32_t selected_object : selected_entities) {
                if (selected_object != 0 && guid_single_component.guid_to_entity.count(selected_object) > 0) {
                    if (normal_input_single_component.is_down(Control::KEY_ALT)) {
                        selected_entity_single_component.remove_from_selection(world, guid_single_component.guid_to_entity[selected_object]);
                    } else {
                        selected_entity_single_component.add_to_selection(world, guid_single_component.guid_to_entity[selected_object]);
                    }
                }
            }

            if (selected_entities.empty()) {
                if (!normal_input_single_component.is_down(Control::KEY_SHIFT) && !normal_input_single_component.is_down(Control::KEY_ALT)) {
                    selected_entity_single_component.select_entity(world, entt::null);
                }
            }
        }
    } else {
        if (normal_input_single_component.is_pressed(Control::BUTTON_LEFT)) {
            selected_entity_single_component.is_selecting = true;
            selected_entity_single_component.selection_start_x = normal_input_single_component.get_mouse_x();
            selected_entity_single_component.selection_start_y = normal_input_single_component.get_mouse_y();
        }

        if (selected_entity_single_component.is_selecting) {
            ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
            if (selected_entity_single_component.selection_start_x != normal_input_single_component.get_mouse_x() ||
                selected_entity_single_component.selection_start_y != normal_input_single_component.get_mouse_y()) {
                draw_list->AddRectFilled(ImVec2(float(selected_entity_single_component.selection_start_x), float(selected_entity_single_component.selection_start_y)),
                                         ImVec2(float(normal_input_single_component.get_mouse_x()), float(normal_input_single_component.get_mouse_y())),
                                         ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 0.f, 0.15f)));
                draw_list->AddRect(ImVec2(float(selected_entity_single_component.selection_start_x), float(selected_entity_single_component.selection_start_y)),
                                   ImVec2(float(normal_input_single_component.get_mouse_x()), float(normal_input_single_component.get_mouse_y())),
                                   ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 0.f, 1.f)));
            }

            if (normal_input_single_component.is_released(Control::BUTTON_LEFT)) {
                picking_pass_single_component.perform_picking = true;
                selected_entity_single_component.waiting_for_pick = true;
                selected_entity_single_component.selection_end_x = normal_input_single_component.get_mouse_x();
                selected_entity_single_component.selection_end_y = normal_input_single_component.get_mouse_y();
            }

            if (!normal_input_single_component.is_down(Control::BUTTON_LEFT)) {
                selected_entity_single_component.is_selecting = false;
            }
        }
    }

    if ((*selected_entity_single_component.delete_selected_entities || normal_input_single_component.is_pressed(Control::KEY_DELETE)) && !selected_entity_single_component.selected_entities.empty()) {
        std::string description;
        if (selected_entity_single_component.selected_entities.size() == 1) {
            assert(world.has<EditorComponent>(selected_entity_single_component.selected_entities[0]));
            auto& editor_component = world.get<EditorComponent>(selected_entity_single_component.selected_entities[0]);
            description = fmt::format("Delete entity \"{}\"", editor_component.name);
        } else {
            description = "Delete entities";
        }

        auto* change = history_single_component.begin(world, description);
        if (change != nullptr) {
            std::vector<entt::entity> entities_to_delete = selected_entity_single_component.selected_entities;

            // Remove outline components from these entities first.
            selected_entity_single_component.clear_selection(world);

            for (entt::entity entity : entities_to_delete) {
                assert(world.valid(entity));
                change->delete_entity(world, entity);
            }
        }
    }
    *selected_entity_single_component.delete_selected_entities = false;

    if (*selected_entity_single_component.select_all_entities || (normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_pressed(Control::KEY_A))) {
        selected_entity_single_component.clear_selection(world);

        world.view<EditorComponent>().each([&](entt::entity entity, EditorComponent &editor_component) {
            selected_entity_single_component.add_to_selection(world, entity);
        });
    }
    *selected_entity_single_component.select_all_entities = false;

    if (*selected_entity_single_component.clear_selected_entities || (normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_pressed(Control::KEY_D))) {
        selected_entity_single_component.clear_selection(world);
    }
    *selected_entity_single_component.clear_selected_entities = false;
}

} // namespace hg
