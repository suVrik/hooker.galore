#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/outline_pass/outline_pass.vertex.h"
#include "shaders/picking_pass/picking_pass.fragment.h"
#include "world/render/camera_single_component.h"
#include "world/render/model_component.h"
#include "world/render/picking_pass_single_component.h"
#include "world/render/picking_pass_system.h"
#include "world/render/render_single_component.h"
#include "world/render/render_tags.h"
#include "world/shared/name_component.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>
#include <glm/gtc/type_ptr.hpp>

namespace hg {

namespace picking_pass_system_details {

static const bgfx::EmbeddedShader PICKING_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(outline_pass_vertex),
        BGFX_EMBEDDED_SHADER(picking_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

static const uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_READ_BACK | BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
static const uint64_t RT_ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

} // namespace picking_pass_system

SYSTEM_DESCRIPTOR(
    SYSTEM(PickingPassSystem),
    TAGS(render),
    BEFORE("RenderSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "CameraSystem")
)

PickingPassSystem::PickingPassSystem(World& world)
        : NormalSystem(world) {
    using namespace picking_pass_system_details;

    auto& picking_pass_single_component = world.set<PickingPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    reset(picking_pass_single_component, window_single_component.width, window_single_component.height);

    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(PICKING_PASS_SHADER, type, "outline_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(PICKING_PASS_SHADER, type, "picking_pass_fragment");
    picking_pass_single_component.program     = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    picking_pass_single_component.object_index_uniform = bgfx::createUniform("u_object_index", bgfx::UniformType::Vec4);

    bgfx::setViewClear(PICKING_PASS, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xFFFFFFFF, 1.f, 0);
    bgfx::setViewName(PICKING_PASS, "picking_pass");
}

PickingPassSystem::~PickingPassSystem() {
    auto& picking_pass_single_component = world.ctx<PickingPassSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(picking_pass_single_component.buffer);
    destroy_valid(picking_pass_single_component.color_texture);
    destroy_valid(picking_pass_single_component.object_index_uniform);
    destroy_valid(picking_pass_single_component.program);
}

void PickingPassSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& picking_pass_single_component = world.ctx<PickingPassSingleComponent>();
    auto& render_single_component = world.ctx<RenderSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(picking_pass_single_component, window_single_component.width, window_single_component.height);
    }

    if (!picking_pass_single_component.perform_picking || render_single_component.current_frame < picking_pass_single_component.target_frame) {
        return;
    }
    picking_pass_single_component.perform_picking = false;

    picking_pass_single_component.target_data.resize(static_cast<size_t>(window_single_component.width) * window_single_component.height * 4);

    bgfx::setViewTransform(PICKING_PASS, glm::value_ptr(camera_single_component.view_matrix), glm::value_ptr(camera_single_component.projection_matrix));

    world.view<ModelComponent, TransformComponent>().each([&](const entt::entity entity, ModelComponent& model_component, TransformComponent& transform_component) {
        glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component.translation);
        transform = transform * glm::mat4_cast(transform_component.rotation);
        transform = glm::scale(transform, transform_component.scale);

        for (const Model::Node& node : model_component.model.children) {
            draw_node(picking_pass_single_component, node, transform, static_cast<uint32_t>(entity));
        }
    });

    bgfx::blit(PICKING_BLIT_PASS, picking_pass_single_component.color_texture, 0, 0, picking_pass_single_component.rt_color_texture);
    picking_pass_single_component.target_frame = bgfx::readTexture(picking_pass_single_component.color_texture, picking_pass_single_component.target_data.data());
}

void PickingPassSystem::reset(PickingPassSingleComponent& picking_pass_single_component, uint16_t width, uint16_t height) const {
    using namespace picking_pass_system_details;

    if (bgfx::isValid(picking_pass_single_component.color_texture)) {
        bgfx::destroy(picking_pass_single_component.color_texture);
    }

    if (bgfx::isValid(picking_pass_single_component.buffer)) {
        bgfx::destroy(picking_pass_single_component.buffer);
    }

    picking_pass_single_component.color_texture    = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, ATTACHMENT_FLAGS);
    picking_pass_single_component.rt_color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, RT_ATTACHMENT_FLAGS);
    picking_pass_single_component.rt_depth_buffer  = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D24S8, RT_ATTACHMENT_FLAGS);

    const bgfx::TextureHandle attachments[] = {
            picking_pass_single_component.rt_color_texture,
            picking_pass_single_component.rt_depth_buffer
    };

    picking_pass_single_component.buffer = bgfx::createFrameBuffer(static_cast<uint8_t>(std::size(attachments)), attachments, true);

    bgfx::setViewFrameBuffer(PICKING_PASS, picking_pass_single_component.buffer);
    bgfx::setViewRect(PICKING_PASS, 0, 0, width, height);
}

void PickingPassSystem::draw_node(const PickingPassSingleComponent& picking_pass_single_component, const Model::Node& node, const glm::mat4& transform, uint32_t object_index) const {
    glm::mat4 local_transform = glm::translate(glm::mat4(1.f), node.translation);
    local_transform = local_transform * glm::mat4_cast(node.rotation);
    local_transform = glm::scale(local_transform, node.scale);

    const glm::mat4 world_transform = transform * local_transform;

    if (node.mesh) {
        for (const Model::Primitive& primitive : node.mesh->primitives) {
            assert(bgfx::isValid(primitive.vertex_buffer));
            assert(bgfx::isValid(primitive.index_buffer));

            bgfx::setVertexBuffer(0, primitive.vertex_buffer, 0, static_cast<uint32_t>(primitive.num_vertices));
            bgfx::setIndexBuffer(primitive.index_buffer, 0, static_cast<uint32_t>(primitive.num_indices));

            glm::vec4 uniform_value;
            uniform_value.x = ((object_index >> 16) & 0xFF) / 255.f;
            uniform_value.y = ((object_index >> 8) & 0xFF) / 255.f;
            uniform_value.z = (object_index & 0xFF) / 255.f;
            uniform_value.w = ((object_index >> 24) & 0xFF) / 255.f;
            bgfx::setUniform(picking_pass_single_component.object_index_uniform, glm::value_ptr(uniform_value));

            bgfx::setTransform(glm::value_ptr(world_transform), 1);

            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_Z | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW);

            assert(bgfx::isValid(picking_pass_single_component.program));

            bgfx::submit(PICKING_PASS, picking_pass_single_component.program);
        }
    }

    for (const Model::Node& child_node : node.children) {
        draw_node(picking_pass_single_component, child_node, world_transform, object_index);
    }
}

} // namespace hg
