#include "tiny_text.h"


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

void SetText(GLTtext* text, const std::string& msg) {
    gltSetText(text, msg.c_str());
}

void UnloadText(GLTtext* txt) {
    gltDeleteText(txt);
}