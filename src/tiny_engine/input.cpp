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


}

// mouse callbacks
void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
    MouseInput& mouseInput = MouseInput::GetMouse();
    static bool firstMouse = true;
    if (firstMouse)
    {
        mouseInput.lastX = xpos;
        mouseInput.lastY = ypos;
        firstMouse = false;
    }
  
    f32 xoffset = xpos - mouseInput.lastX;
    f32 yoffset = mouseInput.lastY - ypos; 
    mouseInput.lastX = xpos;
    mouseInput.lastY = ypos;

    f32 sensitivity = mouseInput.sensitivity;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    mouseInput.yaw   += xoffset;
    mouseInput.pitch += yoffset;

    // clamp looking up/down
    if(mouseInput.pitch > 89.0f)
        mouseInput.pitch = 89.0f;
    if(mouseInput.pitch < -89.0f)
        mouseInput.pitch = -89.0f;
}
void cursor_position_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
    MouseInput::GetMouse().mousePos = glm::vec2(xpos, ypos);
}