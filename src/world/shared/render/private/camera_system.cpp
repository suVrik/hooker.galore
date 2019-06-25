#include "core/ecs/world.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/camera_system.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

namespace hg {

CameraSystem::CameraSystem(World& world) noexcept
        : NormalSystem(world) {
}

void CameraSystem::update(float /*elapsed_time*/) {
    constexpr float FOV    = glm::radians(60.f);
    constexpr float Z_NEAR = 0.1f;
    constexpr float Z_FAR  = 1000.f;

    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (world.valid(camera_single_component.active_camera)) {
        auto& transform_component = world.get<TransformComponent>(camera_single_component.active_camera);

        const glm::vec3 forward = transform_component.rotation * glm::vec3(0.f, 0.f, 1.f);
        const glm::vec3 up = transform_component.rotation * glm::vec3(0.f, 1.f, 0.f);

        camera_single_component.translation = transform_component.translation;
        camera_single_component.rotation = transform_component.rotation;
        camera_single_component.view_matrix = glm::lookAtLH(transform_component.translation, transform_component.translation + forward, up);
        camera_single_component.projection_matrix = glm::perspectiveFovLH(FOV, float(window_single_component.width), float(window_single_component.height), Z_NEAR, Z_FAR);
        camera_single_component.view_projection_matrix = camera_single_component.projection_matrix * camera_single_component.view_matrix;
        camera_single_component.inverse_view_matrix = glm::inverse(camera_single_component.view_matrix);
        camera_single_component.inverse_projection_matrix = glm::inverse(camera_single_component.projection_matrix);
        camera_single_component.inverse_view_projection_matrix = glm::inverse(camera_single_component.view_projection_matrix);
    }
}

} // namespace hg
