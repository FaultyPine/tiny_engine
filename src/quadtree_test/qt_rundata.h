#pragma once

#include "pch.h"
#include "tiny_engine/sprite.h"
#include "tiny_engine/tiny_types.h"
#include "tiny_engine/QuadTree.h"

constexpr u32 MAX_ENTITIES = 500;

struct NPC {
    Transform2D tf;
    glm::vec2 desiredPosition;
};

struct QTRundata {
    QuadTree<NPC*> tree = {};
    std::array<NPC, MAX_ENTITIES> npcs = {};
    Sprite character;



    static QTRundata& Instance() {
        static QTRundata rd;
        return rd;
    }
};