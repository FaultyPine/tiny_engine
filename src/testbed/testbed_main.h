#pragma once

#include "tiny_engine/pch.h"
#include "tiny_engine/model.h"
#include "tiny_engine/tiny_types.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/sprite.h"
#include "tiny_engine/framebuffer.h"

struct WorldEntity {
    Transform transform;
    Model model;
    u32 hash = 0;
    WorldEntity(){}
    WorldEntity(const Transform& tf, const Model& mod, const char* name = "") {
        transform = tf;
        model = mod;
        if (strlen(name) < 1) {
            hash = std::hash<f64>{}(GetTime());
        }
        else {
            hash = std::hash<std::string>{}(std::string(name));
        }
    }
    void Delete() {
        model.Delete();
    }
    bool isValid() { return hash != 0; }
};

struct GameState {
    std::vector<WorldEntity> entities = {};
    std::vector<Light> lights = {};

    ShadowMap shadowMap;
    Sprite depthSprite;

    static GameState& get() {
        static GameState gs;
        return gs; 
    }
    void Terminate() {
        for (auto& ent : entities) {
            ent.Delete();
        }
        shadowMap.Delete();
        depthSprite.Delete();
    }
    // TODO: return shared_ptr?
    WorldEntity* GetEntity(const char* name) {
        u32 hash = std::hash<std::string>{}(std::string(name));
        for (auto& ent : entities) {
            if (ent.hash == hash) {
                return &ent;
            }
        }
        return nullptr;
    }
};


void testbed_init();
void testbed_tick();
void testbed_terminate();