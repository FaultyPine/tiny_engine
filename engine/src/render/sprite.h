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

    // custom texture, custom shader
    TAPI void DrawSprite(
        const Shader& shader,
        const Texture& texture, 
        glm::vec2 position, 
        glm::vec2 size = glm::vec2(25.0, 25.0), 
        f32 rotate = 0.0f, 
        glm::vec3 rotationAxis = glm::vec3(0.0, 0.0, 1.0), 
        glm::vec4 color = glm::vec4(1.0f), 
        bool shouldFlipY = false) const;

    // custom texture, use cached shader
    TAPI void DrawSprite(
        const Texture& texture, 
        glm::vec2 position, 
        glm::vec2 size = glm::vec2(25.0, 25.0), 
        f32 rotate = 0.0f, 
        glm::vec3 rotationAxis = glm::vec3(0.0, 0.0, 1.0), 
        glm::vec4 color = glm::vec4(1.0f), 
        bool shouldFlipY = false) const 
        { DrawSprite(mainShader, texture, position, size, rotate, rotationAxis, color, shouldFlipY); }
        
    // custom shader, use cached texture
    TAPI void DrawSprite(
        const Shader& shader, 
        glm::vec2 position, 
        glm::vec2 size = glm::vec2(25.0, 25.0), 
        f32 rotate = 0.0f, 
        glm::vec3 rotationAxis = glm::vec3(0.0, 0.0, 1.0), 
        glm::vec4 color = glm::vec4(1.0f), 
        bool shouldFlipY = false) const 
        { DrawSprite(shader, mainTex, position, size, rotate, rotationAxis, color, shouldFlipY); }
    
    // use cached texture + shader
    TAPI void DrawSprite(
        glm::vec2 position = glm::vec2(0), 
        glm::vec2 size = glm::vec2(25.0f, 25.0f),
        f32 rotation = 0.0f,
        glm::vec3 rotationAxis = glm::vec3(0.0, 0.0, 1.0),
        glm::vec4 color = glm::vec4(1.0f),
        bool shouldFlipY = false) const { DrawSprite(mainShader, mainTex, position, size, rotation, rotationAxis, color, shouldFlipY); }
    TAPI void DrawSpriteFullscreen(glm::vec4 color = glm::vec4(1.0f)) const;

    bool isValid() const { return mainShader.isValid() && quadVAO != 0; }
    inline f32 GetTextureWidth() const { return mainTex.GetWidth(); }
    inline f32 GetTextureHeight() const { return mainTex.GetHeight(); }

    TAPI const Texture& GetMainTex() const { return mainTex; }
    TAPI const Shader& GetShader() const { return mainShader; }

private:
    Texture mainTex = {};
    Shader mainShader = {};
    u32 quadVAO = 0;
    void initRenderData();
};


#endif