#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/aa_pass/aa_pass.fragment.h"
#include "shaders/quad_pass/quad_pass.vertex.h"
#include "world/shared/render/aa_pass_single_component.h"
#include "world/shared/render/aa_pass_system.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/quad_single_component.h"
#include "world/shared/render/skybox_pass_single_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>
#include <glm/gtc/type_ptr.hpp>

namespace hg {

namespace aa_pass_system_details {

static const bgfx::EmbeddedShader AA_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(quad_pass_vertex),
        BGFX_EMBEDDED_SHADER(aa_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

static const uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

} // namespace aa_pass_system_details

AAPassSystem::AAPassSystem(World& world)
        : NormalSystem(world) {
    using namespace aa_pass_system_details;

    auto& aa_pass_single_component = world.set<AAPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    bgfx::RendererType::Enum type = bgfx::getRendererType();
    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(AA_PASS_SHADER, type, "quad_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(AA_PASS_SHADER, type, "aa_pass_fragment");
    aa_pass_single_component.aa_pass_program  = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    aa_pass_single_component.texture_uniform    = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
    aa_pass_single_component.pixel_size_uniform = bgfx::createUniform("u_pixel_size", bgfx::UniformType::Vec4);

    reset(aa_pass_single_component, window_single_component.width, window_single_component.height);

    bgfx::setViewClear(AA_PASS, BGFX_CLEAR_NONE, 0x000000FF, 1.f, 0);
    bgfx::setViewName(AA_PASS, "aa_pass");
}

AAPassSystem::~AAPassSystem() {
    auto& aa_pass_single_component = world.ctx<AAPassSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(aa_pass_single_component.aa_pass_program);
    destroy_valid(aa_pass_single_component.buffer);
    destroy_valid(aa_pass_single_component.pixel_size_uniform);
    destroy_valid(aa_pass_single_component.texture_uniform);
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

    bgfx::setViewTransform(AA_PASS, glm::value_ptr(camera_single_component.view_matrix), glm::value_ptr(camera_single_component.projection_matrix));

    bgfx::setVertexBuffer(0, quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);
    bgfx::setIndexBuffer(quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    bgfx::setTexture(0, aa_pass_single_component.texture_uniform, skybox_pass_single_component.color_texture);

    const float pixel_resolution[4] = { 1.f / window_single_component.width, 1.f / window_single_component.height, 0.f, 0.f};
    bgfx::setUniform(aa_pass_single_component.pixel_size_uniform, &pixel_resolution);

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);

    bgfx::submit(AA_PASS, aa_pass_single_component.aa_pass_program);
}

void AAPassSystem::reset(AAPassSingleComponent &aa_pass_single_component, uint16_t width, uint16_t height) const noexcept {
    using namespace aa_pass_system_details;

    if (bgfx::isValid(aa_pass_single_component.buffer)) {
        bgfx::destroy(aa_pass_single_component.buffer);
    }

    aa_pass_single_component.color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA16F, ATTACHMENT_FLAGS);

    const bgfx::TextureHandle attachments[] = {
            aa_pass_single_component.color_texture
    };

    aa_pass_single_component.buffer = bgfx::createFrameBuffer(std::size(attachments), attachments, true);

    bgfx::setViewFrameBuffer(AA_PASS, aa_pass_single_component.buffer);
    bgfx::setViewRect(AA_PASS, 0, 0, width, height);
}

} // namespace hg
