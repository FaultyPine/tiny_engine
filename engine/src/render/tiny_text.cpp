//#include "pch.h"
#include "tiny_text.h"

#define GLT_DEBUG
#define GLT_DEBUG_PRINT
#define GLT_IMPLEMENTATION
// because this is defined, make sure to call gltViewport(width, height) when the screen is resized
// this optimizes away a call to glGetIntegerv
#define GLT_MANUAL_VIEWPORT 
#include "render/tiny_ogl.h"
#include "external/GlText/glText.h"

void UpdateGLTViewport(s32 width, s32 height) {
    // for some reason gltViewport can't handle width/height of 0 (which happens when you minimize the window)
    if (width > 0 && height > 0)
    {
        gltViewport(width, height);
    }
}
bool GLTInitialize() {
    return gltInit();
}
void GLTTerminate() {
    gltTerminate();
}


GLTtext* CreateText(const char* msg) {
    GLTtext* txt = gltCreateText();
    gltSetText(txt, msg);
    return txt;
}

void DrawText(GLTtext* text, f32 x, f32 y, f32 scale, f32 r, f32 g, f32 b, f32 a) {
    gltBeginDraw();
    gltColor(r, g, b, a);
    gltDrawText2D(text, x, y, scale);
    gltEndDraw();
}

void SetText(GLTtext* text, const char* msg) {
    gltSetText(text, msg);
}

void UnloadText(GLTtext* txt) {
    gltDeleteText(txt);
}