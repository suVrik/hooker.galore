#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/imgui_pass/imgui_pass.fragment.h"
#include "shaders/imgui_pass/imgui_pass.vertex.h"
#include "world/shared/imgui_render_system.h"
#include "world/shared/imgui_single_component.h"
#include "world/shared/window_single_component.h"

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <imgui.h>

namespace hg {

namespace imgui_render_system_details {

static const bgfx::EmbeddedShader IMGUI_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(imgui_pass_vertex),
        BGFX_EMBEDDED_SHADER(imgui_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

} // namespace imgui_render_system_details

ImguiRenderSystem::ImguiRenderSystem(World& world) noexcept
        : NormalSystem(world) {
    auto& imgui_context_single_component = world.ctx<ImguiSingleComponent>();

    imgui_context_single_component.vertex_declaration
            .begin()
            .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true, true)
            .end();

    unsigned char* texture_data;
    int texture_width, texture_height;

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    io.Fonts->GetTexDataAsRGBA32(&texture_data, &texture_width, &texture_height);

    imgui_context_single_component.font_texture_handle = bgfx::createTexture2D(static_cast<uint16_t>(texture_width), static_cast<uint16_t>(texture_height), false, 1, bgfx::TextureFormat::BGRA8, 0, bgfx::copy(texture_data, static_cast<uint32_t>(texture_width * texture_height * 4)));
    imgui_context_single_component.font_uniform_handle = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);

    bgfx::RendererType::Enum type = bgfx::getRendererType();
    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(imgui_render_system_details::IMGUI_PASS_SHADER, type, "imgui_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(imgui_render_system_details::IMGUI_PASS_SHADER, type, "imgui_pass_fragment");
    imgui_context_single_component.program_handle = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    auto& window_single_component = world.ctx<WindowSingleComponent>();
    bgfx::setViewClear(IMGUI_PASS, BGFX_CLEAR_NONE,  0x00000000, 1.f, 0);
    bgfx::setViewRect(IMGUI_PASS, 0, 0, window_single_component.width, window_single_component.height);
}

ImguiRenderSystem::~ImguiRenderSystem() {
    auto& imgui_context_single_component = world.ctx<ImguiSingleComponent>();
    if (bgfx::isValid(imgui_context_single_component.font_texture_handle)) {
        bgfx::destroy(imgui_context_single_component.font_texture_handle);
    }
    if (bgfx::isValid(imgui_context_single_component.font_uniform_handle)) {
        bgfx::destroy(imgui_context_single_component.font_uniform_handle);
    }
    if (bgfx::isValid(imgui_context_single_component.program_handle)) {
        bgfx::destroy(imgui_context_single_component.program_handle);
    }
}

void ImguiRenderSystem::update(float /*elapsed_time*/) {
    assert(world.after("WindowSystem") && world.after("ImguiFetchSystem") && world.before("RenderSystem"));

    auto& imgui_context_single_component = world.ctx<ImguiSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        bgfx::setViewRect(IMGUI_PASS, 0, 0, window_single_component.width, window_single_component.height);
    }

    ImGui::Render();

    ImDrawData* draw_data = ImGui::GetDrawData();
    for (int i = 0; i < draw_data->CmdListsCount; i++) {
        const ImDrawList* draw_list = draw_data->CmdLists[i];
        const auto num_vertices = uint32_t(draw_list->VtxBuffer.size());
        const auto num_indices  = uint32_t(draw_list->IdxBuffer.size());

        if (!bgfx::getAvailTransientVertexBuffer(num_vertices, imgui_context_single_component.vertex_declaration) || !bgfx::getAvailTransientIndexBuffer(num_indices)) {
            break;
        }

        bgfx::TransientVertexBuffer vertex_buffer {};
        bgfx::allocTransientVertexBuffer(&vertex_buffer, num_vertices, imgui_context_single_component.vertex_declaration);
        memcpy(vertex_buffer.data, draw_list->VtxBuffer.begin(), num_vertices * sizeof(ImDrawVert));

        bgfx::TransientIndexBuffer index_buffer {};
        bgfx::allocTransientIndexBuffer(&index_buffer, num_indices);
        memcpy(index_buffer.data, draw_list->IdxBuffer.begin(), num_indices * sizeof(ImDrawIdx));

        uint32_t offset = 0;
        for (const ImDrawCmd* command = draw_list->CmdBuffer.begin(); command != draw_list->CmdBuffer.end(); command++) {
            if (command->UserCallback != nullptr) {
                command->UserCallback(draw_list, command);
            } else if (command->ElemCount != 0) {
                bgfx::TextureHandle texture_handle = imgui_context_single_component.font_texture_handle;
                if (command->TextureId != nullptr) {
                    texture_handle.idx = uint16_t(reinterpret_cast<uintptr_t>(command->TextureId));
                }

                const auto x = uint16_t(std::max(command->ClipRect.x, 0.f));
                const auto y = uint16_t(std::max(command->ClipRect.y, 0.f));
                const auto width = uint16_t(std::min(command->ClipRect.z, float(std::numeric_limits<uint16_t>::max())) - x);
                const auto height = uint16_t(std::min(command->ClipRect.w, float(std::numeric_limits<uint16_t>::max())) - y);
                bgfx::setScissor(x, y, width, height);

                bgfx::setVertexBuffer(0, &vertex_buffer, 0, num_vertices);
                bgfx::setIndexBuffer(&index_buffer, offset, command->ElemCount);

                bgfx::setTexture(0, imgui_context_single_component.font_uniform_handle, texture_handle);

                bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));

                bgfx::submit(IMGUI_PASS, imgui_context_single_component.program_handle);
            }
            offset += command->ElemCount;
        }
    }
}

} // namespace hg
