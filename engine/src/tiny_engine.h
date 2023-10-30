#ifndef OPENGL_WRAPPER_H
#define OPENGL_WRAPPER_H

//#include "pch.h"
#include "tiny_defines.h"


#define TARGET_FPS 60

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

/// Game loop should be while(!ShouldCloseWindow())
TAPI bool ShouldCloseWindow();

TAPI void TerminateGame();

TAPI void InitEngine(u32 windowWidth, u32 windowHeight, u32 aspectRatioW, u32 aspectRatioH,
            const s8* windowName, bool false2DTrue3D, const char* resourceDirectory);
TAPI void ClearGLBuffers();
TAPI void ClearGLColorBuffer();
TAPI void SetMinAndMaxWindowSize(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);

TAPI void SetWireframeDrawing(bool shouldDrawWireframes);

struct GLFWwindow;
TAPI GLFWwindow* GetMainGLFWWindow();

#endif