#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/hdr_pass/hdr_pass.fragment.h"
#include "shaders/quad_pass/quad_pass.vertex.h"
#include "world/shared/render/aa_pass_single_component.h"
#include "world/shared/render/hdr_pass_single_component.h"
#include "world/shared/render/hdr_pass_system.h"
#include "world/shared/render/quad_single_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>

namespace hg {

namespace hdr_pass_system_details {

static const bgfx::EmbeddedShader HDR_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(quad_pass_vertex),
        BGFX_EMBEDDED_SHADER(hdr_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

} // namespace hdr_pass_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(HDRPassSystem),
    REQUIRE("render"),
    BEFORE("RenderSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "CameraSystem", "AAPassSystem")
)

HDRPassSystem::HDRPassSystem(World& world)
        : NormalSystem(world) {
    using namespace hdr_pass_system_details;

    auto& hdr_pass_single_component = world.set<HDRPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    bgfx::RendererType::Enum type = bgfx::getRendererType();
    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(HDR_PASS_SHADER, type, "quad_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(HDR_PASS_SHADER, type, "hdr_pass_fragment");
    hdr_pass_single_component.hdr_pass_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);
    hdr_pass_single_component.texture_uniform  = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);

    reset(hdr_pass_single_component, window_single_component.width, window_single_component.height);

    bgfx::setViewClear(HDR_PASS, BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x000000FF, 1.f, 0);
    bgfx::setViewName(HDR_PASS, "hdr_pass");
}

HDRPassSystem::~HDRPassSystem() {
    auto& hdr_pass_single_component = world.ctx<HDRPassSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(hdr_pass_single_component.hdr_pass_program);
    destroy_valid(hdr_pass_single_component.texture_uniform);
}

void HDRPassSystem::update(float /*elapsed_time*/) {
    auto& aa_pass_single_component = world.ctx<AAPassSingleComponent>();
    auto& hdr_pass_single_component = world.ctx<HDRPassSingleComponent>();
    auto& quad_single_component = world.ctx<QuadSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(hdr_pass_single_component, window_single_component.width, window_single_component.height);
    }

    bgfx::setVertexBuffer(0, quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);
    bgfx::setIndexBuffer(quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    bgfx::setTexture(0, hdr_pass_single_component.texture_uniform, aa_pass_single_component.color_texture);

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);

    bgfx::submit(HDR_PASS, hdr_pass_single_component.hdr_pass_program);
}

void HDRPassSystem::reset(HDRPassSingleComponent &hdr_pass_single_component, uint16_t width, uint16_t height) const noexcept {
    using namespace hdr_pass_system_details;

    bgfx::setViewRect(HDR_PASS, 0, 0, width, height);
}

} // namespace hg
