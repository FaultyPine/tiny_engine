#include "tiny_imgui.h"

#include "render/tiny_ogl.h" // imgui custom opengl loader definitions
#include "tiny_engine.h" // for glfw window
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
// ====== imgui source ==================
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_impl_glfw.cpp"
#include "imgui/imgui_impl_opengl3.cpp"
#include "imgui/imgui_tables.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui.cpp"
// ======================================
#undef IMGUI_IMPL_OPENGL_LOADER_CUSTOM

// because we're calling imgui across DLL bounds, and
// imgui uses a global ctx pointer, we need to explicitly use our own
// ctx so that we are always using the one that exists in the engine dll.
// otherwise, we might end up using two instances of the global ctx that gets duplicated
// across DLL boundaries.
static ImGuiContext* engine_imgui_ctx = nullptr;

void InitImGui() {
    engine_imgui_ctx = ImGui::CreateContext();
    TINY_ASSERT(engine_imgui_ctx);
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(GetMainGLFWWindow(), true);
    static const char* glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);
}


void ImGuiBeginFrame() {
    ImGui::SetCurrentContext(engine_imgui_ctx);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


void ImGuiEndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiTerminate() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}