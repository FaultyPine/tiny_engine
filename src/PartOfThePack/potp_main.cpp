#include "potp_main.h"

#include "tiny_engine/input.h"
#include "tiny_engine/tiny_audio.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/framebuffer.h"
#include "tiny_engine/tiny_text.h"
#include "tiny_engine/tiny_fs.h"
#include "tiny_engine/shapes.h"

void PotpInitTitleScreenScene(GameState& gs) {
    gs.scene = PotpScene::TITLE;
    if (!gs.titleScreenSplash.isValid()) {
        TextureProperties texProps = TextureProperties::Default();
        texProps.imgFormat = TextureProperties::ImageFormat::RGB;
        texProps.magFilter = TextureProperties::TexMagFilter::NEAREST;
        texProps.minFilter = TextureProperties::TexMinFilter::NEAREST;
        gs.titleScreenSplash = Sprite(LoadTexture(UseResPath("potp/title_screen.jpg").c_str(), texProps));
    }

    if (!gs.titleScreenText)
        gs.titleScreenText = CreateText("Press start to play");
}

void PotpInitControllerSetupMenu(GameState& gs) {
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
    for (s32 i = 0; i < MAX_NUM_PLAYERS; i++) {
        if (!gs.playerTexts[i]) {
            const char* playerText = ("Player " + std::to_string(i+1)).c_str();
            gs.playerTexts[i] = CreateText(playerText);
        }
        gs.isReady[i] = false;
    }
    gs.instructionsText = CreateText("Press start to join\nPress start again when ready");
    gs.countdownText = CreateText("Starting in ...");
}

void PotpInitAssassinScene(GameState& gs) {
    ASSERT(gs.numPlayers > 1);
    gs.scene = PotpScene::ASSASSIN;
    gs.winningPlayer = -1;
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
    gs.playerWonText = CreateText("Winner!");

    // ninjas
    InitializeNinjas(gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, gs.numPlayers);
}

void PotpInit(GameState& gs, UserInput& inputs) {
    // manually init random seed so we can serialize it (with the rest of gamestate) if need be
    f64 time = GetTime();
    gs.initialRandomSeed = hash((const char*)&time, sizeof(double));
    InitializeRandomSeed(gs.initialRandomSeed);

    Audio::SetMute(true);

    // SFX
    Audio::InitAudioEngine();
    Audio::PlayAudio(UseResPath("potp/clarkPapple_Song.mp3").c_str());

    // textures
    TextureProperties texProps = TextureProperties::Default();
    texProps.imgFormat = TextureProperties::ImageFormat::RGB;
    texProps.magFilter = TextureProperties::TexMagFilter::NEAREST;
    texProps.minFilter = TextureProperties::TexMinFilter::NEAREST;
    
    Texture backgroundTex = LoadTexture(UseResPath("potp/background.jpg").c_str(), texProps);
    // Sprites
    gs.background = Sprite(backgroundTex);

    /*{ // for debugging, init directly to other scenes
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
        //PotpInitControllerSetupMenu(gs);
        //PotpInitAssassinScene(gs); // using this for debugging to boot directly into gameplay
    }*/
    // init game to title screen
    PotpInitTitleScreenScene(gs);
}

void PotpControllerSetupTick(GameState& gs, UserInput& inputs) {
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
            PotpInitAssassinScene(gs);
        }
    }
    // if we're not all ready, but countdown already started, restart countdown
    else if (gs.allReadyCountdown != ASSASSIN_ALL_READY_COUNTDOWN_FRAMES) {
        gs.allReadyCountdown = ASSASSIN_ALL_READY_COUNTDOWN_FRAMES;
    }
}

// return -1 if more than 1 player is alive, return playeridx of player if only one is alive
s32 getOnePlayerAliveOrNone(Ninja* playerNinjas, u32 numPlayerNinjas) {
    u32 numPlayersAlive = 0;
    s32 playerIdx = -1;
    for (s32 playerNinjaIdx = 0; playerNinjaIdx < numPlayerNinjas; playerNinjaIdx++) {
        Ninja& playerNinja = playerNinjas[playerNinjaIdx];
        if (!playerNinja.isDead) {
            numPlayersAlive++;
            playerIdx = playerNinjaIdx;
        }
    }
    return numPlayersAlive == 1 ? playerIdx : -1;
}

void onPlayerWon(GameState& gs) { // called once when player wins
    SetText(gs.playerWonText, ("Player " + std::to_string(gs.winningPlayer) + " wins!").c_str());
}
void playerWonTick(GameState& gs) {
    gs.playerWonTimer--;
    if (gs.playerWonTimer <= 0) {
        gs.playerWonTimer = PLAYER_WON_MAX_TIMER;
        PotpInitControllerSetupMenu(gs);
    }
}

void CheckWinConditions(GameState& gs, Ninja* aiNinjas, u32 numAINinjas, Ninja* playerNinjas, u32 numPlayerNinjas) {
    if (gs.winningPlayer == -1) {
        // player has won if all other players are dead
        s32 playerIdxWon = getOnePlayerAliveOrNone(playerNinjas, numPlayerNinjas);

        if (playerIdxWon != -1) {
            // kill all ninjas that are not our winning player
            for (s32 aiNinjaIdx = 0; aiNinjaIdx < numAINinjas; aiNinjaIdx++) {
                Ninja& aiNinja = aiNinjas[aiNinjaIdx];
                aiNinja.Die();
            }
            for (s32 playerNinjaIdx = 0; playerNinjaIdx < numPlayerNinjas; playerNinjaIdx++) {
                Ninja& playerNinja = playerNinjas[playerNinjaIdx];
                if (playerNinjaIdx != playerIdxWon) {
                    playerNinja.Die();
                }
            }
            // player won!
            gs.winningPlayer = playerIdxWon;
            onPlayerWon(gs);
            std::cout << "Player " << playerIdxWon << " wins!\n";
        }
    }
    else {
        playerWonTick(gs);
    }
}

void PotpUpdate(GameState& gs, UserInput& inputs) {
    if (gs.scene == PotpScene::CONTROLLER_SETUP) {
        PotpControllerSetupTick(gs, inputs);
    }
    else if (gs.scene == PotpScene::ASSASSIN) {
        UpdateNinjas(inputs, gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, gs.numPlayers);
        CheckWinConditions(gs, gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, gs.numPlayers);
    }
    else if (gs.scene == PotpScene::TITLE) {
        for (s32 i = 0; i < MAX_NUM_PLAYERS; i++) {
            bool isStartPressed = pollRawInput(i, ControllerType::KEYBOARD, ButtonValues::START) || pollRawInput(i, ControllerType::CONTROLLER, ButtonValues::START);
            if (isStartPressed) {
                PotpInitControllerSetupMenu(gs);
            }
        }
    }
}

void DrawControllerSetupScene(const GameState& gs, const UserInput& inputs) {
    Camera& cam = Camera::GetMainCamera();
    // draw player indicators at bottom of screen
    // "player indicators" here refers to the UI that shows what players are ingame (1-4) and if they are using keyboard/controller
    const f32 playerIndicatorsSize = 100.0;
    const f32 playerTextSize = 1.0;
    const glm::vec2 playerIndicatorsSizeVec = {playerIndicatorsSize, playerIndicatorsSize};
    const f32 playerIndicatorsY = (f32)Camera::GetScreenHeight()/2.0 + (playerIndicatorsSizeVec.y/2.0);
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
        DrawText(gs.countdownText, 10.0, 10.0, 1.5, 1.0, 1.0, 1.0, 1.0);

    DrawText(gs.instructionsText, Camera::GetScreenWidth()/2.0 - 50.0, 30.0, 1.5,    1.0, 1.0, 1.0, 1.0);
}


void PotpDraw(const GameState& gs, const UserInput& inputs) {
    const Camera& cam = Camera::GetMainCamera();
    gs.background.DrawSprite(cam, glm::vec2(0.0f, 0.0f), glm::vec2(cam.screenWidth, cam.screenHeight));

    switch (gs.scene) {

        case PotpScene::NO_SCENE:
        {

        } break;
        case PotpScene::CONTROLLER_SETUP:
        {
            DrawControllerSetupScene(gs, inputs);
        } break;
        case PotpScene::ASSASSIN:
        {

            for (const Statue& statue : gs.statues) {
                statue.Draw();
            }
            DrawNinjas(gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, gs.numPlayers);

            if (gs.winningPlayer != -1) {
                DrawText(gs.playerWonText, 15.0, 15.0, 2.0, 1.0, 1.0, 1.0, 1.0);
            }

        } break;
        case PotpScene::TITLE:
        {
            gs.titleScreenSplash.DrawSprite(cam, {0.0, 0.0}, {Camera::GetScreenWidth(), Camera::GetScreenHeight()}, 0.0, {0.0, 0.0, 1.0}, {1.0, 1.0, 1.0, 1.0});
            DrawText(gs.titleScreenText, Camera::GetScreenWidth() / 4.0, 5.0, 1.5, 1.0, 1.0, 1.0, 1.0);
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
    PotpDraw(gs, inputs);
}

} // namespace Potp