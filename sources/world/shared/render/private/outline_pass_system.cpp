#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/outline_blur_pass/outline_blur_pass.fragment.h"
#include "shaders/outline_pass/outline_pass.fragment.h"
#include "shaders/outline_pass/outline_pass.vertex.h"
#include "shaders/quad_pass/quad_pass.vertex.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/outline_pass_single_component.h"
#include "world/shared/render/outline_pass_system.h"
#include "world/shared/render/quad_single_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/embedded_shader.h>
#include <glm/gtc/type_ptr.hpp>

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

static const uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

} // namespace outline_pass_system

OutlinePassSystem::OutlinePassSystem(World& world) noexcept
        : NormalSystem(world)
        , m_group(world.group<OutlineComponent>(entt::get<ModelComponent, TransformComponent>)) {
    using namespace outline_pass_system_details;

    auto& outline_pass_single_component = world.set<OutlinePassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    reset(outline_pass_single_component, window_single_component.width, window_single_component.height);

    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(OUTLINE_PASS_SHADER, type, "outline_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(OUTLINE_PASS_SHADER, type, "outline_pass_fragment");
    outline_pass_single_component.outline_pass_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    vertex_shader_handle   = bgfx::createEmbeddedShader(OUTLINE_BLUR_PASS_SHADER, type, "quad_pass_vertex");
    fragment_shader_handle = bgfx::createEmbeddedShader(OUTLINE_BLUR_PASS_SHADER, type, "outline_blur_pass_fragment");
    outline_pass_single_component.outline_blur_pass_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    outline_pass_single_component.texture_uniform       = bgfx::createUniform("s_texture",       bgfx::UniformType::Sampler);
    outline_pass_single_component.outline_color_uniform = bgfx::createUniform("u_outline_color", bgfx::UniformType::Vec4);
    outline_pass_single_component.group_index_uniform   = bgfx::createUniform("u_group_index",   bgfx::UniformType::Vec4);

    bgfx::setViewClear(OUTLINE_PASS, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.f, 0);
    bgfx::setViewName(OUTLINE_PASS, "outline_pass");

    bgfx::setViewClear(OUTLINE_BLUR_PASS, BGFX_CLEAR_NONE, 0x000000FF, 1.f, 0);
    bgfx::setViewName(OUTLINE_BLUR_PASS, "outline_blur_pass");
}

OutlinePassSystem::~OutlinePassSystem() {
    auto& outline_pass_single_component = world.ctx<OutlinePassSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(outline_pass_single_component.buffer);
    destroy_valid(outline_pass_single_component.group_index_uniform);
    destroy_valid(outline_pass_single_component.outline_blur_pass_program);
    destroy_valid(outline_pass_single_component.outline_color_uniform);
    destroy_valid(outline_pass_single_component.outline_pass_program);
    destroy_valid(outline_pass_single_component.texture_uniform);
}

void OutlinePassSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& outline_pass_single_component = world.ctx<OutlinePassSingleComponent>();
    auto& quad_single_component = world.ctx<QuadSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(outline_pass_single_component, window_single_component.width, window_single_component.height);
    }

    bgfx::setViewTransform(OUTLINE_PASS, glm::value_ptr(camera_single_component.view_matrix), glm::value_ptr(camera_single_component.projection_matrix));

    bgfx::touch(OUTLINE_PASS);

    m_group.each([&](entt::entity entity, OutlineComponent& outline_component, ModelComponent& model_component, TransformComponent& transform_component) {
        glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component.translation);
        transform = transform * glm::mat4_cast(transform_component.rotation);
        transform = glm::scale(transform, transform_component.scale);

        for (const Model::Node& node : model_component.model.children) {
            draw_node(outline_pass_single_component, node, transform, outline_component.group_index);
        }
    });

    bgfx::setVertexBuffer(0, quad_single_component.vertex_buffer, 0, QuadSingleComponent::NUM_VERTICES);
    bgfx::setIndexBuffer(quad_single_component.index_buffer, 0, QuadSingleComponent::NUM_INDICES);

    bgfx::setTexture(0, outline_pass_single_component.texture_uniform, outline_pass_single_component.color_texture);
    bgfx::setUniform(outline_pass_single_component.outline_color_uniform, glm::value_ptr(outline_pass_single_component.outline_color));

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));

    bgfx::submit(OUTLINE_BLUR_PASS, outline_pass_single_component.outline_blur_pass_program);
}

void OutlinePassSystem::reset(OutlinePassSingleComponent& outline_pass_single_component, uint16_t width, uint16_t height) const noexcept {
    using namespace outline_pass_system_details;

    if (bgfx::isValid(outline_pass_single_component.buffer)) {
        bgfx::destroy(outline_pass_single_component.buffer);
    }

    outline_pass_single_component.color_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, ATTACHMENT_FLAGS);
    outline_pass_single_component.depth_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D24S8, ATTACHMENT_FLAGS);

    const bgfx::TextureHandle attachments[] = {
            outline_pass_single_component.color_texture,
            outline_pass_single_component.depth_texture
    };
    outline_pass_single_component.buffer = bgfx::createFrameBuffer(static_cast<uint8_t>(std::size(attachments)), attachments, true);

    bgfx::setViewFrameBuffer(OUTLINE_PASS, outline_pass_single_component.buffer);
    bgfx::setViewRect(OUTLINE_PASS, 0, 0, width, height);

    bgfx::setViewRect(OUTLINE_BLUR_PASS, 0, 0, width, height);
}

void OutlinePassSystem::draw_node(const OutlinePassSingleComponent& outline_pass_single_component, const Model::Node& node, const glm::mat4& transform, uint32_t group_index) const noexcept {
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
            uniform_value.x = ((group_index >> 16) & 0xFF) / 255.f;
            uniform_value.y = ((group_index >> 8) & 0xFF) / 255.f;
            uniform_value.z = (group_index & 0xFF) / 255.f;
            uniform_value.w = 1.f;
            bgfx::setUniform(outline_pass_single_component.group_index_uniform, glm::value_ptr(uniform_value));

            bgfx::setTransform(glm::value_ptr(world_transform), 1);

            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW);

            assert(bgfx::isValid(outline_pass_single_component.outline_pass_program));

            bgfx::submit(OUTLINE_PASS, outline_pass_single_component.outline_pass_program);
        }
    }

    for (const Model::Node& child_node : node.children) {
        draw_node(outline_pass_single_component, child_node, world_transform, group_index);
    }
}

} // namespace hg
