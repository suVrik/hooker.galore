#include "core/ecs/world.h"
#include "world/editor/editor_camera_component.h"
#include "world/editor/editor_camera_system.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/transform_component.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/vector_query.hpp>

namespace hg {

EditorCameraSystem::EditorCameraSystem(World& world) noexcept
        : NormalSystem(world) {
    auto& camera_single_component = world.set<CameraSingleComponent>();
    camera_single_component.active_camera = world.create();

    // TODO: Load all these settings from file.
    auto& editor_camera_component = world.assign<EditorCameraComponent>(camera_single_component.active_camera);
    editor_camera_component.yaw = -glm::pi<float>() * 3.f / 4.f;
    editor_camera_component.pitch = glm::pi<float>() / 4.f;

    auto& transform_component = world.assign<TransformComponent>(camera_single_component.active_camera);
    transform_component.translation.x = 10.f;
    transform_component.translation.y = 10.f;
    transform_component.translation.z = 10.f;
    transform_component.rotation = glm::rotate(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), editor_camera_component.yaw, glm::vec3(0.f, 1.f, 0.f)), editor_camera_component.pitch, glm::vec3(1.f, 0.f, 0.f));
}

void EditorCameraSystem::update(float elapsed_time) {
    constexpr float CAMERA_SPEED                 = 5.f;
    constexpr float CAMERA_SPEED_INCREASE_FACTOR = 3.f;
    constexpr float CAMERA_SPEED_DECREASE_FACTOR = 0.25f;
    constexpr float MOUSE_SENSITIVITY            = 0.002f;

    assert(world.after("WindowSystem") && world.before("CameraSystem"));

    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();
    auto& selected_entity_single_component = world.ctx<SelectedEntitySingleComponent>();

    if (world.valid(camera_single_component.active_camera) && world.has<EditorCameraComponent, TransformComponent>(camera_single_component.active_camera)) {
        auto [editor_camera_component, transform_component] = world.get<EditorCameraComponent, TransformComponent>(camera_single_component.active_camera);

        float speed = CAMERA_SPEED * elapsed_time;
        if (normal_input_single_component.is_down(Control::KEY_LSHIFT)) {
            speed *= CAMERA_SPEED_INCREASE_FACTOR;
        }
        if (normal_input_single_component.is_down(Control::KEY_LCTRL)) {
            speed *= CAMERA_SPEED_DECREASE_FACTOR;
        }

        if ((normal_input_single_component.is_down(Control::KEY_LALT) || normal_input_single_component.is_down(Control::BUTTON_MIDDLE)) && world.valid(selected_entity_single_component.selected_entity)) {
            auto& object_transform_component = world.get<TransformComponent>(selected_entity_single_component.selected_entity);

            const float distance_to_object = glm::distance(transform_component.translation, object_transform_component.translation);
            if (distance_to_object > glm::epsilon<float>()) {
                float delta_yaw = 0.f;
                float delta_pitch = 0.f;

                if (normal_input_single_component.is_down(Control::BUTTON_RIGHT) || normal_input_single_component.is_down(Control::BUTTON_MIDDLE)) {
                    const float previous_yaw = editor_camera_component.yaw;
                    const float previous_pitch = editor_camera_component.pitch;
                    editor_camera_component.yaw -= normal_input_single_component.get_delta_mouse_x() * MOUSE_SENSITIVITY;
                    editor_camera_component.pitch += normal_input_single_component.get_delta_mouse_y() * MOUSE_SENSITIVITY;
                    editor_camera_component.pitch = glm::clamp(editor_camera_component.pitch, -glm::half_pi<float>() * 15.f / 16.f, glm::half_pi<float>() * 15.f / 16.f);
                    delta_yaw = editor_camera_component.yaw - previous_yaw;
                    delta_pitch = editor_camera_component.pitch - previous_pitch;
                }

                const glm::vec3 vector1 = transform_component.translation - object_transform_component.translation;
                const glm::vec3 vector2 = glm::cross(vector1, glm::vec3(0.f, 1.f, 0.f));

                float distance = glm::length(vector1);
                if (normal_input_single_component.is_down(Control::KEY_W)) {
                    distance = std::max(distance - speed, 0.1f);
                }
                if (normal_input_single_component.is_down(Control::KEY_S)) {
                    distance = distance + speed;
                }

                const glm::quat delta_rotation = glm::rotate(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), delta_yaw, glm::vec3(0.f, 1.f, 0.f)), delta_pitch, vector2);
                transform_component.rotation = glm::rotate(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), editor_camera_component.yaw, glm::vec3(0.f, 1.f, 0.f)), editor_camera_component.pitch, glm::vec3(1.f, 0.f, 0.f));
                transform_component.translation = object_transform_component.translation + delta_rotation * glm::normalize(vector1) * distance;
            }
        } else {
            float delta_x = 0.f;
            if (normal_input_single_component.is_down(Control::KEY_A)) {
                delta_x = speed;
            }
            if (normal_input_single_component.is_down(Control::KEY_D)) {
                delta_x = -speed;
            }

            float delta_y = 0.f;
            if (normal_input_single_component.is_down(Control::KEY_SPACE)) {
                delta_y = speed;
            }
            if (normal_input_single_component.is_down(Control::KEY_C)) {
                delta_y = -speed;
            }

            float delta_z = 0.f;
            if (normal_input_single_component.is_down(Control::KEY_W)) {
                delta_z = speed;
            }
            if (normal_input_single_component.is_down(Control::KEY_S)) {
                delta_z = -speed;
            }

            if (normal_input_single_component.is_down(Control::BUTTON_RIGHT)) {
                editor_camera_component.yaw -= normal_input_single_component.get_delta_mouse_x() * MOUSE_SENSITIVITY;
                editor_camera_component.pitch += normal_input_single_component.get_delta_mouse_y() * MOUSE_SENSITIVITY;
                editor_camera_component.pitch = glm::clamp(editor_camera_component.pitch, -glm::half_pi<float>() * 15.f / 16.f, glm::half_pi<float>() * 15.f / 16.f);
            }

            transform_component.rotation = glm::rotate(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), editor_camera_component.yaw, glm::vec3(0.f, 1.f, 0.f)), editor_camera_component.pitch, glm::vec3(1.f, 0.f, 0.f));
            transform_component.translation += transform_component.rotation * glm::vec3(delta_x, 0.f, 0.f) + glm::vec3(0.f, delta_y, 0.f) + transform_component.rotation * glm::vec3(0.f, 0.f, delta_z);
        }
    }
}

} // namespace hg
