#include "ninja.h"

#include "PoissonGenerator.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/shapes.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/sprite.h"

glm::vec2 GetRandomAIDesiredPos() {
    u32 screenWidth = Camera::GetScreenWidth();
    u32 screenHeight = Camera::GetScreenHeight();
    
    u32 randXPos = GetRandom(0, screenWidth);
    u32 randYPos = GetRandom(0, screenHeight);

    glm::vec2 ret = glm::vec2(randXPos, randYPos);
    CLAMP(ret.x, 0.0f, (f32)screenWidth-NINJA_SPRITE_SIZE);
    CLAMP(ret.y, 0.0f, (f32)screenHeight-NINJA_SPRITE_SIZE);
    return ret;
}

void InitNinjaDefault(Ninja& ninja, Texture ninjaTex, glm::vec2 pos) {
    ninja.isDead = false;
    ninja.isSpriteFlipped = false;
    const f32 ninjaSpriteSize = NINJA_SPRITE_SIZE;
    ninja.entity.sprite = Sprite(ninjaTex);
    ninja.numSmokeGrenadesLeft = NINJA_MAX_SMOKE_GRENADES;
    ninja.entity.position = glm::vec3(pos.x, pos.y, 0.0);
    ninja.entity.size = glm::vec2(ninjaSpriteSize, ninjaSpriteSize);
    // initial idle is between 30-180 frames
    ninja.positionIdleFramesMax = GetRandom(30, 180);
}

void InitializeNinjas(Ninja* aiNinjas, u32 numAINinjas, Ninja* playerNinjas, u32 numPlayerNinjas) {
    u32 totalNinjas = numAINinjas + numPlayerNinjas;
    u32 screenWidth = Camera::GetScreenWidth();
    u32 screenHeight = Camera::GetScreenHeight();

    TextureProperties texProps = TextureProperties::Default();
    texProps.magFilter = TextureProperties::TexMagFilter::NEAREST;
    texProps.minFilter = TextureProperties::TexMinFilter::NEAREST;
    Texture ninjaDefaultTex = LoadTexture(UseResPath("potp/ninja_sprites/tile000.png").c_str(), texProps);

    Spritesheet ninjaSpritesheet = Spritesheet(UseResPath("potp/ninja_spritesheet_32x32.png").c_str(), 12, 7, texProps);
    ninjaSpritesheet.SetFPS(15);
    ninjaSpritesheet.SetAnimationIndices(NinjaAnimStates::IDLE, {0});
    ninjaSpritesheet.SetAnimationIndices(NinjaAnimStates::WALK, {1,1,1,8,8,8,15,15,15,22,22,22});
    ninjaSpritesheet.SetAnimationIndices(NinjaAnimStates::PUNCH, {29,29,29,29,45,45,45,45,45,45,45});
    ninjaSpritesheet.SetAnimationIndices(NinjaAnimStates::SMOKE, {16,16,16,16,16,16,17,17,17,17,17});
    ninjaSpritesheet.SetAnimationIndices(NinjaAnimStates::DEAD, {26});
    ninjaSpritesheet.SetAnimation(NinjaAnimStates::IDLE);

    PoissonGenerator::DefaultPRNG poissonRNG;
    // points is a vector of xyz in the range 0-1
	const auto Points = PoissonGenerator::generatePoissonPoints(totalNinjas, poissonRNG, false);

    for (int i = 0; i < numAINinjas; i++) {
        Ninja& ninja = aiNinjas[i];
        ninja.spritesheet = ninjaSpritesheet;
        glm::vec2 ninjaPos = glm::vec2(Points[i].x*screenWidth, Points[i].y*screenHeight);
        InitNinjaDefault(ninja, ninjaDefaultTex, ninjaPos);
        long long rand = GetRandom(0, 99999);
        bool shouldMoveInitially = rand % 2 == 0 ? true : false;
        ninja.aiDesiredPos = shouldMoveInitially ? GetRandomAIDesiredPos() : ninjaPos;
    }
    for (int i = 0; i < numPlayerNinjas; i++) {
        Ninja& ninja = playerNinjas[i];
        ninja.spritesheet = ninjaSpritesheet;
        glm::vec2 ninjaPos = glm::vec2(Points[numAINinjas+i].x*screenWidth, Points[numAINinjas+i].y*screenHeight);
        InitNinjaDefault(ninja, ninjaDefaultTex, ninjaPos);
    }
}

glm::vec2 GetPlayerInputDir(UserInput inputs, u32 playerIdx) {
    glm::vec2 inputDir = glm::vec2(0.0f, 0.0f); // range [-1, -1]
    if (inputs.isUp(playerIdx)) {
        inputDir.y -= 1.0f;
    }
    if (inputs.isDown(playerIdx)) {
        inputDir.y += 1.0f;
    }
    if (inputs.isLeft(playerIdx)) {
        inputDir.x -= 1.0f;
    }
    if (inputs.isRight(playerIdx)) {
        inputDir.x += 1.0f;
    }
    if (inputDir.x != 0.0 && inputDir.y != 0) {
        inputDir = glm::normalize(inputDir);
    }
    return inputDir;
}

void ProcessPlayerInput(UserInput inputs, Ninja& playerNinja, u32 playerIdx) {
    if (!playerNinja.entity.active) return;

    glm::vec2& ninjaPos = playerNinja.entity.position;
    glm::vec2 inputDir = GetPlayerInputDir(inputs, playerIdx);
    if (glm::length(inputDir) > 0) {
        // we're moving, switch to walking anim
        playerNinja.spritesheet.SetAnimation(NinjaAnimStates::WALK);
        bool isInputLeft = glm::dot(inputDir, glm::vec2(1.0, 0.0)) < 0.0;
        playerNinja.isSpriteFlipped = isInputLeft; // if we're going left, flip sprite
    }
    else {
        playerNinja.spritesheet.SetAnimation(NinjaAnimStates::IDLE);
    }
    glm::vec2 posDelta = inputDir * playerNinja.ninjaSpeed;
    ninjaPos += posDelta * GetDeltaTime();

    CLAMP(ninjaPos.x, 0.0f, (f32)Camera::GetScreenWidth() - NINJA_SPRITE_SIZE);
    CLAMP(ninjaPos.y, 0.0f, (f32)Camera::GetScreenHeight() - NINJA_SPRITE_SIZE);
}

void UpdateNinjaAI(Ninja& aiNinja) {
    if (aiNinja.isDead) return;
    const glm::vec2& pos = aiNinja.entity.position;
    // if we're within a reasonable distance of our goal
    if (glm::distance(aiNinja.aiDesiredPos, pos) < 1.0f) {
        aiNinja.spritesheet.SetAnimation(NinjaAnimStates::IDLE);
        // if we are at our ai's desired spot, wait and then regenerate desired spot
        if (aiNinja.positionIdleFrames >= aiNinja.positionIdleFramesMax) {
            aiNinja.aiDesiredPos = GetRandomAIDesiredPos();
            aiNinja.positionIdleFrames = 0;
            aiNinja.positionIdleFramesMax = GetRandom(30, 180);
        }
        else aiNinja.positionIdleFrames++;
    }
    else {
        // not at desired spot, move towards
        glm::vec2 dir = glm::normalize(aiNinja.aiDesiredPos - pos);
        glm::vec2 posDelta = dir * aiNinja.ninjaSpeed * GetDeltaTime();
        aiNinja.entity.position += posDelta;

        aiNinja.spritesheet.SetAnimation(NinjaAnimStates::WALK);
        bool isInputLeft = glm::dot(dir, glm::vec2(1.0, 0.0)) < 0.0;
        aiNinja.isSpriteFlipped = isInputLeft; // if we're going left, flip sprite
    }
}
void UpdateNinjaDefault(Ninja& ninja) {
    ninja.spritesheet.Tick();
}

void UpdateNinjas(UserInput inputs, Ninja* aiNinjas, u32 numAINinjas, Ninja* playerNinjas, u32 numPlayerNinjas) {
    for (u32 i = 0; i < numAINinjas; i++) {
        Ninja& aiNinja = aiNinjas[i];
        if (aiNinja.entity.active) {
            UpdateNinjaDefault(aiNinja);
            UpdateNinjaAI(aiNinja);
        }
    }
    for (u32 i = 0; i < numPlayerNinjas; i++) {
        Ninja& playerNinja = playerNinjas[i];
        UpdateNinjaDefault(playerNinja);
        ProcessPlayerInput(inputs, playerNinja, i);
    }
}

void DrawNinja(const Ninja& ninja, bool horzFlip) {
    f32 rotation = ninja.entity.rotation;
    if (horzFlip) rotation = 180.0;
    
    ninja.spritesheet.Draw(Camera::GetMainCamera(), 
            ninja.entity.position, ninja.entity.size, rotation, glm::vec3(0.0, 1.0, 0.0), ninja.entity.color);
}

void DrawNinjas(const Ninja* aiNinjas, u32 numAINinjas, const Ninja* playerNinjas, u32 numPlayerNinjas) {
    for (int i = 0; i < numAINinjas; i++) {
        const Ninja& ninja = aiNinjas[i];
        // would be better to keep two lists of ninjas and
        // put deactivated ones in a "deactivated" array.
        // That'd be a lot better for the cache
        if (ninja.entity.active) {
            DrawNinja(ninja, ninja.isSpriteFlipped);
        }
    }
    for (int i = 0; i < numPlayerNinjas; i++) {
        const Ninja& ninja = playerNinjas[i];
        if (ninja.entity.active) {
            DrawNinja(ninja, ninja.isSpriteFlipped);
        }
    }
}