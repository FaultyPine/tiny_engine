#include "pch.h"
#include "shapes.h"

#include "camera.h"
#include "texture.h"
#include "shader.h"
#include "tiny_fs.h"
#include "math.h"

// shader var is just called "shader" after this macro
#define SHAPE_SHADER(shaderName, vertShaderPath, fragShaderPath) \
static Shader shaderName; \
if (shaderName.ID == 0) \
    shaderName = Shader(ResPath(vertShaderPath), ResPath(fragShaderPath))


namespace Shapes3D {

const static f32 cubeVertices[] = {
    // position (v3), normal (v3), texcoords (v2)
    // back face
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
    1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
    1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
    1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
    // front face
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
    1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
    1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
    1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
    // left face
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
    // right face
    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
    1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
    1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
    // bottom face
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
    1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
    1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
    1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
    // top face
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
    1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
    1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
    1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
};
// this scales from the center out
const static f32 planeVerticesCentered[] = {
    // positions            // normals         // texcoords
    1.0f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
    -1.0f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
    -1.0f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,

    1.0f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
    -1.0f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
    1.0f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
};
const static f32 planeVerticesTopLeft[] = {
    // positions            // normals         // texcoords
    1.0f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
    0.0f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,

    1.0f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
    1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
};


Mesh GenCubeMesh() {
    std::vector<Vertex> cubeverts = {};
    for (u32 i = 0; i < ARRAY_SIZE(cubeVertices); i += 8) {
        const f32* vertexData = &cubeVertices[i];
        Vertex v;
        v.position = {vertexData[0], vertexData[1], vertexData[2] };
        v.normal = {vertexData[3], vertexData[4], vertexData[5]};
        v.texCoords = {vertexData[6], vertexData[7]};
        cubeverts.push_back(v);
    }
    return Mesh(cubeverts, {}, {});
}
Mesh GenPlaneMesh(u32 resolution) {
    resolution++; // resolution of 1 should really be 2
    std::vector<Vertex> planeverts = {};

    // https://github.com/raysan5/raylib/blob/master/src/rmodels.c#L2171
    for (s32 z = 0; z < resolution; z++) {
        // [-length/2, length/2]
        f32 zPos = ((f32)z/(resolution - 1) - 0.5f);
        for (s32 x = 0; x < resolution; x++) {
            // [-width/2, width/2]
            f32 xPos = ((f32)x/(resolution - 1) - 0.5f);
            Vertex v;
            v.normal = {0,1,0};
            v.texCoords = {xPos, zPos};
            v.position = {xPos, 0, zPos};
            planeverts.push_back(v);
        }
    }

    s32 numFaces = (resolution - 1)*(resolution - 1);
    std::vector<u32> indices = {};
    for (s32 face = 0; face < numFaces; face++) {
        // Retrieve lower left corner from face ind
        s32 i = face % (resolution - 1) + (face/(resolution - 1)*resolution);

        indices.push_back(i + resolution);
        indices.push_back(i + 1);
        indices.push_back(i);

        indices.push_back(i + resolution);
        indices.push_back(i + resolution + 1);
        indices.push_back(i + 1);
    }

    return Mesh(planeverts, indices, {}, "GeneratedPlane");
}

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
        //GLCall(glBindVertexArray(quadVAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        // copy into gpu vertex buffer with offset 0
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineVerts), lineVerts));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));  
    }
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 model = glm::mat4(1); // specifying start/end pos in vertex data, don't need anything here
    glm::mat4 mvp = proj * view * model;
    shader.use();
    shader.setUniform("mvp", mvp);

    GLCall(glBindVertexArray(quadVAO));
    glLineWidth(width);
    GLCall(glDrawArrays(GL_LINES, 0, 2));
    glLineWidth(1.0);
    GLCall(glBindVertexArray(0));
}

void DrawCube(const Transform& tf, const glm::vec4& color) {
    SHAPE_SHADER(shader, "shaders/shapes/shape_3d.vert", "shaders/shapes/default_3d.frag");
    static u32 cubeVAO = 0;
    static u32 cubeVBO = 0;
    // initialize (if necessary)
    if (cubeVAO == 0) {
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    shader.use();
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 model = tf.ToModelMatrix();
    glm::mat4 mvp = proj * view * model;
    shader.setUniform("mvp", mvp);
    shader.setUniform("color", color);
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

}

void DrawPlane(const Transform& tf, const glm::vec4& color) {
    SHAPE_SHADER(shader, "shaders/shapes/shape_3d.vert", "shaders/shapes/default_3d.frag");
    static u32 planeVAO = 0;
    static u32 planeVBO = 0;
    if (planeVAO == 0) {
        glGenVertexArrays(1, &planeVAO);
        glGenBuffers(1, &planeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerticesCentered), planeVerticesCentered, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(planeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    shader.use();
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 model = tf.ToModelMatrix();
    glm::mat4 mvp = proj * view * model;
    shader.setUniform("mvp", mvp);
    shader.setUniform("color", color);
    // render Cube
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


} // namespace Shapes3D


namespace Shapes2D {
// always draws a quad, but uses shaders to derive desired shape
// all shapes have model/projection/color matrices that need to be set
// but different shapes can also set other uniforms if they need it



void DrawSquare(const glm::vec2& pos, const glm::vec2& size, 
            f32 rotation, const glm::vec3& rotationAxis, 
            const glm::vec4& color, bool isHollow) {
    SHAPE_SHADER(shader, "shaders/shapes/shape.vert", "shaders/shapes/square.frag");
    s32 hollow = isHollow ? 1 : 0;
    shader.use();
    shader.setUniform("isHollow", hollow);
    Shapes2D::DrawShape(pos, size, rotation, rotationAxis, color, shader);
}

void DrawCircle(const glm::vec2& pos, f32 radius, const glm::vec4& color, bool isHollow) {
    SHAPE_SHADER(shader, "shaders/shapes/shape.vert", "shaders/shapes/circle.frag");
    s32 hollow = isHollow ? 1 : 0;
    shader.use();
    shader.setUniform("isHollow", hollow);
    Shapes2D::DrawShape(pos, glm::vec2(radius, radius), 0.0, glm::vec3(0.0, 0.0, 1.0), color, shader);
}

// to draw vector shapes, just draw a quad and then use the shader to actually derive the shape
void DrawShape(const glm::mat4& model, const glm::vec4& color, const Shader& shader) {
    static u32 quadVAO = 0;
    if (quadVAO == 0) { // only need to init vert data once
        /*static const f32 tex_quad[] = { 
            // pos      // tex
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 

            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f
        };*/
        static const f32 tex_quad[] = { 
            // pos      // tex
            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 

            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f
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
    glm::mat4 model = Math::Position2DToModelMat(pos, size, rotationDegrees, rotationAxis);
    DrawShape(model, color, shader);
}


void DrawLine(const glm::vec2& origin, const glm::vec2& dest, const glm::vec4& color, f32 width) {
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
        origin.x, origin.y, 0.0f, // vert pos
        color.r, color.g, color.b, color.a, // vert col

        dest.x,   dest.y,   0.0f,
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
        //GLCall(glBindVertexArray(quadVAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        // copy into gpu vertex buffer with offset 0
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineVerts), lineVerts));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));  
    }
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = glm::mat4(1);
    glm::mat4 model = glm::mat4(1); // specifying start/end pos in vertex data, don't need anything here
    glm::mat4 mvp = proj * view * model;
    shader.use();
    shader.setUniform("mvp", mvp);

    GLCall(glBindVertexArray(quadVAO));
    glLineWidth(width);
    GLCall(glDrawArrays(GL_LINES, 0, 2));
    glLineWidth(1.0);
    GLCall(glBindVertexArray(0));
}

void DrawWireframeSquare(const glm::vec2& start, const glm::vec2& end, glm::vec4 color, f32 width) {
    Shapes2D::DrawLine(start, glm::vec2(end.x, start.y), color, width);
    Shapes2D::DrawLine(start, glm::vec2(start.x, end.y), color, width);

    Shapes2D::DrawLine(end, glm::vec2(end.x, start.y), color, width);
    Shapes2D::DrawLine(end, glm::vec2(start.x, end.y), color, width);
}

    
}