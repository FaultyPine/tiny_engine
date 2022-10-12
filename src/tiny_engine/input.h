#ifndef TINY_INPUT_H
#define TINY_INPUT_H

#include "pch.h"


extern GLFWwindow* glob_glfw_window;



inline s32 GetKeyState(s32 key, s32 keyState) {
    return glfwGetKey(glob_glfw_window, key) == keyState;
}


// Mouse

void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos);
void cursor_position_callback(GLFWwindow* window, f64 xpos, f64 ypos);

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