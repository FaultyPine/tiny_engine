#ifndef ASSASSIN_SCENE_H
#define ASSASSIN_SCENE_H

#include "potp_fwd.h"
#include "potp_main.h"

void AssassinSceneInit(GameState& gs);
void AssassinSceneTick(GameState& gs, UserInput& inputs);
void AssassinSceneDraw(const GameState& gs, const UserInput& inputs);
void AssassinSceneEnd(GameState& gs, UserInput& inputs);

#endif