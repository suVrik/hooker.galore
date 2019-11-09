#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/editor/editor_gizmo_single_component.h"
#include "world/editor/editor_gizmo_system.h"
#include "world/editor/editor_history_single_component.h"
#include "world/editor/editor_menu_single_component.h"
#include "world/editor/editor_selection_single_component.h"
#include "world/shared/name_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/model_component.h"
#include "world/shared/render/outline_component.h"
#include "world/shared/transform_component.h"

#include <ImGuizmo.h>
#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(EditorGizmoSystem),
    REQUIRE("editor"),
    BEFORE("ImguiPassSystem", "GeometryPassSystem"),
    AFTER("EditorMenuSystem", "WindowSystem", "ImguiFetchSystem", "CameraSystem", "EditorSelectionSystem")
)

EditorGizmoSystem::EditorGizmoSystem(World& world)
        : NormalSystem(world) {
    auto& editor_gizmo_single_component = world.set<EditorGizmoSingleComponent>();
    editor_gizmo_single_component.switch_space   = std::make_shared<bool>(false);
    editor_gizmo_single_component.translate_tool = std::make_shared<bool>(false);
    editor_gizmo_single_component.rotate_tool    = std::make_shared<bool>(false);
    editor_gizmo_single_component.scale_tool     = std::make_shared<bool>(false);
    editor_gizmo_single_component.bounds_tool    = std::make_shared<bool>(false);

    auto& editor_menu_single_component = world.ctx<EditorMenuSingleComponent>();
    editor_menu_single_component.add_item("5Tools/0Switch space", editor_gizmo_single_component.switch_space,   "~");
    editor_menu_single_component.add_item("5Tools/1Translate",    editor_gizmo_single_component.translate_tool, "1");
    editor_menu_single_component.add_item("5Tools/2Rotate",       editor_gizmo_single_component.rotate_tool,    "2");
    editor_menu_single_component.add_item("5Tools/3Scale",        editor_gizmo_single_component.scale_tool,     "3");
    editor_menu_single_component.add_item("5Tools/4Bounds",       editor_gizmo_single_component.bounds_tool,    "4");
}

void EditorGizmoSystem::update(float /*elapsed_time*/) {
    auto& editor_gizmo_single_component = world.ctx<EditorGizmoSingleComponent>();
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();

    show_gizmo_window(editor_gizmo_single_component, normal_input_single_component);
    process_gizmo(editor_gizmo_single_component, normal_input_single_component);
}

void EditorGizmoSystem::show_gizmo_window(EditorGizmoSingleComponent& editor_gizmo_single_component, 
                                          NormalInputSingleComponent& normal_input_single_component) const {
    if (ImGui::Begin("Gizmo", nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
        const ImVec2 window_size = ImGui::GetWindowSize();

        auto button = [&](const char* title, Control shortcut, std::shared_ptr<bool>& flag, ImGuizmo::OPERATION operation) {
            const ImGuizmo::OPERATION old_operation = editor_gizmo_single_component.operation;
            if (old_operation == operation) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.8f, 0.1f, 1.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.85f, 0.2f, 1.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.7f, 0.f, 1.f));
            }
            if (ImGui::Button(title, ImVec2(window_size.x / 2 - 15.f, window_size.y / 2 - 33.f)) ||
                normal_input_single_component.is_pressed(shortcut) || *flag) {
                editor_gizmo_single_component.operation = operation;
            }
            *flag = false;
            if (old_operation == operation) {
                ImGui::PopStyleColor(3);
            }
        };

        if (normal_input_single_component.is_pressed(Control::KEY_GRAVE) || *editor_gizmo_single_component.switch_space) {
            *editor_gizmo_single_component.switch_space = false;
            editor_gizmo_single_component.is_local_space = !editor_gizmo_single_component.is_local_space;
        }

        if (ImGui::RadioButton("Local space (~)", editor_gizmo_single_component.is_local_space)) {
            editor_gizmo_single_component.is_local_space = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("World space", !editor_gizmo_single_component.is_local_space)) {
            editor_gizmo_single_component.is_local_space = false;
        }

        button("Translate (1)", Control::KEY_1, editor_gizmo_single_component.translate_tool, ImGuizmo::OPERATION::TRANSLATE);
        ImGui::SameLine();
        button("Rotate (2)", Control::KEY_2, editor_gizmo_single_component.rotate_tool, ImGuizmo::OPERATION::ROTATE);
        button("Scale (3)", Control::KEY_3, editor_gizmo_single_component.scale_tool, ImGuizmo::OPERATION::SCALE);
        ImGui::SameLine();
        button("Bounds (4)", Control::KEY_4, editor_gizmo_single_component.bounds_tool, ImGuizmo::OPERATION::BOUNDS);
    }
    ImGui::End();
}

void EditorGizmoSystem::process_gizmo(EditorGizmoSingleComponent& editor_gizmo_single_component, 
                                      NormalInputSingleComponent& normal_input_single_component) const {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& editor_history_single_component = world.ctx<EditorHistorySingleComponent>();
    auto& editor_selection_single_component = world.ctx<EditorSelectionSingleComponent>();

    bool is_disabled = false;
    if (!ImGuizmo::IsUsing()) {
        is_disabled = normal_input_single_component.is_down(Control::BUTTON_LEFT) || 
                      normal_input_single_component.is_down(Control::BUTTON_RIGHT) || 
                      normal_input_single_component.is_down(Control::BUTTON_MIDDLE);
    }

    if (!is_disabled) {
        if (editor_selection_single_component.selected_entities.size() == 1) {
            process_single_entity(camera_single_component, editor_gizmo_single_component, editor_history_single_component, normal_input_single_component, editor_selection_single_component);
        } else if (editor_selection_single_component.selected_entities.size() > 1) {
            process_multiple_entities(camera_single_component, editor_gizmo_single_component, editor_history_single_component, normal_input_single_component, editor_selection_single_component);
        }
    }

    if (!ImGuizmo::IsUsing() && editor_gizmo_single_component.is_changing) {
        editor_history_single_component.end_continuous();
        editor_gizmo_single_component.is_changing = false;
    }
}

void EditorGizmoSystem::process_single_entity(CameraSingleComponent& camera_single_component, 
                                              EditorGizmoSingleComponent& editor_gizmo_single_component,
                                              EditorHistorySingleComponent& editor_history_single_component,
                                              NormalInputSingleComponent& normal_input_single_component,
                                              EditorSelectionSingleComponent& editor_selection_single_component) const {
    const bool was_using = ImGuizmo::IsUsing();
    const bool is_snapping = normal_input_single_component.is_down(Control::KEY_CTRL);

    entt::entity selected_entity = editor_selection_single_component.selected_entities[0];
    assert(world.valid(selected_entity));
    assert(world.has<NameComponent>(selected_entity));

    // Avoid reference, because `create_entity` changes the `NameComponent` pool and `original_editor_component` reference becomes corrupted.
    const NameComponent name_component = world.get<NameComponent>(selected_entity);

    const auto* const transform_component = world.try_get<TransformComponent>(selected_entity);
    if (transform_component != nullptr) {
        glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component->translation);
        transform = transform * glm::mat4_cast(transform_component->rotation);
        transform = glm::scale(transform, transform_component->scale);

        float* snap = nullptr;
        if (is_snapping) {
            static float SNAP[3] = { 45.f, 45.f, 45.f };
            if (editor_gizmo_single_component.operation == ImGuizmo::OPERATION::TRANSLATE) {
                SNAP[0] = 1.f;
                SNAP[1] = 1.f;
                SNAP[2] = 1.f;
            } else if (editor_gizmo_single_component.operation == ImGuizmo::OPERATION::SCALE) {
                if (!was_using) {
                    SNAP[0] = 1.f / transform_component->scale.x;
                    SNAP[1] = 1.f / transform_component->scale.y;
                    SNAP[2] = 1.f / transform_component->scale.z;
                }
            } else if (editor_gizmo_single_component.operation == ImGuizmo::OPERATION::ROTATE) {
                SNAP[0] = 45.f;
            }
            snap = SNAP;
        }

        float* local_bounds = nullptr;
        float* bounds_snap = nullptr;
        if (editor_gizmo_single_component.operation == ImGuizmo::OPERATION::BOUNDS) {
            if (auto* const model_component = world.try_get<ModelComponent>(selected_entity); model_component != nullptr) {
                local_bounds = reinterpret_cast<float*>(&model_component->model.bounds);
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

        ImGuizmo::MODE mode = editor_gizmo_single_component.is_local_space ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD;
        if (editor_gizmo_single_component.operation == ImGuizmo::SCALE) {
            // Scaling in world space is not available.
            mode = ImGuizmo::LOCAL;
        }

        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGuizmo::Manipulate(glm::value_ptr(camera_single_component.view_matrix), glm::value_ptr(camera_single_component.projection_matrix),
                             editor_gizmo_single_component.operation, mode, glm::value_ptr(transform), nullptr, snap, local_bounds, bounds_snap);

        if (ImGuizmo::IsUsing()) {
            if (!was_using) {
                if (normal_input_single_component.is_down(Control::KEY_SHIFT)) {
                    auto* change = editor_history_single_component.begin(world, fmt::format("Clone entity \"{}\"", name_component.name));
                    if (change != nullptr) {
                        editor_selection_single_component.clear_selection(world);

                        const entt::entity new_entity = change->create_entity(world, name_component.name);
                        world.each_editable_component(selected_entity, [&](const entt::meta_handle component) {
                            if (component.type() != entt::resolve<NameComponent>()) {
                                change->assign_component_copy(world, new_entity, component);
                            }
                        });

                        editor_selection_single_component.select_entity(world, new_entity);
                        selected_entity = new_entity;
                    }
                }

                EditorHistorySingleComponent::HistoryChange* change = editor_history_single_component.begin_continuous(world, fmt::format("Transform entity \"{}\"", name_component.name));
                if (change != nullptr) {
                    change->replace_component_copy(world, selected_entity, world.get<TransformComponent>(selected_entity));
                    editor_gizmo_single_component.is_changing = true;
                }
            }

            if (editor_gizmo_single_component.is_changing) {
                assert(editor_history_single_component.is_continuous);

                // Avoid assigning a value to itself.
                TransformComponent changed_transform_component = world.get<TransformComponent>(selected_entity);

                changed_transform_component.translation = glm::vec3(transform[3].x, transform[3].y, transform[3].z);
                changed_transform_component.scale = glm::vec3(std::max(0.05f, glm::length(transform[0])), std::max(0.05f, glm::length(transform[1])), std::max(0.05f, glm::length(transform[2])));
                transform[0] /= glm::length(transform[0]);
                transform[1] /= glm::length(transform[1]);
                transform[2] /= glm::length(transform[2]);
                changed_transform_component.rotation = glm::quat(transform);

                // Notify all systems watching for TransformComponent.
                world.replace<TransformComponent>(selected_entity, changed_transform_component);
            }
        }
    }
}

void EditorGizmoSystem::process_multiple_entities(CameraSingleComponent& camera_single_component, 
                                                  EditorGizmoSingleComponent& editor_gizmo_single_component, 
                                                  EditorHistorySingleComponent& editor_history_single_component,
                                                  NormalInputSingleComponent& normal_input_single_component, 
                                                  EditorSelectionSingleComponent& editor_selection_single_component) const {
    if (editor_gizmo_single_component.operation == ImGuizmo::BOUNDS || editor_gizmo_single_component.operation == ImGuizmo::SCALE) {
        return;
    }

    const bool was_using = ImGuizmo::IsUsing();
    const bool is_snapping = normal_input_single_component.is_down(Control::KEY_CTRL);

    glm::vec3 middle_translation(0.f, 0.f, 0.f);
    size_t selected_entities_with_transform_component = 0;
    for (entt::entity selected_entity : editor_selection_single_component.selected_entities) {
        auto* const object_transform_component = world.try_get<TransformComponent>(selected_entity);
        if (object_transform_component != nullptr) {
            middle_translation += object_transform_component->translation;
            selected_entities_with_transform_component++;
        }
    }
    if (selected_entities_with_transform_component > 0) {
        middle_translation /= selected_entities_with_transform_component;
    }

    if (!ImGuizmo::IsUsing()) {
        editor_gizmo_single_component.transform = glm::translate(glm::mat4(1.f), middle_translation);
    }

    glm::mat4 transform = editor_gizmo_single_component.transform;
    glm::mat4 delta_transform;

    float* snap = nullptr;
    if (is_snapping) {
        static float SNAP[3];
        if (editor_gizmo_single_component.operation == ImGuizmo::OPERATION::TRANSLATE) {
            SNAP[0] = 1.f;
            SNAP[1] = 1.f;
            SNAP[2] = 1.f;
        } else if (editor_gizmo_single_component.operation == ImGuizmo::OPERATION::ROTATE) {
            SNAP[0] = 45.f;
        }
        snap = SNAP;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Manipulate(glm::value_ptr(camera_single_component.view_matrix), glm::value_ptr(camera_single_component.projection_matrix),
                         editor_gizmo_single_component.operation, ImGuizmo::MODE::WORLD, glm::value_ptr(transform), glm::value_ptr(delta_transform), snap);

    if (ImGuizmo::IsUsing()) {
        if (!was_using) {
            if (normal_input_single_component.is_down(Control::KEY_SHIFT)) {
                auto* change = editor_history_single_component.begin(world, "Clone entities");
                if (change != nullptr) {
                    std::vector<entt::entity> old_selected_entities = editor_selection_single_component.selected_entities;
                    std::vector<entt::entity> new_selected_entities;

                    editor_selection_single_component.clear_selection(world);

                    for (const entt::entity original_entity : old_selected_entities) {
                        // Avoid reference, because `create_entity` changes the `NameComponent` pool and `original_editor_component` reference becomes corrupted.
                        const NameComponent original_editor_component = world.get<NameComponent>(original_entity);

                        const entt::entity new_entity = change->create_entity(world, original_editor_component.name);
                        world.each_editable_component(original_entity, [&](const entt::meta_handle component) {
                            if (component.type() != entt::resolve<NameComponent>()) {
                                change->assign_component_copy(world, new_entity, component);
                            }
                        });

                        new_selected_entities.push_back(new_entity);
                    }

                    for (const entt::entity copy_entity : new_selected_entities) {
                        editor_selection_single_component.add_to_selection(world, copy_entity);
                    }
                }
            }

            EditorHistorySingleComponent::HistoryChange* change = editor_history_single_component.begin_continuous(world, "Transform entities");
            if (change != nullptr) {
                for (const entt::entity entity : editor_selection_single_component.selected_entities) {
                    auto* const transform_component = world.try_get<TransformComponent>(entity);
                    if (transform_component != nullptr) {
                        change->replace_component_copy(world, entity, *transform_component);
                    }
                }
                editor_gizmo_single_component.is_changing = true;
            }
        }

        if (editor_gizmo_single_component.is_changing) {
            assert(editor_history_single_component.is_continuous);

            editor_gizmo_single_component.transform = transform;

            glm::vec3 delta_translation(delta_transform[3].x, delta_transform[3].y, delta_transform[3].z);
            glm::quat delta_rotation(delta_transform);

            for (const entt::entity selected_entity : editor_selection_single_component.selected_entities) {
                auto* const changed_transform_component = world.try_get<TransformComponent>(selected_entity);
                if (changed_transform_component != nullptr) {
                    if (editor_gizmo_single_component.operation == ImGuizmo::TRANSLATE) {
                        changed_transform_component->translation += delta_translation;
                    } else {
                        const glm::vec3 origin_translation = changed_transform_component->translation - middle_translation;
                        const glm::vec3 new_origin_translation = delta_rotation * origin_translation;
                        changed_transform_component->translation = middle_translation + new_origin_translation;
                        changed_transform_component->rotation = delta_rotation * changed_transform_component->rotation;
                    }

                    // Notify all systems watching for TransformComponent.
                    world.replace<TransformComponent>(selected_entity, *changed_transform_component);
                }
            }
        }
    }
}

} // namespace hg
