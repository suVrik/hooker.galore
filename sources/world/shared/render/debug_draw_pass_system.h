#pragma once

#include "core/ecs/system.h"

#include <debug_draw.hpp>

namespace hg {

struct DebugDrawPassSingleComponent;

/** `DebugDrawSystem` performs debug draw. Debug draw API is available via dd:: namespace. */
class DebugDrawPassSystem final : public NormalSystem, public dd::RenderInterface {
public:
    explicit DebugDrawPassSystem(World& world);
    ~DebugDrawPassSystem() override;
    void update(float elapsed_time) override;
    void reset(DebugDrawPassSingleComponent& debug_draw_pass_single_component, uint16_t width, uint16_t height) const noexcept;

    dd::GlyphTextureHandle createGlyphTexture(int width, int height, const void* pixels) override;
    void destroyGlyphTexture(dd::GlyphTextureHandle glyph_texture) override;

    void drawPointList(const dd::DrawVertex* points, int count, bool depth_enabled) override;
    void drawLineList(const dd::DrawVertex* lines, int count, bool depth_enabled) override;
    void drawGlyphList(const dd::DrawVertex* glyphs, int count, dd::GlyphTextureHandle glyph_texture) override;
};

} // namespace hg
