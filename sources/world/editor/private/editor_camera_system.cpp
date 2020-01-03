#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/editor/editor_camera_component.h"
#include "world/editor/editor_camera_system.h"
#include "world/editor/editor_menu_single_component.h"
#include "world/editor/editor_selection_single_component.h"
#include "world/editor/editor_tags.h"
#include "world/render/camera_single_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/transform_component.h"

#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_timer.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/vector_query.hpp>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(EditorCameraSystem),
    TAGS(editor),
    BEFORE("CameraSystem"),
    AFTER("EditorMenuSystem", "WindowSystem", "EditorSelectionSystem")
)

EditorCameraSystem::EditorCameraSystem(World& world)
        : NormalSystem(world) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    camera_single_component.active_camera = world.create();

    auto& editor_camera_component = world.assign<EditorCameraComponent>(camera_single_component.active_camera);
    editor_camera_component.yaw = -glm::pi<float>() * 3.f / 4.f;
    editor_camera_component.pitch = glm::pi<float>() / 4.f;

    auto& transform_component = world.assign<TransformComponent>(camera_single_component.active_camera);
    transform_component.translation.x = 10.f;
    transform_component.translation.y = 10.f;
    transform_component.translation.z = 10.f;
    transform_component.rotation = glm::rotate(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), editor_camera_component.yaw, glm::vec3(0.f, 1.f, 0.f)), editor_camera_component.pitch, glm::vec3(1.f, 0.f, 0.f));

    editor_camera_component.reset_camera_position = std::make_shared<bool>();

    auto& editor_menu_single_component = world.ctx<EditorMenuSingleComponent>();
    editor_menu_single_component.add_item("2View/Reset camera position", editor_camera_component.reset_camera_position, "Home");
}

void EditorCameraSystem::update(float elapsed_time) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& editor_selection_single_component = world.ctx<EditorSelectionSingleComponent>();
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();

    if (world.valid(camera_single_component.active_camera) && world.has<EditorCameraComponent, TransformComponent>(camera_single_component.active_camera)) {
        auto [editor_camera_component, transform_component] = world.get<EditorCameraComponent, TransformComponent>(camera_single_component.active_camera);

        if (normal_input_single_component.is_pressed(Control::KEY_HOME) || *editor_camera_component.reset_camera_position) {
            *editor_camera_component.reset_camera_position = false;

            editor_camera_component.yaw = -glm::pi<float>() * 3.f / 4.f;
            editor_camera_component.pitch = glm::pi<float>() / 4.f;

            transform_component.translation.x = 10.f;
            transform_component.translation.y = 10.f;
            transform_component.translation.z = 10.f;
            transform_component.rotation = glm::rotate(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), editor_camera_component.yaw, glm::vec3(0.f, 1.f, 0.f)), editor_camera_component.pitch, glm::vec3(1.f, 0.f, 0.f));
        }

        float speed = CAMERA_SPEED * elapsed_time;
        if (normal_input_single_component.is_down(Control::KEY_SHIFT)) {
            speed *= CAMERA_SPEED_INCREASE_FACTOR;
        }
        if (normal_input_single_component.get_mouse_wheel() != 0) {
            speed *= std::abs(normal_input_single_component.get_mouse_wheel()) * CAMERA_SPEED_WHEEL_FACTOR;
        }

        if (normal_input_single_component.is_pressed(Control::BUTTON_LEFT)) {
            editor_camera_component.press_time = SDL_GetTicks();
        }

        const bool attached_mode = normal_input_single_component.is_down(Control::KEY_ALT) || normal_input_single_component.is_down(Control::BUTTON_MIDDLE);
        if (!editor_selection_single_component.selected_entities.empty() && (attached_mode || normal_input_single_component.get_mouse_wheel() != 0)) {
            update_attached_camera(camera_single_component, normal_input_single_component, editor_selection_single_component, editor_camera_component, transform_component, speed);
        } else {
            update_free_camera(camera_single_component, normal_input_single_component, editor_camera_component, transform_component, speed);
        }
    }
}

void EditorCameraSystem::update_attached_camera(CameraSingleComponent& camera_single_component, 
                                                NormalInputSingleComponent& normal_input_single_component,
                                                EditorSelectionSingleComponent& editor_selection_single_component,
                                                EditorCameraComponent& editor_camera_component,
                                                TransformComponent& transform_component,
                                                const float speed) const {
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

    const float distance_to_object = glm::distance(transform_component.translation, middle_translation);
    if (distance_to_object > glm::epsilon<float>()) {
        float delta_yaw = 0.f;
        float delta_pitch = 0.f;

        const bool is_dragging_camera = normal_input_single_component.is_down(Control::BUTTON_LEFT) && SDL_GetTicks() - editor_camera_component.press_time >= 50;
        if (is_dragging_camera || normal_input_single_component.is_down(Control::BUTTON_MIDDLE)) {
            SDL_SetRelativeMouseMode(SDL_TRUE);

            const float previous_yaw = editor_camera_component.yaw;
            const float previous_pitch = editor_camera_component.pitch;
            editor_camera_component.yaw += normal_input_single_component.get_delta_mouse_x() * MOUSE_SENSITIVITY;
            editor_camera_component.pitch += normal_input_single_component.get_delta_mouse_y() * MOUSE_SENSITIVITY;
            editor_camera_component.pitch = glm::clamp(editor_camera_component.pitch, -glm::half_pi<float>() * 15.f / 16.f, glm::half_pi<float>() * 15.f / 16.f);
            delta_yaw = editor_camera_component.yaw - previous_yaw;
            delta_pitch = editor_camera_component.pitch - previous_pitch;
        } else {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }

        const glm::vec3 vector1 = transform_component.translation - middle_translation;
        const glm::vec3 vector2 = glm::cross(vector1, glm::vec3(0.f, 1.f, 0.f));

        float distance = glm::length(vector1);
        if ((!normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_down(Control::KEY_W)) || 
            normal_input_single_component.get_mouse_wheel() > 0) {
            distance = std::max(distance - speed, 0.1f);
        }
        if ((!normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_down(Control::KEY_S)) || 
            normal_input_single_component.get_mouse_wheel() < 0) {
            distance = distance + speed;
        }

        const glm::quat delta_rotation = glm::rotate(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), delta_yaw, glm::vec3(0.f, 1.f, 0.f)), delta_pitch, vector2);
        transform_component.rotation = glm::rotate(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), editor_camera_component.yaw, glm::vec3(0.f, 1.f, 0.f)), editor_camera_component.pitch, glm::vec3(1.f, 0.f, 0.f));
        transform_component.translation = middle_translation + delta_rotation * glm::normalize(vector1) * distance;
    }
}

void EditorCameraSystem::update_free_camera(CameraSingleComponent& camera_single_component, 
                                            NormalInputSingleComponent& normal_input_single_component,
                                            EditorCameraComponent& editor_camera_component, 
                                            TransformComponent& transform_component, 
                                            const float speed) const {
    float delta_x = 0.f;
    if (!normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_down(Control::KEY_D)) {
        delta_x = speed;
    }
    if (!normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_down(Control::KEY_A)) {
        delta_x = -speed;
    }

    float delta_y = 0.f;
    if (!normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_down(Control::KEY_SPACE)) {
        delta_y = speed;
    }
    if (!normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_down(Control::KEY_C)) {
        delta_y = -speed;
    }

    float delta_z = 0.f;
    if ((!normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_down(Control::KEY_W)) ||
        normal_input_single_component.get_mouse_wheel() > 0) {
        delta_z = speed;
    }
    if ((!normal_input_single_component.is_down(Control::KEY_CTRL) && normal_input_single_component.is_down(Control::KEY_S)) ||
        normal_input_single_component.get_mouse_wheel() < 0) {
        delta_z = -speed;
    }

    const bool is_dragging_camera = normal_input_single_component.is_down(Control::BUTTON_LEFT) && SDL_GetTicks() - editor_camera_component.press_time >= 50;
    if (is_dragging_camera || normal_input_single_component.is_down(Control::BUTTON_MIDDLE)) {
        SDL_SetRelativeMouseMode(SDL_TRUE);

        editor_camera_component.yaw += normal_input_single_component.get_delta_mouse_x() * MOUSE_SENSITIVITY;
        editor_camera_component.pitch += normal_input_single_component.get_delta_mouse_y() * MOUSE_SENSITIVITY;
        editor_camera_component.pitch = glm::clamp(editor_camera_component.pitch, -glm::half_pi<float>() * 15.f / 16.f, glm::half_pi<float>() * 15.f / 16.f);
    } else {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    transform_component.rotation = glm::rotate(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), editor_camera_component.yaw, glm::vec3(0.f, 1.f, 0.f)), editor_camera_component.pitch, glm::vec3(1.f, 0.f, 0.f));
    transform_component.translation += transform_component.rotation * glm::vec3(delta_x, 0.f, 0.f) + glm::vec3(0.f, delta_y, 0.f) + transform_component.rotation * glm::vec3(0.f, 0.f, delta_z);
}

} // namespace hg
