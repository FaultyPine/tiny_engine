#ifndef TINY_IMGUI_H
#define TINY_IMGUI_H

#include "tiny_defines.h"
#include "imgui/imgui.h"
#include "tiny_engine.h" // for the glfw window

#define ENABLE_IMGUI


TAPI void InitImGui();
TAPI void ImGuiBeginFrame();
TAPI void ImGuiEndFrame();
TAPI void ImGuiTerminate();

#endif
