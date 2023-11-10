#ifndef OPENGL_WRAPPER_H
#define OPENGL_WRAPPER_H

//#include "pch.h"
#include "tiny_defines.h"
#include "tiny_alloc.h"

#define TARGET_FPS 60

struct GLFWwindow;
struct Framebuffer;

typedef void(*InitFunction)(Arena* gameMem);
typedef void(*TickFunction)(Arena* gameMem);
// render functions should output a texture handle to be rendered
typedef Framebuffer(*RenderFunction)(const Arena* const gameMem);
typedef void(*TerminateFunction)(Arena* gameMem);

struct AppRunCallbacks
{
    InitFunction initFunc = 0;
    TickFunction tickFunc = 0;
    RenderFunction renderFunc = 0;
    TerminateFunction terminateFunc = 0;
};

struct EngineState
{
    u32 windowWidth = 0;
    u32 windowHeight = 0; 
    u32 aspectRatioW = 0; 
    u32 aspectRatioH = 0;
    const char* appName = nullptr;
    bool false2DTrue3D = false;
    AppRunCallbacks appCallbacks = {};

    const char* resourceDirectory = nullptr;
    Arena gameArena;
    f32 deltaTime = 0.0f;
    f32 lastFrameTime = 0.0f;
    u32 frameCount = 0;
    GLFWwindow* glob_glfw_window = nullptr;
    u64 randomSeed = 0;
};


TAPI void InitEngine(const EngineState& engineInitState, size_t requestedMemSize, int argc, char *argv[]);

/// Game loop should be while(!ShouldCloseWindow())
TAPI bool ShouldCloseWindow();

TAPI void TerminateGame();

TAPI void CloseGameWindow();
TAPI void SetMode2D();
TAPI void SetMode3D();

// returns the current GLFW time
TAPI f64 GetTime();
// just casts GetTime to f32
TAPI f32 GetTimef();

TAPI void OverwriteRandomSeed(u64 seed);
TAPI u64 GetRandomSeed();
TAPI s32 GetRandom(s32 start, s32 end);
TAPI f32 GetRandomf(f32 start, f32 end);

TAPI f32 GetDeltaTime();

TAPI u32 GetFrameCount();

TAPI void ClearGLBuffers();
TAPI void ClearGLColorBuffer();
TAPI void SetMinAndMaxWindowSize(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);

TAPI void SetWireframeDrawing(bool shouldDrawWireframes);

struct GLFWwindow;
TAPI GLFWwindow* GetMainGLFWWindow();

#endif