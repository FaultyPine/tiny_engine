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

// Live++ hot-reloading
//#define LIVEPP_ACTIVE

#ifdef LIVEPP_ACTIVE
#include <Windows.h>
// https://liveplusplus.tech/docs/documentation.html
#include "../LPP/LivePP/API/LPP_API_x64_CPP.h"
#include "tiny_engine/tiny_fs.h"
#include "tiny_engine/input.h"
void Recompile() {
    system("build.bat norun");
}
lpp::LppSynchronizedAgent lppAgent;
void LivePPInit() {
    lppAgent = lpp::LppCreateSynchronizedAgent(L"LPP/LivePP");
    lppAgent.EnableModule(lpp::LppGetCurrentModulePath(), lpp::LPP_MODULES_OPTION_ALL_IMPORT_MODULES);
}
void LivePPTick() {
    if (Keyboard::isKeyPressed(GLFW_KEY_R)) {
        if (Keyboard::isKeyDown(GLFW_KEY_LEFT_ALT)) {
            std::cout << "Restart\n";
            Recompile();
            lppAgent.Restart(lpp::LPP_RESTART_BEHAVIOUR_DEFAULT_EXIT, 0u);
        }
        else {
            std::cout << "Reload\n";
            lppAgent.ScheduleReload();
        }
    }
    // listen to hot-reload and hot-restart requests
    if (lppAgent.WantsReload()) {
        lppAgent.CompileAndReloadChanges(lpp::LPP_RELOAD_BEHAVIOUR_WAIT_UNTIL_CHANGES_ARE_APPLIED);
    }
    if (lppAgent.WantsRestart()) {
        endGame();
        Recompile();
        lppAgent.Restart(lpp::LPP_RESTART_BEHAVIOUR_DEFAULT_EXIT, 0u);
    }
}
void LivePPTerminate() {
    lpp::LppDestroySynchronizedAgent(&lppAgent);
}
#endif

int main(int argc, char *argv[]) {

    preLoopInit();
#ifdef LIVEPP_ACTIVE
LivePPInit();
#endif

    while(!ShouldCloseWindow()) {
#ifdef LIVEPP_ACTIVE
LivePPTick();
#endif
        gameTick();
    }
    endGame();

#ifdef LIVEPP_ACTIVE
LivePPTerminate();
#endif

    return 0;
}