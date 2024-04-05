#ifndef CAMERA_H
#define CAMERA_H

//#include "pch.h"
#include "tiny_defines.h"
#include "input.h"
#include "tiny_engine.h"

// press this button to unlock the cursor from the window

struct Camera {
    f32 speed = 10.0f;
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
    f32 farClip = 3000.0f;
    bool isSwivelable = false;
    enum Projection {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };
    Projection projection = ORTHOGRAPHIC;

    TAPI glm::mat4 GetProjectionMatrix() const;
    TAPI glm::mat4 GetOrthographicProjection() const;
    TAPI glm::mat4 GetPerspectiveProjection() const;
    TAPI glm::mat4 GetViewMatrix() const;
    TAPI void LookAt(glm::vec3 pos);
    TAPI void SetMode2D();
    TAPI void SetMode3D();
    /// returns glm::vec2(minScreenWidth, minScreenHeight)
    TAPI glm::vec2 GetMinScreenDimensions() const;
    TAPI glm::vec2 GetMaxScreenDimensions() const;
    
    TAPI static void UpdateCamera();
    TAPI static void UpdateTabbingOut();
    TAPI static u32 GetScreenWidth();
    TAPI static u32 GetScreenHeight();
    TAPI static glm::vec2 GetScreenDimensions();
    TAPI static Camera& GetMainCamera();
};




#endif