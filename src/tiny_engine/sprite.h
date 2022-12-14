#ifndef SPRITE_H
#define SPRITE_H

#include "pch.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"

struct Sprite {
    Sprite() { }
    Sprite(const Texture& mainTex);
    Sprite(const Shader& shader, const Texture& mainTex);
    inline void Delete() {
        glDeleteVertexArrays(1, &quadVAO);
        shader.Delete();
        mainTex.Delete();
    }

    void DrawSprite(const Camera& cam, glm::vec2 position = glm::vec2(0), 
                    glm::vec2 size = glm::vec2(25.0f, 25.0f), f32 rotate = 0.0f, 
                    glm::vec3 rotationAxis = glm::vec3(0.0, 0.0, 1.0),
                    glm::vec4 color = glm::vec4(1.0f), bool adjustToScreensize = false,
                    bool shouldFlipY = false) const;

    bool isValid() const { return mainTex.id != 0; }
    inline f32 GetTextureWidth() const { return mainTex.width; }
    inline f32 GetTextureHeight() const { return mainTex.height; }

    template<typename T>
    void setShaderUniform(const char* name, T val) const {
        shader.setUniform(name, val);
    }
    Texture GetMainTex() { return mainTex; }
    Shader GetShader() { return shader; }

private:
    Texture mainTex;
    Shader shader;
    u32 quadVAO;
    void initRenderData();
};


#endif