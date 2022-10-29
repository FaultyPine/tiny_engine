#ifndef POTP_MAIN_H
#define POTP_MAIN_H

#include "tiny_engine/input.h"
#include "statue.h"
#include "ninja.h"
#include "tiny_engine/tiny_text.h"

namespace Potp {

void MainInit();
void MainUpdate();
void Terminate();

}

enum PotpScene {
    NO_SCENE,
    CONTROLLER_SETUP,
    ASSASSIN,
    TITLE,
};


void ChangeScene(PotpScene newScene, GameState& gs);

struct GameState {
    PotpScene scene = PotpScene::NO_SCENE;
    
    // ----- Title screen -----
    Sprite titleScreenSplash;
    GLTtext* titleScreenText;

    // ----- controller setup -----
    bool isReady[MAX_NUM_PLAYERS];
    #define ASSASSIN_ALL_READY_COUNTDOWN_FRAMES ((60)*5)
    u32 allReadyCountdown = ASSASSIN_ALL_READY_COUNTDOWN_FRAMES;
    u32 numPlayers = 0;
    Sprite keyboardSprite;
    Sprite controllerSprite;
    Sprite blankControllerSprite;
    Sprite howToPlayBackgroundSprite;
    GLTtext* playerTexts[MAX_NUM_PLAYERS];
    GLTtext* instructionsText;
    GLTtext* countdownText;


    // ----- ingame -----

    /// storing initial seed for possible replay functionality later
    u64 initialRandomSeed = 0;

    Sprite background;

    #define NUM_STATUES 5
    Statue statues[NUM_STATUES];

    #define MAX_NUM_AI_NINJAS 80
    Ninja aiNinjas[MAX_NUM_AI_NINJAS];
    Ninja playerNinjas[MAX_NUM_PLAYERS];
    s32 winningPlayer = -1;
    GLTtext* playerWonText;
    #define PLAYER_WON_MAX_TIMER (60*5)
    u32 playerWonTimer = PLAYER_WON_MAX_TIMER;
    GLTtext* playerScoresText;
};

#endif