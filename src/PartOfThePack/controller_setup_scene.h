#ifndef CONTROLLER_SETUP_H
#define CONTROLLER_SETUP_H


#include "potp_fwd.h"
#include "potp_main.h"

void ControllerSetupSceneInit(GameState& gs);
void ControllerSetupSceneTick(GameState& gs, UserInput& inputs);
void ControllerSetupSceneDraw(const GameState& gs, const UserInput& inputs);
void ControllerSetupSceneEnd(GameState& gs, UserInput& inputs);


#endif