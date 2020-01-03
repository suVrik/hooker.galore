#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "core/resource/texture.h"
#include "shaders/geometry_pass/geometry_pass.fragment.h"
#include "shaders/geometry_pass/geometry_pass.vertex.h"
#include "world/render/camera_single_component.h"
#include "world/render/geometry_pass_single_component.h"
#include "world/render/geometry_pass_system.h"
#include "world/render/material_component.h"
#include "world/render/render_tags.h"
#include "world/resource/resource_geometry_component.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>
#include <glm/gtc/quaternion.hpp>

namespace hg {

namespace geometry_pass_system_details {

static const bgfx::EmbeddedShader GEOMETRY_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(geometry_pass_vertex),
        BGFX_EMBEDDED_SHADER(geometry_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

} // namespace render_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(GeometryPassSystem),
    TAGS(render),
    CONTEXT(GeometryPassSingleComponent),
    BEFORE("RenderSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "CameraSystem")
)

GeometryPassSystem::GeometryPassSystem(World& world)
        : NormalSystem(world) {
    using namespace geometry_pass_system_details;

    auto& geometry_pass_single_component = world.ctx<GeometryPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    reset(geometry_pass_single_component, window_single_component.width, window_single_component.height);
    
    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(GEOMETRY_PASS_SHADER, type, "geometry_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(GEOMETRY_PASS_SHADER, type, "geometry_pass_fragment");
    geometry_pass_single_component.program    = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    geometry_pass_single_component.color_roughness_sampler_uniform = bgfx::createUniform("s_color_roughness", bgfx::UniformType::Sampler);
    geometry_pass_single_component.normal_metal_ao_sampler_uniform = bgfx::createUniform("s_normal_metal_ao", bgfx::UniformType::Sampler);

    bgfx::setViewClear(GEOMETRY_PASS, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0xFFFFFFFF, 1.f, 0);
    bgfx::setViewName(GEOMETRY_PASS, "geometry_pass");
}

void GeometryPassSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& geometry_pass_single_component = world.ctx<GeometryPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(geometry_pass_single_component, window_single_component.width, window_single_component.height);
    }

    bgfx::setViewTransform(GEOMETRY_PASS, &camera_single_component.view_matrix, &camera_single_component.projection_matrix);

    bgfx::touch(GEOMETRY_PASS);
    world.group<ResourceGeometryComponent, MaterialComponent, TransformComponent>().each([&](entt::entity entity, ResourceGeometryComponent& resource_geometry_component, MaterialComponent& material_component, TransformComponent& transform_component) {
        const std::shared_ptr<ResourceGeometry>& geometry = resource_geometry_component.get_geometry();
        if (geometry && geometry->is_loaded() && material_component.color_roughness != nullptr && material_component.normal_metal_ao != nullptr) {
            assert(bgfx::isValid(geometry->get_index_buffer()));
            bgfx::setIndexBuffer(geometry->get_index_buffer(), 0, geometry->get_indices_count());

            assert(bgfx::isValid(geometry->get_vertex_buffer()));
            bgfx::setVertexBuffer(0, geometry->get_vertex_buffer(), 0, geometry->get_vertices_count());

            assert(bgfx::isValid(*geometry_pass_single_component.color_roughness_sampler_uniform));
            assert(bgfx::isValid(material_component.color_roughness->handle));
            bgfx::setTexture(0, *geometry_pass_single_component.color_roughness_sampler_uniform, material_component.color_roughness->handle);

            assert(bgfx::isValid(*geometry_pass_single_component.normal_metal_ao_sampler_uniform));
            assert(bgfx::isValid(material_component.normal_metal_ao->handle));
            bgfx::setTexture(1, *geometry_pass_single_component.normal_metal_ao_sampler_uniform, material_component.normal_metal_ao->handle);

            glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component.translation);
            transform = transform * glm::mat4_cast(transform_component.rotation);
            transform = glm::scale(transform, transform_component.scale);
            bgfx::setTransform(&transform, 1);

            bgfx::setStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(1) | BGFX_STENCIL_FUNC_RMASK(0xFF) |
                             BGFX_STENCIL_OP_FAIL_S_REPLACE | BGFX_STENCIL_OP_FAIL_Z_REPLACE | BGFX_STENCIL_OP_PASS_Z_REPLACE,
                             BGFX_STENCIL_NONE);
            bgfx::setState(BGFX_STATE_WRITE_MASK | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW);

            assert(bgfx::isValid(*geometry_pass_single_component.program));
            bgfx::submit(GEOMETRY_PASS, *geometry_pass_single_component.program);
        }
    });
}

void GeometryPassSystem::reset(GeometryPassSingleComponent& geometry_pass_single_component, uint16_t width, uint16_t height) {
    constexpr uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP;

    geometry_pass_single_component.color_roughness_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, ATTACHMENT_FLAGS);
    bgfx::setName(*geometry_pass_single_component.color_roughness_texture, "geometry_pass_color_roughness_texture");

    geometry_pass_single_component.normal_metal_ao_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA16F, ATTACHMENT_FLAGS);
    bgfx::setName(*geometry_pass_single_component.normal_metal_ao_texture, "geometry_pass_normal_metal_ao_texture");

    geometry_pass_single_component.depth_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::R32F, ATTACHMENT_FLAGS);
    bgfx::setName(*geometry_pass_single_component.depth_texture, "geometry_pass_depth_texture");

    geometry_pass_single_component.depth_stencil_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D24S8, ATTACHMENT_FLAGS);
    bgfx::setName(*geometry_pass_single_component.depth_stencil_texture, "geometry_pass_depth_stencil_texture");

    const bgfx::TextureHandle attachments[] = {
            *geometry_pass_single_component.color_roughness_texture,
            *geometry_pass_single_component.normal_metal_ao_texture,
            *geometry_pass_single_component.depth_texture,
            *geometry_pass_single_component.depth_stencil_texture
    };

    geometry_pass_single_component.buffer = bgfx::createFrameBuffer(static_cast<uint8_t>(std::size(attachments)), attachments, false);
    bgfx::setName(*geometry_pass_single_component.buffer, "geometry_pass_frame_buffer");

    bgfx::setViewFrameBuffer(GEOMETRY_PASS, *geometry_pass_single_component.buffer);
    bgfx::setViewRect(GEOMETRY_PASS, 0, 0, width, height);
}

} // namespace hg
