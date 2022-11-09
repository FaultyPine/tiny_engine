#ifndef TINY_PARTICLE_BEHAVIORS_H
#define TINY_PARTICLE_BEHAVIORS_H

#include "particles.h"
#include "tiny_engine/tiny_engine.h"

struct ParticleDecay : ParticleBehavior {
    f32 lifeDecayPerTick = 0;
    ParticleDecay(f32 lifeDecayPerTick) { this->lifeDecayPerTick = lifeDecayPerTick; }
    void OnTick(Particle2D& particle) {
        particle.life -= lifeDecayPerTick;
    }
};
struct ParticleAlphaDecay : ParticleBehavior {
    f32 alphaDecayPerTick = 0;
    ParticleAlphaDecay(f32 alphaDecayPerTick) { this->alphaDecayPerTick = alphaDecayPerTick; }
    void OnTick(Particle2D& particle) {
        particle.color.a -= alphaDecayPerTick;
    }
};

struct ParticlesSpreadOut : ParticleBehavior {
    bool InitializeParticle(Particle2D& particleToEmit, glm::vec2 particleSystemPosition);
};

struct ParticleEmitEveryTick : ParticleBehavior {
    u32 ShouldEmitParticle() { return 1; }
};
struct ParticleEmitTickInterval : ParticleBehavior {
    u32 everyXTicks = 1;
    ParticleEmitTickInterval(u32 everyXTicks) {
        this->everyXTicks = everyXTicks;
    }
    u32 ShouldEmitParticle() {
        return GetFrameCount() % everyXTicks == 0;
    }
};
struct ParticleEmitBurst : ParticleBehavior {
    bool hasFired = false;
    u32 numParticlesInBurst = 1;
    ParticleEmitBurst(u32 numParticlesInBurst) {
        this->numParticlesInBurst = numParticlesInBurst;
    }
    u32 ShouldEmitParticle() {
        if (!hasFired) {
            hasFired = true;
            return numParticlesInBurst;
        }
        return 0;
    }
    void Reset() {
        hasFired = false;
    }
};

struct ParticleColorGradient : ParticleBehavior {
    glm::vec4 firstCol, secondCol;
    ParticleColorGradient(glm::vec4 firstCol, glm::vec4 secondCol) {
        this->firstCol = firstCol;
        this->secondCol = secondCol;
    }
    void OnTick(Particle2D& particle) {
        particle.color = glm::mix(firstCol, secondCol, 1.0-particle.life);
    }
};

#endif