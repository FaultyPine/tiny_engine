#ifndef STATUE_H
#define STATUE_H

#include "potp_fwd.h"

struct Statue {
    Entity entity;
    bool isActivated;

    Statue(){}
    void Initialize(glm::vec2 pos);
    void Toggle();
    void Draw() const;
};



#endif