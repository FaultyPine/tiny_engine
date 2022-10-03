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

    PoissonGenerator::DefaultPRNG poissonRNG;
    // points is a vector of xyz in the range 0-1
	const auto Points = PoissonGenerator::generatePoissonPoints(totalNinjas, poissonRNG, false);

    for (int i = 0; i < numAINinjas; i++) {
        Ninja& ninja = aiNinjas[i];
        glm::vec2 ninjaPos = glm::vec2(Points[i].x*screenWidth, Points[i].y*screenHeight);
        InitNinjaDefault(ninja, ninjaDefaultTex, ninjaPos);
        long long rand = GetRandom(0, 99999);
        bool shouldMoveInitially = rand % 2 == 0 ? true : false;
        ninja.aiDesiredPos = shouldMoveInitially ? GetRandomAIDesiredPos() : ninjaPos;
    }
    for (int i = 0; i < numPlayerNinjas; i++) {
        Ninja& ninja = playerNinjas[i];
        glm::vec2 ninjaPos = glm::vec2(Points[numAINinjas+i].x*screenWidth, Points[numAINinjas+i].y*screenHeight);
        InitNinjaDefault(ninja, ninjaDefaultTex, ninjaPos);
    }
}

void ProcessPlayerInput(UserInput inputs, Ninja* playerNinjas, u32 numPlayerNinjas) {
    u32 screenWidth = Camera::GetScreenWidth();
    u32 screenHeight = Camera::GetScreenHeight();
    for (int i = 0; i < numPlayerNinjas; i++) {
        Ninja& playerNinja = playerNinjas[i];
        glm::vec2& ninjaPos = playerNinja.entity.position;
        glm::vec2 inputDir = glm::vec2(0.0f, 0.0f); // range 0-1

        if (inputs.isUp()) {
            inputDir.y -= 1.0f;
        }
        if (inputs.isDown()) {
            inputDir.y += 1.0f;
        }
        if (inputs.isLeft()) {
            inputDir.x -= 1.0f;
        }
        if (inputs.isRight()) {
            inputDir.x += 1.0f;
        }
        if (inputDir.x != 0.0 && inputDir.y != 0) {
            inputDir = glm::normalize(inputDir);
        }

        glm::vec2 posDelta = inputDir * playerNinja.ninjaSpeed;
        ninjaPos += posDelta * GetDeltaTime();

        CLAMP(ninjaPos.x, 0.0f, (f32)screenWidth - NINJA_SPRITE_SIZE);
        CLAMP(ninjaPos.y, 0.0f, (f32)screenHeight - NINJA_SPRITE_SIZE);
    }
}

void UpdateNinjaAI(Ninja& aiNinja) {
    const glm::vec2& pos = aiNinja.entity.position;
    // if we're within a reasonable distance of our goal
    if (glm::distance(aiNinja.aiDesiredPos, pos) < 1.0f) {
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
    }
}

void UpdateNinjas(UserInput inputs, Ninja* aiNinjas, u32 numAINinjas, Ninja* playerNinjas, u32 numPlayerNinjas) {
    for (int i = 0; i < numAINinjas; i++) {
        Ninja& aiNinja = aiNinjas[i];
        UpdateNinjaAI(aiNinja);
    }
    ProcessPlayerInput(inputs, playerNinjas, numPlayerNinjas);
}

void DrawNinjas(const Ninja* aiNinjas, u32 numAINinjas, const Ninja* playerNinjas, u32 numPlayerNinjas) {
    for (int i = 0; i < numAINinjas; i++) {
        const Ninja& ninja = aiNinjas[i];
        ninja.entity.Draw();
    }
    for (int i = 0; i < numPlayerNinjas; i++) {
        const Ninja& ninja = playerNinjas[i];
        ninja.entity.Draw();
    }
}