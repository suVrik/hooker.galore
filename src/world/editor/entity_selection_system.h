#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `EntitySelectionSystem` shows Entity list UI, allows to pick an entity and stores it in
    `SelectedEntitySingleComponent`. */
class EntitySelectionSystem final : public NormalSystem {
public:
    explicit EntitySelectionSystem(World& world) noexcept;
    void update(float elapsed_time) override;
};

} // namespace hg
