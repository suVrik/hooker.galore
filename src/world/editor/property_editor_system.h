#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `PropertyEditorSystem` draws a grid in editor. */
class PropertyEditorSystem final : public NormalSystem {
public:
    explicit PropertyEditorSystem(World& world) noexcept;
    void update(float elapsed_time) override;

private:
    void list_properties(entt::meta_handle object) const noexcept;
};

} // namespace hg
