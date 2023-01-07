#include "particle_behaviors.h"

#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/math.h"

bool ParticlesSpreadOut::InitializeParticle(Particle2D& particleToEmit, glm::vec2 particleSystemPosition) {
    glm::vec2 randomVelDir = glm::normalize(Math::RandomPointInCircle(1));
    particleToEmit.velocity += randomVelDir;
    return true;
}