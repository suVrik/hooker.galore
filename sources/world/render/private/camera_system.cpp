#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/render/camera_single_component.h"
#include "world/render/camera_system.h"
#include "world/render/render_tags.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(CameraSystem),
    TAGS(render),
    AFTER("WindowSystem")
)

CameraSystem::CameraSystem(World& world)
        : NormalSystem(world) {
}

void CameraSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (world.valid(camera_single_component.active_camera)) {
        auto& transform_component = world.get<TransformComponent>(camera_single_component.active_camera);

        const glm::vec3 forward = transform_component.rotation * glm::vec3(0.f, 0.f, 1.f);
        const glm::vec3 up = transform_component.rotation * glm::vec3(0.f, 1.f, 0.f);

        camera_single_component.translation = transform_component.translation;
        camera_single_component.rotation = transform_component.rotation;
        camera_single_component.view_matrix = glm::lookAtLH(transform_component.translation, transform_component.translation + forward, up);
        camera_single_component.projection_matrix = glm::perspectiveFovLH(camera_single_component.fov, float(window_single_component.width), float(window_single_component.height), camera_single_component.z_near, camera_single_component.z_far);
        camera_single_component.view_projection_matrix = camera_single_component.projection_matrix * camera_single_component.view_matrix;
        camera_single_component.inverse_view_matrix = glm::inverse(camera_single_component.view_matrix);
        camera_single_component.inverse_projection_matrix = glm::inverse(camera_single_component.projection_matrix);
        camera_single_component.inverse_view_projection_matrix = glm::inverse(camera_single_component.view_projection_matrix);
    }
}

} // namespace hg
