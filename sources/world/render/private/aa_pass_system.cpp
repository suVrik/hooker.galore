#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/aa_pass/aa_pass.fragment.h"
#include "shaders/quad_pass/quad_pass.vertex.h"
#include "world/render/aa_pass_single_component.h"
#include "world/render/aa_pass_system.h"
#include "world/render/camera_single_component.h"
#include "world/render/quad_single_component.h"
#include "world/render/render_tags.h"
#include "world/render/skybox_pass_single_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>

namespace hg {

namespace aa_pass_system_details {

static const bgfx::EmbeddedShader AA_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(quad_pass_vertex),
        BGFX_EMBEDDED_SHADER(aa_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

} // namespace aa_pass_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(AAPassSystem),
    TAGS(render),
    CONTEXT(AAPassSingleComponent),
    BEFORE("RenderSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "SkyboxPassSystem")
)

AAPassSystem::AAPassSystem(World& world)
        : NormalSystem(world) {
    using namespace aa_pass_system_details;

    auto& aa_pass_single_component = world.ctx<AAPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    bgfx::RendererType::Enum type = bgfx::getRendererType();
    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(AA_PASS_SHADER, type, "quad_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(AA_PASS_SHADER, type, "aa_pass_fragment");
    aa_pass_single_component.program          = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    aa_pass_single_component.color_sampler_uniform = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);

    reset(aa_pass_single_component, window_single_component.width, window_single_component.height);

    bgfx::setViewClear(AA_PASS, BGFX_CLEAR_NONE);
    bgfx::setViewName(AA_PASS, "aa_pass");
}

void AAPassSystem::update(float /*elapsed_time*/) {
    auto& aa_pass_single_component = world.ctx<AAPassSingleComponent>();
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& quad_single_component = world.ctx<QuadSingleComponent>();
    auto& skybox_pass_single_component = world.ctx<SkyboxPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(aa_pass_single_component, window_single_component.width, window_single_component.height);
    }

    bgfx::setViewTransform(AA_PASS, &camera_single_component.view_matrix, &camera_single_component.projection_matrix);

    assert(bgfx::isValid(*quad_single_component.index_buffer));
    bgfx::setIndexBuffer(*quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    assert(bgfx::isValid(*quad_single_component.vertex_buffer));
    bgfx::setVertexBuffer(0, *quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);

    assert(bgfx::isValid(*aa_pass_single_component.color_sampler_uniform));
    assert(bgfx::isValid(*skybox_pass_single_component.color_texture));
    bgfx::setTexture(0, *aa_pass_single_component.color_sampler_uniform, *skybox_pass_single_component.color_texture);

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);

    assert(bgfx::isValid(*aa_pass_single_component.program));
    bgfx::submit(AA_PASS, *aa_pass_single_component.program);
}

void AAPassSystem::reset(AAPassSingleComponent& aa_pass_single_component, uint16_t width, uint16_t height) {
    constexpr uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP;

    aa_pass_single_component.color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA16F, ATTACHMENT_FLAGS);
    bgfx::setName(*aa_pass_single_component.color_texture, "aa_pass_color_texture");

    aa_pass_single_component.buffer = bgfx::createFrameBuffer(1, &(*aa_pass_single_component.color_texture), false);
    bgfx::setName(*aa_pass_single_component.buffer, "aa_pass_frame_buffer");

    bgfx::setViewFrameBuffer(AA_PASS, *aa_pass_single_component.buffer);
    bgfx::setViewRect(AA_PASS, 0, 0, width, height);
}

} // namespace hg
