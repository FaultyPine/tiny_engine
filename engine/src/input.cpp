//#include "pch.h"
#include "input.h"
#include "tiny_defines.h"
#include "tiny_engine.h"
#include "tiny_log.h"
#include "tiny_imgui.h"
#include "GLFW/glfw3.h"
#include <unordered_map>
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
    return glfwGetKey(GetMainGLFWWindow(), key) == keyState;
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

MouseInput& MouseInput::GetMouse()
{
    static MouseInput mouseInput;
    return mouseInput;
}

// mouse callbacks
void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
    MouseInput::GetMouse().UpdateMouse((f32)xpos, (f32)ypos);
}

void MouseInput::UpdateMouse(f32 xpos, f32 ypos) {
    static bool firstMouse = true;
    if (firstMouse)
    {
        lastMousePos = glm::vec2(xpos, ypos);
        firstMouse = false;
    }
  
    lastMousePos = mousePos;
    f32 xoffset = xpos - lastMousePos.x;
    f32 yoffset = lastMousePos.y - ypos; 

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

bool MouseInput::isMouseButtonDown(s32 button)
{
    return glfwGetMouseButton(GetMainGLFWWindow(), button) == GLFW_PRESS;
}
bool MouseInput::isMouseButtonUp(s32 button)
{
    return glfwGetMouseButton(GetMainGLFWWindow(), button) == GLFW_RELEASE;
}

static CursorMode glfwToTinyCursorMode(s32 glfwCursorMode)
{
    switch (glfwCursorMode)
    {
        case GLFW_CURSOR_NORMAL: 
        {
            return CursorMode::NORMAL;
        } break;
        case GLFW_CURSOR_DISABLED: 
        {
            return CursorMode::DISABLED;
        } break;
        case GLFW_CURSOR_HIDDEN: 
        {
            return CursorMode::HIDDEN;
        } break;
        default:
        {
            LOG_ERROR("Invalid glfw cursor mode passed to glfwToTinyCursorMode");
            return CursorMode::NORMAL;
        } break;
    }
}
static s32 tinyToGlfwCursorMode(CursorMode tinyCursorMode)
{
    switch (tinyCursorMode)
    {
        case CursorMode::NORMAL: 
        {
            return GLFW_CURSOR_NORMAL;
        } break;
        case CursorMode::DISABLED: 
        {
            return GLFW_CURSOR_DISABLED;
        } break;
        case CursorMode::HIDDEN: 
        {
            return GLFW_CURSOR_HIDDEN;
        } break;
        default:
        {
            LOG_ERROR("Invalid CursorMode passed to tinyToGlfwCursorMode");
            return GLFW_CURSOR_NORMAL;
        } break;
    }
}

void setCursorPosition(f32 x, f32 y)
{
    glfwSetCursorPos(GetMainGLFWWindow(), x, y);
}
void getCursorPosition(f64& x, f64& y)
{
    glfwGetCursorPos(GetMainGLFWWindow(), &x, &y);
}
CursorMode getCursorMode()
{
    s32 glfwCursorMode = glfwGetInputMode(GetMainGLFWWindow(), GLFW_CURSOR);
    CursorMode result = glfwToTinyCursorMode(glfwCursorMode);
    return result;
}
void setCursorMode(CursorMode cursorMode)
{
    s32 glfwCursorMode = tinyToGlfwCursorMode(cursorMode);
    glfwSetInputMode(GetMainGLFWWindow(), GLFW_CURSOR, glfwCursorMode);
}