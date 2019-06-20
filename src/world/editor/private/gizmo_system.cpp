#include "core/ecs/world.h"
#include "core/render/ImGuizmo.h"
#include "world/editor/gizmo_single_component.h"
#include "world/editor/gizmo_system.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/transform_component.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace hg {

GizmoSystem::GizmoSystem(World& world) noexcept
        : NormalSystem(world) {
    world.set<GizmoSingleComponent>();
}

void GizmoSystem::update(float /*elapsed_time*/) {
    auto& gizmo_single_component = world.ctx<GizmoSingleComponent>();
    auto& selected_entity_single_component = world.ctx<SelectedEntitySingleComponent>();
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();

    if (ImGui::Begin("Tool")) {
        const ImVec2 window_size = ImGui::GetWindowSize();

        auto button = [&](const char* title, Control shortcut, ImGuizmo::OPERATION operation) {
            const ImGuizmo::OPERATION old_operation = gizmo_single_component.operation;
            if (old_operation == operation) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.8f, 0.1f, 1.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.85f, 0.2f, 1.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.7f, 0.f, 1.f));
            }
            if (ImGui::Button(title, ImVec2(window_size.x / 2 - 15.f, window_size.y / 2 - 33.f)) ||
                (normal_input_single_component.is_pressed(shortcut) && !ImGui::IsAnyItemActive())) {
                gizmo_single_component.operation = operation;
            }
            if (old_operation == operation) {
                ImGui::PopStyleColor(3);
            }
        };

        if (normal_input_single_component.is_pressed(Control::KEY_GRAVE)) {
            gizmo_single_component.is_local_space = !gizmo_single_component.is_local_space;
        }
        if (ImGui::RadioButton("Local space (~)", gizmo_single_component.is_local_space)) {
            gizmo_single_component.is_local_space = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("World space", !gizmo_single_component.is_local_space)) {
            gizmo_single_component.is_local_space = false;
        }

        button("Translate (1)", Control::KEY_1, ImGuizmo::OPERATION::TRANSLATE);
        ImGui::SameLine();
        button("Rotate (2)", Control::KEY_2, ImGuizmo::OPERATION::ROTATE);
        button("Scale (3)", Control::KEY_3, ImGuizmo::OPERATION::SCALE);
        ImGui::SameLine();
        button("Bounds (4)", Control::KEY_4, ImGuizmo::OPERATION::BOUNDS);
    }
    ImGui::End();

    if (world.valid(selected_entity_single_component.selected_entity)) {
        auto* transform_component = &world.get<TransformComponent>(selected_entity_single_component.selected_entity);

        glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component->translation);
        transform = transform * glm::mat4_cast(transform_component->rotation);
        transform = glm::scale(transform, transform_component->scale);

        const bool was_using = ImGuizmo::IsUsing();
        
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGuizmo::Manipulate(glm::value_ptr(camera_single_component.view_matrix),
                glm::value_ptr(camera_single_component.projection_matrix),
                gizmo_single_component.operation,
                gizmo_single_component.is_local_space ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD,
                glm::value_ptr(transform));
        
        if (!was_using && ImGuizmo::IsUsing() && (normal_input_single_component.is_down(Control::KEY_LSHIFT) || normal_input_single_component.is_down(Control::KEY_RSHIFT))) {
            entt::entity new_entity = world.create();
            world.each(selected_entity_single_component.selected_entity, [&](entt::meta_handle component_handle) {
                world.assign(new_entity, component_handle);
            });
            // TODO: Unique editor guid/name as well?
            transform_component = &world.get<TransformComponent>(new_entity);
            selected_entity_single_component.selected_entity = new_entity;
        }

        transform_component->scale = glm::vec3(std::max(1e-2f, glm::length(transform[0])), std::max(1e-2f, glm::length(transform[1])), std::max(1e-2f, glm::length(transform[2])));
        transform[0] /= transform_component->scale.x;
        transform[1] /= transform_component->scale.y;
        transform[2] /= transform_component->scale.z;
        transform_component->translation = glm::vec3(transform[3].x, transform[3].y, transform[3].z);
        transform_component->rotation = glm::quat(transform);
    }
}

} // namespace hg
