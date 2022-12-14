#pragma once

#include "tiny_engine/pch.h"
#include "tiny_engine/model.h"
#include "tiny_engine/tiny_types.h"
#include "tiny_engine/tiny_engine.h"

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
    bool isValid() { return hash != 0; }
};

struct GameState {
    std::vector<WorldEntity> entities = {};
    std::vector<Light> lights = {};

    static GameState& get() {
        static GameState gs;
        return gs;
    }
    bool GetEntity(const char* name, WorldEntity& entity) {
        u32 hash = std::hash<std::string>{}(std::string(name));
        for (auto& ent : entities) {
            if (ent.hash == hash) {
                entity = ent;
                return true;
            }
        }
        return false;
    }
    WorldEntity& GetEntity(const char* name) {
        for (auto& ent : entities) {
            if (ent.hash == std::hash<std::string>{}(std::string(name))) {
                return ent;
            }
        }
        return WorldEntity();
    }
};


void testbed_init();
void testbed_tick();
void testbed_terminate();