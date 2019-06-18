#include "core/resource/filesystem.h"
#include "core/resource/model.h"
#include "world/shared/render/model_single_component.h"

namespace hg {

ModelSingleComponent::ModelSingleComponent() = default;
ModelSingleComponent::ModelSingleComponent(ModelSingleComponent&& another) noexcept = default;
ModelSingleComponent& ModelSingleComponent::operator=(ModelSingleComponent&& another) noexcept = default;
ModelSingleComponent::~ModelSingleComponent() = default;


const Model* ModelSingleComponent::get(const std::string& name) const noexcept {
    const std::string normalized_name = filesystem::path(name).lexically_normal().string();
    if (auto result = m_models.find(normalized_name); result != m_models.end()) {
        return result->second.get();
    }
    return nullptr;
}

} // namespace hg
