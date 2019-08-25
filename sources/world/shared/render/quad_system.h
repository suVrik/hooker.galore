#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `QuadSystem` simply allocated a quad and stores it in `QuadSingleComponent`. */
class QuadSystem final : public NormalSystem {
public:
    explicit QuadSystem(World& world);
    ~QuadSystem() override;
    void update(float elapsed_time) override;
};

} // namespace hg
