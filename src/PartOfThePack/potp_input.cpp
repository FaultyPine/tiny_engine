#include "potp_input.h"

#include "tiny_engine/input.h"
#include "tiny_engine/tiny_engine.h"


// buttons: (u/d/l/r/action1/action2/start)


UserInput::UserInput() {
    ASSERT(sizeof(this->buttons[0])*8 >= ButtonValues::NUM_BUTTONS); // make sure our buttons can fit in a 32bit bitfield
    for (s32 i = 0; i < MAX_NUM_PLAYERS; i++) {
        this->controllers[i].type = ControllerType::NO_CONTROLLER;
    }
}


static ButtonValues inorderButtonMappings[ButtonValues::NUM_BUTTONS] = {
    ButtonValues::UP,
    ButtonValues::DOWN,
    ButtonValues::LEFT,
    ButtonValues::RIGHT,
    ButtonValues::ACTION1,
    ButtonValues::ACTION2,
    ButtonValues::START,
};
static s32 keyboardInputMap[MAX_NUM_PLAYERS][ButtonValues::NUM_BUTTONS] = {
    // (u/d/l/r/action1/action2/start)
    {GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_TAB},
    {GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_RIGHT_CONTROL, GLFW_KEY_RIGHT_SHIFT, GLFW_KEY_ENTER},
    {-1,-1,-1,-1,-1,-1}, // -1 is GLFW_KEY_UNKNOWN
    {-1,-1,-1,-1,-1,-1}, // max two people on one keyboard
};
static s32 gamepadInputMap[ButtonValues::NUM_BUTTONS] = {
    GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_GAMEPAD_BUTTON_A, GLFW_GAMEPAD_BUTTON_B, GLFW_GAMEPAD_BUTTON_START,
};

s32 GetGamepadBinding(ButtonValues button) {
    // since the ButtonValues are a bitfield, counting the leading zeroes
    // is like getting the index of the enum in the ButtonValues enum
    return gamepadInputMap[countLeadingZeroes(button)];
}
s32 GetKeyboardBinding(ButtonValues button, u32 playerIdx) {
    return keyboardInputMap[playerIdx][countLeadingZeroes(button)];
}


void UserInput::UpdateUserInput(UserInput& input) {
    // when we update user input, first thing we do is update prev buttons and blank out current inputs
    memcpy(&input.prevButtons, &input.buttons, sizeof(input.buttons));
    for (u32& buttons : input.buttons) buttons = 0;

    for (u32 playerIdx = 0; playerIdx < MAX_NUM_PLAYERS; playerIdx++) {
        u32& playerButtons = input.buttons[playerIdx];
        u32& prevPlayerButtons = input.prevButtons[playerIdx];
        for (u32 buttonIdx = 0; buttonIdx < ButtonValues::NUM_BUTTONS; buttonIdx++) {
            bool isButtonPressed = false;
            ControllerType controllerType = input.controllers[playerIdx].type;

            if (controllerType == ControllerType::KEYBOARD) {
                s32 inputKey = keyboardInputMap[playerIdx][buttonIdx];
                isButtonPressed = Keyboard::GetKeyState(inputKey, GLFW_PRESS);
            }
            else if (controllerType == ControllerType::CONTROLLER) {
                Gamepad pad;
                GetGamepadState(playerIdx, pad);
                isButtonPressed = pad.buttons[gamepadInputMap[buttonIdx]] == GLFW_PRESS;
            }

            if (isButtonPressed) {
                ButtonValues button = inorderButtonMappings[buttonIdx];
                playerButtons |= button;
            }
        }
    }
    
    if (Keyboard::GetKeyState(GLFW_KEY_ESCAPE, GLFW_PRESS)) {
        CloseGameWindow();
    }
}
