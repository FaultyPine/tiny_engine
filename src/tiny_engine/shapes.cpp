#include "shapes.h"

// NOT WORKING ATM
namespace Shapes {

static Shader shapeShader;

void DrawTriangle(const glm::vec2& pos) {
    if (shapeShader.ID == 0) shapeShader = Shader(UseResPath("shaders/default_sprite.fs"), UseResPath("shaders/default_sprite.fs"));
    u32 VBO;
    u32 quadVAO;

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

    shapeShader.use();
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}



    
}