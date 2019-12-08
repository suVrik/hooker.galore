#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/imgui/imgui_fetch_system.h"
#include "world/imgui/imgui_single_component.h"
#include "world/imgui/imgui_tags.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/window_single_component.h"

#include <ImGuizmo.h>
#include <SDL2/SDL_clipboard.h>
#include <SDL2/SDL_mouse.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace hg {

namespace imgui_fetch_system_details {

static char* clipboard_text = nullptr;

static const char* get_clipboard_text(void* /*user_data*/) {
    if (clipboard_text != nullptr) {
        SDL_free(clipboard_text);
    }
    clipboard_text = SDL_GetClipboardText();
    return clipboard_text;
}

static void set_clipboard_text(void* /*user_data*/, const char* text) {
    SDL_SetClipboardText(text);
}

} // namespace imgui_fetch_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(ImguiFetchSystem),
    TAGS(imgui),
    BEFORE("ImguiPassSystem"),
    AFTER("WindowSystem")
)

ImguiFetchSystem::ImguiFetchSystem(World& world)
        : NormalSystem(world) {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags = ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos;
    io.ConfigFlags = ImGuiConfigFlags_DockingEnable;

    io.KeyMap[ImGuiKey_Tab]        = static_cast<int>(Control::KEY_TAB);
    io.KeyMap[ImGuiKey_LeftArrow]  = static_cast<int>(Control::KEY_LEFT);
    io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(Control::KEY_RIGHT);
    io.KeyMap[ImGuiKey_UpArrow]    = static_cast<int>(Control::KEY_UP);
    io.KeyMap[ImGuiKey_DownArrow]  = static_cast<int>(Control::KEY_DOWN);
    io.KeyMap[ImGuiKey_PageUp]     = static_cast<int>(Control::KEY_PAGEUP);
    io.KeyMap[ImGuiKey_PageDown]   = static_cast<int>(Control::KEY_PAGEDOWN);
    io.KeyMap[ImGuiKey_Home]       = static_cast<int>(Control::KEY_HOME);
    io.KeyMap[ImGuiKey_End]        = static_cast<int>(Control::KEY_END);
    io.KeyMap[ImGuiKey_Insert]     = static_cast<int>(Control::KEY_INSERT);
    io.KeyMap[ImGuiKey_Delete]     = static_cast<int>(Control::KEY_DELETE);
    io.KeyMap[ImGuiKey_Backspace]  = static_cast<int>(Control::KEY_BACKSPACE);
    io.KeyMap[ImGuiKey_Space]      = static_cast<int>(Control::KEY_SPACE);
    io.KeyMap[ImGuiKey_Enter]      = static_cast<int>(Control::KEY_RETURN);
    io.KeyMap[ImGuiKey_Escape]     = static_cast<int>(Control::KEY_ESCAPE);
    io.KeyMap[ImGuiKey_A]          = static_cast<int>(Control::KEY_A);
    io.KeyMap[ImGuiKey_C]          = static_cast<int>(Control::KEY_C);
    io.KeyMap[ImGuiKey_V]          = static_cast<int>(Control::KEY_V);
    io.KeyMap[ImGuiKey_X]          = static_cast<int>(Control::KEY_X);
    io.KeyMap[ImGuiKey_Y]          = static_cast<int>(Control::KEY_Y);
    io.KeyMap[ImGuiKey_Z]          = static_cast<int>(Control::KEY_Z);

    io.SetClipboardTextFn = imgui_fetch_system_details::set_clipboard_text;
    io.GetClipboardTextFn = imgui_fetch_system_details::get_clipboard_text;

    std::fill(std::begin(io.MouseDown), std::end(io.MouseDown), 0);

    auto& imgui_single_component = world.set<ImguiSingleComponent>();
    imgui_single_component.mouse_cursors[ImGuiMouseCursor_Arrow]      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    imgui_single_component.mouse_cursors[ImGuiMouseCursor_TextInput]  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    imgui_single_component.mouse_cursors[ImGuiMouseCursor_ResizeAll]  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
    imgui_single_component.mouse_cursors[ImGuiMouseCursor_ResizeNS]   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    imgui_single_component.mouse_cursors[ImGuiMouseCursor_ResizeEW]   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    imgui_single_component.mouse_cursors[ImGuiMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
    imgui_single_component.mouse_cursors[ImGuiMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
    imgui_single_component.mouse_cursors[ImGuiMouseCursor_Hand]       = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
}

ImguiFetchSystem::~ImguiFetchSystem() {
    auto& imgui_single_component = world.ctx<ImguiSingleComponent>();
    for (SDL_Cursor* mouse_cursor : imgui_single_component.mouse_cursors) {
        SDL_FreeCursor(mouse_cursor);
    }
    ImGui::DestroyContext();
}

void ImguiFetchSystem::update(float elapsed_time) {
    update_imgui(elapsed_time);
    build_dock_space();
}

void ImguiFetchSystem::update_imgui(float elapsed_time) const {
    ImGuiIO& io = ImGui::GetIO();

    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();
    normal_input_single_component.m_previous_disable_keyboard = normal_input_single_component.m_disable_keyboard;
    normal_input_single_component.m_previous_disable_mouse = normal_input_single_component.m_disable_mouse;
    normal_input_single_component.m_disable_keyboard = false;
    normal_input_single_component.m_disable_mouse = false;

    io.MousePos.x   = static_cast<float>(normal_input_single_component.get_mouse_x());
    io.MousePos.y   = static_cast<float>(normal_input_single_component.get_mouse_y());
    io.MouseWheel   = static_cast<float>(normal_input_single_component.get_mouse_wheel());
    io.MouseDown[0] = normal_input_single_component.is_down(Control::BUTTON_LEFT);
    io.MouseDown[1] = normal_input_single_component.is_down(Control::BUTTON_RIGHT);
    io.MouseDown[2] = normal_input_single_component.is_down(Control::BUTTON_MIDDLE);
    io.MouseDown[3] = normal_input_single_component.is_down(Control::BUTTON_EXTRA_1);
    io.MouseDown[4] = normal_input_single_component.is_down(Control::BUTTON_EXTRA_2);

    for (size_t i = 0; i < std::min(CONTROL_KEY_COUNT, std::size(io.KeysDown)); i++) {
        io.KeysDown[i] = normal_input_single_component.is_down(static_cast<Control>(i));
    }

    io.KeyShift = normal_input_single_component.is_down(Control::KEY_LSHIFT) || normal_input_single_component.is_down(Control::KEY_RSHIFT);
    io.KeyCtrl  = normal_input_single_component.is_down(Control::KEY_LCTRL) || normal_input_single_component.is_down(Control::KEY_RCTRL);
    io.KeyAlt   = normal_input_single_component.is_down(Control::KEY_LALT) || normal_input_single_component.is_down(Control::KEY_RALT);
    io.KeySuper = normal_input_single_component.is_down(Control::KEY_LGUI) || normal_input_single_component.is_down(Control::KEY_RGUI);

    io.AddInputCharactersUTF8(normal_input_single_component.get_text());

    normal_input_single_component.m_disable_keyboard = io.WantCaptureKeyboard;
    normal_input_single_component.m_disable_mouse = io.WantCaptureMouse;

    if (io.WantCaptureMouse) {
        normal_input_single_component.m_mouse_wheel = 0;
    }

    io.DeltaTime = std::max(elapsed_time, 1e-4f); // ImGui doesn't accept zero elapsed_time value.

    auto& window_single_component = world.ctx<WindowSingleComponent>();
    io.DisplaySize.x = static_cast<float>(window_single_component.width);
    io.DisplaySize.y = static_cast<float>(window_single_component.height);
    io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

    if (io.WantSetMousePos) {
        SDL_WarpMouseInWindow(window_single_component.window, static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y));
    }

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None) {
        SDL_ShowCursor(SDL_FALSE);
    } else {
        auto& imgui_single_component = world.ctx<ImguiSingleComponent>();
        if (imgui_single_component.mouse_cursors[imgui_cursor] != nullptr) {
            SDL_SetCursor(imgui_single_component.mouse_cursors[imgui_cursor]);
        } else {
            SDL_SetCursor(imgui_single_component.mouse_cursors[ImGuiMouseCursor_Arrow]);
        }
        SDL_ShowCursor(SDL_TRUE);
    }

    SDL_CaptureMouse(io.WantCaptureMouse ? SDL_TRUE : SDL_FALSE);

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

// TODO: Make this a separate Editor system.
void ImguiFetchSystem::build_dock_space() const {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking |
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar |
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
    ImGui::Begin("Main", nullptr, flags);
    ImGui::PopStyleVar(3);

    ImGuiID dock_space_id = ImGui::GetID("Main");
    if (ImGui::DockBuilderGetNode(dock_space_id) == nullptr) {
        ImGui::DockBuilderRemoveNode(dock_space_id);
        ImGui::DockBuilderAddNode(dock_space_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dock_space_id, viewport->Size);

        ImGuiID dock_main_id = dock_space_id;

        ImGuiID dock_id_right_top = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.3f, nullptr, &dock_main_id);
        ImGuiID dock_id_right_middle = ImGui::DockBuilderSplitNode(dock_id_right_top, ImGuiDir_Down, 0.8f, nullptr, &dock_id_right_top);
        ImGuiID dock_id_right_bottom = ImGui::DockBuilderSplitNode(dock_id_right_middle, ImGuiDir_Down, 0.5f, nullptr, &dock_id_right_middle);

        ImGui::DockBuilderDockWindow("Gizmo", dock_id_right_top);
        ImGui::DockBuilderDockWindow("Presets", dock_id_right_middle);
        ImGui::DockBuilderDockWindow("Level", dock_id_right_middle);
        ImGui::DockBuilderDockWindow("Property Editor", dock_id_right_bottom);
        ImGui::DockBuilderDockWindow("History", dock_id_right_bottom);

        ImGui::DockBuilderFinish(dock_space_id);
    }

    ImGui::DockSpace(dock_space_id, ImVec2(0.f, 0.f), ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
}

} // namespace hg
