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
        UP = 2,
        DOWN = 4,
        ACTION = 8,
    };
    glm::vec2 stick;
    u32 buttons;

    static MouseInput& GetMouse() {
        static MouseInput mouseInput;
        return mouseInput;
    }
    
    bool GetKeyUp(s32 key) const {
        return GetKeyState(key, GLFW_RELEASE);
    }
    bool GetKeyDown(s32 key) const {
        return GetKeyState(key, GLFW_PRESS);
    }
    bool GetKeyHold(s32 key) const {
        return GetKeyState(key, GLFW_REPEAT);
    }

    // TODO: deprecate this (see msg in GetUserInput)
    bool isForward() const {
        return stick.y > 0.0;
    }
    bool isBackward() const {
        return stick.y < 0.0;
    }
    bool isRight() const {
        return stick.x > 0.0;
    }
    bool isLeft() const {
        return stick.x < 0.0;
    }
    bool isUp() const {
        return buttons & ButtonValues::UP;
    }
    bool isDown() const {
        return buttons & ButtonValues::DOWN;
    }
    bool isAction() const {
        return buttons & ButtonValues::ACTION;
    }
};

UserInput GetUserInput();


#endif