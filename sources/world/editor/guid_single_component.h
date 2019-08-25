#pragma once

#include <entt/entity/registry.hpp>
#include <unordered_map>

namespace hg {

/** `GuidSingleComponent` contains match from GUID to entity. */
struct GuidSingleComponent final {
    /** Acquire unique GUID value. */
    uint32_t acquire_unique_guid(entt::entity) noexcept;

    std::unordered_map<uint32_t, entt::entity> guid_to_entity;
};

} // namespace hg
