#include "pch.h"
#include "tetris_main.h"

#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/input.h"





// called once when the game boots
void tetris_init() {

}
// called every frame
void tetris_tick() {
    if (Keyboard::isKeyDown(GLFW_KEY_ESCAPE)) {
        CloseGameWindow();
    }
}
// called when game shuts down
void tetris_terminate() {

}