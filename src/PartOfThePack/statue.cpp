#include "statue.h"
#include "tiny_engine/tiny_fs.h"
#include "ninja.h"
#include "tiny_engine/shapes.h"
#include "tiny_engine/tiny_audio.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/particles/particle_behaviors.h"

enum StatueFacingDir {
    NEUTRAL = 0,
    SOUTHWEST,
    SOUTHEAST,
    NORTH,
    SOUTH,
    WEST,
    EAST,
    NORTHEAST,
    NORTHWEST,
    STRAIGHT,
};

void Statue::Initialize(glm::vec2 pos) {
    entity.name = "Statue";
    TextureProperties texProps = TextureProperties::Default();
    texProps.magFilter = TextureProperties::TexMagFilter::NEAREST;
    texProps.minFilter = TextureProperties::TexMinFilter::NEAREST;

    spritesheet = Spritesheet(UseResPath("potp/tdingle_spritesheet.png").c_str(), 4, 3, texProps);
    spritesheet.SetAnimationIndices(0, {0});
    spritesheet.SetAnimationIndices(1, {1,2,3,4,5,6,7,8,9});
    spritesheet.SetAnimation(Spritesheet::Animation(0));

    f32 spriteWidth = spritesheet.GetCurrentSprite().GetTextureWidth();
    f32 spriteHeight = spritesheet.GetCurrentSprite().GetTextureHeight();
    f32 statueSpriteSize = 75.0;
    f32 statueAspectRatio = spriteHeight/spriteWidth;

    entity.size = glm::vec2(statueSpriteSize*statueAspectRatio, statueSpriteSize*statueAspectRatio);
    entity.position = glm::vec3(pos.x - (entity.size.x/2.0), pos.y - (entity.size.y/2.0), 0.0);

    isActivated = false;
    memset(playersActivated, 0, sizeof(playersActivated));
    activationTimer = STATUE_ACTIVATION_TIMER_MAX;

    particleSystem = ParticleSystem2D(35, false);
    particleSystem.AddBehavior(new ParticleEmitBurst(35))
                .AddBehavior(new ParticleDecay(0.01))
                .AddBehavior(new ParticleAlphaDecay(0.01))
                .AddBehavior(new ParticlesSpreadOut())
                .AddBehavior(new ParticleColorGradient({1.0, 0.0, 0.0, 1.0}, {1.0, 1.0, 1.0, 0.0}));
}

void OnStatueActivated(Statue& statue) {
    Spritesheet::Animation anim = Spritesheet::Animation(1);
    anim.framerate = 0; // stay on this frame
    statue.spritesheet.SetAnimation(anim);
    Audio::PlayAudio(UseResPath("potp/statue_activate.wav").c_str());
    statue.particleSystem.isActive = true;
}
void OnStatueDeactivated(Statue& statue) {
    Spritesheet::Animation anim = Spritesheet::Animation(0);
    statue.spritesheet.SetAnimation(anim);
    statue.particleSystem.isActive = false;
    statue.particleSystem.Reset();
}

void Statue::Toggle() {
    isActivated = !isActivated;
    if (isActivated) OnStatueActivated(*this);
    else             OnStatueDeactivated(*this);
    activationTimer = STATUE_ACTIVATION_TIMER_MAX;
}

void Statue::ActivateByNinja(Ninja& ninja, u32 playerIdx) {
    this->playersActivated[playerIdx] = true;
    ninja.numStatuesActivated++;
    Toggle();
    glm::vec2 centeredPosNinja = ninja.entity.position + (ninja.entity.size/2.0f);
    glm::vec2 centeredPosStatue = entity.position + (entity.size/2.0f);
    glm::vec2 statueToNinjaDir = glm::normalize(centeredPosNinja - centeredPosStatue);

    std::vector<s32> lookingDirectionSpritesheetIndices = {};

    // TODO: just doing tons of range checks is icky... whats a better way to do this?
    // maybe dot product & an signof(statueToNinjaDir.x) check? ..... would still result in a lot of branching.. is it unavoidable?

    // if we are to the left of the statue's center
    if (isInRange(statueToNinjaDir.x, -1.0f, -0.3f)) {
        // bottom-left
        if (isInRange(statueToNinjaDir.y, 0.5f, 1.0f)) {
            //std::cout << "Ninja bottom left of statue\n";
            lookingDirectionSpritesheetIndices.push_back(0);
        }
        // to the direct left
        else if (isInRange(statueToNinjaDir.y, -0.5f, 0.5f)) {
            //std::cout << "Ninja left of statue\n";
            lookingDirectionSpritesheetIndices.push_back(4);
        }
        // to the top-left
        else if (isInRange(statueToNinjaDir.y, -1.0f, -0.5f)) {
            //std::cout << "Ninja top-left of statue\n";
            lookingDirectionSpritesheetIndices.push_back(7);
        }
    }
    // above/below
    else if (isInRange(statueToNinjaDir.x, -0.3f, 0.3f)) {
        if (isInRange(statueToNinjaDir.y, -1.0f, -0.5f)) {
            //std::cout << "Ninja above statue\n";
            lookingDirectionSpritesheetIndices.push_back(2);
        }
        else if (isInRange(statueToNinjaDir.y, 0.5f, 1.0f)) {
            //std::cout << "Ninja below statue\n";
            lookingDirectionSpritesheetIndices.push_back(3);
        }
    }
    // to the right
    else if (isInRange(statueToNinjaDir.x, 0.3f, 1.0f)) {
        // bottom-right
        if (isInRange(statueToNinjaDir.y, 0.5f, 1.0f)) {
            //std::cout << "Ninja bottom right of statue\n";
            lookingDirectionSpritesheetIndices.push_back(1);
        }
        // to the direct right
        else if (isInRange(statueToNinjaDir.y, -0.5f, 0.5f)) {
            //std::cout << "Ninja right of statue\n";
            lookingDirectionSpritesheetIndices.push_back(5);
        }
        // to the top-right
        else if (isInRange(statueToNinjaDir.y, -1.0f, -0.5f)) {
            //std::cout << "Ninja top-right of statue\n";
            lookingDirectionSpritesheetIndices.push_back(6);
        }
    }

    if (lookingDirectionSpritesheetIndices.size() == 1) {
        s32 lookingDirIndx = lookingDirectionSpritesheetIndices.at(0);
        spritesheet.SetFrame(lookingDirIndx);
    }

}

void Statue::Tick() {
    spritesheet.Tick();
    if (isActivated) {
        activationTimer--;
        if (activationTimer <= 0) {
            Toggle();
        }
    }
    particleSystem.Tick(entity.GetEntityCenter());
}

void CheckNinjaStatueCollisions(Statue* statues, u32 numStatues, Ninja* playerNinjas, u32 numPlayerNinjas) {
    for (s32 s = 0; s < numStatues; s++) {
        for (s32 n = 0; n < numPlayerNinjas; n++) {
            Statue& statue = statues[s];
            Ninja& ninja = playerNinjas[n];
            if (Math::isOverlappingRect2D(statue.entity.position, statue.entity.size, ninja.entity.position, ninja.entity.size)) {
                if (!statue.isActivated && !statue.playersActivated[n]) {
                    statue.ActivateByNinja(ninja, n);
                }
            }
        }
    }
}

// pretty cringe that statues have to know about ninjas to do collision checks here...
// ideally collision checks would be performed on all entities and info about the collision
// would be given to the corresponding objects to handle it themselves
void UpdateStatues(Statue* statues, u32 numStatues, Ninja* playerNinjas, u32 numPlayerNinjas) {
    CheckNinjaStatueCollisions(statues, numStatues, playerNinjas, numPlayerNinjas);
    for (u32 i = 0; i < numStatues; i++) {
        statues[i].Tick();
    }
}

void Statue::Draw() const {
    spritesheet.Draw(Camera::GetMainCamera(), entity.position, entity.size, 0.0, {0.0,0.0,1.0}, entity.color, true);
    particleSystem.Draw();
}