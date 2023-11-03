#ifndef SPRITE_H
#define SPRITE_H

//#include "pch.h"
#include "shader.h"
#include "texture.h"
#include "tiny_types.h"

struct Transform2D;

struct Sprite {
    Sprite() = default;
    TAPI Sprite(const Texture& mainTex);
    TAPI Sprite(const Shader& shader, const Texture& mainTex);
    TAPI void Delete();

    TAPI void DrawSprite(glm::vec2 position = glm::vec2(0), 
                    glm::vec2 size = glm::vec2(25.0f, 25.0f), f32 rotate = 0.0f, 
                    glm::vec3 rotationAxis = glm::vec3(0.0, 0.0, 1.0),
                    glm::vec4 color = glm::vec4(1.0f),
                    bool shouldFlipY = false) const;
    TAPI void DrawSprite(const Transform2D& tf, glm::vec4 color = glm::vec4(1.0f), bool shouldFlipY = false) const;
    TAPI void DrawSpriteFullscreen(glm::vec4 color = glm::vec4(1.0f)) const;

    bool isValid() const { return mainTex.id != 0; }
    inline f32 GetTextureWidth() const { return mainTex.width; }
    inline f32 GetTextureHeight() const { return mainTex.height; }

    template<typename T>
    TAPI void setShaderUniform(const char* name, T val) const;

    Texture& GetMainTex() { return mainTex; }
    Shader GetShader() { return shader; }

private:
    Texture mainTex = {};
    Shader shader = {};
    u32 quadVAO = 0;
    void initRenderData();
};


#endif