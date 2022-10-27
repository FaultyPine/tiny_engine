#include "controller_setup_scene.h"

#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/shapes.h"
#include "assassin_scene.h"

void ControllerSetupSceneInit(GameState& gs) {
    gs.scene = PotpScene::CONTROLLER_SETUP;
    gs.allReadyCountdown = ASSASSIN_ALL_READY_COUNTDOWN_FRAMES;

    TextureProperties texProps = TextureProperties::Default();
    texProps.magFilter = TextureProperties::TexMagFilter::NEAREST;
    texProps.minFilter = TextureProperties::TexMinFilter::NEAREST;
    // init keyboard/controller sprites
    if (!gs.keyboardSprite.isValid()) 
        gs.keyboardSprite = Sprite(LoadTexture(UseResPath("potp/keyboard.png").c_str(), texProps));
    if (!gs.controllerSprite.isValid()) 
        gs.controllerSprite = Sprite(LoadTexture(UseResPath("potp/controller.png").c_str(), texProps));
    if (!gs.blankControllerSprite.isValid()) 
        gs.blankControllerSprite = Sprite(LoadTexture(UseResPath("potp/x.png").c_str(), texProps));
    if (!gs.howToPlayBackgroundSprite.isValid())
        gs.howToPlayBackgroundSprite = Sprite(LoadTexture(UseResPath("potp/howtoplay.png").c_str(), texProps));
    for (s32 i = 0; i < MAX_NUM_PLAYERS; i++) {
        if (!gs.playerTexts[i]) {
            const char* playerText = ("Player " + std::to_string(i+1)).c_str();
            gs.playerTexts[i] = CreateText(playerText);
        }
        gs.isReady[i] = false;
    }
    gs.instructionsText = CreateText("Press start to join     Press start again when ready");
    gs.countdownText = CreateText("Starting in ...");
}
void ControllerSetupSceneTick(GameState& gs, UserInput& inputs) {
    SetText(gs.countdownText, ("Starting in " + std::to_string(gs.allReadyCountdown/60) + " seconds...").c_str());

    inputs.SetupControllersTick(gs.numPlayers, gs.isReady);

    s32 numReady = 0;
    for (bool isReady : gs.isReady) {
        if (isReady) numReady++;
    }
    bool isAllReady = gs.numPlayers > 1 && numReady == gs.numPlayers;
    if (isAllReady) {
        gs.allReadyCountdown--;
        // check against 1 to avoid collision with check above for 0
        if (gs.allReadyCountdown == 0) { 
            gs.scene = PotpScene::ASSASSIN;
            AssassinSceneInit(gs);
        }
    }
    // if we're not all ready, but countdown already started, restart countdown
    else if (gs.allReadyCountdown != ASSASSIN_ALL_READY_COUNTDOWN_FRAMES) {
        gs.allReadyCountdown = ASSASSIN_ALL_READY_COUNTDOWN_FRAMES;
    }
}
void ControllerSetupSceneDraw(const GameState& gs, const UserInput& inputs) {
    Camera& cam = Camera::GetMainCamera();
    gs.howToPlayBackgroundSprite.DrawSprite(cam, glm::vec2(0.0f, 0.0f), glm::vec2(cam.screenWidth, cam.screenHeight));
    // draw player indicators at bottom of screen
    // "player indicators" here refers to the UI that shows what players are ingame (1-4) and if they are using keyboard/controller
    const f32 playerIndicatorsSize = 80.0 * GetWindowWidthScaleFactor();
    const f32 playerTextSize = 1.0 * GetWindowWidthScaleFactor();
    const glm::vec2 playerIndicatorsSizeVec = {playerIndicatorsSize, playerIndicatorsSize};
    const f32 playerIndicatorsY = 45.0 + (playerIndicatorsSizeVec.y/2.0);
    const f32 playerIndicatorsStartX = (f32)Camera::GetScreenWidth()/playerIndicatorsSizeVec.x+20.0;
    const f32 playerIndicatorsXSpacing = playerIndicatorsSize + (f32)Camera::GetScreenWidth()/8.0;
    const f32 playerTextYOffset = -30.0;

    glm::vec3 rotationAxis = {0.0, 0.0, 1.0};
    glm::vec4 color = {1.0, 1.0, 1.0, 1.0};

    for (s32 playerIdx = 0; playerIdx < MAX_NUM_PLAYERS; playerIdx++) {
        bool isReady = gs.isReady[playerIdx];
        color.a = isReady ? 1.0 : 0.3;
        glm::vec2 pos = {playerIndicatorsStartX + playerIndicatorsXSpacing * playerIdx, playerIndicatorsY};
        Shapes::DrawSquare(pos, playerIndicatorsSizeVec, sin(GetTime())*50.0, rotationAxis, color);

        ControllerType controllerType = inputs.controllers[playerIdx].type;
        Sprite inputDeviceSprite;
        inputDeviceSprite = controllerType == ControllerType::CONTROLLER ? 
                gs.controllerSprite : controllerType == ControllerType::KEYBOARD ? gs.keyboardSprite : gs.blankControllerSprite;

        inputDeviceSprite.DrawSprite(cam, pos, playerIndicatorsSizeVec, 0.0, rotationAxis, color);

        const char* playerText = ("Player " + std::to_string(playerIdx+1) + " [" + std::string(isReady ? "" : "NOT ") + "READY]").c_str();
        SetText(gs.playerTexts[playerIdx], playerText);
        DrawText(gs.playerTexts[playerIdx], pos.x, pos.y + playerTextYOffset, playerTextSize, color.r, color.b, color.g, color.a);
    }

    if (gs.allReadyCountdown != ASSASSIN_ALL_READY_COUNTDOWN_FRAMES)
        DrawText(gs.countdownText, 10.0, 10.0, 1.3, 1.0, 1.0, 1.0, 1.0);

    DrawText(gs.instructionsText, Camera::GetScreenWidth()/2.0 - 100.0, 15.0,   1.2,    1.0, 1.0, 1.0, 1.0);
}
void ControllerSetupSceneEnd(GameState& gs, UserInput& inputs) {

}