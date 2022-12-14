#include "particle_behaviors.h"

#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/math.h"

glm::vec2 RandomPointInCircle(f32 radius) {
    glm::vec2 ret = glm::vec2(0);
    f32 randomRadius = GetRandomf(0, radius);
    f32 len = sqrt(GetRandomf(0, 1)) * radius;
    f32 degrees = 2* PI_F * GetRandomf(0,1);
    ret.x = len * cos(degrees);
    ret.y = len * sin(degrees);
    return ret;
}
bool ParticlesSpreadOut::InitializeParticle(Particle2D& particleToEmit, glm::vec2 particleSystemPosition) {
    glm::vec2 randomVelDir = glm::normalize(RandomPointInCircle(1));
    particleToEmit.velocity += randomVelDir;
    return true;
}