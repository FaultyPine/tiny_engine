// pch's have to be the first thing. If this errors, rebuild again.
#include "tiny_engine/pch.h"

// external implementation files
#include <glad/glad.c>
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_engine/stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "tiny_engine/tiny_engine.h"

// game includes
//#include "PartOfThePack/potp_main.h"
#include "testbed/testbed_main.h"

void preLoopInit() {
    InitGame(800, 600, 4, 3, "Tiny Engine"); 

    //Potp::MainInit();
    testbed_init();
}

void gameTick() {
    //Potp::MainUpdate();
    testbed_tick();
}

void endGame() {
    //Potp::Terminate();
    testbed_terminate();
    TerminateGame();
}

int main(int argc, char *argv[]) {
    preLoopInit();
    while(!ShouldCloseWindow()) {
        gameTick();
    }
    endGame();
    return 0;
}