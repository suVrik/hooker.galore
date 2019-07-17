#include "core/ecs/world.h"
#include "world/editor/editor_component.h"
#include "world/editor/gizmo_single_component.h"
#include "world/editor/gizmo_system.h"
#include "world/editor/history_single_component.h"
#include "world/editor/menu_single_component.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/model_component.h"
#include "world/shared/render/outline_component.h"
#include "world/shared/transform_component.h"

#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

namespace hg {

GizmoSystem::GizmoSystem(World& world) noexcept
        : NormalSystem(world) {
    auto& gizmo_single_component = world.set<GizmoSingleComponent>();
    gizmo_single_component.switch_space = std::make_shared<bool>(false);
    gizmo_single_component.translate_tool = std::make_shared<bool>(false);
    gizmo_single_component.rotate_tool = std::make_shared<bool>(false);
    gizmo_single_component.scale_tool = std::make_shared<bool>(false);
    gizmo_single_component.bounds_tool = std::make_shared<bool>(false);

    auto& menu_single_component = world.ctx<MenuSingleComponent>();
    menu_single_component.items.emplace("5Tools/0Switch space", MenuSingleComponent::MenuItem(gizmo_single_component.switch_space, "~"));
    menu_single_component.items.emplace("5Tools/1Translate", MenuSingleComponent::MenuItem(gizmo_single_component.translate_tool, "1"));
    menu_single_component.items.emplace("5Tools/2Rotate", MenuSingleComponent::MenuItem(gizmo_single_component.rotate_tool, "2"));
    menu_single_component.items.emplace("5Tools/3Scale", MenuSingleComponent::MenuItem(gizmo_single_component.scale_tool, "3"));
    menu_single_component.items.emplace("5Tools/4Bounds", MenuSingleComponent::MenuItem(gizmo_single_component.bounds_tool, "4"));
}

void GizmoSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& gizmo_single_component = world.ctx<GizmoSingleComponent>();
    auto& history_single_component = world.ctx<HistorySingleComponent>();
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();
    auto& selected_entity_single_component = world.ctx<SelectedEntitySingleComponent>();

    if (ImGui::Begin("Tool")) {
        const ImVec2 window_size = ImGui::GetWindowSize();

        auto button = [&](const char* title, Control shortcut, std::shared_ptr<bool>& flag, ImGuizmo::OPERATION operation) {
            const ImGuizmo::OPERATION old_operation = gizmo_single_component.operation;
            if (old_operation == operation) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.8f, 0.1f, 1.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.85f, 0.2f, 1.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.7f, 0.f, 1.f));
            }
            if (ImGui::Button(title, ImVec2(window_size.x / 2 - 15.f, window_size.y / 2 - 33.f)) ||
                normal_input_single_component.is_pressed(shortcut) || *flag) {
                gizmo_single_component.operation = operation;
            }
            *flag = false;
            if (old_operation == operation) {
                ImGui::PopStyleColor(3);
            }
        };

        if (normal_input_single_component.is_pressed(Control::KEY_GRAVE) || *gizmo_single_component.switch_space) {
            gizmo_single_component.is_local_space = !gizmo_single_component.is_local_space;
        }
        *gizmo_single_component.switch_space = false;

        if (ImGui::RadioButton("Local space (~)", gizmo_single_component.is_local_space)) {
            gizmo_single_component.is_local_space = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("World space", !gizmo_single_component.is_local_space)) {
            gizmo_single_component.is_local_space = false;
        }

        button("Translate (1)", Control::KEY_1, gizmo_single_component.translate_tool, ImGuizmo::OPERATION::TRANSLATE);
        ImGui::SameLine();
        button("Rotate (2)", Control::KEY_2, gizmo_single_component.rotate_tool, ImGuizmo::OPERATION::ROTATE);
        button("Scale (3)", Control::KEY_3, gizmo_single_component.scale_tool, ImGuizmo::OPERATION::SCALE);
        ImGui::SameLine();
        button("Bounds (4)", Control::KEY_4, gizmo_single_component.bounds_tool, ImGuizmo::OPERATION::BOUNDS);
    }
    ImGui::End();

    if (!selected_entity_single_component.is_selecting && !selected_entity_single_component.selected_entities.empty()) {
        const bool was_using = ImGuizmo::IsUsing();
        const bool is_snapping = normal_input_single_component.is_down(Control::KEY_CTRL);

        if (selected_entity_single_component.selected_entities.size() == 1) {
            entt::entity selected_entity = selected_entity_single_component.selected_entities[0];
            assert(world.valid(selected_entity));

            auto& editor_component = world.get<EditorComponent>(selected_entity);
            auto& transform_component = world.get<TransformComponent>(selected_entity);

            glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component.translation);
            transform = transform * glm::mat4_cast(transform_component.rotation);
            transform = glm::scale(transform, transform_component.scale);

            float* snap = nullptr;
            if (is_snapping) {
                static float SNAP[3] = { 45.f, 45.f, 45.f };
                if (gizmo_single_component.operation == ImGuizmo::OPERATION::TRANSLATE) {
                    SNAP[0] = 1.f;
                    SNAP[1] = 1.f;
                    SNAP[2] = 1.f;
                } else if (gizmo_single_component.operation == ImGuizmo::OPERATION::SCALE) {
                    if (!was_using) {
                        SNAP[0] = 1.f / transform_component.scale.x;
                        SNAP[1] = 1.f / transform_component.scale.y;
                        SNAP[2] = 1.f / transform_component.scale.z;
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
                        BOUNDS_SNAP[0] = 1.f / transform_component.scale.x;
                        BOUNDS_SNAP[1] = 1.f / transform_component.scale.y;
                        BOUNDS_SNAP[2] = 1.f / transform_component.scale.z;
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

            if (ImGuizmo::IsUsing()) {
                if (!was_using) {
                    if (normal_input_single_component.is_down(Control::KEY_SHIFT)) {
                        auto* change = history_single_component.begin(world, fmt::format("Clone entity \"{}\"", editor_component.name));
                        if (change != nullptr) {
                            selected_entity_single_component.clear_selection(world);

                            entt::entity new_entity = change->create_entity(world, editor_component.name);
                            world.each(selected_entity, [&](entt::meta_handle component_handle) {
                                if (component_handle.type() != entt::resolve<EditorComponent>()) {
                                    entt::meta_any component_copy = world.copy_component(component_handle);
                                    change->assign_component(world, new_entity, component_copy);
                                }
                            });

                            selected_entity_single_component.select_entity(world, new_entity);
                            selected_entity = new_entity;
                        }
                    }

                    HistorySingleComponent::HistoryChange* change = history_single_component.begin_continuous(world, fmt::format("Transform entity \"{}\"", editor_component.name));
                    if (change != nullptr) {
                        entt::meta_any changed_transform_component(world.get<TransformComponent>(selected_entity));
                        change->replace_component(world, selected_entity, changed_transform_component);
                        gizmo_single_component.is_changing = true;
                    }
                }

                if (gizmo_single_component.is_changing) {
                    auto& changed_transform_component = world.get<TransformComponent>(selected_entity);

                    changed_transform_component.translation = glm::vec3(transform[3].x, transform[3].y, transform[3].z);
                    changed_transform_component.scale = glm::vec3(std::max(0.05f, glm::length(transform[0])), std::max(0.05f, glm::length(transform[1])), std::max(0.05f, glm::length(transform[2])));
                    transform[0] /= glm::length(transform[0]);
                    transform[1] /= glm::length(transform[1]);
                    transform[2] /= glm::length(transform[2]);
                    changed_transform_component.rotation = glm::quat(transform);
                }
            }
        } else {
            if (gizmo_single_component.operation == ImGuizmo::BOUNDS || gizmo_single_component.operation == ImGuizmo::SCALE) {
                return;
            }

            glm::vec3 middle_translation(0.f, 0.f, 0.f);
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

            if (ImGuizmo::IsUsing()) {
                if (!was_using) {
                    if (normal_input_single_component.is_down(Control::KEY_SHIFT)) {
                        auto* change = history_single_component.begin(world, "Clone entities");
                        if (change != nullptr) {
                            std::vector<entt::entity> old_selected_entities = selected_entity_single_component.selected_entities;
                            std::vector<entt::entity> new_selected_entities;

                            selected_entity_single_component.clear_selection(world);

                            for (entt::entity original_entity : old_selected_entities) {
                                auto& original_editor_component = world.get<EditorComponent>(original_entity);

                                entt::entity new_entity = change->create_entity(world, original_editor_component.name);
                                world.each(original_entity, [&](entt::meta_handle component_handle) {
                                    if (component_handle.type() != entt::resolve<EditorComponent>()) {
                                        entt::meta_any component_copy = world.copy_component(component_handle);
                                        change->assign_component(world, new_entity, component_copy);
                                    }
                                });

                                new_selected_entities.push_back(new_entity);
                            }

                            for (entt::entity copy_entity : new_selected_entities) {
                                selected_entity_single_component.add_to_selection(world, copy_entity);
                            }
                        }
                    }

                    HistorySingleComponent::HistoryChange* change = history_single_component.begin_continuous(world, "Transform entities");
                    if (change != nullptr) {
                        for (entt::entity entity : selected_entity_single_component.selected_entities) {
                            entt::meta_any transform_component(world.get<TransformComponent>(entity));
                            change->replace_component(world, entity, transform_component);
                        }
                        gizmo_single_component.is_changing = true;
                    }
                }

                if (gizmo_single_component.is_changing) {
                    gizmo_single_component.transform = transform;

                    glm::vec3 delta_translation(delta_transform[3].x, delta_transform[3].y, delta_transform[3].z);
                    glm::quat delta_rotation(delta_transform);

                    for (entt::entity selected_entity : selected_entity_single_component.selected_entities) {
                        auto& changed_transform_component = world.get<TransformComponent>(selected_entity);
                        if (gizmo_single_component.operation == ImGuizmo::TRANSLATE) {
                            changed_transform_component.translation += delta_translation;
                        } else {
                            const glm::vec3 origin_translation = changed_transform_component.translation - middle_translation;
                            const glm::vec3 new_origin_translation = delta_rotation * origin_translation;
                            changed_transform_component.translation = middle_translation + new_origin_translation;
                            changed_transform_component.rotation = delta_rotation * changed_transform_component.rotation;
                        }
                    }
                }
            }
        }
    }

    if (!ImGuizmo::IsUsing() && gizmo_single_component.is_changing) {
        history_single_component.end_continuous();
        gizmo_single_component.is_changing = false;
    }
}

} // namespace hg
