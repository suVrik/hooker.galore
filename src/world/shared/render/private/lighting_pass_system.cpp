#include "core/ecs/world.h"
#include "core/render/debug_draw.h"
#include "core/render/render_pass.h"
#include "shaders/lighting_pass/lighting_pass.fragment.h"
#include "shaders/lighting_pass/lighting_pass.vertex.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/geometry_pass_single_component.h"
#include "world/shared/render/light_component.h"
#include "world/shared/render/lighting_pass_single_component.h"
#include "world/shared/render/lighting_pass_system.h"
#include "world/shared/render/quad_single_component.h"
#include "world/shared/render/skybox_single_component.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <entt/entity/registry.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace hg {

namespace lighting_pass_system_details {

static const bgfx::EmbeddedShader LIGHTING_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(lighting_pass_vertex),
        BGFX_EMBEDDED_SHADER(lighting_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

} // namespace lighting_pass_system_details

LightingPassSystem::LightingPassSystem(World& world)
        : NormalSystem(world) {
    using namespace lighting_pass_system_details;

    auto& lighting_pass_single_component = world.set<LightingPassSingleComponent>();
    auto& skybox_single_component = world.ctx<SkyboxSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    bgfx::RendererType::Enum type = bgfx::getRendererType();
    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(LIGHTING_PASS_SHADER, type, "lighting_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(LIGHTING_PASS_SHADER, type, "lighting_pass_fragment");
    lighting_pass_single_component.lighting_pass_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    lighting_pass_single_component.color_roughness_uniform = bgfx::createUniform("s_color_roughness", bgfx::UniformType::Sampler);
    lighting_pass_single_component.normal_metal_ao_uniform = bgfx::createUniform("s_normal_metal_ao", bgfx::UniformType::Sampler);
    lighting_pass_single_component.depth_uniform           = bgfx::createUniform("s_depth",           bgfx::UniformType::Sampler);
    lighting_pass_single_component.light_position_uniform  = bgfx::createUniform("u_light_position",  bgfx::UniformType::Vec4);
    lighting_pass_single_component.light_color_uniform     = bgfx::createUniform("u_light_color",     bgfx::UniformType::Vec4);

    skybox_single_component.texture_uniform  = bgfx::createUniform("s_skybox", bgfx::UniformType::Sampler);
    skybox_single_component.texture_irradiance_uniform = bgfx::createUniform("s_skybox_irradiance", bgfx::UniformType::Sampler);
    skybox_single_component.texture_prefilter_uniform = bgfx::createUniform("s_skybox_prefilter", bgfx::UniformType::Sampler);
    skybox_single_component.texture_lut_uniform = bgfx::createUniform("s_skybox_lut", bgfx::UniformType::Sampler);
    skybox_single_component.mip_prefilter_max_uniform = bgfx::createUniform("u_mip_prefilter_max", bgfx::UniformType::Sampler);
    skybox_single_component.rotation_uniform = bgfx::createUniform("u_rotation", bgfx::UniformType::Mat4);

    bgfx::setViewClear(LIGHTING_PASS, BGFX_CLEAR_COLOR, 0x000000FF, 1.f, 0);
    bgfx::setViewRect(LIGHTING_PASS, 0, 0, window_single_component.width, window_single_component.height);
}

LightingPassSystem::~LightingPassSystem() {
    auto& lighting_pass_single_component = world.ctx<LightingPassSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(lighting_pass_single_component.lighting_pass_program);
    destroy_valid(lighting_pass_single_component.color_roughness_uniform);
    destroy_valid(lighting_pass_single_component.normal_metal_ao_uniform);
    destroy_valid(lighting_pass_single_component.depth_uniform);
    destroy_valid(lighting_pass_single_component.light_position_uniform);
    destroy_valid(lighting_pass_single_component.light_color_uniform);
}

void LightingPassSystem::update(float /*elapsed_time*/) {
    assert(world.after("WindowSystem") && world.after("RenderFetchSystem") && world.after("GeometryPassSystem") && world.after("QuadSystem") && world.before("RenderSystem"));

    auto& lighting_pass_single_component = world.ctx<LightingPassSingleComponent>();
    auto& skybox_single_component = world.ctx<SkyboxSingleComponent>();
    auto& geometry_pass_single_component = world.ctx<GeometryPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& quad_single_component = world.ctx<QuadSingleComponent>();

    if (window_single_component.resized) {
        bgfx::setViewRect(LIGHTING_PASS, 0, 0, window_single_component.width, window_single_component.height);
    }

    bgfx::setViewTransform(LIGHTING_PASS, glm::value_ptr(camera_single_component.view_matrix), glm::value_ptr(camera_single_component.projection_matrix));

    bgfx::setVertexBuffer(0, quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);
    bgfx::setIndexBuffer(quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    glm::quat quat_rot = camera_single_component.rotation;
    quat_rot.w = -quat_rot.w;
    quat_rot.y = -quat_rot.y;
    glm::mat4 rotation = glm::mat4_cast(quat_rot);

    bgfx::setTexture(0, lighting_pass_single_component.color_roughness_uniform, geometry_pass_single_component.color_roughness_texture);
    bgfx::setTexture(1, lighting_pass_single_component.normal_metal_ao_uniform, geometry_pass_single_component.normal_metal_ao_texture);
    bgfx::setTexture(2, lighting_pass_single_component.depth_uniform,           geometry_pass_single_component.depth_texture);
    bgfx::setTexture(3, skybox_single_component.texture_uniform,                skybox_single_component.texture);
    bgfx::setTexture(4, skybox_single_component.texture_irradiance_uniform,     skybox_single_component.texture_irradiance);
    bgfx::setTexture(5, skybox_single_component.texture_prefilter_uniform,      skybox_single_component.texture_prefilter);
    bgfx::setTexture(6, skybox_single_component.texture_lut_uniform,            skybox_single_component.texture_lut);

    bgfx::setUniform(skybox_single_component.rotation_uniform, &rotation);
    bgfx::setUniform(skybox_single_component.mip_prefilter_max_uniform, &skybox_single_component.mip_prefilter_max);

    world.view<LightComponent, TransformComponent>().each([&](entt::entity, LightComponent& light_component, TransformComponent& transform_component) {
        const glm::vec4 light_position(transform_component.translation, 0.f);
        const glm::vec4 light_color(light_component.color, 0.f);
        bgfx::setUniform(lighting_pass_single_component.light_position_uniform, glm::value_ptr(light_position), 1);
        bgfx::setUniform(lighting_pass_single_component.light_color_uniform,    glm::value_ptr(light_color), 1);
        dd::sphere(transform_component.translation, glm::vec3(1.f, 1.f, 1.f), 0.1f);
    });

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);

    bgfx::submit(LIGHTING_PASS, lighting_pass_single_component.lighting_pass_program);
}

} // namespace hg
