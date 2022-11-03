#ifndef TINY_PARTICLES_H
#define TINY_PARTICLES_H

#include "pch.h"
#include "sprite.h"
#include <functional>

// TODO:
// cramming all these particle settings
// into one struct is not super scalable and 
// makes out emit/update ticks much messier
// To fix this...
// Do this: https://youtu.be/j_H3QHdov-A?t=442
// Use a base "ParticleComponent" class and in the particlesystem
// keep a list of those
// when making a particle system, you can just "add" components to it
// like gravity, starting velocity, rotation, color over time, etc...
// then when init-ing a particle and when updating a particle, loop through all
// components on the particle system and call those init/update methods

struct Particle2D {
    f32 life = 0.0;
    glm::vec2 size, position, velocity = glm::vec2(0);
    f32 rotation = 0.0;
    glm::vec4 color = glm::vec4(1);
};

struct ParticleSystem2DProperties {
    Sprite particleSprite;
    u32 maxParticles = 35;
    u32 particlesPerTick = 2;
    glm::vec2 parentPosition, parentVelocity = glm::vec2(0);
    f32 loopDurationSeconds = 2.0;
    bool isLooping = true;
    f32 lifetimeSeconds = 4.0;
    glm::vec2 startingSpeed = glm::vec2(1);
    glm::vec2 startingSize = glm::vec2(15);
    glm::vec4 startingColor = glm::vec4(1);
    glm::vec2 gravity = glm::vec2(0);
    bool isLocalSpace = true; // false for world space
    f32 simulationSpeed = 1.0;
    f32 spawnRadius = 20.0;

    ParticleSystem2DProperties(){}
};

struct ParticleSystem2D {
    ParticleSystem2D(glm::vec2 pos, const ParticleSystem2DProperties& info);
    ParticleSystem2D() {}
    void Tick();
    void Draw() const;

    glm::vec2 position = glm::vec2(0);
private:
    void RespawnParticle(Particle2D& particle, const ParticleSystem2DProperties& props);

    ParticleSystem2DProperties particleSystemProperties;
    std::vector<Particle2D> particles = {};
};

#endif