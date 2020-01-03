#pragma once

#include "core/ecs/system.h"

namespace hg {

struct HDRPassSingleComponent;

/** `HDRPassSystem` composes the final image. */
class HDRPassSystem final : public NormalSystem {
public:
    explicit HDRPassSystem(World& world);
    void update(float elapsed_time) override;
};

} // namespace hg
