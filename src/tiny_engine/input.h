#ifndef TINY_INPUT_H
#define TINY_INPUT_H

#include "pch.h"


extern GLFWwindow* glob_glfw_window;

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

    bool isButtonPressed(s32 glfwButton);
};

// gamepad api:
bool GetGamepadState(u32 port, Gamepad& gamepad);
Gamepad FetchCurrentGamepadState(u32 port);
bool isGamepadPresent(u32 port);
// getting axis (for player idx)
// getting buttons (for player idx)


// keyboard
namespace Keyboard {

bool GetKeyState(s32 key, s32 keyState);
bool isKeyPressed(s32 key);
bool isKeyReleased(s32 key);
bool isKeyDown(s32 key);
bool isKeyUp(s32 key);

}
// Mouse

void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos);

struct MouseInput {
    f32 lastX = 400;
    f32 lastY = 300;
    f32 yaw = 0.0f;
    f32 pitch = 0.0f;
    f32 sensitivity = 0.1f;
    glm::vec2 mousePos = glm::vec2(0.0f, 0.0f);
    glm::vec3 GetNormalizedLookDir() {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        return glm::normalize(direction);
    }
    // mouse singleton
    static MouseInput& GetMouse() {
        static MouseInput mouseInput;
        return mouseInput;
    }
};




#endif