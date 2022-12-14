#ifndef TINY_POSTPROCESSING_H
#define TINY_POSTPROCESSING_H

#include "pch.h"
#include "sprite.h"
#include "tiny_engine.h"
#include "framebuffer.h"
#include <functional>

// if not using a fullscreen buffer, make sure to set glViewport to prevent stretching/weird stuff happening with the rendered texture

struct PostprocessingFB {
    PostprocessingFB(Shader shader, glm::vec2 size);
    PostprocessingFB() {}

    inline bool isValid() { return fb.isValid(); }
    inline void Bind() { fb.Bind(); }
    inline void BindDefaultFrameBuffer() { fb.BindDefaultFrameBuffer(); }
    void Delete() { 
        fullscreenSprite.UnloadSprite();
        fb.Delete();
    }
    void ClearDepth() { fb.ClearDepth(); }
    void ClearStencil() { fb.ClearStencil(); }
    void DrawToScreen(std::function<void()> drawSceneFunc);

    glm::vec2 GetSize() { return fb.GetSize(); }

private:    
    void DrawToScreen(const Shader& shader);
    
    Shader postProcessingShader;
    Sprite fullscreenSprite; // also stores the shader we can use for postprocessing
    
    Framebuffer fb;

};


#endif