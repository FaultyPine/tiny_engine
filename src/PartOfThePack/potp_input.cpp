#include "potp_input.h"

#include "tiny_engine/input.h"
#include "tiny_engine/tiny_engine.h"

// 6 = number of buttons (u/d/l/r/action1/action2)
#define NUM_BUTTONS 6

static UserInput::ButtonValues inorderButtonMappings[NUM_BUTTONS] = {
    UserInput::ButtonValues::UP,
    UserInput::ButtonValues::DOWN,
    UserInput::ButtonValues::LEFT,
    UserInput::ButtonValues::RIGHT,
    UserInput::ButtonValues::ACTION1,
    UserInput::ButtonValues::ACTION2
};
static s32 inputMap[MAX_NUM_PLAYERS][NUM_BUTTONS] = {
    {GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT},
    {GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_RIGHT_CONTROL, GLFW_KEY_RIGHT_SHIFT},
    {-1,-1,-1,-1,-1,-1}, // TODO
    {-1,-1,-1,-1,-1,-1}, // -1 is GLFW_KEY_UNKNOWN
};

void UserInput::UpdateUserInput(UserInput& input) {
    // when we update user input, first thing we do is update prev buttons and blank out current inputs
    memcpy(&input.prevButtons, &input.buttons, sizeof(input.buttons));
    for (u32& buttons : input.buttons) buttons = 0;

    if (GetKeyState(GLFW_KEY_ESCAPE, GLFW_PRESS)) {
        CloseGameWindow();
    }

    for (u32 playerIdx = 0; playerIdx < MAX_NUM_PLAYERS; playerIdx++) {
        u32& playerButtons = input.buttons[playerIdx];
        u32& prevPlayerButtons = input.prevButtons[playerIdx];
        for (u32 buttonIdx = 0; buttonIdx < NUM_BUTTONS; buttonIdx++) {
            s32 inputKey = inputMap[playerIdx][buttonIdx];
            if (GetKeyState(inputKey, GLFW_PRESS)) {
                ButtonValues button = inorderButtonMappings[buttonIdx];
                playerButtons |= button;
            }
        }
    }
    
}
