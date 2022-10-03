#include "input.h"



UserInput GetUserInput() {
    // making sure to zero init so we don't drift
    UserInput input = {}; 

    if (GetKeyState(GLFW_KEY_ESCAPE, GLFW_PRESS)) {
        CloseGameWindow();
    }
    if (GetKeyState(GLFW_KEY_W, GLFW_PRESS)) {
        input.buttons |= UserInput::ButtonValues::UP;
    }
    if (GetKeyState(GLFW_KEY_S, GLFW_PRESS)) {
        input.buttons |= UserInput::ButtonValues::DOWN;
    }
    if (GetKeyState(GLFW_KEY_A, GLFW_PRESS)) {
        input.buttons |= UserInput::ButtonValues::LEFT;
    }
    if (GetKeyState(GLFW_KEY_D, GLFW_PRESS)) {
        input.buttons |= UserInput::ButtonValues::RIGHT;
    }
    
    return input;
}



void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
    MouseInput& mouseInput = UserInput::GetMouse();
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
    UserInput::GetMouse().mousePos = glm::vec2(xpos, ypos);
}