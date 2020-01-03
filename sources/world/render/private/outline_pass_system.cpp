#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/outline_blur_pass/outline_blur_pass.fragment.h"
#include "shaders/outline_pass/outline_pass.fragment.h"
#include "shaders/outline_pass/outline_pass.vertex.h"
#include "shaders/quad_pass/quad_pass.vertex.h"
#include "world/render/camera_single_component.h"
#include "world/render/outline_component.h"
#include "world/render/outline_pass_single_component.h"
#include "world/render/outline_pass_system.h"
#include "world/render/quad_single_component.h"
#include "world/render/render_tags.h"
#include "world/resource/resource_geometry_component.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>
#include <glm/gtc/quaternion.hpp>

namespace hg {

namespace outline_pass_system_details {

static const bgfx::EmbeddedShader OUTLINE_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(outline_pass_vertex),
        BGFX_EMBEDDED_SHADER(outline_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

static const bgfx::EmbeddedShader OUTLINE_BLUR_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(quad_pass_vertex),
        BGFX_EMBEDDED_SHADER(outline_blur_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

} // namespace outline_pass_system

SYSTEM_DESCRIPTOR(
    SYSTEM(OutlinePassSystem),
    TAGS(render),
    CONTEXT(OutlinePassSingleComponent),
    BEFORE("RenderSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "CameraSystem")
)

OutlinePassSystem::OutlinePassSystem(World& world)
        : NormalSystem(world) {
    using namespace outline_pass_system_details;

    auto& outline_pass_single_component = world.ctx<OutlinePassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    reset(outline_pass_single_component, window_single_component.width, window_single_component.height);

    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ShaderHandle vertex_shader_handle         = bgfx::createEmbeddedShader(OUTLINE_PASS_SHADER, type, "outline_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle       = bgfx::createEmbeddedShader(OUTLINE_PASS_SHADER, type, "outline_pass_fragment");
    outline_pass_single_component.offscreen_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    vertex_shader_handle   = bgfx::createEmbeddedShader(OUTLINE_BLUR_PASS_SHADER, type, "quad_pass_vertex");
    fragment_shader_handle = bgfx::createEmbeddedShader(OUTLINE_BLUR_PASS_SHADER, type, "outline_blur_pass_fragment");
    outline_pass_single_component.onscreen_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    outline_pass_single_component.texture_sampler_uniform = bgfx::createUniform("s_texture",       bgfx::UniformType::Sampler);
    outline_pass_single_component.outline_color_uniform   = bgfx::createUniform("u_outline_color", bgfx::UniformType::Vec4);
    outline_pass_single_component.group_index_uniform     = bgfx::createUniform("u_group_index",   bgfx::UniformType::Vec4);

    bgfx::setViewClear(OUTLINE_OFFSCREEN_PASS, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.f);
    bgfx::setViewName(OUTLINE_OFFSCREEN_PASS, "outline_pass");

    bgfx::setViewClear(OUTLINE_ONSCREEN_PASS, BGFX_CLEAR_NONE);
    bgfx::setViewName(OUTLINE_ONSCREEN_PASS, "outline_blur_pass");
}

void OutlinePassSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& outline_pass_single_component = world.ctx<OutlinePassSingleComponent>();
    auto& quad_single_component = world.ctx<QuadSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(outline_pass_single_component, window_single_component.width, window_single_component.height);
    }

    bgfx::setViewTransform(OUTLINE_OFFSCREEN_PASS, &camera_single_component.view_matrix, &camera_single_component.projection_matrix);

    bgfx::touch(OUTLINE_OFFSCREEN_PASS);
    world.group<OutlineComponent>(entt::get<ResourceGeometryComponent, TransformComponent>).each([&](entt::entity entity, OutlineComponent& outline_component, ResourceGeometryComponent& resource_geometry_component, TransformComponent& transform_component) {
        const std::shared_ptr<ResourceGeometry>& geometry = resource_geometry_component.get_geometry();
        if (geometry && geometry->is_loaded()) {
            assert(bgfx::isValid(geometry->get_index_buffer()));
            bgfx::setIndexBuffer(geometry->get_index_buffer(), 0, geometry->get_indices_count());

            assert(bgfx::isValid(geometry->get_vertex_buffer()));
            bgfx::setVertexBuffer(0, geometry->get_vertex_buffer(), 0, geometry->get_vertices_count());

            auto group_index = static_cast<uint32_t>(entity);

            glm::vec4 uniform_value;
            uniform_value.x = ((group_index >> 16) & 0xFF) / 255.f;
            uniform_value.y = ((group_index >> 8) & 0xFF) / 255.f;
            uniform_value.z = (group_index & 0xFF) / 255.f;
            uniform_value.w = 1.f;

            assert(bgfx::isValid(*outline_pass_single_component.group_index_uniform));
            bgfx::setUniform(*outline_pass_single_component.group_index_uniform, &uniform_value);

            glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component.translation);
            transform = transform * glm::mat4_cast(transform_component.rotation);
            transform = glm::scale(transform, transform_component.scale);
            bgfx::setTransform(&transform, 1);

            bgfx::setState(BGFX_STATE_WRITE_MASK | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW);

            assert(bgfx::isValid(*outline_pass_single_component.offscreen_program));
            bgfx::submit(OUTLINE_OFFSCREEN_PASS, *outline_pass_single_component.offscreen_program);
        }
    });

    assert(bgfx::isValid(*quad_single_component.vertex_buffer));
    bgfx::setVertexBuffer(0, *quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);

    assert(bgfx::isValid(*quad_single_component.index_buffer));
    bgfx::setIndexBuffer(*quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    assert(bgfx::isValid(*outline_pass_single_component.texture_sampler_uniform));
    assert(bgfx::isValid(*outline_pass_single_component.color_texture));
    bgfx::setTexture(0, *outline_pass_single_component.texture_sampler_uniform, *outline_pass_single_component.color_texture);

    assert(bgfx::isValid(*outline_pass_single_component.outline_color_uniform));
    bgfx::setUniform(*outline_pass_single_component.outline_color_uniform, &outline_pass_single_component.outline_color);

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_ALPHA);

    assert(bgfx::isValid(*outline_pass_single_component.onscreen_program));
    bgfx::submit(OUTLINE_ONSCREEN_PASS, *outline_pass_single_component.onscreen_program);
}

void OutlinePassSystem::reset(OutlinePassSingleComponent& outline_pass_single_component, uint16_t width, uint16_t height) {
    constexpr uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP;

    outline_pass_single_component.color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, ATTACHMENT_FLAGS);
    bgfx::setName(*outline_pass_single_component.color_texture, "outline_pass_color_texture");

    outline_pass_single_component.depth_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D24S8, ATTACHMENT_FLAGS);
    bgfx::setName(*outline_pass_single_component.color_texture, "outline_pass_depth_texture");

    const bgfx::TextureHandle attachments[] = {
            *outline_pass_single_component.color_texture,
            *outline_pass_single_component.depth_texture,
    };

    outline_pass_single_component.buffer = bgfx::createFrameBuffer(static_cast<uint8_t>(std::size(attachments)), attachments, false);
    bgfx::setName(*outline_pass_single_component.buffer, "outline_pass_frame_buffer");

    bgfx::setViewFrameBuffer(OUTLINE_OFFSCREEN_PASS, *outline_pass_single_component.buffer);
    bgfx::setViewRect(OUTLINE_OFFSCREEN_PASS, 0, 0, width, height);

    bgfx::setViewRect(OUTLINE_ONSCREEN_PASS, 0, 0, width, height);
}

} // namespace hg
