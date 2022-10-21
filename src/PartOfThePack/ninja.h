#ifndef NINJA_H
#define NINJA_H

#include "potp_fwd.h"
#include "potp_input.h"
#include "tiny_engine/spritesheet.h"


// TODO: (maybe?) abstract out a state machine and use that to manage ninja logic

enum NinjaAnimStates {
    IDLE,
    WALK,
    PUNCH,
    DEAD
};

struct Ninja {
    #define NINJA_SPRITE_SIZE 50
    Entity entity;
    f32 ninjaSpeed = 50.0;
    bool isDead = false;
    bool isSpriteFlipped = false;
    bool isPunching = false;
    Spritesheet spritesheet;

    // Player
    struct SmokeGrenade {
        #define NINJA_MAX_SMOKE_GRENADES 2
        #define NINJA_SMOKE_GRENADE_LIFETIME (60*6)
        #define NINJA_SMOKE_GRENADE_SPRITE_SIZE 240
        #define NINJA_SMOKE_GRENADE_COOLDOWN (60*3)
        u32 numLeft = NINJA_MAX_SMOKE_GRENADES;
        Spritesheet sprite;
        u32 cooldown = NINJA_SMOKE_GRENADE_COOLDOWN;
        u32 life = 0; 
        glm::vec2 size = glm::vec2(NINJA_SMOKE_GRENADE_SPRITE_SIZE, NINJA_SMOKE_GRENADE_SPRITE_SIZE);
        glm::vec2 pos;
    };
    SmokeGrenade smokeGrenade;
    
    // AI
    u32 positionIdleFrames = 0; // number of frames we've idled for
    u32 positionIdleFramesMax = 60; // number of frames TO idle for
    glm::vec2 aiDesiredPos;

    CollisionBox punchHitbox;
    void PunchedOtherNinja(Ninja& punchedNinja);
    void Die();
};

// doing init/updating in here so we can jump around the ninjas array to look at others
// for stuff like ai calculations
void InitializeNinjas(Ninja* aiNinjas, u32 numAINinjas, Ninja* playerNinjas, u32 numPlayerNinjas);
void UpdateNinjas(const UserInput& inputs, Ninja* aiNinjas, u32 numAINinjas, Ninja* playerNinjas, u32 numPlayerNinjas);
void DrawNinjas(const Ninja* aiNinjas, u32 numAINinjas, const Ninja* playerNinjas, u32 numPlayerNinjas);


#endif