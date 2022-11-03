#include "shapes.h"

#include "camera.h"
#include "texture.h"

// always draws a quad, but uses shaders to derive desired shape
// all shapes have model/projection/color matrices that need to be set
// but different shapes can also set other uniforms if they need it

namespace Shapes {

// shader var is just called "shader" after this macro
#define SHAPE_SHADER(shaderName, vertShaderPath, fragShaderPath) static Shader shaderName; \
if (shaderName.ID == 0) \
    shaderName = Shader(UseResPath(vertShaderPath).c_str(), UseResPath(fragShaderPath).c_str())


void DrawSquare(const glm::vec2& pos, const glm::vec2& size, 
            f32 rotation, const glm::vec3& rotationAxis, 
            const glm::vec4& color, bool isHollow) {
    SHAPE_SHADER(shader, "shaders/shapes/shape.vs", "shaders/shapes/square.fs");
    s32 hollow = isHollow ? 1 : 0;
    shader.use();
    shader.setUniform("isHollow", hollow);
    Shapes::DrawShape(pos, size, rotation, rotationAxis, color, shader);
}

void DrawCircle(const glm::vec2& pos, f32 radius, const glm::vec4& color, bool isHollow) {
    SHAPE_SHADER(shader, "shaders/shapes/shape.vs", "shaders/shapes/circle.fs");
    s32 hollow = isHollow ? 1 : 0;
    shader.use();
    shader.setUniform("isHollow", hollow);
    Shapes::DrawShape(pos, glm::vec2(radius, radius), 0.0, glm::vec3(0.0, 0.0, 1.0), color, shader);
}

// to draw vector shapes, just draw a quad and then use the shader to actually derive the shape
void DrawShape(const glm::mat4& model, const glm::vec4& color, const Shader& shader) {
    static Shader staticShader = {};
    static u32 quadVAO = 0;
    if (staticShader.ID != shader.ID) {
        staticShader = shader; // re-set shader when we are trying to draw a different shape
        if (quadVAO == 0) { // only need to init vert data once
            static const f32 tex_quad[] = { 
                // pos      // tex
                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 

                0.0f, 1.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 0.0f, 1.0f, 0.0f
            };

            u32 VBO = 0;
            GLCall(glGenVertexArrays(1, &quadVAO));
            GLCall(glGenBuffers(1, &VBO));
            
            GLCall(glBindVertexArray(quadVAO));
            GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
            GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(tex_quad), tex_quad, GL_STATIC_DRAW));

            // vec2 position, vec2 texcoords, all contained in a vec4
            GLCall(glEnableVertexAttribArray(0));
            GLCall(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)0));
            
            GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));  
            GLCall(glBindVertexArray(0));
        }
    }

    glm::mat4 projection = glm::ortho(0.0f, (f32)Camera::GetScreenWidth(), (f32)Camera::GetScreenHeight(), 0.0f, -1.0f, 1.0f);

    shader.use();
    shader.setUniform("model", model);
    shader.setUniform("projection", projection);
    shader.setUniform("color", color);

    GLCall(glBindVertexArray(quadVAO));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
    GLCall(glBindVertexArray(0));
}

void DrawShape(const glm::vec2& pos, const glm::vec2& size, f32 rotationDegrees, 
                const glm::vec3& rotationAxis, const glm::vec4& color, const Shader& shader) {

    // set up transform of the actual sprite
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(pos.x, pos.y, 0.0f));  

    // rotation is about the "middle" of the shape. 
    // if you want rotation about some other part of the shape, use the overloaded DrawShape and make
    // your own model matrix
    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); 
    model = glm::rotate(model, glm::radians(rotationDegrees), rotationAxis); 
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

    model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f)); 
    DrawShape(model, color, shader);
}


void DrawLine(const glm::vec2& origin, const glm::vec2& dest, const glm::vec4& color, f32 width) {
    SHAPE_SHADER(shader, "shaders/shapes/shape.vs", "shaders/shapes/line.fs");
    shader.use();
    // draw a non-hollow square that has
    // 'top-left' corner at our origin 
    f32 hypotenuse = glm::distance(dest, origin);
    f32 yDelta = dest.y - origin.y;
    f32 xDelta = dest.x - origin.x;
    glm::vec2 lineSize = {width, hypotenuse};

    // square starts at origin and extends downward at first...
    glm::vec2 beginningDir = glm::normalize(glm::vec2(0.0, 1.0));
    glm::vec2 destDir = glm::normalize(dest - origin);
    
    // get angle we need to rotate to align square with dest
    f32 dot = beginningDir.x * destDir.x + beginningDir.y * destDir.y;
    f32 det = beginningDir.x * destDir.y - beginningDir.y * destDir.x;
    f32 theta = atan2(det , dot); // radians

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(origin.x, origin.y, 0.0f));  
    model = glm::rotate(model, theta, {0.0, 0.0, 1.0}); 
    model = glm::scale(model, glm::vec3(lineSize.x, lineSize.y, 1.0f)); 

    Shapes::DrawShape(model, color, shader);
}

    
}