#include "shapes.h"

#include "camera.h"
#include "texture.h"
#include "shader.h"
#include "tiny_fs.h"


// shader var is just called "shader" after this macro
#define SHAPE_SHADER(shaderName, vertShaderPath, fragShaderPath) static Shader shaderName; \
if (shaderName.ID == 0) \
    shaderName = Shader(UseResPath(vertShaderPath).c_str(), UseResPath(fragShaderPath).c_str())


namespace Shapes3D {

void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color, f32 width) {
    static Shader shader;
    if (!shader.isValid()) {
        shader = Shader::CreateShaderFromStr(
R"(
#version 330 core
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec4 vertColor;
out vec4 color;
uniform mat4 mvp;
void main(){
    color = vertColor;
	gl_Position = mvp * vec4(vertPos, 1.0);
}
)",
R"(
#version 330 core
out vec4 FragColor;
in vec4 color;
void main(){
	FragColor = color;
}
)"
        );
    }
    static u32 quadVAO = 0;
    static u32 VBO = 0;
    const f32 lineVerts[] = {
        start.x, start.y, start.z, // vert pos
        color.r, color.g, color.b, color.a, // vert col

        end.x,   end.y,   end.z,
        color.r, color.g, color.b, color.a,
    };
    if (quadVAO == 0) {
        GLCall(glGenVertexArrays(1, &quadVAO));
        GLCall(glGenBuffers(1, &VBO));
        
        GLCall(glBindVertexArray(quadVAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(lineVerts), lineVerts, GL_DYNAMIC_DRAW));

        // this shader has vert attributes: vec3 vertPos  vec3 vertNormal  vec2 vertTexCoord  vec3 vertColor
        GLCall(glEnableVertexAttribArray(0));
        GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(f32), (void*)0));
        GLCall(glEnableVertexAttribArray(1));
        GLCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(f32), (void*)(3 * sizeof(f32))));
        

        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));  
        GLCall(glBindVertexArray(0));
    }
    else {
        // each update, reupload the vertex data to the gpu since the line positions may have changed
        GLCall(glBindVertexArray(quadVAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(lineVerts), lineVerts, GL_DYNAMIC_DRAW));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));  
    }
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 model = glm::mat4(1); // specifying start/end pos in vertex data, don't need anything here
    glm::mat4 mvp = proj * view * model;
    shader.use();
    shader.setUniform("mvp", mvp);

    GLCall(glBindVertexArray(quadVAO));
    GLCall(glDrawArrays(GL_LINES, 0, 2));
    GLCall(glBindVertexArray(0));
}

} // namespace Shapes3D


namespace Shapes2D {
// always draws a quad, but uses shaders to derive desired shape
// all shapes have model/projection/color matrices that need to be set
// but different shapes can also set other uniforms if they need it



void DrawSquare(const glm::vec2& pos, const glm::vec2& size, 
            f32 rotation, const glm::vec3& rotationAxis, 
            const glm::vec4& color, bool isHollow) {
    SHAPE_SHADER(shader, "shaders/shapes/shape.vs", "shaders/shapes/square.fs");
    s32 hollow = isHollow ? 1 : 0;
    shader.use();
    shader.setUniform("isHollow", hollow);
    Shapes2D::DrawShape(pos, size, rotation, rotationAxis, color, shader);
}

void DrawCircle(const glm::vec2& pos, f32 radius, const glm::vec4& color, bool isHollow) {
    SHAPE_SHADER(shader, "shaders/shapes/shape.vs", "shaders/shapes/circle.fs");
    s32 hollow = isHollow ? 1 : 0;
    shader.use();
    shader.setUniform("isHollow", hollow);
    Shapes2D::DrawShape(pos, glm::vec2(radius, radius), 0.0, glm::vec3(0.0, 0.0, 1.0), color, shader);
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

    glm::mat4 projection = Camera::GetMainCamera().GetProjectionMatrix();

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

    Shapes2D::DrawShape(model, color, shader);
}

    
}