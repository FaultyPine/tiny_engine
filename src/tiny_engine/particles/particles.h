#ifndef TINY_PARTICLES_H
#define TINY_PARTICLES_H

#include "tiny_engine/pch.h"
#include "tiny_engine/sprite.h"
#include "tiny_engine/texture.h"
#include "tiny_engine/tiny_fs.h"
#include "tiny_engine/model.h"

struct Particle {
    // value [0, 1] where 1 is "full life", and 0 is dead
    f32 life = 0.0;
    glm::vec3 size = glm::vec3(5);
    glm::vec3 position = glm::vec3(0);
    glm::vec3 velocity = glm::vec3(0,1,0);
    f32 rotation = 0.0;
    glm::vec4 color = glm::vec4(1);

    Transform GetTransform() const {
        // TODO: have particle store a transform
        return Transform(position, size, rotation, {0,1,0});
    }
};

struct ParticleBehavior {
    // return whether or not to emit a new particle
    // also (if we end up emitting a new particle...) change the
    // properties of the one we want to emit
    virtual void InitializeParticle(Particle& particleToEmit, glm::vec3 particleSystemPosition) { }
    // called every frame on active particles
    virtual void OnTick(std::vector<Particle>& particles) {}
    // called every frame
    virtual u32 ShouldEmitParticle() { return 0; }
    virtual void Reset() {}
};

// default behavior that goes onto every particle system by default
struct DefaultParticleBehavior : ParticleBehavior {
    void InitializeParticle(Particle& particleToEmit, glm::vec3 particleSystemPosition) {
        particleToEmit.life = 1.0;
        particleToEmit.position = particleSystemPosition;
    }
    void OnTick(std::vector<Particle>& particles) {
        for (Particle& particle : particles) {
            particle.position += particle.velocity;
        }
    }
};

struct ParticleSystem {
    ParticleSystem(const Sprite& particleSprite, u32 maxParticles, bool isStartingActive) 
                    : ParticleSystem(maxParticles, isStartingActive) {
        this->particleSprite = particleSprite; 
    }
    ParticleSystem(const Model& particleModel, u32 maxParticles, bool isStartingActive) 
                    : ParticleSystem(maxParticles, isStartingActive) {
        this->particleModel = particleModel;
    }
    ParticleSystem(u32 maxParticles, bool isStartingActive) {
        isActive = isStartingActive;
        for (u32 i = 0; i < maxParticles; i++)
            particles.emplace_back(Particle());
        AddBehavior(new DefaultParticleBehavior());
    }
    ParticleSystem() {}
    void Tick(glm::vec3 position);
    void Tick(glm::vec2 position) {
        this->Tick(glm::vec3(position.x, position.y, 0.0));
    }
    void Draw() const;
    ParticleSystem& AddBehavior(ParticleBehavior* behavior) {
        behaviors.emplace_back(std::shared_ptr<ParticleBehavior>(behavior));
        return *this;
    }
    void Reset();

    bool isActive = true;
    Sprite particleSprite = {};
    Model particleModel = {};
    std::vector<std::shared_ptr<ParticleBehavior>> behaviors = {};
    std::vector<Particle> particles = {};

private:
    void TrySpawnNewParticles(glm::vec3 position);
    void TrySpawnNewParticles(glm::vec2 position) {
        this->TrySpawnNewParticles(glm::vec3(position.x, position.y, 0.0));
    }
};

#endif