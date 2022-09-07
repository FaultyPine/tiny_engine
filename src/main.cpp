#include "tiny_engine/pch.h"
#include "tiny_engine/tiny_engine.h"
#include "ObjParser.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) ( sizeof(arr) / sizeof(arr[0]) )
#endif

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;



void advanceGameSimulation(UserInput inputs, GameState& gs) {
    Camera& cam = gs.camera;
    f32 cameraSpeed = cam.speed * GetDeltaTime();
    if (inputs.isForward()) {
        cam.cameraPos += cameraSpeed * cam.cameraFront;
    }
    if (inputs.isBackward()) {
        cam.cameraPos -= cameraSpeed * cam.cameraFront;
    }
    if (inputs.isLeft()) {
        glm::vec3 cameraRight = glm::normalize(glm::cross(cam.cameraFront, cam.cameraUp));
        cam.cameraPos -= cameraRight * cameraSpeed;
    }
    if (inputs.isRight()) {
        glm::vec3 cameraRight = glm::normalize(glm::cross(cam.cameraFront, cam.cameraUp));
        cam.cameraPos += cameraRight * cameraSpeed;
    }
    if (inputs.isUp()) {
        cam.cameraPos += cam.cameraUp * cameraSpeed;
    }
    if (inputs.isDown()) {
        cam.cameraPos -= cam.cameraUp * cameraSpeed;
    }
}

void Set3DMatrixUniforms(Shader& shader, Camera& cam) {
    MouseInput& mouseInput = UserInput::GetMouse();

    cam.cameraFront = mouseInput.GetNormalizedLookDir();

    
    // identity matrix to start out with
    glm::mat4 model = glm::mat4(1.0f);
    // rotate it a little bit about x axis so it looks like it's laying on the floor
    //model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 

    glm::mat4 view = glm::mat4(1.0f);
    // note that we're translating the scene in the reverse direction of where we want to move
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f)); 

    // Camera
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // base cam pos
    const glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // target is world origin
    // dir is the normalized vector between the current position and it's target
    // note: this is actually pointing in the reverse direction of what it's targeting
    glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

    // get up/right vectors from camera's pov
    glm::vec3 upVec = glm::vec3(0.0f, 1.0f, 0.0f);
    // cross product between the up vec and cam facing direction vec  gives the right vec
    glm::vec3 cameraRight = glm::normalize(glm::cross(upVec, cameraDirection));
    // sooo.... camera up is just cross between the cam facing dir and the cam right dir
    glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

    // set view matrix (which is essentially our camera's transform)
    view = glm::lookAt(
        cam.cameraPos,
        cam.cameraPos + cam.cameraFront, cam.cameraUp
    );
    
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), 
        (f32)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

    shader.use();
    // set transform uniforms with the raw data of our matricies
    shader.setUniform("model", model);
    shader.setUniform("view", view);
    shader.setUniform("projection", projection);
}


void drawGameState(GameState& gs) {
    Mesh& mesh = gs.objs[0];
    Shader& shader = mesh.cachedShader;
    Set3DMatrixUniforms(shader, gs.camera);
    f32 time = (f32)GetTime();
    shader.setUniform("time", time);
    mesh.Draw();
}

void renderGame(GameState& gs) {
    #if 0
    // draw in wireframe polygons.
    EnableWireframeDrawing();
    #endif

    drawGameState(gs);
}


void initShadersAndVerts(GameState& gs) {
    // set up mesh/shader
    std::vector<Vertex> meshVerts = {};
    std::vector<u32> meshIndices = {};
    std::vector<Texture> meshTexs = {};
    std::vector<Material> materials = {};

    load_obj(UseResPath("warrior.obj").c_str(), UseResPath().c_str(), 
            meshVerts, meshIndices, materials);

    // Shader
    Shader shader = Shader(UseResPath("shaders/default.vs").c_str(), UseResPath("shaders/default.fs").c_str());
    
    // Texture stuff
    #if 0
    const char* imgPath = UseResPath("container.jpg").c_str();
    TextureProperties texProps;
    texProps.texWrapMode = TextureProperties::TexWrapMode::MIRRORED_REPEAT;
    texProps.minFilter = TextureProperties::TexMinFilter::LINEAR_MIPMAP_LINEAR;
    texProps.magFilter = TextureProperties::TexMagFilter::LINEAR;
    texProps.texFormat = TextureProperties::TexFormat::RGB;
    texProps.imgFormat = TextureProperties::ImageFormat::RGB;
    texProps.imgDataType = TextureProperties::ImageDataType::UNSIGNED_BYTE;
    Texture texture1 = LoadTexture(imgPath, texProps, TextureMaterialType::DIFFUSE, true);
    
    imgPath = UseResPath("awesomeface.png").c_str();
    texProps.texFormat = TextureProperties::TexFormat::RGBA;
    texProps.imgFormat = TextureProperties::ImageFormat::RGBA;
    Texture texture2 = LoadTexture(imgPath, texProps, TextureMaterialType::DIFFUSE, true);
    
    meshTexs.push_back(texture1);
    meshTexs.push_back(texture2);
    #endif

    gs.objs[0] = Mesh(shader, meshVerts, meshIndices, meshTexs);

}


int main(int argc, char *argv[]) {
    InitWindow(SCR_WIDTH, SCR_HEIGHT, "Tiny Engine"); 

    GameState gs = {};
    initShadersAndVerts(gs);

    while(!ShouldCloseWindow())
    {
        // poll inputs from os layer
        UserInput inputs = GetUserInput();
        // pass inputs into game
        advanceGameSimulation(inputs, gs);
        // render new state of the game
        renderGame(gs); 
    }

    TerminateGame(gs);
    return 0;
}