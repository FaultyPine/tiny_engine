#ifndef INPUT_H
#define INPUT_H

#include "pch.h"
#include "tiny_engine.h"

void CloseGameWindow();
extern GLFWwindow* glob_glfw_window;


int GetKeyState(s32 key, s32 keyState) {
    return glfwGetKey(glob_glfw_window, key) == keyState;
}

struct UserInput {
    glm::vec2 stick;

    bool isForward() {
        return stick.y > 0.0;
    }
    bool isBackward() {
        return stick.y < 0.0;
    }
    bool isRight() {
        return stick.x > 0.0;
    }
    bool isLeft() {
        return stick.x < 0.0;
    }
};

UserInput GetUserInput() {
    // making sure to zero init so we don't drift
    UserInput input = {}; 

    if (GetKeyState(GLFW_KEY_ESCAPE, GLFW_PRESS)) {
        CloseGameWindow();
    }
    if (GetKeyState(GLFW_KEY_W, GLFW_PRESS)) {
        input.stick.y = 1.0;
    }
    if (GetKeyState(GLFW_KEY_S, GLFW_PRESS)) {
        input.stick.y = -1.0;
    }
    if (GetKeyState(GLFW_KEY_A, GLFW_PRESS)) {
        input.stick.x = -1.0;
    }
    if (GetKeyState(GLFW_KEY_D, GLFW_PRESS)) {
        input.stick.x = 1.0;
    }
    
    return input;
}


#endif