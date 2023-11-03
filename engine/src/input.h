#ifndef TINY_INPUT_H
#define TINY_INPUT_H

//#include "pch.h"
#include "tiny_math.h"
#include "input_keys.h"

// gamepad

// https://www.glfw.org/docs/latest/input_guide.html

struct Gamepad {
    /*! The states of each [gamepad button](@ref gamepad_buttons), `GLFW_PRESS`
     *  or `GLFW_RELEASE`.
     */
    u8 buttons[15];
    /*! The states of each [gamepad axis](@ref gamepad_axes), in the range -1.0
     *  to 1.0 inclusive.
     */
    f32 axes[6];
    const char* name = 0;

    TAPI bool isButtonPressed(s32 glfwButton);
};

// gamepad api:
TAPI bool GetGamepadState(u32 port, Gamepad& gamepad);
TAPI Gamepad FetchCurrentGamepadState(u32 port);
TAPI bool isGamepadPresent(u32 port);
// getting axis (for player idx)
// getting buttons (for player idx)


// keyboard
namespace Keyboard {

TAPI bool GetKeyState(s32 key, s32 keyState);
TAPI bool isKeyPressed(s32 key);
TAPI bool isKeyReleased(s32 key);
TAPI bool isKeyDown(s32 key);
TAPI bool isKeyUp(s32 key);

}
// Mouse
struct GLFWwindow;
void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos);

struct MouseInput {
    f32 lastX = 400;
    f32 lastY = 300;
    f32 yaw = 0.0f;
    f32 pitch = 0.0f;
    f32 sensitivity = 0.1f;
    glm::vec2 mousePos = glm::vec2(0.0f, 0.0f);
    glm::vec3 GetNormalizedLookDir() {
        glm::vec3 direction = glm::vec3(0);
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        return glm::normalize(direction);
    }
    TAPI void UpdateMouse(f64 xpos, f64 ypos);
    // mouse singleton
    static MouseInput& GetMouse() {
        static MouseInput mouseInput;
        return mouseInput;
    }
};

// maps to GLFW_CURSOR_XXXX
enum class CursorMode : s32
{
    NORMAL,
    DISABLED,
    HIDDEN,
};

TAPI void setCursorPosition(f32 x, f32 y);
TAPI CursorMode getCursorMode();
TAPI void setCursorMode(CursorMode cursorMode);


#endif