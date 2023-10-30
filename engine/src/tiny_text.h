#ifndef TINY_TEXT_H
#define TINY_TEXT_H

//#include "pch.h"
#include "tiny_defines.h"

struct GLTtext;
TAPI GLTtext* CreateText(const char* msg);
TAPI void SetText(GLTtext* text, const char* msg);
TAPI void DrawText(GLTtext* text, f32 x = 0.0f, f32 y = 0.0f, f32 scale = 1.0f, f32 r = 1.0f, f32 g = 1.0f, f32 b = 1.0f, f32 a = 1.0f);
TAPI void UnloadText(GLTtext* text);

void UpdateGLTViewport(s32 width, s32 height);
bool GLTInitialize();
void GLTTerminate();

#endif