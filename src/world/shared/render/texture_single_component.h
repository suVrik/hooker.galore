#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace hg {

class ResourceSystem;
class Texture;

/** `TextureSingleComponent` contains all available textures. */
class TextureSingleComponent final {
public:
    TextureSingleComponent();
    TextureSingleComponent(const TextureSingleComponent& another) = delete;
    TextureSingleComponent(TextureSingleComponent&& another) noexcept;
    TextureSingleComponent& operator=(const TextureSingleComponent& another) = delete;
    TextureSingleComponent& operator=(TextureSingleComponent&& another) noexcept;
    ~TextureSingleComponent();

    /** Return texture with the specified name or nullptr if such texture doesn't exists. */
    const Texture* get(const std::string& name) const noexcept;

private:
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;

    friend class ResourceSystem;
};

} // namespace hg
