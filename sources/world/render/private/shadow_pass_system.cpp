#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "core/resource/texture.h"
#include "shaders/shadow_pass/shadow_pass.fragment.h"
#include "shaders/shadow_pass/shadow_pass.vertex.h"
#include "world/render/blockout_component.h"
#include "world/render/camera_single_component.h"
#include "world/render/shadow_pass_single_component.h"
#include "world/render/shadow_pass_system.h"
#include "world/render/render_tags.h"
#include "world/shared/window_single_component.h"

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <glm/gtc/type_ptr.hpp>

namespace hg {

namespace shadow_pass_system_details {

static const bgfx::EmbeddedShader SHADOW_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(shadow_pass_vertex),
        BGFX_EMBEDDED_SHADER(shadow_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

static const uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

} // namespace render_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(ShadowPassSystem),
    TAGS(render),
    BEFORE("RenderSystem", "LightingPassSystem", "GeometryPassSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "CameraSystem")
)

ShadowPassSystem::ShadowPassSystem(World& world)
        : NormalSystem(world)
        , m_group(world.group<ModelComponent, MaterialComponent, TransformComponent>()) {
    using namespace shadow_pass_system_details;

    auto& shadow_pass_single_component = world.set<ShadowPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    reset(shadow_pass_single_component, 2048, 2048);
    
    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(SHADOW_PASS_SHADER, type, "shadow_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(SHADOW_PASS_SHADER, type, "shadow_pass_fragment");
    shadow_pass_single_component.shadow_pass_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    bgfx::setViewClear(SHADOW_PASS, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xFFFFFFFF, 1.f, 0);
    bgfx::setViewName(SHADOW_PASS, "shadow_pass");
}

ShadowPassSystem::~ShadowPassSystem() {
    auto& shadow_pass_single_component = world.ctx<ShadowPassSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(shadow_pass_single_component.gbuffer);
    destroy_valid(shadow_pass_single_component.shadow_pass_program);
}

void ShadowPassSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& shadow_pass_single_component = world.ctx<ShadowPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(shadow_pass_single_component, 2048, 2048);
    }

    const float area = 30.f, near_plane = 0.1f, far_plane = 80.f;
    glm::vec3 light_model = glm::vec3(0.0f, 10.0f, 10.0f);
    glm::mat4 light_view = glm::lookAtLH(
            light_model,
            glm::vec3( 0.f, 0.f,  0.f),
            glm::vec3( 0.f, 1.f,  0.f));
    glm::mat4 light_projection = glm::orthoLH(-area, area, -area, area, near_plane, far_plane);

    bgfx::setViewTransform(SHADOW_PASS, &light_view, &light_projection);

    bgfx::touch(SHADOW_PASS);
    
    m_group.each([&](entt::entity entity, ModelComponent& model_component, MaterialComponent& material_component, TransformComponent& transform_component) {
        if (material_component.color_roughness != nullptr && material_component.normal_metal_ao != nullptr && !model_component.model.children.empty()) {
            glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component.translation);
            transform = transform * glm::mat4_cast(transform_component.rotation);
            transform = glm::scale(transform, transform_component.scale);

            for (const Model::Node& node : model_component.model.children) {
                draw_node(shadow_pass_single_component.shadow_pass_program, node, transform);
            }
        }
    });
}

void ShadowPassSystem::reset(ShadowPassSingleComponent& shadow_pass_single_component, uint16_t width, uint16_t height) const {
    using namespace shadow_pass_system_details;

    if (bgfx::isValid(shadow_pass_single_component.gbuffer)) {
        bgfx::destroy(shadow_pass_single_component.gbuffer);
    }

    shadow_pass_single_component.shadow_map_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D32F, BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_BORDER | BGFX_SAMPLER_V_BORDER | BGFX_SAMPLER_BORDER_COLOR(0xFFFFFFFF));
    bgfx::setName(shadow_pass_single_component.shadow_map_texture, "shadow_pass_output");

    const bgfx::TextureHandle attachments[] = {
            shadow_pass_single_component.shadow_map_texture
    };

    shadow_pass_single_component.gbuffer = bgfx::createFrameBuffer(static_cast<uint8_t>(std::size(attachments)), attachments, true);

    bgfx::setViewFrameBuffer(SHADOW_PASS, shadow_pass_single_component.gbuffer);
    bgfx::setViewRect(SHADOW_PASS, 0, 0, width, height);
}

void ShadowPassSystem::draw_node(bgfx::ProgramHandle program, const Model::Node& node, const glm::mat4& transform) const {
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

            bgfx::setTransform(glm::value_ptr(world_transform), 1);

            bgfx::setState(BGFX_STATE_WRITE_MASK | BGFX_STATE_DEPTH_TEST_LESS);

            assert(bgfx::isValid(program));
            
            bgfx::submit(SHADOW_PASS, program);
        }
    }

    for (const Model::Node& child_node : node.children) {
        draw_node(program, child_node, world_transform);
    }
}

} // namespace hg
