#ifndef OPENGL_WRAPPER_H
#define OPENGL_WRAPPER_H

#include "pch.h"


void framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height);

void CloseGameWindow();
void SetMode2D();
void SetMode3D();

// returns the current GLFW time
f64 GetTime();
// just casts GetTime to f32
f32 GetTimef();

void OverwriteRandomSeed(u64 seed);
u64 GetRandomSeed();
s32 GetRandom(s32 start, s32 end);
f32 GetRandomf(f32 start, f32 end);

f32 GetDeltaTime();

u32 GetFrameCount();

/// Game loop should be while(!ShouldCloseWindow())
bool ShouldCloseWindow();

void TerminateGame();

void InitGame(u32 windowWidth, u32 windowHeight, u32 aspectRatioW, u32 aspectRatioH,
            const s8* windowName, bool false2DTrue3D);
void ClearGLBuffers();
void ClearGLColorBuffer();
void SetMinAndMaxWindowSize(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);

void SetWireframeDrawing(bool shouldDrawWireframes);


#endif