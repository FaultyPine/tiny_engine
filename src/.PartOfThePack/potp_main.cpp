#include "potp_main.h"

#include "tiny_engine/input.h"
#include "tiny_engine/tiny_audio.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/framebuffer.h"
#include "tiny_engine/tiny_fs.h"

#include "assassin_scene.h"
#include "controller_setup_scene.h"
#include "title_screen_scene.h"
//#include "tiny_engine/job_system.h"

#include "tiny_engine/external/imgui/tiny_imgui.h"

static const SceneInit initScenes[] = {
    NoSceneInit,
    ControllerSetupSceneInit,
    AssassinSceneInit,
    TitleScreenSceneInit
};
static const SceneTick tickScenes[] = {
    NoSceneTick,
    ControllerSetupSceneTick,
    AssassinSceneTick,
    TitleScreenSceneTick
};
static const SceneDraw drawScenes[] = {
    NoSceneDraw,
    ControllerSetupSceneDraw,
    AssassinSceneDraw,
    TitleScreenSceneDraw
};
static const SceneEnd endScenes[] = {
    NoSceneEnd,
    ControllerSetupSceneEnd,
    AssassinSceneEnd,
    TitleScreenSceneEnd
};

void ChangeScene(PotpScene newScene, GameState& gs) {
    endScenes[newScene](gs);
    gs.scene = newScene;
    initScenes[newScene](gs);
}

void PotpInit(GameState& gs, UserInput& inputs) {
    //JobSystem::Instance().Initialize();
    Camera& cam = Camera::GetMainCamera();
    SetMinAndMaxWindowSize(cam.GetMinScreenDimensions().x, cam.GetMinScreenDimensions().y, 
                            cam.GetMaxScreenDimensions().x, cam.GetMaxScreenDimensions().y);
    // manually init random seed so we can serialize it (with the rest of gamestate) if need be
    f64 time = GetTime();
    gs.initialRandomSeed = hash((const char*)&time, sizeof(f64));
    OverwriteRandomSeed(gs.initialRandomSeed);

    // SFX
    Audio::InitAudioEngine();
    #ifdef TINY_DEBUG
    Audio::SetMute(true);
    #else
    Audio::SetMute(false);
    #endif
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
        //initScenes[PotpScene::CONTROLLER_SETUP](gs);
        // using this for debugging to boot directly into gameplay
        initScenes[PotpScene::ASSASSIN](gs);
    }
    #else
    // init game to title screen
    initScenes[PotpScene::TITLE](gs);
    #endif
}


void PotpUpdate(GameState& gs, UserInput& inputs) {
    tickScenes[gs.scene](gs, inputs);
}

void PotpDraw(const GameState& gs, const UserInput& inputs) {
    drawScenes[gs.scene](gs, inputs);
}

void PotpTerminate(GameState& gs) {
    //JobSystem::Instance().Shutdown();
}



namespace Potp {

// TODO: alloc big mem block and use custom allocator

// putting the gamestate/inputs here since i don't want to initialize it in the
// main engine loop (since that would couple the engine to the game, which i want to try to avoid)

GameState gs = {};
UserInput inputs = {};

void MainInit() {
    PotpInit(gs, inputs);
    InitImGui();
}
void MainUpdate() {
    // poll inputs
    UserInput::UpdateUserInput(inputs);
    // update gamestate
    PotpUpdate(gs, inputs);
    // render gamestate

    static FullscreenFrameBuffer fb;
    // render scene to framebuffer and render that framebuffer to screen with postprocessing shader applied
    fb.DrawToScreen([](){
        PotpDraw(gs, inputs);
    });


    // debug imgui drawing
    #ifdef TINY_DEBUG
    ImGuiBeginFrame();
    //ImGui::ColorEdit4("Color", &gs.statues[0].entity.color);
    ImGuiEndFrame();
    #endif
}

void Terminate() {
    PotpTerminate(gs);
    ImGuiTerminate();
}

} // namespace Potp