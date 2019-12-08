#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "shaders/imgui_pass/imgui_pass.fragment.h"
#include "shaders/imgui_pass/imgui_pass.vertex.h"
#include "world/imgui/imgui_pass_system.h"
#include "world/imgui/imgui_single_component.h"
#include "world/imgui/imgui_tags.h"
#include "world/shared/window_single_component.h"

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <imgui.h>

namespace hg {

namespace imgui_pass_system_details {

static const bgfx::EmbeddedShader IMGUI_PASS_SHADER[] = {
        BGFX_EMBEDDED_SHADER(imgui_pass_vertex),
        BGFX_EMBEDDED_SHADER(imgui_pass_fragment),
        BGFX_EMBEDDED_SHADER_END()
};

} // namespace imgui_pass_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(ImguiPassSystem),
    TAGS(imgui),
    BEFORE("RenderSystem"),
    AFTER("WindowSystem", "RenderFetchSystem", "ImguiFetchSystem")
)

ImguiPassSystem::ImguiPassSystem(World& world)
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
    bgfx::ShaderHandle vertex_shader_handle   = bgfx::createEmbeddedShader(imgui_pass_system_details::IMGUI_PASS_SHADER, type, "imgui_pass_vertex");
    bgfx::ShaderHandle fragment_shader_handle = bgfx::createEmbeddedShader(imgui_pass_system_details::IMGUI_PASS_SHADER, type, "imgui_pass_fragment");
    imgui_context_single_component.program_handle = bgfx::createProgram(vertex_shader_handle, fragment_shader_handle, true);

    auto& window_single_component = world.ctx<WindowSingleComponent>();
    bgfx::setViewClear(IMGUI_PASS, BGFX_CLEAR_NONE,  0x00000000, 1.f, 0);
    bgfx::setViewRect(IMGUI_PASS, 0, 0, window_single_component.width, window_single_component.height);
    bgfx::setViewName(IMGUI_PASS, "imgui_pass");
}

ImguiPassSystem::~ImguiPassSystem() {
    auto& imgui_context_single_component = world.ctx<ImguiSingleComponent>();

    auto destroy_valid = [](auto handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    };

    destroy_valid(imgui_context_single_component.font_texture_handle);
    destroy_valid(imgui_context_single_component.font_uniform_handle);
    destroy_valid(imgui_context_single_component.program_handle);
}

void ImguiPassSystem::update(float /*elapsed_time*/) {
    auto& imgui_context_single_component = world.ctx<ImguiSingleComponent>();
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    if (window_single_component.resized) {
        bgfx::setViewRect(IMGUI_PASS, 0, 0, window_single_component.width, window_single_component.height);
    }

    ImGui::Render();

    bgfx::touch(IMGUI_PASS);

    ImDrawData* draw_data = ImGui::GetDrawData();
    for (int i = 0; i < draw_data->CmdListsCount; i++) {
        const ImDrawList* draw_list = draw_data->CmdLists[i];
        auto num_vertices = static_cast<uint32_t>(draw_list->VtxBuffer.size());
        auto num_indices  = static_cast<uint32_t>(draw_list->IdxBuffer.size());

        if (!bgfx::getAvailTransientVertexBuffer(num_vertices, imgui_context_single_component.vertex_declaration) || !bgfx::getAvailTransientIndexBuffer(num_indices)) {
            break;
        }

        bgfx::TransientVertexBuffer vertex_buffer {};
        bgfx::allocTransientVertexBuffer(&vertex_buffer, num_vertices, imgui_context_single_component.vertex_declaration);
        std::copy(draw_list->VtxBuffer.begin(), draw_list->VtxBuffer.end(), reinterpret_cast<ImDrawVert*>(vertex_buffer.data));

        bgfx::TransientIndexBuffer index_buffer {};
        bgfx::allocTransientIndexBuffer(&index_buffer, num_indices);
        std::copy(draw_list->IdxBuffer.begin(), draw_list->IdxBuffer.end(), reinterpret_cast<ImDrawIdx*>(index_buffer.data));

        uint32_t offset = 0;
        for (const ImDrawCmd* command = draw_list->CmdBuffer.begin(); command != draw_list->CmdBuffer.end(); command++) {
            if (command->UserCallback != nullptr) {
                command->UserCallback(draw_list, command);
            } else if (command->ElemCount != 0) {
                bgfx::TextureHandle texture_handle = imgui_context_single_component.font_texture_handle;
                if (command->TextureId != nullptr) {
                    texture_handle.idx = uint16_t(reinterpret_cast<uintptr_t>(command->TextureId));
                }

                auto x = static_cast<uint16_t>(std::max(command->ClipRect.x, 0.f));
                auto y = static_cast<uint16_t>(std::max(command->ClipRect.y, 0.f));
                auto width = static_cast<uint16_t>(std::min(command->ClipRect.z, static_cast<float>(std::numeric_limits<uint16_t>::max())) - x);
                auto height = static_cast<uint16_t>(std::min(command->ClipRect.w, static_cast<float>(std::numeric_limits<uint16_t>::max())) - y);
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
