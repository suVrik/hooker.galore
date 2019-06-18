#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace hg {

class Material;
class ResourceSystem;

/** `MaterialSingleComponent` contains all available materials. */
class MaterialSingleComponent final {
public:
    MaterialSingleComponent();
    MaterialSingleComponent(const MaterialSingleComponent& another) = delete;
    MaterialSingleComponent(MaterialSingleComponent&& another) noexcept;
    MaterialSingleComponent& operator=(const MaterialSingleComponent& another) = delete;
    MaterialSingleComponent& operator=(MaterialSingleComponent&& another) noexcept;
    ~MaterialSingleComponent();

    /** Return material with the specified name or nullptr if such material doesn't exists. */
    const Material* get(const std::string& name) const noexcept;

private:
    std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;

    friend class ResourceSystem;
};

} // namespace hg
