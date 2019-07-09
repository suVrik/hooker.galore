#pragma once

#include "core/resource/texture.h"

#include <string>
#include <unordered_map>

namespace hg {

class ResourceSystem;

/** `TextureSingleComponent` contains all available textures. */
class TextureSingleComponent final {
public:
    /** Return texture with the specified name or default texture if such texture doesn't exist. */
    const Texture& get(const std::string& name) const noexcept;

    /** Return texture with the specified name or nullptr if such texture doesn't exist. */
    const Texture* get_if(const std::string& name) const noexcept;

private:
    std::unordered_map<std::string, Texture> m_textures;
    Texture m_default_texture;

    friend class ResourceSystem;
};

} // namespace hg
