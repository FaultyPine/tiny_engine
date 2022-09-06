#ifndef INPUT_H
#define INPUT_H

#include "pch.h"
#include "tiny_engine.h"

void CloseGameWindow();
extern GLFWwindow* glob_glfw_window;


int GetKeyState(s32 key, s32 keyState) {
    return glfwGetKey(glob_glfw_window, key) == keyState;
}

// mouse
struct MouseInput {
    f32 lastX = 400;
    f32 lastY = 300;
    f32 yaw = 0.0f;
    f32 pitch = 0.0f;
    glm::vec3 GetNormalizedLookDir() {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        return glm::normalize(direction);
    }
};
static MouseInput mouseInput;

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
        return mouseInput;
    }
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
    bool isUp() {
        return buttons & ButtonValues::UP;
    }
    bool isDown() {
        return buttons & ButtonValues::DOWN;
    }
    bool isAction() {
        return buttons & ButtonValues::ACTION;
    }
};

void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
    static bool firstMouse = true;
    if (firstMouse)
    {
        mouseInput.lastX = xpos;
        mouseInput.lastY = ypos;
        firstMouse = false;
    }
  
    float xoffset = xpos - mouseInput.lastX;
    float yoffset = mouseInput.lastY - ypos; 
    mouseInput.lastX = xpos;
    mouseInput.lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    mouseInput.yaw   += xoffset;
    mouseInput.pitch += yoffset;

    if(mouseInput.pitch > 89.0f)
        mouseInput.pitch = 89.0f;
    if(mouseInput.pitch < -89.0f)
        mouseInput.pitch = -89.0f;
}

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
    if (GetKeyState(GLFW_KEY_SPACE, GLFW_PRESS)) {
        input.buttons |= UserInput::ButtonValues::UP;
    }
    if (GetKeyState(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS)) {
        input.buttons |= UserInput::ButtonValues::DOWN;
    }
    
    
    return input;
}


#endif