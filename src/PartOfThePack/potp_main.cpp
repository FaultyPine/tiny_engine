#include "potp_main.h"

#include "tiny_engine/input.h"
#include "tiny_engine/tiny_audio.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/framebuffer.h"
#include "tiny_engine/tiny_fs.h"

#include "assassin_scene.h"
#include "controller_setup_scene.h"
#include "title_screen_scene.h"


void PotpInit(GameState& gs, UserInput& inputs) {
    Camera& cam = Camera::GetMainCamera();
    SetMinAndMaxWindowSize(cam.GetMinScreenDimensions().x, cam.GetMinScreenDimensions().y, 
                            cam.GetMaxScreenDimensions().x, cam.GetMaxScreenDimensions().y);
    // manually init random seed so we can serialize it (with the rest of gamestate) if need be
    f64 time = GetTime();
    gs.initialRandomSeed = hash((const char*)&time, sizeof(f64));
    InitializeRandomSeed(gs.initialRandomSeed);

    // SFX
    Audio::InitAudioEngine();
    Audio::SetMute(true);
    Audio::PlayAudio(UseResPath("potp/clarkPapple_Song.mp3").c_str());

    //Audio::SetVolume(2.0);

    // textures
    TextureProperties texProps = TextureProperties::Default();
    texProps.imgFormat = TextureProperties::ImageFormat::RGB;
    texProps.magFilter = TextureProperties::TexMagFilter::NEAREST;
    texProps.minFilter = TextureProperties::TexMinFilter::NEAREST;
    
    Texture backgroundTex = LoadTexture(UseResPath("potp/background.jpg").c_str(), texProps);
    // Sprites
    gs.background = Sprite(backgroundTex);

    #if 0
    { // for debugging, init directly to other scenes
        #ifdef TINY_DEBUG 
        // This is for debugging - when I boot directly into the gameplay scene without controller setup
        // just setup controls with two keyboard players by default
        if (gs.numPlayers < 1) {
            std::cout << "[WARNING] Num players was less than 1, setting to 2 keyboards by default.\n";
            gs.numPlayers = 2;
            for (s32 i = 0; i < gs.numPlayers; i++) {
                inputs.controllers[i].type = ControllerType::KEYBOARD;
                inputs.controllers[i].port = i;
            }
        }
        #endif
        //ControllerSetupSceneInit(gs);
        AssassinSceneInit(gs); // using this for debugging to boot directly into gameplay
    }
    #else
    // init game to title screen
    TitleScreenSceneInit(gs);
    #endif
}


void PotpUpdate(GameState& gs, UserInput& inputs) {
    if (gs.scene == PotpScene::CONTROLLER_SETUP) {
        ControllerSetupSceneTick(gs, inputs);
    }
    else if (gs.scene == PotpScene::ASSASSIN) {
        AssassinSceneTick(gs, inputs);
    }
    else if (gs.scene == PotpScene::TITLE) {
        TitleScreenSceneTick(gs, inputs);
    }
}



void PotpDraw(const GameState& gs, const UserInput& inputs) {
    const Camera& cam = Camera::GetMainCamera();

    switch (gs.scene) {

        case PotpScene::NO_SCENE:
        {

        } break;
        case PotpScene::CONTROLLER_SETUP:
        {
            ControllerSetupSceneDraw(gs, inputs);
        } break;
        case PotpScene::ASSASSIN:
        {
            AssassinSceneDraw(gs, inputs);
        } break;
        case PotpScene::TITLE:
        {
            TitleScreenSceneDraw(gs, inputs);
        } break;
    }
}




namespace Potp {

// TODO: alloc big mem block and use custom allocator

// putting the gamestate/inputs here since i don't want to initialize it in the
// main engine loop (since that would couple the engine to the game, which i want to try to avoid)

GameState gs = {};
UserInput inputs = {};

void MainInit() {
    PotpInit(gs, inputs);
}
void MainUpdate() {
    // poll inputs
    UserInput::UpdateUserInput(inputs);
    // update gamestate
    PotpUpdate(gs, inputs);
    // render gamestate

    // render game to framebuffer
    glm::vec2 screenDimensions = {Camera::GetScreenWidth(), Camera::GetScreenHeight()};
    static FullscreenFrameBuffer fb;
    static Shader postProcessingShader;
    if (!postProcessingShader.isValid()) {
        postProcessingShader = Shader(UseResPath("shaders/screen_texture.vs").c_str(), UseResPath("shaders/screen_texture.fs").c_str());
    }
    if (!fb.isValid()) {
        fb = FullscreenFrameBuffer(postProcessingShader, {screenDimensions.x, screenDimensions.y});
    }

    if (fb.GetSize().x != screenDimensions.x && fb.GetSize().y != screenDimensions.y) {
        fb.Delete();
        fb = FullscreenFrameBuffer(postProcessingShader, screenDimensions);
    }
    fb.Bind();
    ClearGLColorBuffer();
    PotpDraw(gs, inputs);

    // draw framebuffer to screen with post processing shader
    fb.DrawToScreen();
}

} // namespace Potp