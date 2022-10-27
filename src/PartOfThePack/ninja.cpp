#include "ninja.h"

#include "PoissonGenerator.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/sprite.h"
#include "tiny_engine/tiny_fs.h"
#include "tiny_engine/shapes.h"
#include "tiny_engine/tiny_audio.h"

glm::vec2 GetRandomAIDesiredPos() {
    u32 screenWidth = Camera::GetMainCamera().GetMinScreenDimensions().x;
    u32 screenHeight = Camera::GetMainCamera().GetMinScreenDimensions().y;
    
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
    ninja.numStatuesActivated = 0;
    ninja.smokeGrenade.Reset();
    const f32 ninjaSpriteSize = NINJA_SPRITE_SIZE;
    ninja.entity.sprite = Sprite(ninjaTex);
    ninja.entity.position = glm::vec3(pos.x, pos.y, 0.0);
    ninja.entity.size = glm::vec2(ninjaSpriteSize, ninjaSpriteSize);
    // initial idle is between 30-180 frames
    ninja.positionIdleFramesMax = GetRandom(30, 180);
    ninja.punchHitbox.pos = {(ninjaSpriteSize/2)+5.0, 20.0};
    ninja.punchHitbox.size = {20.0, 20.0};
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
    ninjaSpritesheet.SetDefaultFramerate(24);
    ninjaSpritesheet.SetAnimationIndices(NinjaAnimStates::IDLE, {0});
    ninjaSpritesheet.SetAnimationIndices(NinjaAnimStates::WALK, {1,1,1,8,8,8,15,15,15,22,22,22});
    ninjaSpritesheet.SetAnimationIndices(NinjaAnimStates::PUNCH, {29,29,29,29,45,45,45,45,45,45,45});
    //ninjaSpritesheet.SetAnimationIndices(NinjaAnimStates::SMOKE, {16,16,16,16,16,16,17,17,17,17,17});
    ninjaSpritesheet.SetAnimationIndices(NinjaAnimStates::DEAD, {26});
    Spritesheet::Animation anim = {};
    anim.animKey = NinjaAnimStates::IDLE;
    ninjaSpritesheet.SetAnimation(anim);
    Spritesheet smokeGrenade = Spritesheet(UseResPath("potp/tdingle_smoke.png").c_str(), 3, 2, texProps);
    smokeGrenade.SetDefaultFramerate(12);
    smokeGrenade.SetAnimationIndices(0, {0,1,2,3,4});
    anim = {};
    anim.animKey = 0;
    smokeGrenade.SetAnimation(anim);

    PoissonGenerator::DefaultPRNG poissonRNG(GetRandomSeed());
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
        ninja.smokeGrenade.sprite = smokeGrenade;
        glm::vec2 ninjaPos = glm::vec2(Points[numAINinjas+i].x*screenWidth, Points[numAINinjas+i].y*screenHeight);
        InitNinjaDefault(ninja, ninjaDefaultTex, ninjaPos);
    }
}

glm::vec2 GetPlayerInputDir(const UserInput& inputs, u32 playerIdx) {
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
glm::vec2 GetAIInputDir(Ninja& aiNinja) {
    glm::vec2 desiredPosition = aiNinja.aiDesiredPos;
    glm::vec2 ninjaPosition = aiNinja.entity.position;
    glm::vec2 desiredDirection = glm::normalize(desiredPosition - ninjaPosition);

    glm::vec2 resultDir = glm::vec2(0, 0);
    const f32 rangeLeniency = 3.0;
    if (!isInRange(ninjaPosition.x, desiredPosition.x-rangeLeniency, desiredPosition.x+rangeLeniency)) {
        resultDir.x = signof(desiredDirection.x);
    }
    if (!isInRange(ninjaPosition.y, desiredPosition.y-rangeLeniency, desiredPosition.y+rangeLeniency)) {
        resultDir.y = signof(desiredDirection.y);
    }
    if (resultDir.x != 0 && resultDir.y != 0) {
        resultDir = glm::normalize(resultDir);
    }

    return resultDir;
}

void NinjaInitPunch(Ninja& ninja) {
    Spritesheet::Animation anim = {};
    anim.animKey = NinjaAnimStates::PUNCH;
    anim.isLoop = false;
    anim.nonLoopNextAnim = NinjaAnimStates::IDLE;
    ninja.spritesheet.SetAnimation(anim);
    Audio::PlayAudio(UseResPath("potp/hit.wav").c_str()); // TODO: can we preload this? This probably reads the file in every time...
}
void NinjaEndPunch(Ninja& ninja) {

}
void NinjaUpdatePunch(Ninja& ninja) {

}

void NinjaSmokeBombProcedure(const UserInput& inputs, Ninja& ninja, u32 playerIdx) {

    if (ninja.smokeGrenade.life > 0) {
        ninja.smokeGrenade.sprite.Tick();
        ninja.smokeGrenade.life--;
    }

    // if we're punching or dead or already in smoke anim or have no bombs left, can't use smoke bomb
    bool canActivateSmokeBomb = 
                    !ninja.isPunching &&
                    !ninja.isDead &&
                    ninja.smokeGrenade.numLeft > 0;
    if (canActivateSmokeBomb && inputs.isAction2Pressed(playerIdx)) {
        ninja.smokeGrenade.numLeft--;
        // spawn smoke grenade
        glm::vec2 ninjaPos = ninja.entity.position;
        f32 ninjaSize = NINJA_SPRITE_SIZE;
        f32 smokePosDiagonalOffset = (sqrt(2)*(ninja.smokeGrenade.size.y - ninjaSize))/2.0;
        ninja.smokeGrenade.pos = (ninjaPos - smokePosDiagonalOffset);
        ninja.smokeGrenade.pos += (ninjaSize/2.0);
        ninja.smokeGrenade.life = NINJA_SMOKE_GRENADE_LIFETIME;
    }
}

void ProcessPlayerInput(const UserInput& inputs, Ninja& playerNinja, u32 playerIdx) {
    if (playerNinja.isDead) return;
    glm::vec2& ninjaPos = playerNinja.entity.position;
    glm::vec2 inputDir = GetPlayerInputDir(inputs, playerIdx);
    // if player is trying to punch
    if (inputs.isAction1Pressed(playerIdx)) {
        if (!playerNinja.isPunching) {
            playerNinja.isPunching = true;
            NinjaInitPunch(playerNinja);
        }
    }
    else if (playerNinja.isPunching && playerNinja.spritesheet.GetCurrentAnimation().animKey == NinjaAnimStates::IDLE) {
        playerNinja.isPunching = false;
        NinjaEndPunch(playerNinja);
    }
    else if (playerNinja.isPunching) {
        NinjaUpdatePunch(playerNinja);
    }

    if (!playerNinja.isPunching) {
        // not trying to punch, and is trying to move
        Spritesheet::Animation anim = {};
        if (glm::length(inputDir) > 0) {
            // we're moving, switch to walking anim
            anim.animKey = NinjaAnimStates::WALK;
            playerNinja.spritesheet.SetAnimation(anim);

            f32 inputDotProduct = glm::dot(inputDir, glm::vec2(1.0, 0.0));
            bool isInputLeft = inputDotProduct < 0.0;
            if (isInputLeft != playerNinja.isSpriteFlipped && inputDotProduct != 0.0)
                playerNinja.isSpriteFlipped = isInputLeft; // if we're going left, flip sprite
        }
        else {
            anim.animKey = NinjaAnimStates::IDLE;
            playerNinja.spritesheet.SetAnimation(anim);
        }
        glm::vec2 posDelta = inputDir * playerNinja.ninjaSpeed * GetDeltaTime();
        ninjaPos += posDelta;
    }

    NinjaSmokeBombProcedure(inputs, playerNinja, playerIdx);

    CLAMP(ninjaPos.x, 0.0f, (f32)Camera::GetScreenWidth() - NINJA_SPRITE_SIZE);
    CLAMP(ninjaPos.y, 0.0f, (f32)Camera::GetScreenHeight() - NINJA_SPRITE_SIZE);
}

void UpdateNinjaAI(Ninja& aiNinja) {
    if (aiNinja.isDead) return;
    glm::vec2& pos = aiNinja.entity.position;
    Spritesheet::Animation anim = {};
    // if we're within a reasonable distance of our goal
    if (glm::distance(aiNinja.aiDesiredPos, pos) < 10.0f) {
        anim.animKey = NinjaAnimStates::IDLE;
        aiNinja.spritesheet.SetAnimation(anim);
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
        glm::vec2 dir = GetAIInputDir(aiNinja);
        glm::vec2 posDelta = dir * aiNinja.ninjaSpeed * GetDeltaTime();
        aiNinja.entity.position += posDelta;
        anim.animKey = NinjaAnimStates::WALK;
        aiNinja.spritesheet.SetAnimation(anim);
        f32 inputDotProduct = glm::dot(dir, glm::vec2(1.0, 0.0));
        bool isInputLeft = inputDotProduct < 0.0;
        if (isInputLeft != aiNinja.isSpriteFlipped && inputDotProduct != 0.0)
            aiNinja.isSpriteFlipped = isInputLeft; // if we're going left, flip sprite
    }
}
void UpdateNinjaDefault(Ninja& ninja) {
    f32 basePunchHitboxX = ninja.entity.size.x/2.0; // reliant on punch hitbox being initialized to this + some offset
    f32 punchHitboxOffset = 5.0;
    f32 punchHitBoxX = ninja.isSpriteFlipped ? 
            basePunchHitboxX - punchHitboxOffset - ninja.punchHitbox.size.x : basePunchHitboxX + punchHitboxOffset;
    ninja.punchHitbox.pos = {punchHitBoxX, ninja.punchHitbox.pos.y};
    ninja.spritesheet.Tick();
}

void Ninja::PunchedOtherNinja(Ninja& punchedNinja) {
    if (!punchedNinja.isDead) {
        punchedNinja.Die();
    }
}
void Ninja::Die() {
    Spritesheet::Animation anim = {};
    anim.animKey = NinjaAnimStates::DEAD;
    this->spritesheet.SetAnimation(anim);
    this->isDead = true;
}
// checks if ninjaToCheck is being hit by ninja
bool NinjaHitboxCheck(const Ninja& ninja, const Ninja& ninjaToCheck) {
    return Math::isOverlappingRect2D(ninja.punchHitbox.pos + ninja.entity.position, ninja.punchHitbox.size, 
                ninjaToCheck.entity.position, ninjaToCheck.entity.size);
}
// is there a better way to do this? of course... but for the scope of this project, I don't need anything more complex
void CalculateNinjaCollisions(Ninja* aiNinjas, u32 numAINinjas, Ninja* playerNinjas, u32 numPlayerNinjas) {
    // check if any players are punching, and if so check their hitbox collision
    for (s32 i = 0; i < numPlayerNinjas; i++) {
        Ninja& playerNinja = playerNinjas[i];
        if (playerNinja.isPunching) {
            // loop through AI and player ninjas and check hitbox collisions
            for (s32 aiNinjaIdx = 0; aiNinjaIdx < numAINinjas; aiNinjaIdx++) {
                if (NinjaHitboxCheck(playerNinja, aiNinjas[aiNinjaIdx])) {
                    playerNinja.PunchedOtherNinja(aiNinjas[aiNinjaIdx]);
                }
            }
            for (s32 playerNinjaIdx = 0; playerNinjaIdx < numAINinjas; playerNinjaIdx++) {
                if (playerNinjaIdx != i && NinjaHitboxCheck(playerNinja, playerNinjas[playerNinjaIdx])) {
                    playerNinja.PunchedOtherNinja(playerNinjas[playerNinjaIdx]);
                }
            }
        }
    }
}

void UpdateNinjas(const UserInput& inputs, Ninja* aiNinjas, u32 numAINinjas, Ninja* playerNinjas, u32 numPlayerNinjas) {
    CalculateNinjaCollisions(aiNinjas, numAINinjas, playerNinjas, numPlayerNinjas);
    for (u32 i = 0; i < numAINinjas; i++) {
        Ninja& aiNinja = aiNinjas[i];
        if (aiNinja.entity.active) {
            UpdateNinjaDefault(aiNinja);
            UpdateNinjaAI(aiNinja);
        }
    }
    for (u32 i = 0; i < numPlayerNinjas; i++) {
        Ninja& playerNinja = playerNinjas[i];
        if (playerNinja.entity.active) {
            UpdateNinjaDefault(playerNinja);
            ProcessPlayerInput(inputs, playerNinja, i);
        }
    }
}

void DrawNinja(const Ninja& ninja, bool horzFlip, bool isPlayer) {
    f32 rotation = ninja.entity.rotation;
    if (horzFlip) rotation = 180.0;
    
    ninja.spritesheet.Draw(Camera::GetMainCamera(), 
            ninja.entity.position, ninja.entity.size, rotation, glm::vec3(0.0, 1.0, 0.0), ninja.entity.color, true);
    
    // NOTE: FOR DEBUGGING
    glm::vec4 col = isPlayer ? glm::vec4(0.0, 1.0, 0.0, 1.0) : glm::vec4(1.0, 0.0, 0.0, 1.0);
    //Shapes::DrawSquare(ninja.entity.position, ninja.entity.size, 0.0, {0.0, 0.0, 1.0}, col, true);
    //Shapes::DrawSquare(ninja.entity.position + ninja.punchHitbox.pos, ninja.punchHitbox.size, 0.0, {0.0, 0.0, 1.0}, col, true);
}

void DrawSmokeGrenade(const Ninja& ninja) {
    if (ninja.smokeGrenade.life) {
        glm::vec2 smokeSpriteSize = ninja.smokeGrenade.size;
        smokeSpriteSize += sin((f32)GetTime()*2.0)*8.0;
        f32 smokeSpriteRotation = (f32)GetTime()*15.0;
        ninja.smokeGrenade.sprite.Draw(Camera::GetMainCamera(),
                ninja.smokeGrenade.pos, smokeSpriteSize, 
                smokeSpriteRotation, {0.0, 0.0, 1.0}, 
                {1.0, 1.0, 1.0, 1.0}, true
        );
    }
}

void DrawNinjas(const Ninja* aiNinjas, u32 numAINinjas, const Ninja* playerNinjas, u32 numPlayerNinjas) {
    for (s32 i = 0; i < numAINinjas; i++) {
        const Ninja& ninja = aiNinjas[i];
        if (ninja.entity.active) {
            DrawNinja(ninja, ninja.isSpriteFlipped, false);
        }
    }
    for (s32 i = 0; i < numPlayerNinjas; i++) {
        const Ninja& ninja = playerNinjas[i];
        if (ninja.entity.active) {
            DrawNinja(ninja, ninja.isSpriteFlipped, true);
        }
    }
    // draw smoke grenades on top of ninjas
    for (s32 i = 0; i < numPlayerNinjas; i++) {
        const Ninja& ninja = playerNinjas[i];
        if (ninja.entity.active) {
            DrawSmokeGrenade(ninja);
        }
    }
}