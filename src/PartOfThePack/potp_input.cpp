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


bool isPlayerslotAlreadyBound(UserInput& inputs, u32 playerIdx) {
    for (const InputDevice& ID : inputs.controllers) {
        if (ID.port == playerIdx) return true;
    }
    return false;
}

bool AttemptBindInputDeviceToPort(s32 portToBind, UserInput& inputs) {
    // NOTE: Not going to account for the situation where two players
    // push start on the same frame
    bool isSuccessfulBind = false;
    // iterating *ports* which represent the 4 possible gamepad/keyboard input devices
    for (s32 port = 0; port < MAX_NUM_PLAYERS; port++) {
        bool isStartPressed = false;
        ControllerType controllerType = ControllerType::NO_CONTROLLER;
        if (isGamepadPresent(port)) {
            Gamepad pad;
            GetGamepadState(port, pad);
            isStartPressed = pad.isButtonPressed(GetGamepadBinding(ButtonValues::START));
            controllerType = ControllerType::CONTROLLER;
        }
        else {
            isStartPressed = Keyboard::isKeyPressed(GetKeyboardBinding(ButtonValues::START, port));
            controllerType = ControllerType::KEYBOARD;
        }
        bool shouldBind = isStartPressed && inputs.controllers[port].type == ControllerType::NO_CONTROLLER;
        if (shouldBind) {
            const char* inputType = controllerType == ControllerType::CONTROLLER ? "Controller" : "Keyboard";
            std::cout << "Bound " << inputType << " in 'port' " << port << " to playerIdx " << portToBind << "\n";
            inputs.controllers[port].type = controllerType;
            inputs.controllers[port].port = portToBind;
            isSuccessfulBind = true;
        }
    }
    return isSuccessfulBind;
}



void UserInput::SetupControllersTick(u32& numPlayers, bool isReady[MAX_NUM_PLAYERS]) {
    // iterating *player slots*, which represents player 1-4
    for (s32 playerIdx = 0; playerIdx < MAX_NUM_PLAYERS; playerIdx++) {
        if (isPlayerslotAlreadyBound(*this, playerIdx)) {
            if (Keyboard::isKeyPressed(GetKeyboardBinding(ButtonValues::START, playerIdx)) && !isReady[playerIdx]) {
                std::cout << "Player " << playerIdx << " is ready.\n";
                isReady[playerIdx] = true;
            }
        }
        else {
            // already bound slot, poll for if they're ready
            if (AttemptBindInputDeviceToPort(playerIdx, *this)) {
                numPlayers++;
            }
        }
    }
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
