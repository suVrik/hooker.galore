#pragma once

#include "core/ecs/system.h"

namespace hg {

struct GeometryPassSingleComponent;

/** `GeometryPassSystem` performs geometry pass for all objects with geometry, material and transform.
    The result is stored in `GeometryPassSingleComponent`. */
class GeometryPassSystem final : public NormalSystem {
public:
    explicit GeometryPassSystem(World& world);
    void update(float elapsed_time) override;

private:
    void reset(GeometryPassSingleComponent& geometry_pass_single_component, uint16_t width, uint16_t height);
};

} // namespace hg
