#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace hg {

class ResourceSystem;
class Model;

/** `ModelSingleComponent` contains all available models. */
class ModelSingleComponent final {
public:
    ModelSingleComponent();
    ModelSingleComponent(const ModelSingleComponent& another) = delete;
    ModelSingleComponent(ModelSingleComponent&& another);
    ModelSingleComponent& operator=(const ModelSingleComponent& another) = delete;
    ModelSingleComponent& operator=(ModelSingleComponent&& another);
    ~ModelSingleComponent();

    /** Return model with the specified name or nullptr if such model doesn't exists. */
    const Model* get(const std::string& name) const;

private:
    std::unordered_map<std::string, std::unique_ptr<Model>> m_models;

    friend class ResourceSystem;
};

} // namespace hg
