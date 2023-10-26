#ifndef CAMERA_H
#define CAMERA_H

//#include "pch.h"
#include "input.h"

// press this button to unlock the cursor from the window
const s32 TAB_OUT_OF_WINDOW_KEY = GLFW_KEY_TAB;

struct Camera {
    f32 speed = 6.5f;
    glm::vec3 cameraPos = glm::vec3(0);
    glm::vec3 cameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

    u32 screenWidth = 800;
    u32 screenHeight = 600;
    u32 minScreenWidth = 800;
    u32 maxScreenWidth = 1300;
    u32 minScreenHeight = 600;
    u32 maxScreenHeight = 975;
    f32 FOV = 45.0f;
    f32 nearClip = 0.1f;
    f32 farClip = 100.0f;
    bool isSwivelable = false;
    enum Projection {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };
    Projection projection = ORTHOGRAPHIC;

    glm::mat4 GetProjectionMatrix() const {
        if (projection == PERSPECTIVE) {
            return GetPerspectiveProjection();
        }
        else {
            return GetOrthographicProjection();
        }
    }
    inline glm::mat4 GetOrthographicProjection() const {
        if (projection == PERSPECTIVE) {
            return glm::ortho(0.0f, (f32)screenWidth, (f32)screenHeight, 0.0f, -1.0f, 1.0f); 
        }
        else {
            return glm::ortho(-(f32)screenWidth / 2.0f, (f32)screenWidth / 2.0f, (f32)screenHeight / 2.0f, -(f32)screenHeight / 2.0f, -1.0f, 1.0f);
        }
    }
    inline glm::mat4 GetPerspectiveProjection() const {
        f32 aspect = (f32)screenWidth / screenHeight;
        return glm::perspective(glm::radians(FOV), aspect, nearClip, farClip);
    }
    inline glm::mat4 GetViewMatrix() const {
        return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }
    inline static Camera& GetMainCamera() {
        static Camera mainCamera;
        return mainCamera;
    }
    static void UpdateCamera() {
        Camera& cam = GetMainCamera();
        MouseInput& mouseInput = MouseInput::GetMouse();
        if (cam.isSwivelable) {
            cam.cameraFront = mouseInput.GetNormalizedLookDir();
        }

        if (Keyboard::isKeyPressed(TAB_OUT_OF_WINDOW_KEY)) {
        // when "tabbing" in and out of the game, the cursor position jumps around weirdly
        // so here we save the last cursor pos when we tab out, and re-set it when we tab back in
        static glm::vec2 lastMousePos = glm::vec2(0);
        if (glfwGetInputMode(glob_glfw_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
            glfwSetInputMode(glob_glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            cam.isSwivelable = true;
            glfwSetCursorPos(glob_glfw_window, lastMousePos.x, lastMousePos.y);
        }
        else {
            lastMousePos = {MouseInput::GetMouse().lastX, MouseInput::GetMouse().lastY};
            glfwSetInputMode(glob_glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  
            cam.isSwivelable = false;
        }
    }
    }
    inline void LookAt(glm::vec3 pos) {
        glm::vec3 diff = pos - cameraPos;
        glm::vec3 forward = glm::normalize(diff);
        glm::vec3 side = glm::normalize(glm::cross({0,1,0}, forward));
        glm::vec3 newUp = glm::cross(forward, side);
        cameraFront = forward;
        cameraUp = newUp;
    }
    inline void SetMode2D() {
        isSwivelable = false;
        projection = ORTHOGRAPHIC;
    }
    inline void SetMode3D() {
        isSwivelable = true;
        projection = PERSPECTIVE;
    }
    /// returns glm::vec2(minScreenWidth, minScreenHeight)
    glm::vec2 GetMinScreenDimensions() const { return glm::vec2(minScreenWidth, minScreenHeight); }
    glm::vec2 GetMaxScreenDimensions() const { return glm::vec2(maxScreenWidth, maxScreenHeight); }
    inline static u32 GetScreenWidth() { return GetMainCamera().screenWidth; }
    inline static u32 GetScreenHeight() { return GetMainCamera().screenHeight; }
};




#endif