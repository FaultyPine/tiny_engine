#ifndef STATUE_H
#define STATUE_H

#include "potp_fwd.h"
#include "tiny_engine/spritesheet.h"

struct Ninja;

struct Statue {
    Entity entity;
    bool isActivated;
    bool playersActivated[MAX_NUM_PLAYERS]; // track which players activated this statue
    Spritesheet spritesheet;
    #define STATUE_ACTIVATION_TIMER_MAX (60*4)
    s32 activationTimer = STATUE_ACTIVATION_TIMER_MAX;

    Statue(){}
    void Initialize(glm::vec2 pos);
    void Toggle();
    void ActivateByNinja(Ninja& ninja, u32 playerIdx);
    void Tick();
    void Draw() const;
};

void UpdateStatues(Statue* statues, u32 numStatues, Ninja* playerNinjas, u32 numPlayerNinjas);

#endif