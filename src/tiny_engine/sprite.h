#ifndef SPRITE_H
#define SPRITE_H

#include "pch.h"
#include "shader.h"
#include "texture.h"
#include "tiny_fs.h"
#include "camera.h"

struct Sprite {
    Sprite() { }
    Sprite(Texture &mainTex);
    inline void UnloadSprite() {
        glDeleteVertexArrays(1, &quadVAO);
    }

    void DrawSprite(const Camera& cam, glm::vec2 position, 
                    glm::vec2 size = glm::vec2(10.0f, 10.0f), f32 rotate = 0.0f, 
                    glm::vec3 color = glm::vec3(1.0f));

private:
    Texture mainTex;
    Shader shader; 
    u32 quadVAO;

    void initRenderData();
};


#endif