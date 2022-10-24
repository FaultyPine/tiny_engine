#ifndef OPENGL_WRAPPER_H
#define OPENGL_WRAPPER_H

#include "pch.h"


void framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height);

void CloseGameWindow();

// returns the current GLFW time
f64 GetTime();

void InitializeRandomSeed(u64 seed);
u64 GetRandomSeed();
u32 GetRandom(u32 start = 0, u32 end = 1);

f32 GetDeltaTime();

u32 GetFrameCount();

/// Game loop should be while(!ShouldCloseWindow())
bool ShouldCloseWindow();

void TerminateGame();

void InitGame(u32 windowWidth, u32 windowHeight, u32 aspectRatioW, u32 aspectRatioH, const s8* windowName);
void ClearGLBuffers();
void ClearGLColorBuffer();

void EnableWireframeDrawing();


#endif