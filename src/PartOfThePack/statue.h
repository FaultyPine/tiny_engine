#ifndef STATUE_H
#define STATUE_H

#include "potp_fwd.h"
#include "tiny_engine/spritesheet.h"

struct Ninja;

struct Statue {
    Entity entity;
    bool isActivated;
    Spritesheet spritesheet;
    #define STATUE_ACTIVATION_TIMER_MAX (60*4)
    s32 activationTimer = STATUE_ACTIVATION_TIMER_MAX;

    Statue(){}
    void Initialize(glm::vec2 pos);
    void Toggle();
    void ActivateByNinja(Ninja* ninja);
    void Tick();
    void Draw() const;
};

void UpdateStatues(Statue* statues, u32 numStatues, Ninja* playerNinjas, u32 numPlayerNinjas);

#endif