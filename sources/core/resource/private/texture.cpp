#include "core/resource/texture.h"

namespace hg {

Texture::~Texture() {
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
}

Texture::Texture(Texture&& another) noexcept
        : handle(another.handle)
        , width(another.width)
        , height(another.height)
        , is_cube_map(another.is_cube_map) {
    another.handle      = BGFX_INVALID_HANDLE;
    another.width       = 0;
    another.height      = 0;
    another.is_cube_map = false;
}

Texture& Texture::operator=(Texture&& another) noexcept {
    handle      = another.handle;
    width       = another.width;
    height      = another.height;
    is_cube_map = another.is_cube_map;

    another.handle       = BGFX_INVALID_HANDLE;
    another.width        = 0;
    another.height       = 0;
    another.is_cube_map  = false;

    return *this;
}

} // namespace hg
