#ifndef NINJA_H
#define NINJA_H

#include "potp_fwd.h"

#define NINJA_MAX_SMOKE_GRENADES 2
#define NINJA_SPRITE_SIZE 50
struct Ninja {
    Entity entity;
    u32 numSmokeGrenadesLeft = NINJA_MAX_SMOKE_GRENADES;
    f32 ninjaSpeed = 50.0;
    
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