#include "particles.h"
#include "tiny_engine/tiny_engine.h"

// stores the index of the last particle used (for quick access to next dead particle)
u32 FirstUnusedParticle(const std::vector<Particle2D>& particles) {
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
    // all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
    lastUsedParticle = 0;
    return 0;
}

void ParticleSystem2D::TrySpawnNewParticles(glm::vec2 position) {
    u32 numNewParticles = 0;
    // if any of our behaviors say we should emit, then we emit one particle on this frame
    for (auto& behavior : behaviors) {
        numNewParticles += behavior->ShouldEmitParticle();
    }
    for (u32 i = 0; i < numNewParticles; i++) {
        // allow all behaviors to modify the particle before spawning it in
        Particle2D newParticle = Particle2D();
        for (auto& behavior : behaviors)
            behavior->InitializeParticle(newParticle, position);
        u32 firstUnusedParticleIdx = FirstUnusedParticle(particles);
        // "emitting" a particle just means overwriting a dead one in the pool with a new one
        particles.at(firstUnusedParticleIdx) = newParticle;
    }
}

void ParticleSystem2D::Tick(glm::vec2 position) {    
    if (!defaultParticleSprite.isValid()) defaultParticleSprite = Sprite(LoadTexture(UseResPath("shaders/default_particle.png").c_str(), TextureProperties::RGBA_LINEAR()));
    if (!isActive) return;
    TrySpawnNewParticles(position);
    for (auto& behavior : behaviors) {
        for (Particle2D& particle : particles) {
            behavior->OnTick(particle);
        }
    }
}

void ParticleSystem2D::Draw() const {
    if (!isActive) return;
    for (const Particle2D& particle : particles) {
        defaultParticleSprite.DrawSprite(Camera::GetMainCamera(), particle.position-particle.size, particle.size, particle.rotation, {0.0, 0.0, 1.0}, particle.color, true);
    }
}

void ParticleSystem2D::Reset() {
    u32 maxParticles = particles.size();
    particles.clear();
    for (u32 i = 0; i < maxParticles; i++)
        particles.emplace_back(Particle2D());
    // reset behaviors (if behaviors track their own variables, these should be reset here)
    for (auto& behavior : behaviors) behavior->Reset();
}