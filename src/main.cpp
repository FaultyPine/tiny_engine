// pch's have to be the first thing. If this errors, rebuild again.
#include "pch.h"

// external implementation files
#include <glad/glad.c>
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_engine/external/stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "tiny_engine/tiny_engine.h"

// game includes
#define TESTBED3D 0
#define QUADTREE_TEST 0
#define CC_TEST 1

#if TESTBED3D
#include "testbed/testbed_main.h"
#elif QUADTREE_TEST
#include "quadtree_test/game_main.h" 
#elif CC_TEST
#include "cc/cc_main.h"
#endif

void preLoopInit() {
    bool is3D = false;
    InitGame(1920, 1080, 16, 9, "Tiny Engine", is3D); 
    
    //Potp::MainInit();
    #if TESTBED3D
    testbed_init();
    #endif

    #if QUADTREE_TEST
    game_init();
    #endif

    #if CC_TEST
    cc_init();
    #endif
}

void gameTick() {
    //Potp::MainUpdate();
    #if TESTBED3D
    testbed_tick();
    #endif

    #if QUADTREE_TEST
    game_tick();
    #endif

    #if CC_TEST
    cc_tick();
    #endif
}

void endGame() {
    //Potp::Terminate();
    #if TESTBED3D
    testbed_terminate();
    #endif

    #if QUADTREE_TEST
    game_terminate();
    #endif

    #if CC_TEST
    cc_terminate();
    #endif

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
// this will build and run a new executable whose name is
// the name of the app and some number appended to the end
// I.E. TinyEngine0.exe, then TinyEngine1.exe, etc...
void RestartProcedure() {
    system("build.bat livepp");
}
void Recompile() {
    system("build.bat norun");
}
void CleanDebugExecutables() {
    system("build.bat livepp clean");
}
lpp::LppSynchronizedAgent lppAgent;
void LivePPInit() {
    lppAgent = lpp::LppCreateSynchronizedAgent(L"LPP/LivePP");
    lppAgent.EnableModule(lpp::LppGetCurrentModulePath(), lpp::LPP_MODULES_OPTION_ALL_IMPORT_MODULES);
}
static bool WantsRestartManual = false;
void LivePPTick() {
    if (Keyboard::isKeyPressed(GLFW_KEY_R)) {
        if (Keyboard::isKeyDown(GLFW_KEY_LEFT_ALT)) {
            WantsRestartManual = true;
        }
        else {
            lppAgent.ScheduleReload(); 
        }
    }
    // listen to hot-reload and hot-restart requests
    if (lppAgent.WantsReload()) {
        std::cout << "Reload\n";
        Recompile();
        lppAgent.CompileAndReloadChanges(lpp::LPP_RELOAD_BEHAVIOUR_WAIT_UNTIL_CHANGES_ARE_APPLIED);
    }
    if (lppAgent.WantsRestart() || WantsRestartManual) {
        WantsRestartManual = false;
        std::cout << "Restart\n";
        endGame();
        RestartProcedure();
        // lpp restart didn't work great for some reason
        // using my own restart logic that starts an entirely new exectuable and just closes the current one
        //lppAgent.Restart(lpp::LPP_RESTART_BEHAVIOUR_DEFAULT_EXIT, 0u);
        exit(1);
    }
}
void LivePPTerminate() {
    lpp::LppDestroySynchronizedAgent(&lppAgent);
    CleanDebugExecutables();
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