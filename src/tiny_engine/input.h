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
        ACTION = 1 << 4,
    };
    u32 buttons;

    bool isUp() const {
        return buttons & ButtonValues::UP;
    }
    bool isDown() const {
        return buttons & ButtonValues::DOWN;
    }
    bool isLeft() const {
        return buttons & ButtonValues::LEFT;
    }
    bool isRight() const {
        return buttons & ButtonValues::RIGHT;
    }
    bool isAction() const {
        return buttons & ButtonValues::ACTION;
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