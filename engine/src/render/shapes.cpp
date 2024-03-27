//#include "pch.h"
#include "shapes.h"

#include "camera.h"
#include "texture.h"
#include "shader.h"
#include "tiny_fs.h"
#include "math/tiny_math.h"
#include "tiny_ogl.h"
#include "tiny_renderer.h"

#define PAR_SHAPES_IMPLEMENTATION
#include "par/par_shapes.h"
#undef PAR_SHAPES_IMPLEMENTATION

// shader var is just called "shader" after this macro
#define SHAPE_SHADER(shaderName, vertShaderPath, fragShaderPath) \
static Shader shaderName; \
if (!shaderName.isValid()) \
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
            Vertex v = {};
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

    return Mesh(planeverts, indices, GetDummyMaterial(), "GeneratedPlane");
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
    glm::mat4 model = tf.ToModelMatrix();
    shader.setUniform("modelMat", model);
    shader.setUniform("color", color);
    shader.use();
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

}

void DrawSphere(glm::vec3 center, f32 radius, glm::vec4 color)
{
    SHAPE_SHADER(shader, "shaders/shapes/shape_3d.vert", "shaders/shapes/default_3d.frag");
    static Mesh globalSphereMesh;
    if (!globalSphereMesh.isValid())
    {
        globalSphereMesh = GenSphereMesh(8);
    }
    glm::mat4 model = Math::Position3DToModelMat(center, glm::vec3(radius));
    shader.setUniform("color", color);
    shader.setUniform("modelMat", model);
    shader.use();
    globalSphereMesh.Draw();
}
void DrawWireSphere(glm::vec3 center, f32 radius, glm::vec4 color)
{
    SetWireframeDrawing(true);
    DrawSphere(center, radius, color);
    SetWireframeDrawing(false);
}

Mesh GenSphereMesh(u32 resolution)
{
    f32 radius = 1.0f;
    u32 stackCount = resolution;
    u32 sectorCount = resolution;
    std::vector<Vertex> vertices = {};
    std::vector<u32> indices = {};
    vertices.reserve(stackCount * sectorCount);
    indices.reserve(stackCount * sectorCount);

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi
            Vertex v = {};
            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            glm::vec3 vertexPosition = glm::vec3(x,y,z);
            v.position = vertexPosition;

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            glm::vec3 normal = glm::vec3(nx, ny, nz);
            v.normal = normal;

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            glm::vec2 texcoord = glm::vec2(s,t);
            v.texCoords = texcoord;

            vertices.push_back(v);
        }
    }

    u32 k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if(i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if(i != (stackCount-1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
    vertices.shrink_to_fit();
    indices.shrink_to_fit();
    return Mesh(vertices, indices);
}

void DrawWireCube(BoundingBox box, const glm::vec4& color)
{
    /*SetWireframeDrawing(true);
    glm::vec3 center = box.min + ((box.max - box.min) / 2.0f);
    glm::vec3 scale = box.max - box.min;
    DrawCube(Transform(center, scale));
    SetWireframeDrawing(false);*/
    glm::vec3 min = box.min;
    glm::vec3 max = box.max;

    Renderer::PushLine(box.min, glm::vec3(max.x, min.y, min.z), color); // min-x
    Renderer::PushLine(box.min, glm::vec3(min.x, max.y, min.z), color); // min-y
    Renderer::PushLine(box.min, glm::vec3(min.x, min.y, max.z), color); // min-z

    Renderer::PushLine(box.max, glm::vec3(min.x, max.y, max.z), color); // max-x
    Renderer::PushLine(box.max, glm::vec3(max.x, min.y, max.z), color); // max-y
    Renderer::PushLine(box.max, glm::vec3(max.x, max.y, min.z), color); // max-z

    Renderer::PushLine(glm::vec3(min.x, max.y, min.z), glm::vec3(min.x, max.y, max.z), color); // min-y -> max-x
    Renderer::PushLine(glm::vec3(min.x, min.y, max.z), glm::vec3(min.x, max.y, max.z), color); // min-z -> max-x
    
    Renderer::PushLine(glm::vec3(min.x, max.y, min.z), glm::vec3(max.x, max.y, min.z), color); // min-y -> max-z
    Renderer::PushLine(glm::vec3(max.x, min.y, min.z), glm::vec3(max.x, max.y, min.z), color); // min-x -> max-z
    
    Renderer::PushLine(glm::vec3(max.x, min.y, max.z), glm::vec3(max.x, min.y, min.z), color); // max-y -> min-x
    Renderer::PushLine(glm::vec3(max.x, min.y, max.z), glm::vec3(min.x, min.y, max.z), color); // max-y -> min-z
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
    glm::mat4 proj = Camera::GetMainCamera().GetProjectionMatrix();
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 model = tf.ToModelMatrix();
    glm::mat4 mvp = proj * view * model;
    shader.setUniform("mvp", mvp);
    shader.setUniform("color", color);
    shader.use();
    // render Cube 
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void DrawWirePlane(const Transform& tf, const glm::vec4& color)
{
    SetWireframeDrawing(true);
    DrawPlane(tf, color);
    SetWireframeDrawing(false);
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
    shader.setUniform("isHollow", hollow);
    Shapes2D::DrawShape(pos, size, rotation, rotationAxis, color, shader);
}

void DrawCircle(
    const glm::vec2& pos, 
    f32 radius, 
    const glm::vec4& color, 
    bool isHollow, 
    f32 outlineThickness) 
{
    SHAPE_SHADER(shader, "shaders/shapes/shape.vert", "shaders/shapes/circle.frag");
    s32 hollow = isHollow ? 1 : 0;
    shader.setUniform("isHollow", hollow);
    shader.setUniform("circleThickness", outlineThickness);
    Shapes2D::DrawShape(pos, glm::vec2(radius, radius), 0.0, glm::vec3(0.0, 0.0, 1.0), color, shader);
}

// to draw vector shapes, just draw a quad and then use the shader to actually derive the shape
void DrawShape(const glm::mat4& model, const glm::vec4& color, const Shader& shader) {
    static u32 quadVAO = 0;
    if (quadVAO == 0) { // only need to init vert data once
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

    shader.setUniform("model", model);
    shader.setUniform("projection", projection);
    shader.setUniform("color", color);
    shader.use();

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


void DrawWireframeSquare(const glm::vec2& start, const glm::vec2& end, glm::vec4 color) {
    Renderer::PushLine(glm::vec3(start, 0.0), glm::vec3(end.x, start.y, 0.0), color);
    Renderer::PushLine(glm::vec3(start, 0.0), glm::vec3(start.x, end.y, 0.0), color);

    Renderer::PushLine(glm::vec3(end, 0.0), glm::vec3(end.x, start.y, 0.0), color);
    Renderer::PushLine(glm::vec3(end, 0.0), glm::vec3(start.x, end.y, 0.0), color);
}

    
}