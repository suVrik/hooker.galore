#include "core/ecs/world.h"
#include "core/render/ImGuizmo.h"
#include "world/editor/editor_component.h"
#include "world/editor/gizmo_single_component.h"
#include "world/editor/gizmo_system.h"
#include "world/editor/guid_single_component.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/model_component.h"
#include "world/shared/render/outline_component.h"
#include "world/shared/transform_component.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace hg {

GizmoSystem::GizmoSystem(World& world) noexcept
        : NormalSystem(world) {
    world.set<GizmoSingleComponent>();
}

void GizmoSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& gizmo_single_component = world.ctx<GizmoSingleComponent>();
    auto& guid_single_component = world.ctx<GuidSingleComponent>();
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();
    auto& selected_entity_single_component = world.ctx<SelectedEntitySingleComponent>();

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

    ImGuizmo::Enable(!selected_entity_single_component.is_selecting);

    if (!selected_entity_single_component.selected_entities.empty()) {
        const bool was_using = ImGuizmo::IsUsing();
        const bool is_snapping = normal_input_single_component.is_down(Control::KEY_LCTRL);

        if (selected_entity_single_component.selected_entities.size() == 1) {
            entt::entity selected_entity = selected_entity_single_component.selected_entities[0];
            assert(world.valid(selected_entity));

            auto* transform_component = &world.get<TransformComponent>(selected_entity);

            glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component->translation);
            transform = transform * glm::mat4_cast(transform_component->rotation);
            transform = glm::scale(transform, transform_component->scale);

            float* snap = nullptr;
            if (is_snapping) {
                static float SNAP[3] = { 45.f, 45.f, 45.f };
                if (gizmo_single_component.operation == ImGuizmo::OPERATION::TRANSLATE) {
                    SNAP[0] = 1.f;
                    SNAP[1] = 1.f;
                    SNAP[2] = 1.f;
                } else if (gizmo_single_component.operation == ImGuizmo::OPERATION::SCALE) {
                    if (!was_using) {
                        SNAP[0] = 1.f / transform_component->scale.x;
                        SNAP[1] = 1.f / transform_component->scale.y;
                        SNAP[2] = 1.f / transform_component->scale.z;
                    }
                } else if (gizmo_single_component.operation == ImGuizmo::OPERATION::ROTATE) {
                    SNAP[0] = 45.f;
                }
                snap = SNAP;
            }

            float* local_bounds = nullptr;
            float* bounds_snap = nullptr;
            if (gizmo_single_component.operation == ImGuizmo::OPERATION::BOUNDS) {
                if (auto *model_component = world.try_get<ModelComponent>(selected_entity); model_component != nullptr) {
                    local_bounds = reinterpret_cast<float *>(&model_component->model.bounds);
                }

                if (is_snapping) {
                    static float BOUNDS_SNAP[3];
                    if (!was_using) {
                        BOUNDS_SNAP[0] = 1.f / transform_component->scale.x;
                        BOUNDS_SNAP[1] = 1.f / transform_component->scale.y;
                        BOUNDS_SNAP[2] = 1.f / transform_component->scale.z;
                    }
                    bounds_snap = BOUNDS_SNAP;
                }
            }

            ImGuizmo::MODE mode = gizmo_single_component.is_local_space ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD;
            if (gizmo_single_component.operation == ImGuizmo::SCALE) {
                mode = ImGuizmo::LOCAL;
            }

            ImGuiIO& io = ImGui::GetIO();
            ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
            ImGuizmo::Manipulate(glm::value_ptr(camera_single_component.view_matrix),
                                 glm::value_ptr(camera_single_component.projection_matrix),
                                 gizmo_single_component.operation, mode,
                                 glm::value_ptr(transform), nullptr, snap, local_bounds, bounds_snap);

            if (!was_using && ImGuizmo::IsUsing() && (normal_input_single_component.is_down(Control::KEY_LSHIFT) || normal_input_single_component.is_down(Control::KEY_RSHIFT))) {
                world.reset<OutlineComponent>(selected_entity);

                entt::entity new_entity = world.create();
                world.each(selected_entity, [&](entt::meta_handle component_handle) {
                    world.assign(new_entity, component_handle);
                });

                auto& editor_component = world.get<EditorComponent>(new_entity);
                // TODO: editor_component.name = name_single_component.acquire_unique_name(new_entity, editor_component.name);
                editor_component.guid = guid_single_component.acquire_unique_guid(new_entity);

                selected_entity_single_component.selected_entities.clear();
                selected_entity_single_component.selected_entities.push_back(new_entity);
                world.assign<OutlineComponent>(new_entity);

                transform_component = &world.get<TransformComponent>(new_entity);
            }

            transform_component->scale = glm::vec3(std::max(1e-2f, glm::length(transform[0])), std::max(1e-2f, glm::length(transform[1])), std::max(1e-2f, glm::length(transform[2])));
            transform[0] /= transform_component->scale.x;
            transform[1] /= transform_component->scale.y;
            transform[2] /= transform_component->scale.z;
            transform_component->translation = glm::vec3(transform[3].x, transform[3].y, transform[3].z);
            transform_component->rotation = glm::quat(transform);
        } else {
            if (gizmo_single_component.operation == ImGuizmo::BOUNDS || gizmo_single_component.operation == ImGuizmo::SCALE) {
                return;
            }

            glm::vec3 middle_translation;
            for (entt::entity selected_entity : selected_entity_single_component.selected_entities) {
                auto& object_transform_component = world.get<TransformComponent>(selected_entity);
                middle_translation += object_transform_component.translation;
            }
            middle_translation /= selected_entity_single_component.selected_entities.size();

            if (!ImGuizmo::IsUsing()) {
                gizmo_single_component.transform = glm::translate(glm::mat4(1.f), middle_translation);
            }

            glm::mat4 transform = gizmo_single_component.transform;
            glm::mat4 delta_transform;

            float* snap = nullptr;
            if (is_snapping) {
                static float SNAP[3];
                if (gizmo_single_component.operation == ImGuizmo::OPERATION::TRANSLATE) {
                    SNAP[0] = 1.f;
                    SNAP[1] = 1.f;
                    SNAP[2] = 1.f;
                } else if (gizmo_single_component.operation == ImGuizmo::OPERATION::ROTATE) {
                    SNAP[0] = 45.f;
                }
                snap = SNAP;
            }

            ImGuiIO& io = ImGui::GetIO();
            ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
            ImGuizmo::Manipulate(glm::value_ptr(camera_single_component.view_matrix),
                                 glm::value_ptr(camera_single_component.projection_matrix),
                                 gizmo_single_component.operation, ImGuizmo::MODE::WORLD,
                                 glm::value_ptr(transform), glm::value_ptr(delta_transform), snap);

            if (!was_using && ImGuizmo::IsUsing() && (normal_input_single_component.is_down(Control::KEY_LSHIFT) || normal_input_single_component.is_down(Control::KEY_RSHIFT))) {
                std::vector<entt::entity> old_selected_entities = selected_entity_single_component.selected_entities;
                selected_entity_single_component.clear_selection(world);

                std::vector<entt::entity> copied_entities;

                for (entt::entity original_entity : old_selected_entities) {
                    entt::entity new_entity = world.create();
                    world.each(original_entity, [&](entt::meta_handle component_handle) {
                        world.assign(new_entity, component_handle);
                    });

                    auto& editor_component = world.get<EditorComponent>(new_entity);
                    // TODO: editor_component.name = name_single_component.acquire_unique_name(new_entity, editor_component.name);
                    editor_component.guid = guid_single_component.acquire_unique_guid(new_entity);

                    copied_entities.push_back(new_entity);
                }

                for (entt::entity copy_entity : copied_entities) {
                    selected_entity_single_component.add_to_selection(world, copy_entity);
                }
            }

            gizmo_single_component.transform = transform;

            glm::vec3 delta_translation(delta_transform[3].x, delta_transform[3].y, delta_transform[3].z);
            glm::quat delta_rotation(delta_transform);

            for (entt::entity selected_entity : selected_entity_single_component.selected_entities) {
                auto& object_transform_component = world.get<TransformComponent>(selected_entity);
                if (gizmo_single_component.operation == ImGuizmo::TRANSLATE) {
                    object_transform_component.translation += delta_translation;
                } else {
                    const glm::vec3 origin_translation = object_transform_component.translation - middle_translation;
                    const glm::vec3 new_origin_translation = delta_rotation * origin_translation;
                    object_transform_component.translation = middle_translation + new_origin_translation;
                    object_transform_component.rotation = delta_rotation * object_transform_component.rotation;
                }
            }
        }
    }
}

} // namespace hg
