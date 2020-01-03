#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/outline_pass/outline_pass.vertex.h"
#include "shaders/picking_pass/picking_pass.fragment.h"
#include "world/render/camera_single_component.h"
#include "world/render/picking_pass_single_component.h"
#include "world/render/picking_pass_system.h"
#include "world/render/render_single_component.h"
#include "world/render/render_tags.h"
#include "world/resource/resource_geometry_component.h"
#include "world/shared/name_component.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>
#include <glm/gtc/quaternion.hpp>

namespace hg {

namespace picking_pass_system_details {

static const bgfx::EmbeddedShader PICKING_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(outline_pass_vertex),
        BGFX_EMBEDDED_SHADER(picking_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

} // namespace picking_pass_system

SYSTEM_DESCRIPTOR(
    SYSTEM(PickingPassSystem),
    TAGS(render),
    CONTEXT(PickingPassSingleComponent),
    BEFORE("RenderSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "CameraSystem")
)

PickingPassSystem::PickingPassSystem(World& world)
        : NormalSystem(world) {
    using namespace picking_pass_system_details;

    auto& picking_pass_single_component = world.ctx<PickingPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    reset(picking_pass_single_component, window_single_component.width, window_single_component.height);

    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(PICKING_PASS_SHADER, type, "outline_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(PICKING_PASS_SHADER, type, "picking_pass_fragment");
    picking_pass_single_component.program     = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    picking_pass_single_component.object_index_uniform = bgfx::createUniform("u_object_index", bgfx::UniformType::Vec4);

    bgfx::setViewClear(PICKING_PASS, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xFFFFFFFF, 1.f);
    bgfx::setViewName(PICKING_PASS, "picking_pass");
}

void PickingPassSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& picking_pass_single_component = world.ctx<PickingPassSingleComponent>();
    auto& render_single_component = world.ctx<RenderSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(picking_pass_single_component, window_single_component.width, window_single_component.height);
    }

    bool picking_requested = picking_pass_single_component.perform_picking;
    bool picking_in_progress = render_single_component.current_frame < picking_pass_single_component.target_frame;
    if (!picking_requested || picking_in_progress) {
        return;
    }

    picking_pass_single_component.perform_picking = false;
    picking_pass_single_component.target_data.resize(static_cast<size_t>(window_single_component.width) * window_single_component.height * 4);

    bgfx::setViewTransform(PICKING_PASS, &camera_single_component.view_matrix, &camera_single_component.projection_matrix);

    world.view<ResourceGeometryComponent, TransformComponent>().each([&](entt::entity entity, ResourceGeometryComponent& resource_geometry_component, TransformComponent& transform_component) {
        const std::shared_ptr<ResourceGeometry>& geometry = resource_geometry_component.get_geometry();
        if (geometry && geometry->is_loaded()) {
            assert(bgfx::isValid(geometry->get_index_buffer()));
            bgfx::setIndexBuffer(geometry->get_index_buffer(), 0, geometry->get_indices_count());

            assert(bgfx::isValid(geometry->get_vertex_buffer()));
            bgfx::setVertexBuffer(0, geometry->get_vertex_buffer(), 0, geometry->get_vertices_count());

            auto object_index = static_cast<uint32_t>(entity);

            glm::vec4 uniform_value;
            uniform_value.x = ((object_index >> 16) & 0xFF) / 255.f;
            uniform_value.y = ((object_index >> 8) & 0xFF) / 255.f;
            uniform_value.z = (object_index & 0xFF) / 255.f;
            uniform_value.w = ((object_index >> 24) & 0xFF) / 255.f;
            bgfx::setUniform(*picking_pass_single_component.object_index_uniform, &uniform_value);

            glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component.translation);
            transform = transform * glm::mat4_cast(transform_component.rotation);
            transform = glm::scale(transform, transform_component.scale);
            bgfx::setTransform(&transform, 1);

            bgfx::setState(BGFX_STATE_WRITE_MASK | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW);

            assert(bgfx::isValid(*picking_pass_single_component.program));
            bgfx::submit(PICKING_PASS, *picking_pass_single_component.program);
        }
    });

    assert(bgfx::isValid(*picking_pass_single_component.color_texture));
    assert(bgfx::isValid(*picking_pass_single_component.rt_color_texture));
    bgfx::blit(PICKING_BLIT_PASS, *picking_pass_single_component.color_texture, 0, 0, *picking_pass_single_component.rt_color_texture);
    picking_pass_single_component.target_frame = bgfx::readTexture(*picking_pass_single_component.color_texture, picking_pass_single_component.target_data.data());
}

void PickingPassSystem::reset(PickingPassSingleComponent& picking_pass_single_component, uint16_t width, uint16_t height) {
    constexpr uint64_t COLOR_TEXTURE_FLAGS = BGFX_TEXTURE_READ_BACK | BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP;
    constexpr uint64_t RENDER_TARGET_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP;

    picking_pass_single_component.color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, COLOR_TEXTURE_FLAGS);
    bgfx::setName(*picking_pass_single_component.color_texture, "picking_pass_color_texture");

    picking_pass_single_component.rt_color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, RENDER_TARGET_FLAGS);
    bgfx::setName(*picking_pass_single_component.rt_color_texture, "picking_pass_rt_color_texture");

    picking_pass_single_component.rt_depth_buffer = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D24S8, RENDER_TARGET_FLAGS);
    bgfx::setName(*picking_pass_single_component.rt_depth_buffer, "picking_pass_rt_depth_buffer");

    bgfx::TextureHandle attachments[] = {
            *picking_pass_single_component.rt_color_texture,
            *picking_pass_single_component.rt_depth_buffer
    };

    picking_pass_single_component.buffer = bgfx::createFrameBuffer(static_cast<uint8_t>(std::size(attachments)), attachments, false);
    bgfx::setName(*picking_pass_single_component.buffer, "picking_pass_frame_buffer");

    bgfx::setViewFrameBuffer(PICKING_PASS, *picking_pass_single_component.buffer);
    bgfx::setViewRect(PICKING_PASS, 0, 0, width, height);
}

} // namespace hg
