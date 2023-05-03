#include "pch.h"
#include "cc_main.h"

#include "tiny_engine/camera.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/shapes.h"
#include "tiny_engine/external/imgui/tiny_imgui.h"
#include "tiny_engine/tiny_fs.h"
#include "tiny_engine/tiny_profiler.h"
#include "tiny_engine/shader.h"

#include "rundata.h"

void PollInputs() {
    if (Keyboard::isKeyDown(GLFW_KEY_ESCAPE)) {
        CloseGameWindow();
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_R)) {
        Shader::ReloadShaders();
    }

#define CAMERA_SPEED 5.0
    if (Keyboard::isKeyDown(GLFW_KEY_W)) {
        Camera::GetMainCamera().cameraPos.y += CAMERA_SPEED;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_A)) {
        Camera::GetMainCamera().cameraPos.x += CAMERA_SPEED;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_S)) {
        Camera::GetMainCamera().cameraPos.y -= CAMERA_SPEED;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_D)) {
        Camera::GetMainCamera().cameraPos.x -= CAMERA_SPEED;
    }

}

void DrawDebug() {
    ImGuiBeginFrame();
    ImGui::Text("%.3fms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGuiEndFrame();
}

void UpdateGroup(GuyGroup* group) {

}

void Draw(Rundata& rd) {
    DrawGuyGroup(&rd.guygroup);
    DrawDebug();
}
void Update(Rundata& rd) {

    UpdateGroup(&rd.guygroup);
}

void cc_init() {
    PROFILE_FUNCTION();
    InitImGui();
    Rundata& rd = Rundata::get();
    GuyGroup& gg = rd.guygroup;
    MakeGuyGroup(&gg);
    for (int i = 0; i < 100; i++) {
        MakeGuy(&gg, glm::vec2(sin(i) * i * 5, cos(i) * i * 5));
    }
}

void cc_tick() {
    PROFILE_FUNCTION();
    Rundata& rd = Rundata::get();
    PollInputs();
    Update(rd);
    Draw(rd);
}
void cc_terminate() {
    ImGuiTerminate();
}