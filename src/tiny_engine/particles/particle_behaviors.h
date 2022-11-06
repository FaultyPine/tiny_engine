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
    bool ShouldEmitParticle(glm::vec2& offset) { return true; }
};
struct ParticleEmitTickInterval : ParticleBehavior {
    u32 everyXTicks = 1;
    ParticleEmitTickInterval(u32 everyXTicks) {
        this->everyXTicks = everyXTicks;
    }
    bool ShouldEmitParticle(glm::vec2& offset) {
        return GetFrameCount() % everyXTicks == 0;
    }
};

#endif