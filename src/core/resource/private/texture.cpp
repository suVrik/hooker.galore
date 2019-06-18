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
        , channels(another.channels) {
    another.handle   = BGFX_INVALID_HANDLE;
    another.width    = 0;
    another.height   = 0;
    another.channels = 0;
}

Texture& Texture::operator=(Texture&& another) noexcept {
    handle   = another.handle;
    width    = another.width;
    height   = another.height;
    channels = another.channels;

    another.handle   = BGFX_INVALID_HANDLE;
    another.width    = 0;
    another.height   = 0;
    another.channels = 0;

    return *this;
}

} // namespace hg
