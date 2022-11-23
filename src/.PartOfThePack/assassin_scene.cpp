#include "assassin_scene.h"

#include "tiny_engine/tiny_text.h"
#include "statue.h"
#include "ninja.h"
#include "controller_setup_scene.h"

void AssassinSceneInit(GameState& gs) {
    ASSERT(gs.numPlayers > 1);
    gs.scene = PotpScene::ASSASSIN;
    gs.winningPlayer = -1;
    // statues
    u32 screenWidth = Camera::GetMainCamera().GetMinScreenDimensions().x;
    u32 screenHeight = Camera::GetMainCamera().GetMinScreenDimensions().y;
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
    std::string playerScoresTextStr = "";
    for (s32 i = 0; i < MAX_NUM_PLAYERS; i++) {
        const char* playerScore = ("Player " + std::to_string(i+1) + ": 0        ").c_str();
        playerScoresTextStr.append(playerScore);
    }
    gs.playerScoresText = CreateText(playerScoresTextStr.c_str());

    // ninjas
    InitializeNinjas(gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, gs.numPlayers);
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
        ChangeScene(PotpScene::CONTROLLER_SETUP, gs);
    }
}


s32 CheckNinjaActivatedAllStatues(Ninja* playerNinjas, u32 numPlayerNinjas) {
    for (s32 playerNinjaIdx = 0; playerNinjaIdx < numPlayerNinjas; playerNinjaIdx++) {
        Ninja& playerNinja = playerNinjas[playerNinjaIdx];
        if (!playerNinja.isDead && playerNinja.numStatuesActivated >= NUM_STATUES) {
            return playerNinjaIdx;
        }
    }
    return -1;
}

void CheckWinConditions(GameState& gs, Ninja* aiNinjas, u32 numAINinjas, Ninja* playerNinjas, u32 numPlayerNinjas) {
    if (gs.winningPlayer == -1) {
        // player has won if all other players are dead
        s32 playerIdxWon = getOnePlayerAliveOrNone(playerNinjas, numPlayerNinjas);
        if (playerIdxWon == -1) {
            // or if someone activated all statues
            playerIdxWon = CheckNinjaActivatedAllStatues(playerNinjas, numPlayerNinjas);
        }

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
            gs.winningPlayer = playerIdxWon + 1;
            onPlayerWon(gs);
            std::cout << "Player " << playerIdxWon << " wins!\n";
        }
    }
    else {
        playerWonTick(gs);
    }
}

void AssassinSceneTick(GameState& gs, UserInput& inputs) {
    UpdateNinjas(inputs, gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, gs.numPlayers);
    CheckWinConditions(gs, gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, gs.numPlayers);
    UpdateStatues(gs.statues, ARRAY_SIZE(gs.statues), gs.playerNinjas, gs.numPlayers);
    std::string playerScores = "";
    for (s32 i = 0; i < MAX_NUM_PLAYERS; i++) {
        Ninja& playerNinja = gs.playerNinjas[i];
        std::string playerStatus = playerNinja.isDead ? "DEAD" : std::to_string(playerNinja.numStatuesActivated);
        const char* playerScore = ("Player " + std::to_string(i+1) + ": " + playerStatus + "        ").c_str();
        playerScores.append(playerScore);
    }
    SetText(gs.playerScoresText, playerScores);
}
void AssassinSceneDraw(const GameState& gs, const UserInput& inputs) {
    Camera& cam = Camera::GetMainCamera();
    gs.background.DrawSprite(cam, glm::vec2(0.0f, 0.0f), glm::vec2(cam.screenWidth, cam.screenHeight));
    for (const Statue& statue : gs.statues) {
        statue.Draw();
    }
    DrawNinjas(gs.aiNinjas, MAX_NUM_AI_NINJAS, gs.playerNinjas, gs.numPlayers);

    DrawText(gs.playerScoresText, 5.0, 5.0, 1.0 * GetWindowWidthScaleFactor());
    if (gs.winningPlayer != -1) {
        DrawText(gs.playerWonText, 15.0, 15.0, 2.0 * GetWindowWidthScaleFactor(), 1.0, 1.0, 1.0, 1.0);
    }
}
void AssassinSceneEnd(GameState& gs) {

}