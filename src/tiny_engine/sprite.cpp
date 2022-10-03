#include "sprite.h"


Sprite::Sprite(const Texture& mainTex) {
    this->mainTex = mainTex;
    this->shader = Shader(UseResPath("shaders/default_sprite.vs").c_str(), UseResPath("shaders/default_sprite.fs").c_str());
    initRenderData();
}

void Sprite::LoadSpritesFromSpriteSheet(const char* spritesheetPath, Sprite* resultTextures, u32 numRows, u32 numCols, TextureProperties props) {
    // load spritesheet
    s32 width, height, numChannels;
    u8* imgData = LoadImageData(spritesheetPath, &width, &height, &numChannels);

    // determine how to iterate through the image
    s32 singleSpriteWidth = width / numCols;
    s32 singleSpriteHeight = height / numRows;

    // iterate through each sprite img
    for (s32 row = 0; row < numRows; row++) {
        for (s32 col = 0; col < numCols; col++) {
            // get sprite position
            f32 spriteY = row * singleSpriteHeight;
            f32 spriteX = col * singleSpriteWidth;
            ASSERT(spriteY >= 0 && spriteY <= height);
            ASSERT(spriteX >= 0 && spriteX <= width);

            SetPixelReadSettings(width, singleSpriteWidth*col, singleSpriteHeight*row, 4);
            // NOTE: using glPixelStorei to tell opengl to skip pixels so that I can just use the x,y,width,height to
            // read the relevant part of the spritesheet
            Texture thisTex = GenTextureFromImg(imgData, singleSpriteWidth, singleSpriteHeight, props);
            
            s32 resultTexturesIdx = col + (row*numCols);
            resultTextures[resultTexturesIdx] = Sprite(thisTex);
            
        }
    }

    // back to defaults
    SetPixelReadSettings(0, 0, 0, 4);

    free(imgData);
}


void Sprite::DrawSprite(const Camera& cam, glm::vec2 position, 
                glm::vec2 size, f32 rotate, glm::vec3 color) const {
    if (!isValid()) {
        std::cout << "Tried to draw invalid sprite!\n";
        exit(1);
        return;
    }
    shader.use();
    // set up transform of the actual sprite
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));  

    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); 
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f)); 
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

    model = glm::scale(model, glm::vec3(size, 1.0f)); 
    
    glm::mat4 projection = glm::ortho(0.0f, (f32)cam.screenWidth, (f32)cam.screenHeight, 0.0f, -1.0f, 1.0f);  

    shader.setUniform("model", model);
    shader.setUniform("spriteColor", color);
    shader.setUniform("projection", projection);
    shader.setUniform("image", mainTex.id);

    glActiveTexture(GL_TEXTURE0);
    mainTex.bind();

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


void Sprite::initRenderData() {
    // configure VAO/VBO
    u32 VBO;

    const float tex_quad[] = { 
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &VBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tex_quad), tex_quad, GL_STATIC_DRAW);

    glBindVertexArray(quadVAO);
    // vec2 position, vec2 texcoords, all contained in a vec4
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);  
    glBindVertexArray(0);
}