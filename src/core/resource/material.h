#pragma once

namespace hg {

class Texture;

/** `Material` is a set of textures and properties that define visual look of the model. */
class Material final {
public:
    const Texture* color_roughness = nullptr;
    const Texture* normal_metal_ao = nullptr;
    const Texture* parallax        = nullptr; // Optional.
    float parallax_scale           = 0.f;     // Optional.
};

} // namespace hg
