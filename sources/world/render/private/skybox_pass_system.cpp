#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/quad_pass/quad_pass.vertex.h"
#include "shaders/skybox_pass/skybox_pass.fragment.h"
#include "world/render/camera_single_component.h"
#include "world/render/geometry_pass_single_component.h"
#include "world/render/lighting_pass_single_component.h"
#include "world/render/quad_single_component.h"
#include "world/render/render_tags.h"
#include "world/render/skybox_pass_single_component.h"
#include "world/render/skybox_pass_system.h"
#include "world/render/texture_single_component.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>
#include <glm/gtc/quaternion.hpp>

namespace hg {

namespace skybox_pass_system_details {

static const bgfx::EmbeddedShader SKYBOX_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(quad_pass_vertex),
        BGFX_EMBEDDED_SHADER(skybox_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

} // namespace skybox_pass_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(SkyboxPassSystem),
    TAGS(render),
    CONTEXT(SkyboxPassSingleComponent),
    BEFORE("RenderSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "CameraSystem", "GeometryPassSystem", "LightingPassSystem")
)

SkyboxPassSystem::SkyboxPassSystem(World& world)
        : NormalSystem(world) {
    using namespace skybox_pass_system_details;

    auto& skybox_pass_single_component = world.ctx<SkyboxPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(SKYBOX_PASS_SHADER, type, "quad_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(SKYBOX_PASS_SHADER, type, "skybox_pass_fragment");
    skybox_pass_single_component.program      = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    skybox_pass_single_component.skybox_sampler_uniform = bgfx::createUniform("s_skybox",   bgfx::UniformType::Sampler);
    skybox_pass_single_component.rotation_uniform       = bgfx::createUniform("u_rotation", bgfx::UniformType::Mat4);

    reset(skybox_pass_single_component, window_single_component.width, window_single_component.height);

    bgfx::setViewClear(SKYBOX_PASS, BGFX_CLEAR_COLOR, 0x000000FF);
    bgfx::setViewName(SKYBOX_PASS, "skybox_pass");
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

    // TODO: Get actual skybox texture from level file.
    const Texture& skybox_texture = texture_single_component.get("house.dds");
    if (!skybox_texture.is_cube_map) {
        return;
    }

    bgfx::setViewTransform(SKYBOX_PASS, &camera_single_component.view_matrix, &camera_single_component.projection_matrix);

    assert(bgfx::isValid(*quad_single_component.vertex_buffer));
    bgfx::setVertexBuffer(0, *quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);

    assert(bgfx::isValid(*quad_single_component.index_buffer));
    bgfx::setIndexBuffer(*quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    assert(bgfx::isValid(*quad_single_component.texture_sampler_uniform));
    assert(bgfx::isValid(*lighting_pass_single_component.color_texture));
    bgfx::setTexture(0, *quad_single_component.texture_sampler_uniform, *lighting_pass_single_component.color_texture);

    bgfx::setStencil(BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_FUNC_REF(1) | BGFX_STENCIL_FUNC_RMASK(1) |
                     BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP,
                     BGFX_STENCIL_NONE);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);

    assert(bgfx::isValid(*quad_single_component.program));
    bgfx::submit(SKYBOX_PASS, *quad_single_component.program);

    assert(bgfx::isValid(*quad_single_component.vertex_buffer));
    bgfx::setVertexBuffer(0, *quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);

    assert(bgfx::isValid(*quad_single_component.index_buffer));
    bgfx::setIndexBuffer(*quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    assert(bgfx::isValid(*skybox_pass_single_component.skybox_sampler_uniform));
    assert(bgfx::isValid(skybox_texture.handle));
    bgfx::setTexture(0, *skybox_pass_single_component.skybox_sampler_uniform, skybox_texture.handle);

    const glm::mat4 rotation = glm::mat4_cast(camera_single_component.rotation);
    assert(bgfx::isValid(*skybox_pass_single_component.rotation_uniform));
    bgfx::setUniform(*skybox_pass_single_component.rotation_uniform, &rotation);

    bgfx::setStencil(BGFX_STENCIL_TEST_NOTEQUAL | BGFX_STENCIL_FUNC_REF(1) | BGFX_STENCIL_FUNC_RMASK(1) |
                     BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP,
                     BGFX_STENCIL_NONE);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);

    assert(bgfx::isValid(*skybox_pass_single_component.program));
    bgfx::submit(SKYBOX_PASS, *skybox_pass_single_component.program);
}

void SkyboxPassSystem::reset(SkyboxPassSingleComponent& skybox_pass_single_component, uint16_t width, uint16_t height) {
    constexpr uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP;

    skybox_pass_single_component.color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA16F, ATTACHMENT_FLAGS);
    bgfx::setName(*skybox_pass_single_component.color_texture, "skybox_pass_color_texture");

    auto& geometry_pass_single_component = world.ctx<GeometryPassSingleComponent>();
    assert(bgfx::isValid(*geometry_pass_single_component.depth_stencil_texture));

    const bgfx::TextureHandle attachments[] = {
            *skybox_pass_single_component.color_texture,
            *geometry_pass_single_component.depth_stencil_texture,
    };

    skybox_pass_single_component.buffer = bgfx::createFrameBuffer(static_cast<uint8_t>(std::size(attachments)), attachments, false);
    bgfx::setName(*skybox_pass_single_component.color_texture, "skybox_pass_frame_buffer");

    bgfx::setViewFrameBuffer(SKYBOX_PASS, *skybox_pass_single_component.buffer);
    bgfx::setViewRect(SKYBOX_PASS, 0, 0, width, height);
}

} // namespace hg
