#include "potp_main.h"

#include "tiny_engine/input.h"
#include "tiny_engine/tiny_audio.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/framebuffer.h"
#include "tiny_engine/tiny_text.h"

void PotpInit(GameState& gs) {
    Audio::SetMute(true);

    // SFX
    Audio::InitAudioEngine();
    Audio::PlayAudio(UseResPath("potp/backgroundMusic.mp3").c_str());

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
    InitializeNinjas(gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, MAX_NUM_PLAYER_NINJAS);
}

void PotpUpdate(GameState& gs, UserInput inputs) {
    if (GetFrameCount() == 120) {
        Audio::PlayAudio(UseResPath("potp/hit.wav").c_str());
    }

    UpdateNinjas(inputs, gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, MAX_NUM_PLAYER_NINJAS);
}

void PotpDraw(const GameState& gs) {
    const Camera& cam = Camera::GetMainCamera();
    gs.background.DrawSprite(cam, glm::vec2(0.0f, 0.0f), glm::vec2(cam.screenWidth, cam.screenHeight));

    // statues
    for (const Statue& statue : gs.statues) {
        statue.Draw();
    }

    DrawNinjas(gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, MAX_NUM_PLAYER_NINJAS);
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