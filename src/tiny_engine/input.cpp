#include "input.h"

// gamepad

bool GetGamepadState(u32 port, Gamepad& gamepad) {
    GLFWgamepadstate state;
    glfwGetGamepadState(port, &state);
    memcpy(gamepad.buttons, state.buttons, sizeof(gamepad.buttons));
    memcpy(gamepad.axes, state.axes, sizeof(gamepad.axes));
    bool isPresent = isGamepadPresent(port);
    if (isPresent) {
        gamepad.name = glfwGetGamepadName(port);
    }
    return isPresent;
}
Gamepad FetchCurrentGamepadState(u32 port) {
    Gamepad pad;
    GetGamepadState(port, pad);
    return pad;
}


bool isGamepadPresent(u32 playerIdx) {
    return glfwJoystickPresent(playerIdx);
}

static std::unordered_map<s32, bool> gamepadButtonStates;
bool Gamepad::isButtonPressed(s32 glfwButton) {
    // initialize to false so next computation works on first press
    if (!gamepadButtonStates.count(glfwButton)) {
        gamepadButtonStates[glfwButton] = false;
    }

    bool isPressed = buttons[glfwButton] == GLFW_PRESS;
    bool isJustPressed = isPressed && !gamepadButtonStates.at(glfwButton);
    gamepadButtonStates[glfwButton] = isPressed;
    return isJustPressed;
}

// keyboard
namespace Keyboard {
static std::unordered_map<s32, bool> keyboardButtonStates = {};

bool GetKeyState(s32 key, s32 keyState) {
    return glfwGetKey(glob_glfw_window, key) == keyState;
}
// returns true on the frame the specified key is pressed
bool isKeyPressed(s32 key) {
    // initialize to false so next computation works on first press
    if (!keyboardButtonStates.count(key)) {
        keyboardButtonStates[key] = false;
    }
    bool isPressed = GetKeyState(key, GLFW_PRESS);
    bool isJustPressed = isPressed && !keyboardButtonStates.at(key);
    keyboardButtonStates[key] = isPressed;
    return isJustPressed;
}
// returns true on the frame the specified key is released
bool isKeyReleased(s32 key) {
    // initialize to false so next computation works on first press
    if (!keyboardButtonStates.count(key)) {
        keyboardButtonStates[key] = false;
    }
    bool isPressed = GetKeyState(key, GLFW_PRESS);
    bool isJustReleased = !isPressed && keyboardButtonStates.at(key);
    keyboardButtonStates[key] = isPressed;
    return isJustReleased;
}
bool isKeyDown(s32 key) {
    return GetKeyState(key, GLFW_PRESS);
}
bool isKeyUp(s32 key) {
    return !isKeyDown(key);
}


}

// mouse callbacks
void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
    MouseInput::GetMouse().UpdateMouse(xpos, ypos);
}

void MouseInput::UpdateMouse(f64 xpos, f64 ypos) {
    static bool firstMouse = true;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
  
    f32 xoffset = xpos - lastX;
    f32 yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    f32 sensitivity = this->sensitivity;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // clamp looking up/down
    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    mousePos = glm::vec2(xpos, ypos);
}