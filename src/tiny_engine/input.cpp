#include "input.h"

// TODO: realized this a bit late, but
// i've ended up kinda coding my game input
// logic into the engine. In the future, seperate this
// out of the engine and into my game


// 6 = number of buttons (u/d/l/r/action1/action2)
#define NUM_BUTTONS 6

static UserInput::ButtonValues inorderButtonMappings[NUM_BUTTONS] = {
    UserInput::ButtonValues::UP,
    UserInput::ButtonValues::DOWN,
    UserInput::ButtonValues::LEFT,
    UserInput::ButtonValues::RIGHT,
    UserInput::ButtonValues::ACTION1,
    UserInput::ButtonValues::ACTION2
};
static s32 inputMap[MAX_NUM_PLAYERS][NUM_BUTTONS] = {
    {GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT},
    {GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_RIGHT_CONTROL, GLFW_KEY_RIGHT_SHIFT},
    {-1,-1,-1,-1,-1,-1}, // TODO
    {-1,-1,-1,-1,-1,-1}, // -1 is GLFW_KEY_UNKNOWN
};


UserInput GetUserInput() {
    // making sure to zero init so we don't drift
    UserInput input = {}; 
    
    if (GetKeyState(GLFW_KEY_ESCAPE, GLFW_PRESS)) {
        CloseGameWindow();
    }

    for (u32 playerIdx = 0; playerIdx < MAX_NUM_PLAYERS; playerIdx++) {
        u32& playerButtons = input.buttons[playerIdx];
        for (u32 buttonIdx = 0; buttonIdx < NUM_BUTTONS; buttonIdx++) {
            if (GetKeyState(inputMap[playerIdx][buttonIdx], GLFW_PRESS)) {
                playerButtons |= inorderButtonMappings[buttonIdx];
            }
        }
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