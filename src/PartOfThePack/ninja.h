#ifndef NINJA_H
#define NINJA_H

#include "potp_fwd.h"
#include "tiny_engine/spritesheet.h"

#define NINJA_MAX_SMOKE_GRENADES 2
#define NINJA_SPRITE_SIZE 50

// TODO: (maybe?) abstract out a state machine and use that to manage ninja logic

enum NinjaAnimStates {
    IDLE,
    WALK,
    PUNCH,
    SMOKE,
    DEAD
};

struct Ninja {
    Entity entity;
    f32 ninjaSpeed = 50.0;
    bool isDead = false;
    bool isSpriteFlipped = false;
    Spritesheet spritesheet;

    // Player
    u32 numSmokeGrenadesLeft = NINJA_MAX_SMOKE_GRENADES;
    
    // AI
    u32 positionIdleFrames = 0; // number of frames we've idled for
    u32 positionIdleFramesMax = 60; // number of frames TO idle for
    glm::vec2 aiDesiredPos;
};

// doing init/updating in here so we can jump around the ninjas array to look at others
// for stuff like ai calculations
void InitializeNinjas(Ninja* aiNinjas, u32 numAINinjas, Ninja* playerNinjas, u32 numPlayerNinjas);
void UpdateNinjas(UserInput inputs, Ninja* aiNinjas, u32 numAINinjas, Ninja* playerNinjas, u32 numPlayerNinjas);
void DrawNinjas(const Ninja* aiNinjas, u32 numAINinjas, const Ninja* playerNinjas, u32 numPlayerNinjas);

#endif