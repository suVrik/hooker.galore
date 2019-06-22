#include "core/ecs/world.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/render/outline_component.h"

namespace hg {

void SelectedEntitySingleComponent::select_entity(World& world, entt::entity entity) noexcept {
    clear_selection(world);
    if (world.valid(entity)) {
        selected_entities.push_back(entity);
        world.assign<OutlineComponent>(entity);
    }
}

void SelectedEntitySingleComponent::add_to_selection(World& world, entt::entity entity) noexcept {
    if (std::find(selected_entities.begin(), selected_entities.end(), entity) == selected_entities.end()) {
        if (world.valid(entity)) {
            selected_entities.push_back(entity);
            world.assign<OutlineComponent>(entity);
        }
    }
}

void SelectedEntitySingleComponent::clear_selection(World& world) noexcept {
    for (entt::entity entity : selected_entities) {
        if (world.valid(entity)) {
            world.reset<OutlineComponent>(entity);
        }
    }
    selected_entities.clear();
}

} // namespace hg
