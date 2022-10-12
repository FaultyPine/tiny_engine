#include "input.h"

// mouse callbacks
void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
    MouseInput& mouseInput = MouseInput::GetMouse();
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
    MouseInput::GetMouse().mousePos = glm::vec2(xpos, ypos);
}