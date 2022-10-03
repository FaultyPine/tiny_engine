#ifndef SPRITE_H
#define SPRITE_H

#include "pch.h"
#include "shader.h"
#include "texture.h"
#include "tiny_fs.h"
#include "camera.h"

struct Sprite {
    Sprite() { }
    Sprite(const Texture &mainTex);
    inline void UnloadSprite() {
        glDeleteVertexArrays(1, &quadVAO);
    }

    static void LoadSpritesFromSpriteSheet(const char* spritesheetPath, Sprite* resultTextures, u32 numRows, u32 numCols, TextureProperties props);

    void DrawSprite(const Camera& cam, glm::vec2 position, 
                    glm::vec2 size = glm::vec2(10.0f, 10.0f), f32 rotate = 0.0f, 
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