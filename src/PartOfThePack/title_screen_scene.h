#ifndef TITLE_SCREEN_SCENE_H
#define TITLE_SCREEN_SCENE_H

#include "potp_fwd.h"
#include "potp_main.h"


void TitleScreenSceneInit(GameState& gs);
void TitleScreenSceneTick(GameState& gs, UserInput& inputs);
void TitleScreenSceneDraw(const GameState& gs, const UserInput& inputs);
void TitleScreenSceneEnd(GameState& gs);



#endif