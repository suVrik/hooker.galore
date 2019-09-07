#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/lighting_pass/lighting_pass.fragment.h"
#include "shaders/lighting_pass/lighting_pass.vertex.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/geometry_pass_single_component.h"
#include "world/shared/render/light_component.h"
#include "world/shared/render/lighting_pass_single_component.h"
#include "world/shared/render/lighting_pass_system.h"
#include "world/shared/render/quad_single_component.h"
#include "world/shared/render/texture_single_component.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>
#include <debug_draw.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace hg {

namespace lighting_pass_system_details {

static const bgfx::EmbeddedShader LIGHTING_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(lighting_pass_vertex),
        BGFX_EMBEDDED_SHADER(lighting_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

static const uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

} // namespace lighting_pass_system_details

LightingPassSystem::LightingPassSystem(World& world)
        : NormalSystem(world) {
    using namespace lighting_pass_system_details;

    auto& lighting_pass_single_component = world.set<LightingPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    bgfx::RendererType::Enum type = bgfx::getRendererType();
    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(LIGHTING_PASS_SHADER, type, "lighting_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(LIGHTING_PASS_SHADER, type, "lighting_pass_fragment");
    lighting_pass_single_component.lighting_pass_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    lighting_pass_single_component.color_roughness_uniform     = bgfx::createUniform("s_color_roughness",     bgfx::UniformType::Sampler);
    lighting_pass_single_component.depth_uniform               = bgfx::createUniform("s_depth",               bgfx::UniformType::Sampler);
    lighting_pass_single_component.light_color_uniform         = bgfx::createUniform("u_light_color",         bgfx::UniformType::Vec4);
    lighting_pass_single_component.light_position_uniform      = bgfx::createUniform("u_light_position",      bgfx::UniformType::Vec4);
    lighting_pass_single_component.normal_metal_ao_uniform     = bgfx::createUniform("s_normal_metal_ao",     bgfx::UniformType::Sampler);

    lighting_pass_single_component.skybox_texture_irradiance_uniform = bgfx::createUniform("s_skybox_irradiance", bgfx::UniformType::Sampler);
    lighting_pass_single_component.skybox_texture_lut_uniform        = bgfx::createUniform("s_skybox_lut",        bgfx::UniformType::Sampler);
    lighting_pass_single_component.skybox_texture_prefilter_uniform  = bgfx::createUniform("s_skybox_prefilter",  bgfx::UniformType::Sampler);
    lighting_pass_single_component.skybox_mip_prefilter_max_uniform  = bgfx::createUniform("u_mip_prefilter_max", bgfx::UniformType::Vec4);

    reset(lighting_pass_single_component, window_single_component.width, window_single_component.height);

    bgfx::setViewClear(LIGHTING_PASS, BGFX_CLEAR_COLOR, 0x00000000, 1.f, 0);
    bgfx::setViewName(LIGHTING_PASS, "lighting_pass");
}

LightingPassSystem::~LightingPassSystem() {
    auto& lighting_pass_single_component = world.ctx<LightingPassSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(lighting_pass_single_component.buffer);
    destroy_valid(lighting_pass_single_component.color_roughness_uniform);
    destroy_valid(lighting_pass_single_component.color_texture);
    destroy_valid(lighting_pass_single_component.depth_uniform);
    destroy_valid(lighting_pass_single_component.light_color_uniform);
    destroy_valid(lighting_pass_single_component.light_position_uniform);
    destroy_valid(lighting_pass_single_component.lighting_pass_program);
    destroy_valid(lighting_pass_single_component.normal_metal_ao_uniform);
    destroy_valid(lighting_pass_single_component.texture_uniform);

    destroy_valid(lighting_pass_single_component.skybox_mip_prefilter_max_uniform);
    destroy_valid(lighting_pass_single_component.skybox_texture_irradiance_uniform);
    destroy_valid(lighting_pass_single_component.skybox_texture_lut_uniform);
    destroy_valid(lighting_pass_single_component.skybox_texture_prefilter_uniform);
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

    // TODO: Get actual skybox from level file or something.
    const Texture& irradiance_texture = texture_single_component.get("house_irradiance.dds");
    const Texture& prefilter_texture = texture_single_component.get("house_prefilter.dds");
    const Texture& brdf_lut_texture = texture_single_component.get("brdf_lut.dds");

    if (!irradiance_texture.is_cube_map || !prefilter_texture.is_cube_map) {
        // Skybox textures are missing.
        return;
    }

    bgfx::setViewTransform(LIGHTING_PASS, glm::value_ptr(camera_single_component.view_matrix), glm::value_ptr(camera_single_component.projection_matrix));

    bgfx::setVertexBuffer(0, quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);
    bgfx::setIndexBuffer(quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    bgfx::setTexture(0, lighting_pass_single_component.color_roughness_uniform, geometry_pass_single_component.color_roughness_texture);
    bgfx::setTexture(1, lighting_pass_single_component.normal_metal_ao_uniform, geometry_pass_single_component.normal_metal_ao_texture);
    bgfx::setTexture(2, lighting_pass_single_component.depth_uniform,           geometry_pass_single_component.depth_texture);
    bgfx::setTexture(3, lighting_pass_single_component.skybox_texture_irradiance_uniform, irradiance_texture.handle);
    bgfx::setTexture(4, lighting_pass_single_component.skybox_texture_prefilter_uniform,  prefilter_texture.handle);
    bgfx::setTexture(5, lighting_pass_single_component.skybox_texture_lut_uniform,        brdf_lut_texture.handle, BGFX_SAMPLER_UVW_CLAMP);

    const glm::vec4 mip_prefilter_max(4.f, 0.f, 0.f, 0.f);
    bgfx::setUniform(lighting_pass_single_component.skybox_mip_prefilter_max_uniform, &mip_prefilter_max);

    world.view<LightComponent, TransformComponent>().each([&](entt::entity, LightComponent& light_component, TransformComponent& transform_component) {
        const glm::vec4 light_position(transform_component.translation, 0.f);
        const glm::vec4 light_color(light_component.color, 0.f);
        bgfx::setUniform(lighting_pass_single_component.light_position_uniform, glm::value_ptr(light_position), 1);
        bgfx::setUniform(lighting_pass_single_component.light_color_uniform,    glm::value_ptr(light_color), 1);
        dd::sphere(transform_component.translation, glm::vec3(1.f, 1.f, 1.f), 0.1f);
    });

    bgfx::setStencil(BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_FUNC_REF(1) | BGFX_STENCIL_FUNC_RMASK(1) |
                     BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP,
                     BGFX_STENCIL_NONE);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);

    bgfx::submit(LIGHTING_PASS, lighting_pass_single_component.lighting_pass_program);
}

void LightingPassSystem::reset(LightingPassSingleComponent &lighting_pass_single_component, uint16_t width, uint16_t height) const noexcept {
    using namespace lighting_pass_system_details;

    if (bgfx::isValid(lighting_pass_single_component.color_texture)) {
        bgfx::destroy(lighting_pass_single_component.color_texture);
    }

    if (bgfx::isValid(lighting_pass_single_component.buffer)) {
        bgfx::destroy(lighting_pass_single_component.buffer);
    }

    lighting_pass_single_component.color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA16F, ATTACHMENT_FLAGS);

    const bgfx::TextureHandle attachments[] = {
            lighting_pass_single_component.color_texture,
            world.ctx<GeometryPassSingleComponent>().depth_stencil_texture
    };

    lighting_pass_single_component.buffer = bgfx::createFrameBuffer(static_cast<uint8_t>(std::size(attachments)), attachments, false);

    bgfx::setViewFrameBuffer(LIGHTING_PASS, lighting_pass_single_component.buffer);
    bgfx::setViewRect(LIGHTING_PASS, 0, 0, width, height);
}

} // namespace hg
