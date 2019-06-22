#include "core/ecs/world.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/render/outline_component.h"

namespace hg {

void SelectedEntitySingleComponent::select_entity(World& world, entt::entity entity) noexcept {
    if (selected_entity != entity) {
        if (world.valid(selected_entity)) {
            world.reset<OutlineComponent>(selected_entity);
        }
        if (world.valid(entity)) {
            selected_entity = entity;
            world.assign<OutlineComponent>(entity);
        } else {
            selected_entity = entt::null;
        }
    }
}

} // namespace hg
