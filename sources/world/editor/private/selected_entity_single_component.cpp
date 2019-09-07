#include "core/ecs/world.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/render/outline_component.h"

namespace hg {

void SelectedEntitySingleComponent::select_entity(World& world, entt::entity entity) noexcept {
    clear_selection(world);
    if (world.valid(entity)) {
        selected_entities.push_back(entity);
        auto& outline_component = world.assign<OutlineComponent>(entity);
        outline_component.group_index = static_cast<uint32_t>(entity);
    }
}

void SelectedEntitySingleComponent::add_to_selection(World& world, entt::entity entity) noexcept {
    if (std::find(selected_entities.begin(), selected_entities.end(), entity) == selected_entities.end()) {
        if (world.valid(entity)) {
            selected_entities.push_back(entity);
            auto& outline_component = world.assign<OutlineComponent>(entity);
            outline_component.group_index = static_cast<uint32_t>(entity);
        }
    }
}

void SelectedEntitySingleComponent::remove_from_selection(World& world, entt::entity entity) noexcept {
    auto it = std::remove(selected_entities.begin(), selected_entities.end(), entity);
    if (it != selected_entities.end()) {
        world.reset<OutlineComponent>(entity);
        selected_entities.erase(it, selected_entities.end());
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
