#include "camera.h"
#include "input.h"


const s32 TAB_OUT_OF_WINDOW_KEY = TINY_KEY_TAB;


glm::mat4 Camera::GetProjectionMatrix() const {
    if (projection == PERSPECTIVE) {
        return GetPerspectiveProjection();
    }
    else {
        return GetOrthographicProjection();
    }
}
glm::mat4 Camera::GetOrthographicProjection() const {
    if (projection == PERSPECTIVE) {
        return glm::ortho(0.0f, (f32)screenWidth, (f32)screenHeight, 0.0f, -1.0f, 1.0f); 
    }
    else {
        return glm::ortho(-(f32)screenWidth / 2.0f, (f32)screenWidth / 2.0f, (f32)screenHeight / 2.0f, -(f32)screenHeight / 2.0f, -1.0f, 1.0f);
    }
}
glm::mat4 Camera::GetPerspectiveProjection() const {
    f32 aspect = (f32)screenWidth / (f32)screenHeight;
    if (screenHeight == 0)
    {
        aspect = 0.0f;
    } 
    return glm::perspective(glm::radians(FOV), aspect, nearClip, farClip);
}
glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}
Camera& Camera::GetMainCamera() {
    static Camera mainCamera;
    return mainCamera;
}
void Camera::UpdateCamera() {
    Camera& cam = GetMainCamera();
    MouseInput& mouseInput = MouseInput::GetMouse();
    mouseInput.UpdateMouse(mouseInput.mousePos.x, mouseInput.mousePos.y);
    if (cam.isSwivelable) {
        cam.cameraFront = mouseInput.GetNormalizedLookDir();
    }
    UpdateTabbingOut();
    cam.isSwivelable = getCursorMode() == CursorMode::DISABLED;
}
void Camera::UpdateTabbingOut()
{
    if (Keyboard::isKeyPressed(TAB_OUT_OF_WINDOW_KEY)) {
        // when "tabbing" in and out of the game, the cursor position jumps around weirdly
        // so here we save the last cursor pos when we tab out, and re-set it when we tab back in
        static glm::vec2 lastMousePos = glm::vec2(0);
        if (getCursorMode() == CursorMode::NORMAL) {
            setCursorMode(CursorMode::DISABLED);
            setCursorPosition(lastMousePos.x, lastMousePos.y);
            MouseInput::GetMouse().UpdateMouse(lastMousePos.x, lastMousePos.y);
            UpdateCamera();
        }
        else {
            lastMousePos = {MouseInput::GetMouse().lastX, MouseInput::GetMouse().lastY};
            setCursorMode(CursorMode::NORMAL);
            setCursorPosition(GetMainCamera().screenWidth/2.0f, GetMainCamera().screenHeight/2.0f);
        }
    }
}
void Camera::LookAt(glm::vec3 pos) {
    glm::vec3 diff = pos - cameraPos;
    glm::vec3 forward = glm::normalize(diff);
    glm::vec3 side = glm::normalize(glm::cross(cameraUp, forward));
    glm::vec3 newUp = glm::cross(forward, side);
    cameraFront = forward;
    cameraUp = newUp;
}
void Camera::SetMode2D() {
    isSwivelable = false;
    projection = ORTHOGRAPHIC;
}
void Camera::SetMode3D() {
    isSwivelable = true;
    projection = PERSPECTIVE;
}
/// returns glm::vec2(minScreenWidth, minScreenHeight)
glm::vec2 Camera::GetMinScreenDimensions() const { return glm::vec2(minScreenWidth, minScreenHeight); }
glm::vec2 Camera::GetMaxScreenDimensions() const { return glm::vec2(maxScreenWidth, maxScreenHeight); }
u32 Camera::GetScreenWidth() 
{ 
    return GetMainCamera().screenWidth; 
}
u32 Camera::GetScreenHeight() 
{ 
    return GetMainCamera().screenHeight; 
}