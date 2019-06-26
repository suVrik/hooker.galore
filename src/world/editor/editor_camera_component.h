#pragma once

namespace hg {

/** The `EditorCameraComponent` contains state of an editor's camera. */
class EditorCameraComponent final {
public:
    float yaw   = 0.f;
    float pitch = 0.f;

    // Menu items.
    std::shared_ptr<bool> reset_camera_position;
};

} // namespace hg
