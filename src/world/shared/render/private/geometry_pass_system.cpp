#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "core/resource/texture.h"
#include "shaders/geometry_blockout_pass/geometry_blockout_pass.vertex.h"
#include "shaders/geometry_no_parallax_pass/geometry_no_parallax_pass.fragment.h"
#include "shaders/geometry_no_parallax_pass/geometry_no_parallax_pass.vertex.h"
#include "shaders/geometry_pass/geometry_pass.fragment.h"
#include "shaders/geometry_pass/geometry_pass.vertex.h"
#include "world/shared/render/blockout_component.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/geometry_pass_single_component.h"
#include "world/shared/render/geometry_pass_system.h"
#include "world/shared/window_single_component.h"

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <glm/gtc/type_ptr.hpp>

namespace hg {

namespace geometry_pass_system_details {

static const bgfx::EmbeddedShader GEOMETRY_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(geometry_pass_vertex),
        BGFX_EMBEDDED_SHADER(geometry_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

static const bgfx::EmbeddedShader GEOMETRY_BLOCKOUT_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(geometry_blockout_pass_vertex),
        BGFX_EMBEDDED_SHADER(geometry_no_parallax_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

static const bgfx::EmbeddedShader GEOMETRY_NO_PARALLAX_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(geometry_no_parallax_pass_vertex),
        BGFX_EMBEDDED_SHADER(geometry_no_parallax_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

static const uint64_t ATTACHMENT_FLAGS = BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

} // namespace render_system_details

struct GeometryPassSystem::DrawNodeContext final {
    bgfx::UniformHandle color_roughness_uniform   = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle normal_metal_ao_uniform   = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle parallax_uniform          = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle parallax_settings_uniform = BGFX_INVALID_HANDLE;

    const Texture* color_roughness = nullptr;
    const Texture* normal_metal_ao = nullptr;
    const Texture* parallax        = nullptr;
    glm::vec4 parallax_settings    = glm::vec4(0.f);

    bgfx::ProgramHandle program    = BGFX_INVALID_HANDLE;
};

GeometryPassSystem::GeometryPassSystem(World& world) noexcept
        : NormalSystem(world)
        , m_group(world.group<ModelComponent, MaterialComponent, TransformComponent>()) {
    using namespace geometry_pass_system_details;

    auto& geometry_pass_single_component = world.set<GeometryPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    reset(geometry_pass_single_component, window_single_component.width, window_single_component.height);
    
    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(GEOMETRY_PASS_SHADER, type, "geometry_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(GEOMETRY_PASS_SHADER, type, "geometry_pass_fragment");
    geometry_pass_single_component.geometry_pass_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    vertex_shader_handle   = bgfx::createEmbeddedShader(GEOMETRY_NO_PARALLAX_PASS_SHADER, type, "geometry_no_parallax_pass_vertex");
    fragment_shader_handle = bgfx::createEmbeddedShader(GEOMETRY_NO_PARALLAX_PASS_SHADER, type, "geometry_no_parallax_pass_fragment");
    geometry_pass_single_component.geometry_no_parallax_pass_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    vertex_shader_handle   = bgfx::createEmbeddedShader(GEOMETRY_BLOCKOUT_PASS_SHADER, type, "geometry_blockout_pass_vertex");
    fragment_shader_handle = bgfx::createEmbeddedShader(GEOMETRY_BLOCKOUT_PASS_SHADER, type, "geometry_no_parallax_pass_fragment");
    geometry_pass_single_component.geometry_blockout_pass_program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    geometry_pass_single_component.color_roughness_uniform   = bgfx::createUniform("s_color_roughness",   bgfx::UniformType::Sampler);
    geometry_pass_single_component.normal_metal_ao_uniform   = bgfx::createUniform("s_normal_metal_ao",   bgfx::UniformType::Sampler);
    geometry_pass_single_component.parallax_uniform          = bgfx::createUniform("s_parallax",          bgfx::UniformType::Sampler);
    geometry_pass_single_component.parallax_settings_uniform = bgfx::createUniform("u_parallax_settings", bgfx::UniformType::Vec4);

    bgfx::setViewClear(GEOMETRY_PASS, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0xFFFFFFFF, 1.f, 0);
    bgfx::setViewName(GEOMETRY_PASS, "geometry_pass");
}

GeometryPassSystem::~GeometryPassSystem() {
    auto& geometry_pass_single_component = world.ctx<GeometryPassSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(geometry_pass_single_component.color_roughness_uniform);
    destroy_valid(geometry_pass_single_component.gbuffer);
    destroy_valid(geometry_pass_single_component.geometry_blockout_pass_program);
    destroy_valid(geometry_pass_single_component.geometry_no_parallax_pass_program);
    destroy_valid(geometry_pass_single_component.geometry_pass_program);
    destroy_valid(geometry_pass_single_component.normal_metal_ao_uniform);
    destroy_valid(geometry_pass_single_component.parallax_settings_uniform);
    destroy_valid(geometry_pass_single_component.parallax_uniform);
}

void GeometryPassSystem::update(float /*elapsed_time*/) {
    auto& camera_single_component = world.ctx<CameraSingleComponent>();
    auto& geometry_pass_single_component = world.ctx<GeometryPassSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        reset(geometry_pass_single_component, window_single_component.width, window_single_component.height);
    }

    bgfx::setViewTransform(GEOMETRY_PASS, glm::value_ptr(camera_single_component.view_matrix), glm::value_ptr(camera_single_component.projection_matrix));

    DrawNodeContext context;
    context.color_roughness_uniform   = geometry_pass_single_component.color_roughness_uniform;
    context.normal_metal_ao_uniform   = geometry_pass_single_component.normal_metal_ao_uniform;
    context.parallax_uniform          = geometry_pass_single_component.parallax_uniform;
    context.parallax_settings_uniform = geometry_pass_single_component.parallax_settings_uniform;

    bgfx::touch(GEOMETRY_PASS);
    
    m_group.each([&](entt::entity entity, ModelComponent& model_component, MaterialComponent& material_component, TransformComponent& transform_component) {
        if (material_component.color_roughness != nullptr && material_component.normal_metal_ao != nullptr && !model_component.model.children.empty()) {
            context.color_roughness   = material_component.color_roughness;
            context.normal_metal_ao   = material_component.normal_metal_ao;
            context.parallax          = material_component.parallax;
            context.parallax_settings = glm::vec4(material_component.parallax_scale, material_component.parallax_steps, 1.f / material_component.parallax_steps, 0.f);

            glm::mat4 transform = glm::translate(glm::mat4(1.f), transform_component.translation);
            transform = transform * glm::mat4_cast(transform_component.rotation);
            transform = glm::scale(transform, transform_component.scale);

            if (!world.has<BlockoutComponent>(entity)) {
                if (context.parallax != nullptr) {
                    context.program = geometry_pass_single_component.geometry_pass_program;
                    for (const Model::Node& node : model_component.model.children) {
                        draw_node(context, node, transform);
                    }
                } else {
                    context.program = geometry_pass_single_component.geometry_no_parallax_pass_program;
                    for (const Model::Node& node : model_component.model.children) {
                        draw_node(context, node, transform);
                    }
                }
            } else {
                context.program = geometry_pass_single_component.geometry_blockout_pass_program;
                for (const Model::Node& node : model_component.model.children) {
                    draw_node(context, node, transform);
                }
            }
        }
    });
}

void GeometryPassSystem::reset(GeometryPassSingleComponent& geometry_pass_single_component, uint16_t width, uint16_t height) const noexcept {
    using namespace geometry_pass_system_details;

    if (bgfx::isValid(geometry_pass_single_component.gbuffer)) {
        bgfx::destroy(geometry_pass_single_component.gbuffer);
    }

    geometry_pass_single_component.color_roughness_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA16F, ATTACHMENT_FLAGS);
    bgfx::setName(geometry_pass_single_component.color_roughness_texture, "geometry_pass_output_bcr");

    geometry_pass_single_component.normal_metal_ao_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA16F, ATTACHMENT_FLAGS);
    bgfx::setName(geometry_pass_single_component.normal_metal_ao_texture, "geometry_pass_output_nmao");

    geometry_pass_single_component.depth_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::R32F, ATTACHMENT_FLAGS);
    bgfx::setName(geometry_pass_single_component.depth_texture, "geometry_pass_output_depth");

    geometry_pass_single_component.depth_stencil_texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D24S8, ATTACHMENT_FLAGS);
    bgfx::setName(geometry_pass_single_component.depth_stencil_texture, "geometry_pass_output_depth_stencil");

    const bgfx::TextureHandle attachments[] = {
            geometry_pass_single_component.color_roughness_texture,
            geometry_pass_single_component.normal_metal_ao_texture,
            geometry_pass_single_component.depth_texture,
            geometry_pass_single_component.depth_stencil_texture
    };

    geometry_pass_single_component.gbuffer = bgfx::createFrameBuffer(std::size(attachments), attachments, true);

    bgfx::setViewFrameBuffer(GEOMETRY_PASS, geometry_pass_single_component.gbuffer);
    bgfx::setViewRect(GEOMETRY_PASS, 0, 0, width, height);
}

void GeometryPassSystem::draw_node(const DrawNodeContext& context, const Model::Node& node, const glm::mat4& transform) const noexcept {
    glm::mat4 local_transform = glm::translate(glm::mat4(1.f), node.translation);
    local_transform = local_transform * glm::mat4_cast(node.rotation);
    local_transform = glm::scale(local_transform, node.scale);

    const glm::mat4 world_transform = transform * local_transform;

    if (node.mesh) {
        for (const Model::Primitive& primitive : node.mesh->primitives) {
            assert(bgfx::isValid(primitive.vertex_buffer));
            assert(bgfx::isValid(primitive.index_buffer));
            
            bgfx::setVertexBuffer(0, primitive.vertex_buffer, 0, primitive.num_vertices);
            bgfx::setIndexBuffer(primitive.index_buffer, 0, primitive.num_indices);

            assert(bgfx::isValid(context.color_roughness_uniform));
            assert(bgfx::isValid(context.normal_metal_ao_uniform));
            
            bgfx::setTexture(0, context.color_roughness_uniform, context.color_roughness->handle);
            bgfx::setTexture(1, context.normal_metal_ao_uniform, context.normal_metal_ao->handle);

            if (context.parallax != nullptr) {
                assert(bgfx::isValid(context.parallax_uniform));
                assert(bgfx::isValid(context.parallax_settings_uniform));

                bgfx::setTexture(2, context.parallax_uniform, context.parallax->handle);
                bgfx::setUniform(context.parallax_settings_uniform, glm::value_ptr(context.parallax_settings), 1);
            }

            bgfx::setTransform(glm::value_ptr(world_transform), 1);

            bgfx::setStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(1) | BGFX_STENCIL_FUNC_RMASK(0xFF) |
                             BGFX_STENCIL_OP_FAIL_S_REPLACE | BGFX_STENCIL_OP_FAIL_Z_REPLACE | BGFX_STENCIL_OP_PASS_Z_REPLACE,
                             BGFX_STENCIL_NONE);
            bgfx::setState(BGFX_STATE_WRITE_MASK | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW);

            assert(bgfx::isValid(context.program));
            
            bgfx::submit(GEOMETRY_PASS, context.program);
        }
    }

    for (const Model::Node& child_node : node.children) {
        draw_node(context, child_node, world_transform);
    }
}

} // namespace hg
