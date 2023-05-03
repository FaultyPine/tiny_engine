#ifndef TINY_IMGUI_H
#define TINY_IMGUI_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define ENABLE_IMGUI

inline void InitImGui() {
#ifdef ENABLE_IMGUI
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(glob_glfw_window, true);
    static const char* glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);
#endif
}
inline void ImGuiBeginFrame() {
#ifdef ENABLE_IMGUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
#endif
}
inline void ImGuiEndFrame() {
#ifdef ENABLE_IMGUI
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}
inline void ImGuiTerminate() {
#ifdef ENABLE_IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
#endif
}

#endif