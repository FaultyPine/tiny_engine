//#include "pch.h"
#include "particles.h"
#include "tiny_engine.h"
#include "tiny_log.h"

// stores the index of the last particle used (for quick access to next dead particle)
template <typename P>
u32 FirstUnusedParticle(const std::vector<P>& particles) {
    static u32 lastUsedParticle = 0;
    // first search from last used particle, this will usually return almost instantly
    for (u32 i = lastUsedParticle; i < particles.size(); i++){
        if (particles[i].life <= 0.0f){
            lastUsedParticle = i;
            return i;
        }
    }
    // otherwise, do a linear search
    for (u32 i = 0; i < lastUsedParticle; i++){
        if (particles[i].life <= 0.0f){
            lastUsedParticle = i;
            return i;
        }
    }
    // all particles are taken, override the first one 
    // (note that if it repeatedly hits this case, more particles should be reserved)
    lastUsedParticle = 0;
    return 0;
}

void ParticleSystem::TrySpawnNewParticles(glm::vec3 position) {
    // add up all particles we should emit
    u32 numNewParticles = 0;
    for (auto& behavior : behaviors) {
        numNewParticles += behavior->ShouldEmitParticle();
    }
    if (numNewParticles) {
        LOG_INFO("Spawning %i particles. Adjusted numNewParticles: %i", numNewParticles, Math::MIN(numNewParticles, (u32)particles.size()));
    }
    numNewParticles = Math::MIN(numNewParticles, (u32)particles.size());

    for (u32 i = 0; i < numNewParticles; i++) {
        Particle newParticle = Particle();
        // allow all behaviors to modify the new particle before spawning it in
        for (auto& behavior : behaviors) {
            behavior->InitializeParticle(newParticle, position);
        }
        u32 firstUnusedParticleIdx = FirstUnusedParticle(particles);
        LOG_INFO("New particle %i life: %f", firstUnusedParticleIdx, newParticle.life);
        // "emitting" a particle just means overwriting a dead one in the pool with a new one
        particles.at(firstUnusedParticleIdx) = newParticle;
    }
}

void ParticleSystem::Tick(glm::vec3 position) {    
    if (!isActive) return;
    TrySpawnNewParticles(position);
    // TODO: Don't tick dead/unused particles
    for (std::shared_ptr<ParticleBehavior> behavior : behaviors) {
        behavior->OnTick(particles);
    }
}

void ParticleSystem::Draw() const {
    if (!isActive) return;
    for (const Particle& particle : particles) {
        if (particle.life <= 0.0f) continue;
        
        if (particleSprite.isValid()) {
            particleSprite.DrawSprite(particle.position-particle.size, particle.size, particle.rotation, {0.0, 0.0, 1.0}, particle.color, true);
        }
        if (particleModel.isValid()) {
            LOG_INFO("%f", particle.life);
            //particleModel.cachedShader.use();
            particleModel.cachedShader.setUniform("color", particle.color);
            particleModel.Draw(particle.GetTransform());
        }
    }
}

void ParticleSystem::Reset() {
    u32 maxParticles = particles.size();
    particles.clear();
    for (u32 i = 0; i < maxParticles; i++) {
        particles.at(i) = Particle();
    }
    // reset behaviors (if behaviors track their own variables, these should be reset here)
    for (auto& behavior : behaviors) behavior->Reset();
}