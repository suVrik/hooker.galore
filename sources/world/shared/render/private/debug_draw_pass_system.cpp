#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/debug_solid_pass/debug_solid_pass.fragment.h"
#include "shaders/debug_solid_pass/debug_solid_pass.vertex.h"
#include "shaders/debug_textured_pass/debug_textured_pass.fragment.h"
#include "shaders/debug_textured_pass/debug_textured_pass.vertex.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/debug_draw_pass_single_component.h"
#include "world/shared/render/debug_draw_pass_system.h"
#include "world/shared/render/geometry_pass_single_component.h"
#include "world/shared/render/quad_single_component.h"
#include "world/shared/render/render_tags.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>

namespace hg {

namespace debug_draw_pass_system_details {

static const bgfx::EmbeddedShader SOLID_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(debug_solid_pass_vertex),
        BGFX_EMBEDDED_SHADER(debug_solid_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

static const bgfx::EmbeddedShader TEXTURED_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(debug_textured_pass_vertex),
        BGFX_EMBEDDED_SHADER(debug_textured_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

static const bgfx::VertexDecl SOLID_VERTEX_DECLARATION = [] {
    bgfx::VertexDecl result;
    result.begin()
          .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
          .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Float)
          .end();
    return result;
}();

static const bgfx::VertexDecl TEXTURED_VERTEX_DECLARATION = [] {
    bgfx::VertexDecl result;
    result.begin()
          .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
          .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
          .add(bgfx::Attrib::Color0,    3, bgfx::AttribType::Float)
          .end();
    return result;
}();

static const uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

} // namespace debug_draw_pass_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(DebugDrawPassSystem),
    TAGS(render),
    BEFORE("RenderSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "CameraSystem")
)

DebugDrawPassSystem::DebugDrawPassSystem(World& world)
        : NormalSystem(world) {
    using namespace debug_draw_pass_system_details;

    auto& debug_draw_single_component = world.set<DebugDrawPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    reset(debug_draw_single_component, window_single_component.width, window_single_component.height);

    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(SOLID_PASS_SHADER, type, "debug_solid_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(SOLID_PASS_SHADER, type, "debug_solid_pass_fragment");
    debug_draw_single_component.solid_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    vertex_shader_handle   = bgfx::createEmbeddedShader(TEXTURED_PASS_SHADER, type, "debug_textured_pass_vertex");
    fragment_shader_handle = bgfx::createEmbeddedShader(TEXTURED_PASS_SHADER, type, "debug_textured_pass_fragment");
    debug_draw_single_component.textured_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    debug_draw_single_component.texture_uniform = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);

    bgfx::setViewClear(DEBUG_DRAW_OFFSCREEN_PASS, BGFX_CLEAR_COLOR, 0x00000000, 1.f, 0);
    bgfx::setViewName(DEBUG_DRAW_OFFSCREEN_PASS, "debug_draw_offscreen_pass");

    bgfx::setViewClear(DEBUG_DRAW_ONSCREEN_PASS,  BGFX_CLEAR_NONE,  0x00000000, 1.f, 0);
    bgfx::setViewName(DEBUG_DRAW_ONSCREEN_PASS, "debug_draw_onscreen_pass");

    dd::initialize(this);
}

DebugDrawPassSystem::~DebugDrawPassSystem() {
    auto& debug_draw_single_component = world.ctx<DebugDrawPassSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(debug_draw_single_component.buffer);
    destroy_valid(debug_draw_single_component.color_texture);
    destroy_valid(debug_draw_single_component.solid_program);
    destroy_valid(debug_draw_single_component.texture_uniform);
    destroy_valid(debug_draw_single_component.textured_program);

    dd::shutdown();
}

void DebugDrawPassSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& debug_draw_single_component = world.ctx<DebugDrawPassSingleComponent>();
    auto& quad_single_component = world.ctx<QuadSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(debug_draw_single_component, window_single_component.width, window_single_component.height);
    }

    bgfx::setViewTransform(DEBUG_DRAW_OFFSCREEN_PASS, glm::value_ptr(camera_single_component.view_matrix), glm::value_ptr(camera_single_component.projection_matrix));
    bgfx::touch(DEBUG_DRAW_OFFSCREEN_PASS);

    dd::flush(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    bgfx::setVertexBuffer(0, quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);
    bgfx::setIndexBuffer(quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    bgfx::setTexture(0, debug_draw_single_component.texture_uniform, debug_draw_single_component.color_texture);

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));

    bgfx::submit(DEBUG_DRAW_ONSCREEN_PASS, quad_single_component.program);
}

void DebugDrawPassSystem::reset(DebugDrawPassSingleComponent& debug_draw_pass_single_component, uint16_t width, uint16_t height) const {
    using namespace debug_draw_pass_system_details;

    if (bgfx::isValid(debug_draw_pass_single_component.color_texture)) {
        bgfx::destroy(debug_draw_pass_single_component.color_texture);
    }

    if (bgfx::isValid(debug_draw_pass_single_component.buffer)) {
        bgfx::destroy(debug_draw_pass_single_component.buffer);
    }

    debug_draw_pass_single_component.color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, ATTACHMENT_FLAGS);

    const bgfx::TextureHandle attachments[] = {
            debug_draw_pass_single_component.color_texture,
            world.ctx<GeometryPassSingleComponent>().depth_stencil_texture
    };

    debug_draw_pass_single_component.buffer = bgfx::createFrameBuffer(static_cast<uint8_t>(std::size(attachments)), attachments, false);

    bgfx::setViewFrameBuffer(DEBUG_DRAW_OFFSCREEN_PASS, debug_draw_pass_single_component.buffer);
    bgfx::setViewRect(DEBUG_DRAW_OFFSCREEN_PASS, 0, 0, width, height);

    bgfx::setViewRect(DEBUG_DRAW_ONSCREEN_PASS, 0, 0, width, height);
}

dd::GlyphTextureHandle DebugDrawPassSystem::createGlyphTexture(int width, int height, const void* pixels) {
    return reinterpret_cast<dd::GlyphTextureHandle>(static_cast<uintptr_t>(bgfx::createTexture2D(static_cast<uint16_t>(width), static_cast<uint16_t>(height), false, 1, bgfx::TextureFormat::R8, 0, bgfx::copy(pixels, static_cast<uint32_t>(width * height))).idx));
}

void DebugDrawPassSystem::destroyGlyphTexture(dd::GlyphTextureHandle glyph_texture) {
    bgfx::destroy(bgfx::TextureHandle{ static_cast<uint16_t>(reinterpret_cast<uintptr_t>(glyph_texture)) });
}

void DebugDrawPassSystem::drawPointList(const dd::DrawVertex* points, int count, bool depth_enabled) {
    using namespace debug_draw_pass_system_details;

    if (bgfx::getAvailTransientVertexBuffer(count, SOLID_VERTEX_DECLARATION)) {
        auto& debug_draw_single_component = world.ctx<DebugDrawPassSingleComponent>();

        bgfx::TransientVertexBuffer vertex_buffer {};
        bgfx::allocTransientVertexBuffer(&vertex_buffer, count, SOLID_VERTEX_DECLARATION);
        std::memcpy(vertex_buffer.data, points, count * sizeof(dd::DrawVertex));

        bgfx::setVertexBuffer(0, &vertex_buffer, 0, count);

        uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW | BGFX_STATE_PT_POINTS;
        if (depth_enabled) {
            state |= BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS;
        }
        bgfx::setState(state);

        bgfx::submit(DEBUG_DRAW_OFFSCREEN_PASS, debug_draw_single_component.solid_program);
    }
}

void DebugDrawPassSystem::drawLineList(const dd::DrawVertex* lines, int count, bool depth_enabled) {
    using namespace debug_draw_pass_system_details;

    if (bgfx::getAvailTransientVertexBuffer(count, SOLID_VERTEX_DECLARATION)) {
        auto& debug_draw_single_component = world.ctx<DebugDrawPassSingleComponent>();

        bgfx::TransientVertexBuffer vertex_buffer {};
        bgfx::allocTransientVertexBuffer(&vertex_buffer, count, SOLID_VERTEX_DECLARATION);
        std::memcpy(vertex_buffer.data, lines, count * sizeof(dd::DrawVertex));

        bgfx::setVertexBuffer(0, &vertex_buffer, 0, count);

        uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW | BGFX_STATE_PT_LINES;
        if (depth_enabled) {
            state |= BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS;
        }
        bgfx::setState(state);

        bgfx::submit(DEBUG_DRAW_OFFSCREEN_PASS, debug_draw_single_component.solid_program);
    }
}

void DebugDrawPassSystem::drawGlyphList(const dd::DrawVertex* glyphs, int count, dd::GlyphTextureHandle glyph_texture) {
    using namespace debug_draw_pass_system_details;

    if (bgfx::getAvailTransientVertexBuffer(count, TEXTURED_VERTEX_DECLARATION)) {
        auto& debug_draw_single_component = world.ctx<DebugDrawPassSingleComponent>();

        bgfx::TransientVertexBuffer vertex_buffer {};
        bgfx::allocTransientVertexBuffer(&vertex_buffer, count, TEXTURED_VERTEX_DECLARATION);
        std::memcpy(vertex_buffer.data, glyphs, count * sizeof(dd::DrawVertex));

        bgfx::setVertexBuffer(0, &vertex_buffer, 0, count);
        bgfx::setTexture(0, debug_draw_single_component.texture_uniform, bgfx::TextureHandle{ static_cast<uint16_t>(reinterpret_cast<uintptr_t>(glyph_texture)) });
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A  | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
        bgfx::submit(DEBUG_DRAW_OFFSCREEN_PASS, debug_draw_single_component.textured_program);
    }
}

} // namespace hg
