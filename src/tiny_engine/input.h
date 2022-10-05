#ifndef TINY_INPUT_H
#define TINY_INPUT_H

#include "pch.h"


void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos);
void cursor_position_callback(GLFWwindow* window, f64 xpos, f64 ypos);

void CloseGameWindow();
extern GLFWwindow* glob_glfw_window;



inline int GetKeyState(s32 key, s32 keyState) {
    return glfwGetKey(glob_glfw_window, key) == keyState;
}

// mouse
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
};

struct UserInput {
    enum ButtonValues {
        NONE = 0,
        UP = 1 << 0,
        DOWN = 1 << 1,
        LEFT = 1 << 2,
        RIGHT = 1 << 3,
        ACTION1 = 1 << 4, // punch
        ACTION2 = 1 << 5, // smoke
    };
    #define MAX_NUM_PLAYERS 4
    // 1 bitfield of buttons pressed for each player
    u32 buttons[MAX_NUM_PLAYERS];

    u32 getButtons(u32 playerIdx) const { return buttons[playerIdx]; }
    bool isUp(u32 playerIdx) const {
        return getButtons(playerIdx) & ButtonValues::UP;
    }
    bool isDown(u32 playerIdx) const {
        return getButtons(playerIdx) & ButtonValues::DOWN;
    }
    bool isLeft(u32 playerIdx) const {
        return getButtons(playerIdx) & ButtonValues::LEFT;
    }
    bool isRight(u32 playerIdx) const {
        return getButtons(playerIdx) & ButtonValues::RIGHT;
    }
    bool isAction1(u32 playerIdx) const {
        return getButtons(playerIdx) & ButtonValues::ACTION1;
    }
    bool isAction2(u32 playerIdx) const {
        return getButtons(playerIdx) & ButtonValues::ACTION2;
    }

    // static input getters
    static MouseInput& GetMouse() {
        static MouseInput mouseInput;
        return mouseInput;
    }
    static bool GetKeyUp(s32 key) {
        return GetKeyState(key, GLFW_RELEASE);
    }
    static bool GetKeyDown(s32 key) {
        return GetKeyState(key, GLFW_PRESS);
    }
    static bool GetKeyHold(s32 key) {
        return GetKeyState(key, GLFW_REPEAT);
    }
};

UserInput GetUserInput();


#endif