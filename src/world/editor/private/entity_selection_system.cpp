#include "core/ecs/world.h"
#include "core/render/ImGuizmo.h"
#include "world/editor/editor_component.h"
#include "world/editor/entity_selection_system.h"
#include "world/editor/guid_single_component.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/render/outline_component.h"
#include "world/shared/render/picking_pass_single_component.h"
#include "world/shared/render/render_single_component.h"
#include "world/shared/window_single_component.h"

#include <algorithm>
#include <imgui.h>

namespace hg {

EntitySelectionSystem::EntitySelectionSystem(World& world) noexcept
        : NormalSystem(world) {
    world.set<SelectedEntitySingleComponent>();
}

void EntitySelectionSystem::update(float /*elapsed_time*/) {
    auto& guid_single_component = world.ctx<GuidSingleComponent>();
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

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::BeginChildFrame(ImGui::GetID("level-frame"), ImVec2(0.f, 0.f));
        ImGui::PopStyleColor();

        size_t idx = 0;
        world.view<EditorComponent>().each([&](entt::entity entity, EditorComponent &editor_component) {
            std::string lower_case_name = editor_component.name;
            std::transform(lower_case_name.begin(), lower_case_name.end(), lower_case_name.begin(), ::tolower);
            if (buffer[0] == '\0' || lower_case_name.find(buffer) != std::string::npos) {
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf;
                if (selected_entity_single_component.selected_entity == entity) {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }
                ImGui::TreeNodeEx(reinterpret_cast<void*>(++idx), flags, "%s", editor_component.name.c_str());
                if (ImGui::IsItemClicked()) {
                    if (world.valid(selected_entity_single_component.selected_entity)) {
                        world.reset<OutlineComponent>(selected_entity_single_component.selected_entity);
                    }
                    selected_entity_single_component.selected_entity = entity;
                    world.assign<OutlineComponent>(entity);
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
                selected_entity_single_component.selection_end_y = window_single_component.height - selected_entity_single_component.selection_end_y;
            }

            const size_t offset = (window_single_component.width * size_t(selected_entity_single_component.selection_end_y) + size_t(selected_entity_single_component.selection_end_x)) * 4;
            if (offset + 4 <= picking_pass_single_component.target_data.size()) {
                const uint32_t selected_object = *reinterpret_cast<uint32_t*>(picking_pass_single_component.target_data.data() + offset) & 0x00FFFFFF;
                if (selected_object != 0 && guid_single_component.guid_to_entity.count(selected_object) > 0) {
                    selected_entity_single_component.select_entity(world, guid_single_component.guid_to_entity[selected_object]);
                } else {
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
            draw_list->AddRect(ImVec2(float(selected_entity_single_component.selection_start_x), float(selected_entity_single_component.selection_start_y)),
                               ImVec2(float(normal_input_single_component.get_mouse_x()), float(normal_input_single_component.get_mouse_y())),
                               ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 0.f, 1.f)));

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

    if (normal_input_single_component.is_pressed(Control::KEY_DELETE) && world.valid(selected_entity_single_component.selected_entity)) {
        auto& editor_component = world.get<EditorComponent>(selected_entity_single_component.selected_entity);
        guid_single_component.guid_to_entity.erase(editor_component.guid);
        world.destroy(selected_entity_single_component.selected_entity);
        selected_entity_single_component.selected_entity = entt::null;
    }
}

} // namespace hg
