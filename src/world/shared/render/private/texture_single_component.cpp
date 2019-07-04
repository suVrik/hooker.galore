#include "world/shared/render/texture_single_component.h"

#include <ghc/filesystem.hpp>

namespace hg {

const Texture& TextureSingleComponent::get(const std::string& name) const noexcept {
    const std::string normalized_name = ghc::filesystem::path(name).lexically_normal().string();
    if (auto result = m_textures.find(normalized_name); result != m_textures.end()) {
        return result->second;
    }
    return m_default_texture;
}

} // namespace hg
