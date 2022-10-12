#include "potp_main.h"

#include "tiny_engine/input.h"
#include "tiny_engine/tiny_audio.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/framebuffer.h"
#include "tiny_engine/tiny_text.h"
#include "tiny_engine/tiny_fs.h"
#include "tiny_engine/shapes.h"

// this is the number of actual players in the session
const u32 numPlayers = 2; // TEMP, FOR DEBUGGING
static_assert(numPlayers <= MAX_NUM_PLAYERS);

void PotpInit(GameState& gs) {
    // manually init random seed so we can serialize it (with the rest of gamestate) if need be
    gs.initialRandomSeed = GetCPUCycles();
    InitializeRandomSeed(gs.initialRandomSeed);

    Audio::SetMute(true);

    // SFX
    Audio::InitAudioEngine();
    Audio::PlayAudio(UseResPath("potp/clarkPapple_Song.mp3").c_str());

    // textures
    TextureProperties texProps = TextureProperties::Default();
    texProps.imgFormat = TextureProperties::ImageFormat::RGB;
    
    Texture backgroundTex = LoadTexture(UseResPath("potp/background.jpg").c_str(), texProps);
    // Sprites
    gs.background = Sprite(backgroundTex);

    // statues
    u32 screenWidth = Camera::GetMainCamera().screenWidth;
    u32 screenHeight = Camera::GetMainCamera().screenHeight;
    f32 screenMargin = 80.0;
    glm::vec2 statuePositions[NUM_STATUES] = {
        glm::vec2(screenMargin, screenMargin),
        glm::vec2(screenWidth-screenMargin, screenMargin),
        glm::vec2(screenMargin, screenHeight-screenMargin),
        glm::vec2(screenWidth-screenMargin, screenHeight-screenMargin),
        glm::vec2(screenWidth/2, screenHeight/2)
    };
    for (u32 i = 0; i < NUM_STATUES; i++) {
        gs.statues[i].Initialize(statuePositions[i]);
    }

    // ninjas
    InitializeNinjas(gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, numPlayers);
}

void PotpUpdate(GameState& gs, UserInput inputs) {
    if (GetFrameCount() == 120) {
        Audio::PlayAudio(UseResPath("potp/hit.wav").c_str());
    }

    UpdateNinjas(inputs, gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, numPlayers);
}

// TODO: move this to seperate file
void DrawUI(const GameState& gs) {
    static Sprite keyboardSprite;
    if (!keyboardSprite.isValid()) keyboardSprite = Sprite(LoadTexture(UseResPath("potp/keyboard.png").c_str(), TextureProperties::Default()));
    static Sprite controllerSprite;
    if (!controllerSprite.isValid()) controllerSprite = Sprite(LoadTexture(UseResPath("potp/controller.png").c_str(), TextureProperties::Default()));
    
    // draw player indicators at bottom of screen
    // "player indicators" here refers to the UI that shows what players are ingame (1-4) and if they are using keyboard/controller
    const glm::vec2 playerIndicatorsSize = {30.0, 30.0};
    const f32 playerIndicatorsY = (f32)Camera::GetScreenHeight() - playerIndicatorsSize.y - 20.0;
    const f32 playerIndicatorsStartX = (f32)Camera::GetScreenWidth()/6.0;
    const f32 playerIndicatorsXSpacing = playerIndicatorsSize.x + (f32)Camera::GetScreenWidth()/8.0;
    const f32 indicatorAlpha = 0.6;

    // p1
    glm::vec2 p1Pos = {playerIndicatorsStartX + playerIndicatorsXSpacing * 0, playerIndicatorsY};
    Shapes::DrawSquare(p1Pos, playerIndicatorsSize, 0.0, {0.0,0.0,1.0}, {1.0, 0.0, 0.0, indicatorAlpha});
    keyboardSprite.DrawSprite(Camera::GetMainCamera(), p1Pos, playerIndicatorsSize, 0.0, {0.0,0.0,indicatorAlpha});
    // p2
    glm::vec2 p2Pos = {playerIndicatorsStartX + playerIndicatorsXSpacing * 1, playerIndicatorsY};
    Shapes::DrawSquare(p2Pos, playerIndicatorsSize, 0.0, {0.0,0.0,1.0}, {0.0, 0.0, 1.0, indicatorAlpha});
    keyboardSprite.DrawSprite(Camera::GetMainCamera(), p2Pos, playerIndicatorsSize, 0.0, {0.0,0.0,indicatorAlpha});
    // p3
    glm::vec2 p3Pos = {playerIndicatorsStartX + playerIndicatorsXSpacing * 2, playerIndicatorsY};
    Shapes::DrawSquare(p3Pos, playerIndicatorsSize, 0.0, {0.0,0.0,1.0}, {0.0, 1.0, 0.0, indicatorAlpha});
    controllerSprite.DrawSprite(Camera::GetMainCamera(), p3Pos, playerIndicatorsSize, 0.0, {0.0,0.0,indicatorAlpha});
    // p4
    glm::vec2 p4Pos = {playerIndicatorsStartX + playerIndicatorsXSpacing * 3, playerIndicatorsY};
    Shapes::DrawSquare(p4Pos, playerIndicatorsSize, 0.0, {0.0,0.0,1.0}, {1.0, 1.0, 0.0, indicatorAlpha});
    controllerSprite.DrawSprite(Camera::GetMainCamera(), p4Pos, playerIndicatorsSize, 0.0, {0.0,0.0,indicatorAlpha});
}


void PotpDraw(const GameState& gs) {
    const Camera& cam = Camera::GetMainCamera();
    gs.background.DrawSprite(cam, glm::vec2(0.0f, 0.0f), glm::vec2(cam.screenWidth, cam.screenHeight));

    // statues
    for (const Statue& statue : gs.statues) {
        statue.Draw();
    }

    DrawNinjas(gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, numPlayers);
    
    DrawUI(gs);
}




namespace Potp {

// TODO: alloc big mem block and use custom allocator
GameState gs = {};

void MainInit() {
    PotpInit(gs);
}
void MainUpdate(UserInput inputs) {
    PotpUpdate(gs, inputs);
}
void MainDraw() {
    PotpDraw(gs);
}

} // namespace Potp