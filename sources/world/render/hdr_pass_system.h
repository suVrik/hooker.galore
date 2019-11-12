#pragma once

#include "core/ecs/system.h"

namespace hg {

struct HDRPassSingleComponent;

/** `HDRPassSystem` composes the final image. */
class HDRPassSystem final : public NormalSystem {
public:
    explicit HDRPassSystem(World& world);
    ~HDRPassSystem() override;
    void update(float elapsed_time) override;
    void reset(HDRPassSingleComponent& hdr_pass_single_component, uint16_t width, uint16_t height) const;
};

} // namespace hg
