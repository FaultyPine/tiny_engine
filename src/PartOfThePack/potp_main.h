#ifndef POTP_MAIN_H
#define POTP_MAIN_H

#include "tiny_engine/input.h"
#include "statue.h"
#include "ninja.h"

namespace Potp {

void MainInit();
void MainUpdate(UserInput inputs);
void MainDraw();

}

struct GameState {
    /// storing initial seed for possible replay functionality later
    u64 initialRandomSeed;

    Sprite background;

    #define NUM_STATUES 5
    Statue statues[NUM_STATUES];

    #define MAX_NUM_AI_NINJAS 35
    Ninja aiNinjas[MAX_NUM_AI_NINJAS];
    Ninja playerNinjas[MAX_NUM_PLAYERS];
};

#endif