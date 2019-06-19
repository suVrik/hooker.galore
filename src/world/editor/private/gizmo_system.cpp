#include "core/ecs/world.h"
#include "core/render/ImGuizmo.h"
#include "world/editor/gizmo_single_component.h"
#include "world/editor/gizmo_system.h"
#include "world/editor/selected_entity_single_component.h"
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

    if (world.valid(selected_entity_single_component.selected_entity)) {
        // TODO: Better looking tool selection. Shortcuts as well.
        ImGui::SetNextWindowDockID(ImGui::GetID("Main"), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Tool")) {
            if (ImGui::Button("Translate")) {
                gizmo_single_component.operation = ImGuizmo::OPERATION::TRANSLATE;
            }
            ImGui::SameLine();
            if (ImGui::Button("Rotate")) {
                gizmo_single_component.operation = ImGuizmo::OPERATION::ROTATE;
            }
            ImGui::SameLine();
            if (ImGui::Button("Scale")) {
                gizmo_single_component.operation = ImGuizmo::OPERATION::SCALE;
            }
            ImGui::SameLine();
            // TODO: Implement bounds tool as well.
            if (ImGui::Button("Bounds")) {
                gizmo_single_component.operation = ImGuizmo::OPERATION::BOUNDS;
            }
        }
        ImGui::End();

        auto& transform_component = world.get<TransformComponent>(selected_entity_single_component.selected_entity);

        glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component.translation);
        transform = transform * glm::mat4_cast(transform_component.rotation);
        transform = glm::scale(transform, transform_component.scale);

        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGuizmo::Manipulate(glm::value_ptr(camera_single_component.view_matrix), glm::value_ptr(camera_single_component.projection_matrix), gizmo_single_component.operation, ImGuizmo::MODE::WORLD, glm::value_ptr(transform));

        transform_component.scale = glm::vec3(glm::length(transform[0]), glm::length(transform[1]), glm::length(transform[2]));
        transform[0] /= transform_component.scale.x;
        transform[1] /= transform_component.scale.y;
        transform[2] /= transform_component.scale.z;
        transform_component.translation = glm::vec3(transform[3].x, transform[3].y, transform[3].z);
        transform_component.rotation = glm::quat(transform);
    }
}

} // namespace hg
