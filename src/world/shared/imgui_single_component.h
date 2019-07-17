#pragma once

#include <bgfx/bgfx.h>
#include <imgui.h>

struct SDL_Cursor;

namespace hg {

/** `ImguiSingleComponent` holds ImGui internal data. */
struct ImguiSingleComponent final {
    SDL_Cursor* mouse_cursors[ImGuiMouseCursor_COUNT] = { nullptr };

    bgfx::VertexDecl vertex_declaration;

    bgfx::ProgramHandle program_handle      = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle font_texture_handle = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle font_uniform_handle = BGFX_INVALID_HANDLE;
};

} // namespace hg
