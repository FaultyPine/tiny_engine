#include "sprite.h"
#include "tiny_engine/tiny_fs.h"
#include "tiny_engine/math.h"

Sprite::Sprite(const Texture& mainTex) {
    this->mainTex = mainTex;
    this->shader = Shader(ResPath("shaders/default_sprite.vs"), ResPath("shaders/default_sprite.fs"));
    initRenderData();
}
Sprite::Sprite(const Shader& shader, const Texture& mainTex) {
    this->mainTex = mainTex;
    this->shader = shader;
    initRenderData();
}

void AdjustDimensionsForSceenSize(glm::vec2& position, glm::vec2& size) {
    const Camera& cam = Camera::GetMainCamera();
    glm::vec2 screenMin = cam.GetMinScreenDimensions();
    size.x = Math::Remap(size.x, 0, screenMin.x, 0, cam.GetScreenWidth()) /1.1;
    size.y = Math::Remap(size.y, 0, screenMin.y, 0, cam.GetScreenHeight()) /1.1;
    position.x = Math::Remap(position.x, 0, screenMin.x, 0, cam.GetScreenWidth());
    position.y = Math::Remap(position.y, 0, screenMin.y, 0, cam.GetScreenHeight());
}


void Sprite::DrawSprite(const Camera& cam, glm::vec2 position, 
                glm::vec2 size, f32 rotate, glm::vec3 rotationAxis, glm::vec4 color, bool adjustToScreensize,
                bool shouldFlipY) const {
    ASSERT(isValid() && "Tried to draw invalid sprite!\n");

    if (adjustToScreensize) {
        AdjustDimensionsForSceenSize(position, size);
    }

    // set up transform of the actual sprite
    glm::mat4 model = Math::Position2DToModelMat(position, size, rotate, rotationAxis);
    glm::mat4 projection = cam.GetProjectionMatrix();
    
    shader.use();
    shader.setUniform("model", model);
    shader.setUniform("color", color);
    shader.setUniform("projection", projection);
    // Texture unit indexes are bound to samplers, not texture objects.
    // https://stackoverflow.com/questions/46122353/opengl-texture-is-all-black-when-rendered-with-shader
    shader.setUniform("mainTex", 0);
    shader.setUniform("shouldFlipY", shouldFlipY);

    Texture::activate(0); // not technically necessary since we're just using one texture in sprite frag shader (unit 0 is activated by default)
    mainTex.bind();

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
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