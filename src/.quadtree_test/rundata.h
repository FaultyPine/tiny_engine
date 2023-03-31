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

struct Rundata {
    QuadTree<NPC*> tree = {};
    std::vector<NPC> npcs = {};
    Sprite character;



    static Rundata& Instance() {
        static Rundata rd; 
        return rd;
    }
};