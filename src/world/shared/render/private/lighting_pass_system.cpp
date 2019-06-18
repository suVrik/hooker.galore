#include "core/ecs/world.h"
#include "core/render/debug_draw.h"
#include "core/render/render_pass.h"
#include "shaders/lighting_pass/lighting_pass.fragment.h"
#include "shaders/lighting_pass/lighting_pass.vertex.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/geometry_pass_single_component.h"
#include "world/shared/render/lighting_pass_single_component.h"
#include "world/shared/render/lighting_pass_system.h"
#include "world/shared/render/quad_single_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
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
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    bgfx::RendererType::Enum type = bgfx::getRendererType();
    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(LIGHTING_PASS_SHADER, type, "lighting_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(LIGHTING_PASS_SHADER, type, "lighting_pass_fragment");
    lighting_pass_single_component.lighting_pass_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    lighting_pass_single_component.color_roughness_uniform = bgfx::createUniform("s_color_roughness", bgfx::UniformType::Sampler);
    lighting_pass_single_component.normal_metal_ao_uniform = bgfx::createUniform("s_normal_metal_ao", bgfx::UniformType::Sampler);
    lighting_pass_single_component.depth_uniform           = bgfx::createUniform("s_depth",           bgfx::UniformType::Sampler);

    lighting_pass_single_component.u_light_position = bgfx::createUniform("u_light_position", bgfx::UniformType::Vec4);

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
    destroy_valid(lighting_pass_single_component.u_light_position);
}

void LightingPassSystem::update(float /*elapsed_time*/) {
    assert(world.after("WindowSystem") && world.after("RenderFetchSystem") && world.after("GeometryPassSystem") && world.after("QuadSystem") && world.before("RenderSystem"));

    auto& lighting_pass_single_component = world.ctx<LightingPassSingleComponent>();
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

    static float t = 0.f;
    t += 0.02;
    float pos[4] = { std::cos(t) * 3.f, std::sin(t) + 5.f, std::sin(t) * 3.f, 1.f };

    bgfx::setTexture(0, lighting_pass_single_component.color_roughness_uniform, geometry_pass_single_component.color_roughness_texture);
    bgfx::setTexture(1, lighting_pass_single_component.normal_metal_ao_uniform, geometry_pass_single_component.normal_metal_ao_texture);
    bgfx::setTexture(2, lighting_pass_single_component.depth_uniform,           geometry_pass_single_component.depth_texture);
    bgfx::setUniform(lighting_pass_single_component.u_light_position, pos, 1);

    dd::sphere(glm::vec3(pos[0], pos[1], pos[2]), glm::vec3(1.f, 1.f, 1.f), 0.1f);

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);

    bgfx::submit(LIGHTING_PASS, lighting_pass_single_component.lighting_pass_program);
}

} // namespace hg
