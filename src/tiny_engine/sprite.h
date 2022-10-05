#ifndef SPRITE_H
#define SPRITE_H

#include "pch.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"

struct Sprite {
    Sprite() { }
    Sprite(const Texture &mainTex);
    inline void UnloadSprite() {
        glDeleteVertexArrays(1, &quadVAO);
    }

    void DrawSprite(const Camera& cam, glm::vec2 position, 
                    glm::vec2 size = glm::vec2(10.0f, 10.0f), f32 rotate = 0.0f, 
                    glm::vec3 rotationAxis = glm::vec3(0.0, 0.0, 1.0),
                    glm::vec3 color = glm::vec3(1.0f)) const;

    bool isValid() const { return mainTex.id != 0; }
    void GetTextureDimensions(f32& width, f32& height) {
        width = mainTex.width;
        height = mainTex.height;
    }
    
private:
    Texture mainTex;
    Shader shader; 
    u32 quadVAO;

    void initRenderData();
};


#endif