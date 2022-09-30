#ifndef TINY_TEXT_H
#define TINY_TEXT_H

#include "tiny_engine/pch.h"

struct GLTText;
GLTtext* CreateText(const char* msg);
void DrawText(GLTtext* text, f32 x = 0.0f, f32 y = 0.0f, f32 scale = 1.0f, f32 r = 1.0f, f32 g = 1.0f, f32 b = 1.0f, f32 a = 1.0f);
void UnloadText(GLTtext* text);

#endif