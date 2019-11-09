#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `Texture` is an image with certain number of channels and size power of two.
    All `Texture` instances must be destroyed before `RenderFetchSystem` destructor. */
class Texture final {
public:
    Texture() = default;
    ~Texture();
    Texture(const Texture& another) = delete;
    Texture(Texture&& another);
    Texture& operator=(const Texture& another) = delete;
    Texture& operator=(Texture&& another);

    bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
    uint16_t width   = 0;
    uint16_t height  = 0;
    bool is_cube_map = false;
};

} // namespace hg
