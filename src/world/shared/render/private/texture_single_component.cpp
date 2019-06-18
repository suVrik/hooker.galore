#include "core/resource/filesystem.h"
#include "core/resource/texture.h"
#include "world/shared/render/texture_single_component.h"

namespace hg {

TextureSingleComponent::TextureSingleComponent() = default;
TextureSingleComponent::TextureSingleComponent(TextureSingleComponent&& another) noexcept = default;
TextureSingleComponent& TextureSingleComponent::operator=(TextureSingleComponent&& another) noexcept = default;
TextureSingleComponent::~TextureSingleComponent() = default;

const Texture* TextureSingleComponent::get(const std::string& name) const noexcept {
    const std::string normalized_name = filesystem::path(name).lexically_normal().string();
    if (auto result = m_textures.find(normalized_name); result != m_textures.end()) {
        return result->second.get();
    }
    return nullptr;
}

} // namespace hg
