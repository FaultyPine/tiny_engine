#include "potp_main.h"

#include "tiny_engine/input.h"
#include "tiny_engine/tiny_audio.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/framebuffer.h"
#include "tiny_engine/tiny_text.h"
#include "tiny_engine/tiny_fs.h"
#include "tiny_engine/shapes.h"

void PotpInitControllerSetupMenu(GameState& gs) {
    gs.scene = PotpScene::CONTROLLER_SETUP;
    gs.allReadyCountdown = ASSASSIN_ALL_READY_COUNTDOWN_FRAMES;

    TextureProperties texProps = TextureProperties::Default();
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
    }
    gs.instructionsText = CreateText("Press start to join\nPress action1 when ready");
    gs.countdownText = CreateText("Starting in ...");
}

void PotpInitAssassinScene(GameState& gs) {
    // TODO: Remove after I get controller setup scene working
    if (gs.numPlayers < 1) {
        std::cout << "[WARNING] Num players was less than 1, setting to 2 by default.\n";
        gs.numPlayers = 2;
    }

    ASSERT(gs.numPlayers > 1);
    gs.scene = PotpScene::ASSASSIN;
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
    InitializeNinjas(gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, gs.numPlayers);
}

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

    // init game to controller setup menu
    PotpInitControllerSetupMenu(gs);
    //PotpInitAssassinScene(gs);
}


bool isPlayerslotAlreadyBound(UserInput& inputs, u32 playerIdx) {
    for (const InputDevice& ID : inputs.controllers) {
        if (ID.port == playerIdx) return true;
    }
    return false;
}

bool AttemptBindInputDeviceToPort(s32 portToBind, UserInput& inputs) {    
    // NOTE: Not going to account for the situation where two players
    // push start on the same frame
    bool isSuccessfulBind = false;
    // iterating *ports* which represent the 4 possible gamepad/keyboard input devices
    for (s32 port = 0; port < MAX_NUM_PLAYERS; port++) {
        bool isStartPressed = false;
        ControllerType controllerType = ControllerType::NO_CONTROLLER;
        if (isGamepadPresent(port)) {
            Gamepad pad;
            GetGamepadState(port, pad);
            isStartPressed = pad.isButtonPressed(GetGamepadBinding(ButtonValues::START));
            controllerType = ControllerType::CONTROLLER;
        }
        else {
            isStartPressed = Keyboard::isKeyPressed(GetKeyboardBinding(ButtonValues::START, port));
            controllerType = ControllerType::KEYBOARD;
        }
        bool shouldBind = isStartPressed && inputs.controllers[port].type == ControllerType::NO_CONTROLLER;
        if (shouldBind) {
            const char* inputType = controllerType == ControllerType::CONTROLLER ? "Controller" : "Keyboard";
            std::cout << "Bound " << inputType << " in 'port' " << port << " to playerIdx " << portToBind << "\n";
            inputs.controllers[port].type = controllerType;
            inputs.controllers[port].port = portToBind;
            isSuccessfulBind = true;
        }
    }
    return isSuccessfulBind;
}


void PotpControllerSetupTick(GameState& gs, UserInput& inputs) {
    SetText(gs.countdownText, ("Starting in " + std::to_string(gs.allReadyCountdown/60) + " seconds...").c_str());

    // iterating *player slots*, which represents player 1-4
    for (s32 playerIdx = 0; playerIdx < MAX_NUM_PLAYERS; playerIdx++) {
        if (!isPlayerslotAlreadyBound(inputs, playerIdx)) {
            if (AttemptBindInputDeviceToPort(playerIdx, inputs)) {
                gs.numPlayers++;
            }
        }
        else {
            // already bound slot, poll for if they're ready
            if (inputs.isAction1Pressed(playerIdx) && !gs.isReady[playerIdx]) {
                std::cout << "Player " << playerIdx << " is ready.\n";
                gs.isReady[playerIdx] = true;
            }
        }
    }

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

void PotpUpdate(GameState& gs, UserInput& inputs) {
    if (gs.scene == PotpScene::CONTROLLER_SETUP) {
        PotpControllerSetupTick(gs, inputs);
    }
    else if (gs.scene == PotpScene::ASSASSIN) {
        UpdateNinjas(inputs, gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, gs.numPlayers);
    }
}

void DrawControllerSetupScene(const GameState& gs, const UserInput& inputs) {
    Camera& cam = Camera::GetMainCamera();
    // draw player indicators at bottom of screen
    // "player indicators" here refers to the UI that shows what players are ingame (1-4) and if they are using keyboard/controller
    const f32 playerIndicatorsSize = 100.0;
    const f32 playerTextSize = 2.0;
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

        DrawText(gs.playerTexts[playerIdx], pos.x, pos.y + playerTextYOffset, playerTextSize, color.r, color.b, color.g, color.a);
    }

    if (gs.allReadyCountdown != ASSASSIN_ALL_READY_COUNTDOWN_FRAMES)
        DrawText(gs.countdownText, 10.0, 10.0, 1.5, 1.0, 1.0, 1.0, 1.0);

    DrawText(gs.instructionsText, Camera::GetScreenWidth()/2.0 - 50.0, 30.0, 2.0,    1.0, 1.0, 1.0, 1.0);
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
    PotpInit(gs);
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