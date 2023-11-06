//#include "pch.h"
#include "sprite.h"
#include "tiny_fs.h"
#include "math/tiny_math.h"
#include "camera.h"
#include "tiny_ogl.h"

Sprite::Sprite(const Texture& mainTex) {
    this->mainTex = mainTex;
    this->shader = Shader(ResPath("shaders/default_sprite.vert"), ResPath("shaders/default_sprite.frag"));
    initRenderData();
}
Sprite::Sprite(const Shader& shader, const Texture& mainTex) {
    this->mainTex = mainTex;
    this->shader = shader;
    initRenderData();
}
void Sprite::Delete() {
    glDeleteVertexArrays(1, &quadVAO);
    shader.Delete();
    mainTex.Delete();
}

template<typename T>
void Sprite::setShaderUniform(const char* name, T val) const {
    shader.setUniform(name, val);
}


void Sprite::DrawSprite(const Transform2D& tf, glm::vec4 color, bool shouldFlipY) const {
    DrawSprite(tf.position, tf.scale, tf.rotation, {0,0,1}, color, shouldFlipY);
}
void Sprite::DrawSprite(glm::vec2 position, glm::vec2 size, f32 rotate, glm::vec3 rotationAxis, glm::vec4 color, bool shouldFlipY) const {
    TINY_ASSERT(isValid() && "Tried to draw invalid sprite!");

    // set up transform of the actual sprite
    Camera& cam = Camera::GetMainCamera();
    glm::mat4 model = Math::Position2DToModelMat(position, size, rotate, rotationAxis);
    glm::mat4 projection = cam.GetOrthographicProjection();
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, cam.cameraPos);


    shader.use();
    shader.setUniform("model", model);
    shader.setUniform("color", color);
    shader.setUniform("projection", projection);
    shader.setUniform("view", view);
    // Texture unit indexes are bound to samplers, not texture objects.
    // https://stackoverflow.com/questions/46122353/opengl-texture-is-all-black-when-rendered-with-shader
    //shader.setUniform("mainTex", 0);
    shader.setUniform("shouldFlipY", shouldFlipY);
    shader.TryAddSampler(mainTex.id, "mainTex");
    shader.ActivateSamplers();
    //Texture::activate(0); // not technically necessary since we're just using one texture in sprite frag shader (unit 0 is activated by default)
    //mainTex.bind();

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Sprite::DrawSpriteFullscreen(glm::vec4 color) const
{
    DrawSprite(glm::vec2(0), glm::vec2(Camera::GetScreenWidth(), Camera::GetScreenHeight()), true);
}

void Sprite::initRenderData() {
    // configure VAO/VBO
    u32 VBO;

    // one vert attribute with first two components being 2d pos
    // and second 2 components being texcoords
    static const f32 tex_quad[] = { 
        // top left is 0,0  bottom right is 1,1
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    // generate ogl buffers on gpu
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &VBO);
    
    // bind vbo to the ARRAY_BUFFER buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // put vert data into whatever buffer is bound to ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(tex_quad), tex_quad, GL_STATIC_DRAW);

    // using this vert attribute object, tell ogl how to parse vert data
    glBindVertexArray(quadVAO);
    // vec2 position, vec2 texcoords, all contained in a vec4
    glEnableVertexAttribArray(0);
    // inherently binds these options to the whatever VBO is bound to GL_ARRAY_BUFFER
    // since this inherently binds to our VBO above, our "quadVAO" also has some sort of reference to this VBO
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)0);
    
    // unbind once done
    glBindBuffer(GL_ARRAY_BUFFER, 0);  
    glBindVertexArray(0);
}