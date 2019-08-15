#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/skybox_pass/skybox_pass.fragment.h"
#include "shaders/skybox_pass/skybox_pass.vertex.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/geometry_pass_single_component.h"
#include "world/shared/render/light_component.h"
#include "world/shared/render/quad_single_component.h"
#include "world/shared/render/skybox_pass_single_component.h"
#include "world/shared/render/skybox_pass_system.h"
#include "world/shared/render/texture_single_component.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <entt/entity/registry.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <world/shared/render/lighting_pass_single_component.h>

namespace hg {

namespace skybox_pass_system_details {

static const bgfx::EmbeddedShader SKYBOX_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(skybox_pass_vertex),
        BGFX_EMBEDDED_SHADER(skybox_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

static const uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

} // namespace skybox_pass_system_details

SkyboxPassSystem::SkyboxPassSystem(World& world)
        : NormalSystem(world) {
    using namespace skybox_pass_system_details;

    auto& skybox_pass_single_component = world.set<SkyboxPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    bgfx::RendererType::Enum type = bgfx::getRendererType();
    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(SKYBOX_PASS_SHADER, type, "skybox_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(SKYBOX_PASS_SHADER, type, "skybox_pass_fragment");
    skybox_pass_single_component.skybox_pass_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    skybox_pass_single_component.texture_uniform         = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
    skybox_pass_single_component.depth_uniform           = bgfx::createUniform("s_depth", bgfx::UniformType::Sampler);
    skybox_pass_single_component.skybox_texture_uniform  = bgfx::createUniform("s_skybox", bgfx::UniformType::Sampler);
    skybox_pass_single_component.rotation_uniform        = bgfx::createUniform("u_rotation", bgfx::UniformType::Mat4);

    reset(skybox_pass_single_component, window_single_component.width, window_single_component.height);

    bgfx::setViewClear(SKYBOX_PASS, BGFX_CLEAR_COLOR, 0x000000FF, 1.f, 0);
    bgfx::setViewName(SKYBOX_PASS, "skybox_pass");
}

SkyboxPassSystem::~SkyboxPassSystem() {
    auto& skybox_pass_single_component = world.ctx<SkyboxPassSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(skybox_pass_single_component.buffer);
    destroy_valid(skybox_pass_single_component.color_texture);
    destroy_valid(skybox_pass_single_component.depth_uniform);
    destroy_valid(skybox_pass_single_component.rotation_uniform);
    destroy_valid(skybox_pass_single_component.skybox_pass_program);
    destroy_valid(skybox_pass_single_component.skybox_texture_uniform);
    destroy_valid(skybox_pass_single_component.texture_uniform);
}

void SkyboxPassSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& geometry_pass_single_component = world.ctx<GeometryPassSingleComponent>();
    auto& lighting_pass_single_component = world.ctx<LightingPassSingleComponent>();
    auto& quad_single_component = world.ctx<QuadSingleComponent>();
    auto& skybox_pass_single_component = world.ctx<SkyboxPassSingleComponent>();
    auto& texture_single_component = world.ctx<TextureSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(skybox_pass_single_component, window_single_component.width, window_single_component.height);
    }

    const Texture& skybox_texture = texture_single_component.get("house.dds");
    if (!skybox_texture.is_cube_map) {
        // Skybox texture are missing.
        return;
    }

    bgfx::setViewTransform(SKYBOX_PASS, glm::value_ptr(camera_single_component.view_matrix), glm::value_ptr(camera_single_component.projection_matrix));

    bgfx::setVertexBuffer(0, quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);
    bgfx::setIndexBuffer(quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    bgfx::setTexture(0, skybox_pass_single_component.depth_uniform,          geometry_pass_single_component.depth_texture);
    bgfx::setTexture(1, skybox_pass_single_component.skybox_texture_uniform, skybox_texture.handle);

    const glm::mat4 rotation = glm::mat4_cast(camera_single_component.rotation);
    bgfx::setUniform(skybox_pass_single_component.rotation_uniform, &rotation);

    bgfx::setStencil(BGFX_STENCIL_TEST_NOTEQUAL | BGFX_STENCIL_FUNC_REF(1) | BGFX_STENCIL_FUNC_RMASK(1) |
                     BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP,
                     BGFX_STENCIL_NONE);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);

    bgfx::submit(SKYBOX_PASS, skybox_pass_single_component.skybox_pass_program);

    bgfx::setVertexBuffer(0, quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);
    bgfx::setIndexBuffer(quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    bgfx::setTexture(0, skybox_pass_single_component.texture_uniform, lighting_pass_single_component.color_texture);

    bgfx::setStencil(BGFX_STENCIL_NONE, BGFX_STENCIL_NONE);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_ALPHA);

    bgfx::submit(SKYBOX_PASS, quad_single_component.program);
}

void SkyboxPassSystem::reset(SkyboxPassSingleComponent &skybox_pass_single_component, uint16_t width, uint16_t height) const noexcept {
    using namespace skybox_pass_system_details;

    if (bgfx::isValid(skybox_pass_single_component.color_texture)) {
        bgfx::destroy(skybox_pass_single_component.color_texture);
    }

    if (bgfx::isValid(skybox_pass_single_component.buffer)) {
        bgfx::destroy(skybox_pass_single_component.buffer);
    }

    skybox_pass_single_component.color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA16F, ATTACHMENT_FLAGS);

    const bgfx::TextureHandle attachments[] = {
            skybox_pass_single_component.color_texture,
            world.ctx<GeometryPassSingleComponent>().depth_stencil_texture
    };

    skybox_pass_single_component.buffer = bgfx::createFrameBuffer(std::size(attachments), attachments, false);

    bgfx::setViewFrameBuffer(SKYBOX_PASS, skybox_pass_single_component.buffer);
    bgfx::setViewRect(SKYBOX_PASS, 0, 0, width, height);
}

} // namespace hg
