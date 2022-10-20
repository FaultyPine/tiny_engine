#ifndef POTP_MAIN_H
#define POTP_MAIN_H

#include "tiny_engine/input.h"
#include "statue.h"
#include "ninja.h"

namespace Potp {

void MainInit();
void MainUpdate();

}

enum PotpScene {
    NO_SCENE,
    CONTROLLER_SETUP,
    ASSASSIN,
};

struct GameState {
    PotpScene scene = PotpScene::NO_SCENE;
    
    // ----- controller setup -----
    bool isReady[MAX_NUM_PLAYERS];
    u32 numPlayers = 0;
    Sprite keyboardSprite;
    Sprite controllerSprite;
    Sprite playerTextSprite;
    Sprite oneSprite, twoSprite, threeSprite, fourSprite;


    // ----- ingame -----

    /// storing initial seed for possible replay functionality later
    u64 initialRandomSeed;

    Sprite background;

    #define NUM_STATUES 5
    Statue statues[NUM_STATUES];

    #define MAX_NUM_AI_NINJAS 45
    Ninja aiNinjas[MAX_NUM_AI_NINJAS];
    Ninja playerNinjas[MAX_NUM_PLAYERS];
};

#endif