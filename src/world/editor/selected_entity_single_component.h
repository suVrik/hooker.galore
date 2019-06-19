#pragma once

#include <entt/entity/registry.hpp>

namespace hg {

/** `SelectedEntitySingleComponent` contains selected entity. */
class SelectedEntitySingleComponent final {
public:
    entt::entity selected_entity = entt::null;
};

} // namespace hg
