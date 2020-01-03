#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `QuadSystem` simply allocates quad's vertices and indices and stores it in `QuadSingleComponent`. */
class QuadSystem final : public NormalSystem {
public:
    explicit QuadSystem(World& world);
    void update(float elapsed_time) override;
};

} // namespace hg
