#include "world/render/texture_single_component.h"

#include <ghc/filesystem.hpp>

namespace hg {

const Texture& TextureSingleComponent::get(const std::string& name) const {
    const std::string normalized_name = ghc::filesystem::path(name).lexically_normal().string();
    if (auto result = m_textures.find(normalized_name); result != m_textures.end()) {
        return result->second;
    }
    return m_default_texture;
}

const Texture* TextureSingleComponent::get_if(const std::string& name) const {
    const std::string normalized_name = ghc::filesystem::path(name).lexically_normal().string();
    if (auto result = m_textures.find(normalized_name); result != m_textures.end()) {
        return &result->second;
    }
    return nullptr;
}

} // namespace hg
