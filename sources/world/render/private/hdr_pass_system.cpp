#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/hdr_pass/hdr_pass.fragment.h"
#include "shaders/quad_pass/quad_pass.vertex.h"
#include "world/render/aa_pass_single_component.h"
#include "world/render/hdr_pass_single_component.h"
#include "world/render/hdr_pass_system.h"
#include "world/render/quad_single_component.h"
#include "world/render/render_tags.h"
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
    TAGS(render),
    CONTEXT(HDRPassSingleComponent),
    BEFORE("RenderSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "CameraSystem", "AAPassSystem")
)

HDRPassSystem::HDRPassSystem(World& world)
        : NormalSystem(world) {
    using namespace hdr_pass_system_details;

    auto& hdr_pass_single_component = world.ctx<HDRPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ShaderHandle vertex_shader_handle    = bgfx::createEmbeddedShader(HDR_PASS_SHADER, type, "quad_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle  = bgfx::createEmbeddedShader(HDR_PASS_SHADER, type, "hdr_pass_fragment");
    hdr_pass_single_component.program          = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    hdr_pass_single_component.texture_sampler_uniform = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);

    bgfx::setViewRect(HDR_PASS, 0, 0, window_single_component.width, window_single_component.height);
    bgfx::setViewClear(HDR_PASS, BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x000000FF, 1.f, 0);
    bgfx::setViewName(HDR_PASS, "hdr_pass");
}

void HDRPassSystem::update(float /*elapsed_time*/) {
    auto& aa_pass_single_component = world.ctx<AAPassSingleComponent>();
    auto& hdr_pass_single_component = world.ctx<HDRPassSingleComponent>();
    auto& quad_single_component = world.ctx<QuadSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        bgfx::setViewRect(HDR_PASS, 0, 0, window_single_component.width, window_single_component.height);
    }

    assert(bgfx::isValid(*quad_single_component.index_buffer));
    bgfx::setIndexBuffer(*quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    assert(bgfx::isValid(*quad_single_component.vertex_buffer));
    bgfx::setVertexBuffer(0, *quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);

    assert(bgfx::isValid(*hdr_pass_single_component.texture_sampler_uniform));
    assert(bgfx::isValid(*aa_pass_single_component.color_texture));
    bgfx::setTexture(0, *hdr_pass_single_component.texture_sampler_uniform, *aa_pass_single_component.color_texture);

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);

    assert(bgfx::isValid(*hdr_pass_single_component.program));
    bgfx::submit(HDR_PASS, *hdr_pass_single_component.program);
}

} // namespace hg
