#ifndef CAMERA_H
#define CAMERA_H

#include "pch.h"
#include "input.h"


struct Camera {
    f32 speed = 4.5f;
    glm::vec3 cameraPos = glm::vec3(1);
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
    bool isSwivelable = true;

    inline glm::mat4 GetProjectionMatrix() const {
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
    inline static void UpdateCamera() {
        Camera& cam = GetMainCamera();
        MouseInput& mouseInput = MouseInput::GetMouse();
        if (cam.isSwivelable) {
            cam.cameraFront = mouseInput.GetNormalizedLookDir();
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
    /// returns glm::vec2(minScreenWidth, minScreenHeight)
    glm::vec2 GetMinScreenDimensions() const { return glm::vec2(minScreenWidth, minScreenHeight); }
    glm::vec2 GetMaxScreenDimensions() const { return glm::vec2(maxScreenWidth, maxScreenHeight); }
    inline static u32 GetScreenWidth()  { return GetMainCamera().screenWidth; }
    inline static u32 GetScreenHeight() { return GetMainCamera().screenHeight; }
};




#endif