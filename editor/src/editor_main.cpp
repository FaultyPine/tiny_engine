#include "editor_main.h"


#include "tiny_imgui.h"
#include "testbed_main.h"
#include "input.h"
#include "tiny_engine.h"
#include "camera.h"
#include "render/framebuffer.h"

struct EditorState
{
    u32 something = 0;
    float someColor[3] = {};
    Framebuffer editorMainFb;
    AppRunCallbacks gameCallbacks = {};
};
static EditorState* globEditorState;


void* editor_render_game(const Arena* const mem)
{
    Framebuffer outGameWindow = globEditorState->gameCallbacks.renderFunc(mem);
    return (void*)outGameWindow.GetTexture().id;
}

void editor_render_game_window(void* renderedGameFrameHandle)
{
    ImGui::Begin("Game");
    ImVec2 window_dimensions = ImGui::GetContentRegionAvail();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddImage(
        renderedGameFrameHandle, 
        pos, 
        ImVec2(pos.x + window_dimensions.x, pos.y + window_dimensions.y), 
        ImVec2(0,1), ImVec2(1,0)); // uvs flipped b/c opengl
    ImGui::End();
}

void editor_render_toolbar(const Arena* const mem, EditorState* editor)
{
    ImGui::Begin("Toolbar", 0, ImGuiWindowFlags_MenuBar);
    ImGui::BeginMenuBar();
    if (ImGui::MenuItem("File"))
    {
        if (ImGui::Button("Some button"))
        {
            editor->something++;
        }
    }
    ImGui::EndMenuBar();
    ImGui::End();
}

void editor_render_inspector(const Arena* const mem, EditorState* editor)
{
    ImGui::Begin("Inspector");

    ImGui::End();
}

void editor_render_scene_hierarchy(const Arena* const mem, EditorState* editor)
{
    ImGui::Begin("Scene Hierachy");
    if (ImGui::TreeNode("SomeObject1"))
    {
        ImGui::DragInt("Something", (int*)&editor->something);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("SomeObject1"))
    {
        ImGui::ColorEdit3("Property", editor->someColor);
        ImGui::TreePop();
    }
    ImGui::End();
}

void editor_init(Arena* mem)
{
    EditorState* editor = (EditorState*)arena_alloc(mem, sizeof(EditorState));
    globEditorState = editor;
    // fetch our game's callbacks
    GetTestbedAppRunCallbacks(&editor->gameCallbacks);
    editor->gameCallbacks.initFunc(mem);
    
    editor->editorMainFb = Framebuffer(Camera::GetScreenWidth(), Camera::GetScreenHeight(), Framebuffer::FramebufferAttachmentType::COLOR);
}
void editor_tick(Arena* mem)
{
    globEditorState->gameCallbacks.tickFunc(mem);
}
Framebuffer editor_render(const Arena* const mem)
{
    void* renderedGameFrame = editor_render_game(mem);
    //ImGuiBeginFrame();
    EditorState* editor = globEditorState;
    editor->editorMainFb.Bind();
    editor_render_game_window(renderedGameFrame);
    editor_render_toolbar(mem, editor);
    editor_render_inspector(mem, editor);
    editor_render_scene_hierarchy(mem, editor);
    //ImGuiEndFrame();
    return editor->editorMainFb;
}
void editor_terminate(Arena* mem)
{
    globEditorState->gameCallbacks.terminateFunc(mem);
}




void editor_main(int argc, char *argv[])
{
    EngineState engineInitState;
    engineInitState.appName = "Editor";
    engineInitState.windowWidth = 1920;
    engineInitState.windowHeight = 1080;
    engineInitState.aspectRatioW = 16;
    engineInitState.aspectRatioH = 9;
    engineInitState.false2DTrue3D = true;

    // editor callbacks
    // NOTE: with this, we are running our editor as an
    // app of our engine... do we want this? 
    // or should our editor own it's own loop.. how should we tick the game then...
    engineInitState.appCallbacks.initFunc = editor_init;
    engineInitState.appCallbacks.tickFunc = editor_tick;
    engineInitState.appCallbacks.renderFunc = editor_render;
    engineInitState.appCallbacks.terminateFunc = editor_terminate;
    InitEngine(engineInitState, MEGABYTES_BYTES(20), argc, argv);

}