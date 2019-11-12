#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "shaders/quad_pass/quad_pass.fragment.h"
#include "shaders/quad_pass/quad_pass.vertex.h"
#include "world/render/quad_single_component.h"
#include "world/render/quad_system.h"
#include "world/render/render_tags.h"

#include <bgfx/embedded_shader.h>

namespace hg {

namespace quad_system_details {

static const uint16_t QUAD_INDICES[] = {
        0, 1, 2, 0, 2, 3
};

static const float QUAD_VERTICES[] = {
        -1.f, -1.f, 0.f, 1.f,
         1.f, -1.f, 1.f, 1.f,
         1.f,  1.f, 1.f, 0.f,
        -1.f,  1.f, 0.f, 0.f
};

static const bgfx::VertexDecl QUAD_VERTEX_DECLARATION = [] {
    bgfx::VertexDecl result;
    result.begin()
          .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
          .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
          .end();
    return result;
}();

static const bgfx::EmbeddedShader QUAD_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(quad_pass_vertex),
        BGFX_EMBEDDED_SHADER(quad_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

} // namespace quad_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(QuadSystem),
    TAGS(render),
    BEFORE("RenderSystem"),
    AFTER("RenderFetchSystem")
)

QuadSystem::QuadSystem(World& world)
        : NormalSystem(world) {
    using namespace quad_system_details;

    auto& quad_single_component = world.set<QuadSingleComponent>();
    quad_single_component.index_buffer = bgfx::createIndexBuffer(bgfx::makeRef(QUAD_INDICES, sizeof(QUAD_INDICES)));
    quad_single_component.vertex_buffer = bgfx::createVertexBuffer(bgfx::makeRef(QUAD_VERTICES, sizeof(QUAD_VERTICES)), QUAD_VERTEX_DECLARATION);

    bgfx::RendererType::Enum type = bgfx::getRendererType();
    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(QUAD_PASS_SHADER, type, "quad_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(QUAD_PASS_SHADER, type, "quad_pass_fragment");
    quad_single_component.program = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);
}

QuadSystem::~QuadSystem() {
    auto& quad_single_component = world.ctx<QuadSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(quad_single_component.index_buffer);
    destroy_valid(quad_single_component.program);
    destroy_valid(quad_single_component.vertex_buffer);
}

void QuadSystem::update(float /*elapsed_time*/) {
    // Nothing to do here.
}

} // namespace hg
