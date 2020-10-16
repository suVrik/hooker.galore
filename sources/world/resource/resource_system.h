#pragma once

#include "core/ecs/system.h"

namespace hg {

/** TODO: Description. */
class NewResourceSystem : public NormalSystem {
public:
    explicit NewResourceSystem(World& world);
    void update(float elapsed_time) override;
};

} // namespace hg
