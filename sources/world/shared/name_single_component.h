#pragma once

#include <entt/entity/registry.hpp>
#include <string>
#include <unordered_map>

namespace hg {

/** `NameSingleComponent` contains match from entity name to entity itself. */
struct NameSingleComponent final {
    /** Check whether the specified name `prototype` is free and if so, register the given `entity` with this name.
        Otherwise come up with a similar name and register this entity with it. */
    std::string acquire_unique_name(entt::entity entity, const std::string& prototype) noexcept;

    std::unordered_map<std::string, entt::entity> name_to_entity;
};

} // namespace hg
