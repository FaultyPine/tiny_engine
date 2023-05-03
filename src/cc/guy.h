#pragma once
#include "pch.h"
#include "tiny_engine/sprite.h"

struct Guy {
    Transform2D tf;
};
struct GuyGroup {
    std::vector<Guy> guys;
    Sprite sprite;
};


void MakeGuyGroup(GuyGroup* group);
void DrawGuyGroup(GuyGroup* group);
void MakeGuy(GuyGroup* group, glm::vec2 pos);
void DrawGuy(GuyGroup* group, Guy* guy);
