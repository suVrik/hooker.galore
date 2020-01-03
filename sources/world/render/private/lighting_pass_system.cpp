#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/lighting_pass/lighting_pass.fragment.h"
#include "shaders/quad_pass/quad_pass.vertex.h"
#include "world/render/camera_single_component.h"
#include "world/render/geometry_pass_single_component.h"
#include "world/render/light_component.h"
#include "world/render/lighting_pass_single_component.h"
#include "world/render/lighting_pass_system.h"
#include "world/render/quad_single_component.h"
#include "world/render/render_tags.h"
#include "world/render/texture_single_component.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>
#include <debug_draw.hpp>

namespace hg {

namespace lighting_pass_system_details {

static const bgfx::EmbeddedShader LIGHTING_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(quad_pass_vertex),
        BGFX_EMBEDDED_SHADER(lighting_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

} // namespace lighting_pass_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(LightingPassSystem),
    TAGS(render),
    CONTEXT(LightingPassSingleComponent),
    BEFORE("RenderSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "CameraSystem", "GeometryPassSystem")
)

LightingPassSystem::LightingPassSystem(World& world)
        : NormalSystem(world) {
    using namespace lighting_pass_system_details;

    auto& lighting_pass_single_component = world.ctx<LightingPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(LIGHTING_PASS_SHADER, type, "quad_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(LIGHTING_PASS_SHADER, type, "lighting_pass_fragment");
    lighting_pass_single_component.program    = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    lighting_pass_single_component.color_roughness_sampler_uniform = bgfx::createUniform("s_color_roughness", bgfx::UniformType::Sampler);
    lighting_pass_single_component.normal_metal_ao_sampler_uniform = bgfx::createUniform("s_normal_metal_ao", bgfx::UniformType::Sampler);
    lighting_pass_single_component.depth_sampler_uniform           = bgfx::createUniform("s_depth",           bgfx::UniformType::Sampler);

    lighting_pass_single_component.skybox_irradiance_sampler_uniform = bgfx::createUniform("s_skybox_irradiance", bgfx::UniformType::Sampler);
    lighting_pass_single_component.skybox_prefilter_sampler_uniform  = bgfx::createUniform("s_skybox_prefilter", bgfx::UniformType::Sampler);
    lighting_pass_single_component.skybox_lut_sampler_uniform        = bgfx::createUniform("s_skybox_lut",        bgfx::UniformType::Sampler);

    lighting_pass_single_component.light_color_uniform              = bgfx::createUniform("u_light_color",       bgfx::UniformType::Vec4);
    lighting_pass_single_component.light_position_uniform           = bgfx::createUniform("u_light_position",    bgfx::UniformType::Vec4);
    lighting_pass_single_component.skybox_mip_prefilter_max_uniform = bgfx::createUniform("u_mip_prefilter_max", bgfx::UniformType::Vec4);

    reset(lighting_pass_single_component, window_single_component.width, window_single_component.height);

    bgfx::setViewClear(LIGHTING_PASS, BGFX_CLEAR_COLOR, 0x00000000);
    bgfx::setViewName(LIGHTING_PASS, "lighting_pass");
}

void LightingPassSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& geometry_pass_single_component = world.ctx<GeometryPassSingleComponent>();
    auto& lighting_pass_single_component = world.ctx<LightingPassSingleComponent>();
    auto& quad_single_component = world.ctx<QuadSingleComponent>();
    auto& texture_single_component = world.ctx<TextureSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(lighting_pass_single_component, window_single_component.width, window_single_component.height);
    }

    // TODO: Get actual irradiance, prefilter and BRDF textures from level file.
    const Texture& irradiance_texture = texture_single_component.get("house_irradiance.dds");
    const Texture& prefilter_texture = texture_single_component.get("house_prefilter.dds");
    const Texture& brdf_lut_texture = texture_single_component.get("brdf_lut.dds");

    if (!irradiance_texture.is_cube_map || !prefilter_texture.is_cube_map) {
        return;
    }

    bgfx::setViewTransform(LIGHTING_PASS, &camera_single_component.view_matrix, &camera_single_component.projection_matrix);

    assert(bgfx::isValid(*quad_single_component.index_buffer));
    bgfx::setIndexBuffer(*quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    assert(bgfx::isValid(*quad_single_component.vertex_buffer));
    bgfx::setVertexBuffer(0, *quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);

    assert(bgfx::isValid(*lighting_pass_single_component.color_roughness_sampler_uniform));
    assert(bgfx::isValid(*geometry_pass_single_component.color_roughness_texture));
    bgfx::setTexture(0, *lighting_pass_single_component.color_roughness_sampler_uniform, *geometry_pass_single_component.color_roughness_texture);

    assert(bgfx::isValid(*lighting_pass_single_component.normal_metal_ao_sampler_uniform));
    assert(bgfx::isValid(*geometry_pass_single_component.normal_metal_ao_texture));
    bgfx::setTexture(1, *lighting_pass_single_component.normal_metal_ao_sampler_uniform, *geometry_pass_single_component.normal_metal_ao_texture);

    assert(bgfx::isValid(*lighting_pass_single_component.depth_sampler_uniform));
    assert(bgfx::isValid(*geometry_pass_single_component.depth_texture));
    bgfx::setTexture(2, *lighting_pass_single_component.depth_sampler_uniform, *geometry_pass_single_component.depth_texture);

    assert(bgfx::isValid(*lighting_pass_single_component.skybox_irradiance_sampler_uniform));
    assert(bgfx::isValid(irradiance_texture.handle));
    bgfx::setTexture(3, *lighting_pass_single_component.skybox_irradiance_sampler_uniform, irradiance_texture.handle);

    assert(bgfx::isValid(*lighting_pass_single_component.skybox_prefilter_sampler_uniform));
    assert(bgfx::isValid(prefilter_texture.handle));
    bgfx::setTexture(4, *lighting_pass_single_component.skybox_prefilter_sampler_uniform, prefilter_texture.handle);

    assert(bgfx::isValid(*lighting_pass_single_component.skybox_lut_sampler_uniform));
    assert(bgfx::isValid(brdf_lut_texture.handle));
    bgfx::setTexture(5, *lighting_pass_single_component.skybox_lut_sampler_uniform, brdf_lut_texture.handle, BGFX_SAMPLER_UVW_CLAMP);

    glm::vec4 mip_prefilter_max(4.f, 0.f, 0.f, 0.f);
    assert(bgfx::isValid(*lighting_pass_single_component.skybox_mip_prefilter_max_uniform));
    bgfx::setUniform(*lighting_pass_single_component.skybox_mip_prefilter_max_uniform, &mip_prefilter_max);

    world.view<LightComponent, TransformComponent>().each([&](entt::entity /*entity*/, LightComponent& light_component, TransformComponent& transform_component) {
        glm::vec4 light_color(light_component.color, 0.f);
        assert(bgfx::isValid(*lighting_pass_single_component.light_color_uniform));
        bgfx::setUniform(*lighting_pass_single_component.light_color_uniform, &light_color, 1);

        glm::vec4 light_position(transform_component.translation, 0.f);
        assert(bgfx::isValid(*lighting_pass_single_component.light_position_uniform));
        bgfx::setUniform(*lighting_pass_single_component.light_position_uniform, &light_position, 1);

        dd::sphere(transform_component.translation, glm::vec3(1.f, 1.f, 1.f), 0.1f);
    });

    bgfx::setStencil(BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_FUNC_REF(1) | BGFX_STENCIL_FUNC_RMASK(1) |
                     BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP,
                     BGFX_STENCIL_NONE);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);

    assert(bgfx::isValid(*lighting_pass_single_component.program));
    bgfx::submit(LIGHTING_PASS, *lighting_pass_single_component.program);
}

void LightingPassSystem::reset(LightingPassSingleComponent& lighting_pass_single_component, uint16_t width, uint16_t height) {
    constexpr uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP;

    lighting_pass_single_component.color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA16F, ATTACHMENT_FLAGS);
    bgfx::setName(*lighting_pass_single_component.color_texture, "lighting_pass_color_texture");

    auto& geometry_pass_single_component = world.ctx<GeometryPassSingleComponent>();
    assert(bgfx::isValid(*geometry_pass_single_component.depth_stencil_texture));

    bgfx::TextureHandle attachments[] = {
            *lighting_pass_single_component.color_texture,
            *geometry_pass_single_component.depth_stencil_texture,
    };

    lighting_pass_single_component.buffer = bgfx::createFrameBuffer(static_cast<uint8_t>(std::size(attachments)), attachments, false);
    bgfx::setName(*lighting_pass_single_component.buffer, "lighting_pass_frame_buffer");

    bgfx::setViewFrameBuffer(LIGHTING_PASS, *lighting_pass_single_component.buffer);
    bgfx::setViewRect(LIGHTING_PASS, 0, 0, width, height);
}

} // namespace hg
