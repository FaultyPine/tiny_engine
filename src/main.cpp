#include "tiny_engine/pch.h"
#include "tiny_engine/tiny_engine.h"
#include "ObjParser.h"

#include "spaceshooter/spaceshooter.h"


void FPS3DCamMovement(UserInput& inputs, Camera& cam) {
    // 3D fps cam movement
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


void drawTestMesh(Mesh& mesh) {
    // render 3d mesh
    Shader& shader = mesh.cachedShader;
    Camera& cam = Camera::GetMainCamera();;

    MouseInput& mouseInput = UserInput::GetMouse();
    cam.cameraFront = mouseInput.GetNormalizedLookDir();

    // identity matrix to start out with
    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 view = glm::mat4(1.0f);
    // note that we're translating the scene in the reverse direction of where we want to move
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f)); 

    // set view matrix (which is essentially our camera's transform)
    view = cam.GetViewMatrix();
    
    glm::mat4 projection = cam.GetProjectionMatrix();

    shader.use();
    // set transform uniforms with the raw data of our matricies
    shader.setUniform("model", model);
    shader.setUniform("view", view);
    shader.setUniform("projection", projection);

    f32 time = (f32)GetTime();
    shader.setUniform("time", time);
    mesh.Draw();
}



void initTestMesh(Mesh& mesh) {
    // set up mesh
    std::vector<Vertex> meshVerts = {};
    std::vector<u32> meshIndices = {};
    std::vector<Texture> meshTexs = {};
    std::vector<Material> materials = {};

    load_obj(UseResPath("warrior.obj").c_str(), UseResPath().c_str(), 
            meshVerts, meshIndices, materials);

    // Shader
    Shader shader = Shader(UseResPath("shaders/default_3d.vs").c_str(), UseResPath("shaders/default_3d.fs").c_str());
    
    // Texture stuff
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

    mesh = Mesh(shader, meshVerts, meshIndices, meshTexs);
}


Mesh testMesh = {};


void preLoopInit(GameState& gs) {
    InitGame(800, 600, "Tiny Engine"); 

    #if 0
    initTestMesh(testMesh);
    #endif
    
    #if 1
    Spaceshooter::initSpaceshooter(gs);
    #endif
}

void renderGame(GameState& gs) {
    #if 0
    // draw in wireframe polygons.
    EnableWireframeDrawing();
    #endif

    #if 0
    drawTestMesh(testMesh);
    #endif

    #if 1
    Spaceshooter::drawSpaceshooter(gs);
    #endif
}


void advanceGameSimulation(UserInput inputs, GameState& gs) {
    Camera& cam = gs.camera;
    #if 0
    FPS3DCamMovement(inputs, cam);
    #endif

    #if 1
    Spaceshooter::updateSpaceshooterGame(gs, inputs);
    #endif
}

int main(int argc, char *argv[]) {

    GameState& gs = GameState::GetGameState();

    preLoopInit(gs);

    while(!ShouldCloseWindow())
    {
        // poll inputs from os layer
        UserInput inputs = GetUserInput();
        // pass inputs into game
        advanceGameSimulation(inputs, gs);
        // render new state of the game
        renderGame(gs); 
    }

    TerminateGame();
    return 0;
}