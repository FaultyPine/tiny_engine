#pragma once

#include "tiny_engine/sprite.h"
#include "tiny_engine/pch.h"
#include "tiny_engine/tiny_types.h"
#include "QuadTree.h"

constexpr u32 MAX_ENTITIES = 500;

struct NPC {
    Transform2D tf;
    glm::vec2 desiredPosition;
};

// things like positions, flags, things that affect the gamestate directly
struct GameState {
    std::vector<NPC> npcs = {};
    QuadTree<NPC*> tree = {};
};

// things that do not affect gamestate (textures, shaders, sounds, etc)
struct NonGameState {
    Sprite character;
};

struct Rundata {
    GameState gs;
    NonGameState ngs;


    static Rundata& Instance() {
        static Rundata rd;
        return rd;
    }
};