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
                    glm::vec4 color = glm::vec4(1.0f)) const;

    bool isValid() const { return mainTex.id != 0; }
    inline f32 GetTextureWidth() const { return mainTex.width; }
    inline f32 GetTextureHeight() const { return mainTex.height; }
    void OverrideSpriteShader(Shader shader) {
        this->shader = shader;
    }
    template<typename T>
    void setShaderUniform(const char* name, T val) {
        shader.setUniform(name, val);
    }
    
private:
    Texture mainTex;
    Shader shader; 
    u32 quadVAO;

    void initRenderData();
};


#endif