#include "postprocessing.h"
#include "camera.h"
#include "tiny_engine/tiny_fs.h"

// by default this framebuffer is just for the whole screen
PostprocessingFB::PostprocessingFB(Shader shader, glm::vec2 framebufferSize) {
    fb = Framebuffer(framebufferSize.x, framebufferSize.y, Framebuffer::FramebufferAttachmentType::COLOR);

    fullscreenSprite = Sprite(shader, fb.GetTexture());
}

void PostprocessingFB::DrawToScreen(const Shader& shader) {
    // Draws the content of the "fullscreenSprite" to the screen as a 2D quad
    if (!shader.isValid()) {
        std::cout << "[ERROR] Attempted to draw framebuffer with invalid shader!\n";
        exit(1);
    }
    BindDefaultFrameBuffer();
    ClearGLColorBuffer();
    shader.use();
    shader.setUniform("screenWidth", (f32)Camera::GetScreenWidth());
    shader.setUniform("screenHeight", (f32)Camera::GetScreenHeight());
    shader.setUniform("time", (f32)GetTime());
    fullscreenSprite.DrawSprite(
        Camera::GetMainCamera(), 
        {0,0}, fb.GetSize());
}


void PostprocessingFB::DrawToScreen(std::function<void()> drawSceneFunc) {
    glm::vec2 screenDimensions = {Camera::GetScreenWidth(), Camera::GetScreenHeight()};
    if (!postProcessingShader.isValid()) {
        postProcessingShader = Shader(UseResPath("shaders/screen_texture.vs").c_str(), UseResPath("shaders/screen_texture.fs").c_str());
    }
    if (!isValid()) {
        *this = PostprocessingFB(postProcessingShader, {screenDimensions.x, screenDimensions.y});
        glViewport(0, 0, screenDimensions.x, screenDimensions.y);
    }
    // if we resize the screen
    if (GetSize().x != screenDimensions.x && GetSize().y != screenDimensions.y) {
        Delete();
        *this = PostprocessingFB(postProcessingShader, screenDimensions);
        // is there a better way to handle window resizing than recreating the fb?
    }

    // bind this framebuffer and render scene to it
    // since fullscreenSprite's texture is the same as the texture we render the framebuffer to
    // the fullscreenSprite will hold our scene texture
    Bind();
    ClearGLColorBuffer();
    
    drawSceneFunc();

    DrawToScreen(fullscreenSprite.GetShader());
}