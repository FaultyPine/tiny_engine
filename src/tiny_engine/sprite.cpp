#include "sprite.h"


Sprite::Sprite(Texture& mainTex) {
    this->mainTex = mainTex;
    this->shader = Shader(UseResPath("shaders/default_sprite.vs").c_str(), UseResPath("shaders/default_sprite.fs").c_str());
    initRenderData();
}

void Sprite::DrawSprite(const Camera& cam, glm::vec2 position, 
                glm::vec2 size, f32 rotate, glm::vec3 color) {
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