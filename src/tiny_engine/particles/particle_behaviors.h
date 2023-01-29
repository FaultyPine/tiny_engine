#ifndef TINY_PARTICLE_BEHAVIORS_H
#define TINY_PARTICLE_BEHAVIORS_H

#include "particles.h"
#include "tiny_engine/tiny_engine.h"

// NOTE:
// these can be defined elsewhere and plugged into particle systems with AddBehavior
// these are just some common/useful ones

struct ParticleDecay : ParticleBehavior {
    f32 lifeDecayPerTick = 0;
    ParticleDecay(f32 lifeDecayPerTick) { this->lifeDecayPerTick = lifeDecayPerTick; }
    void OnTick(std::vector<Particle>& particles) override {
        for (auto& particle : particles) {
            particle.life -= lifeDecayPerTick;
        }
    }
};

struct ParticleSetSize : ParticleBehavior {
    glm::vec3 size;
    ParticleSetSize(glm::vec3 size) { this->size = size; }
    void InitializeParticle(Particle& particle, glm::vec3 particleSystemPosition) override {
        particle.size = size;
    }
};
struct ParticleSetVelocity : ParticleBehavior {
    glm::vec3 vel;
    ParticleSetVelocity(glm::vec3 vel) { this->vel = vel; }
    void InitializeParticle(Particle& particle, glm::vec3 particleSystemPosition) override {
        particle.velocity = vel;
    }
};
struct ParticleAlphaDecay : ParticleBehavior {
    f32 alphaDecayPerTick = 0;
    ParticleAlphaDecay(f32 alphaDecayPerTick) { this->alphaDecayPerTick = alphaDecayPerTick; }
    void OnTick(std::vector<Particle>& particles) override {
        for (auto& particle : particles) {
            particle.color.a -= alphaDecayPerTick;
            particle.color.a = Math::MAX(0.0f, particle.color.a);
        }
    }
};

struct ParticlesSpreadOut : ParticleBehavior {
    void ParticlesSpreadOut::InitializeParticle(Particle& particleToEmit, glm::vec3 particleSystemPosition) override {
        f32 radius = 1.0f;
        glm::vec3 randomVelDir = Math::RandomPointInSphere(radius);
        particleToEmit.velocity = randomVelDir;
    }
};

struct ParticleEmitEveryTick : ParticleBehavior {
    u32 ShouldEmitParticle() override { return 1; }
};
struct ParticleEmitTickInterval : ParticleBehavior {
    u32 everyXTicks = 1;
    ParticleEmitTickInterval(u32 everyXTicks) {
        this->everyXTicks = everyXTicks;
    }
    u32 ShouldEmitParticle() override {
        u32 framecount = GetFrameCount();
        bool shouldEmit = framecount % everyXTicks == 0;
        return shouldEmit ? 1 : 0;
    }
};
struct ParticleEmitBurst : ParticleBehavior {
    bool hasFired = false;
    u32 numParticlesInBurst = 1;
    ParticleEmitBurst(u32 numParticlesInBurst) {
        this->numParticlesInBurst = numParticlesInBurst;
    }
    u32 ShouldEmitParticle() override {
        if (!hasFired) {
            hasFired = true;
            return numParticlesInBurst;
        }
        return 0;
    }
    void OnTick(std::vector<Particle>& particles) override {
        u32 numParticlesDead = 0;
        for (auto& particle : particles) {
            if (hasFired && particle.life <= 0.0f) {
                numParticlesDead++;
            }
        }
        if (numParticlesDead >= numParticlesInBurst) {
            hasFired = false;
        }
    }
    void Reset() override {
        hasFired = false;
    }
};

struct ParticleColorGradient : ParticleBehavior {
    glm::vec4 firstCol, secondCol;
    ParticleColorGradient(glm::vec4 firstCol, glm::vec4 secondCol) {
        this->firstCol = firstCol;
        this->secondCol = secondCol;
    }
    void OnTick(std::vector<Particle>& particles) override {
        for (auto& particle : particles) {
            particle.color = glm::mix(firstCol, secondCol, 1.0-particle.life);
        }
    }
};

#endif