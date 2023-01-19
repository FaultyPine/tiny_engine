#pragma once

#include "tiny_engine/pch.h"
#include "tiny_engine/model.h"
#include "tiny_engine/tiny_types.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/sprite.h"
#include "tiny_engine/framebuffer.h"
#include "tiny_engine/skybox.h"
#include "tiny_engine/particles/particles.h"

struct WorldEntity {
    Transform transform = {};
    Model model = {};
    u32 hash = 0;
    bool isVisible = true;
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
    void Draw(const Transform &tf, const std::vector<Light> &lights = {}) {
        if (isVisible) {
            model.Draw(tf, lights);
        }
    }
    bool isValid() { return hash != 0; }
};

struct Wave {
    f32 waveSpeed = 1.0;
    f32 wavelength = 10.0;
    f32 steepness = 0.3;
    glm::vec2 direction = glm::vec2(1,0);
    Wave(f32 ws, f32 wl, f32 stp, glm::vec2 dir)
        : waveSpeed(ws), wavelength(wl), steepness(stp), direction(dir) {}
    Wave(){}
};

struct GameState {
    // TODO: use hashmap w/int IDs
    std::vector<WorldEntity> entities = {};
    std::vector<Light> lights = {};

    // Main pond
    #define NUM_WAVES 8
    Wave waves[NUM_WAVES];
    WorldEntity waveEntity = {};
    Texture waterTexture;
    // Waterfall
    Shader waterfallShader = {};
    ParticleSystem waterfallParticles = {};
    
    // grass
    WorldEntity grass = {};
    std::vector<glm::mat4> grassTransforms = {};
    BoundingBox grassSpawnExclusion = {};

    // shadows/depth tex
    ShadowMap shadowMap;
    Sprite depthSprite;

    Skybox skybox = {};

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
    // TODO: use hashmap
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