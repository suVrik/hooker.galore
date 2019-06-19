#include "core/resource/filesystem.h"
#include "core/resource/material.h"
#include "world/shared/render/material_single_component.h"

namespace hg {

MaterialSingleComponent::MaterialSingleComponent() = default;
MaterialSingleComponent::MaterialSingleComponent(MaterialSingleComponent&& another) noexcept = default;
MaterialSingleComponent& MaterialSingleComponent::operator=(MaterialSingleComponent&& another) noexcept = default;
MaterialSingleComponent::~MaterialSingleComponent() = default;

const Material* MaterialSingleComponent::get(const std::string& name) const noexcept {
    const std::string normalized_name = filesystem::path(name).lexically_normal().string();
    if (auto result = m_materials.find(normalized_name); result != m_materials.end()) {
        return result->second.get();
    }
    return nullptr;
}

} // namespace hg