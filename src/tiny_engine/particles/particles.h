#ifndef TINY_PARTICLES_H
#define TINY_PARTICLES_H

#include "tiny_engine/pch.h"
#include "tiny_engine/sprite.h"
#include "tiny_engine/texture.h"
#include "tiny_engine/tiny_fs.h"

struct Particle2D {
    // value [0, 1] where 1 is "full life", and 0 is dead
    f32 life = 0.0;
    glm::vec2 size = glm::vec2(15);
    glm::vec2 position, velocity = glm::vec2(0);
    f32 rotation = 0.0;
    glm::vec4 color = glm::vec4(1);
};

struct ParticleBehavior {
    // return whether or not to emit a new particle
    // also (if we end up emitting a new particle...) change the
    // properties of the one we want to emit
    virtual bool InitializeParticle(Particle2D& particleToEmit, glm::vec2 particleSystemPosition) { return false; }
    // called every frame on active particles
    virtual void OnTick(Particle2D& particle) {}
    // called every frame
    virtual u32 ShouldEmitParticle() { return 0; }
    virtual void Reset() {}
};

// default behavior that goes onto every particle system by default
struct DefaultParticleBehavior : ParticleBehavior {
    bool InitializeParticle(Particle2D& particleToEmit, glm::vec2 particleSystemPosition) {
        particleToEmit = Particle2D();
        particleToEmit.life = 1.0;
        particleToEmit.position = particleSystemPosition;
        return false;
    }
    void OnTick(Particle2D& particle) {
        particle.position += particle.velocity;
    }
};

struct ParticleSystem2D {
    ParticleSystem2D(u32 maxParticles, bool isStartingActive) {
        isActive = isStartingActive;
        for (u32 i = 0; i < maxParticles; i++)
            particles.emplace_back(Particle2D());
        AddBehavior(new DefaultParticleBehavior());
    }
    ParticleSystem2D() {}
    void Tick(glm::vec2 position);
    void Draw() const;
    ParticleSystem2D& AddBehavior(ParticleBehavior* behavior) {
        behaviors.emplace_back(std::shared_ptr<ParticleBehavior>(behavior));
        return *this;
    }
    void Reset();

    bool isActive = true;
    Sprite defaultParticleSprite;
    std::vector<std::shared_ptr<ParticleBehavior>> behaviors = {};
    std::vector<Particle2D> particles = {};

private:
    void TrySpawnNewParticles(glm::vec2 position);
};

#endif