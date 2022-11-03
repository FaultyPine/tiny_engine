#include "particles.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/texture.h"
#include "tiny_engine/tiny_fs.h"

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

glm::vec2 RandomPointInCircle(f32 radius) {
    glm::vec2 ret = glm::vec2(0);
    f32 randomRadius = GetRandomf(0, radius);
    f32 len = sqrt(GetRandomf(0, 1)) * radius;
    f32 degrees = 2* PI_F * GetRandomf(0,1);
    ret.x = len * cos(degrees);
    ret.y = len * sin(degrees);
    return ret;
}

void ParticleSystem2D::RespawnParticle(Particle2D& particle, const ParticleSystem2DProperties& props) {
    glm::vec2 randPointInCircle = RandomPointInCircle(props.spawnRadius);
    if (props.isLocalSpace) {
        particle.position = props.parentPosition + randPointInCircle;
        particle.velocity = props.startingSpeed;
    }
    else { // world space 
        particle.position = randPointInCircle;
        particle.velocity = props.parentVelocity;
    }
    particle.color = props.startingColor;
    particle.size = props.startingSize;
    particle.life = props.lifetimeSeconds;
}

void DefaultTick(Particle2D& particle, const ParticleSystem2DProperties& props, ParticleSystem2D& particleSystem, f32 deltaTime) {
    glm::vec4 col = particle.color;
    col.a -= 0.01;
    particle.color = col;
}
void DefaultDraw(const Particle2D& particle, const ParticleSystem2DProperties& props) {
    props.particleSprite.DrawSprite(Camera::GetMainCamera(), particle.position, particle.size, particle.rotation, {0.0, 0.0, 1.0}, particle.color, true);
}

ParticleSystem2D::ParticleSystem2D(glm::vec2 pos, const ParticleSystem2DProperties& info) {
    this->position = pos;
    this->particleSystemProperties = info;
    if (!this->particleSystemProperties.particleSprite.isValid()) {
        TextureProperties props = TextureProperties::Default();
        Shader particleShader = Shader(UseResPath("shaders/particle.vs").c_str(), UseResPath("shaders/particle.fs").c_str());
        Texture particleTexture = LoadTexture(UseResPath("shaders/default_particle.png").c_str(), props);
        this->particleSystemProperties.particleSprite = Sprite(particleShader, particleTexture);
    }
    this->particles.clear(); // ensure empty before filling with blanks
    for (u32 i = 0; i < info.maxParticles; i++) {
        Particle2D particle = Particle2D();
        RespawnParticle(particle, particleSystemProperties);
        this->particles.push_back(particle);
    }
}

void ParticleSystem2D::Tick() {
    f32 deltaTime = GetDeltaTime();
    // respawn however many "particles per tick" we need
    for (u32 i = 0; i < particleSystemProperties.particlesPerTick; i++) {
        if (particles.size() > i) {
            u32 unusedParticle = FirstUnusedParticle(particles);
            RespawnParticle(particles.at(unusedParticle), particleSystemProperties);
        }
    }

    // update all particles
    for (u32 i = 0; i < particles.size(); i++) {
        Particle2D& p = particles[i];
        // TODO: decrease life based on the lifetime seconds
        p.life -= deltaTime; // reduce life
        if (p.life > 0.0f) { // particle is alive, thus update
            p.position += p.velocity * deltaTime; 
            DefaultTick(p, particleSystemProperties, *this, deltaTime);
        }
    }
}
void ParticleSystem2D::Draw() const {
    const ParticleSystem2DProperties props = this->particleSystemProperties;
    const Sprite& particleSprite = props.particleSprite;
    for (const Particle2D& p : this->particles) {
        particleSprite.setShaderUniform("offset", p.position);
        particleSprite.setShaderUniform("scale", p.size);
        DefaultDraw(p, particleSystemProperties);
    }
}
