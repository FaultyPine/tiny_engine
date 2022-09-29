#ifndef CAMERA_H
#define CAMERA_H

#include "pch.h"
#include "input.h"


struct Camera {
    float speed = 4.5f;
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

    u32 screenWidth = 800;
    u32 screenHeight = 600;
    f32 FOV = 45.0f;
    f32 nearClip = 0.1f;
    f32 farClip = 100.0f;

    inline glm::mat4 GetProjectionMatrix() {
        f32 aspect = (f32)screenWidth / screenHeight;
        return glm::perspective(glm::radians(FOV), aspect, nearClip, farClip);
    }
    inline glm::mat4 GetViewMatrix() {
        return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }
    inline static Camera& GetMainCamera() {
        static Camera mainCamera;
        return mainCamera;
    }
};




#endif