#ifndef SPACESHOOTER_H
#define SPACESHOOTER_H

#include "spaceshooter_fwd.h"
#include "player.h"
#include "laser.h"
#include "enemy.h"
#include "enemyspawner.h"


// TODO: alpha blending



void initSpaceshooterSprites(GameState& gs) {
    TextureProperties texProps;
    texProps.texWrapMode = TextureProperties::TexWrapMode::MIRRORED_REPEAT;
    texProps.minFilter = TextureProperties::TexMinFilter::LINEAR_MIPMAP_LINEAR;
    texProps.magFilter = TextureProperties::TexMagFilter::LINEAR;
    texProps.texFormat = TextureProperties::TexFormat::RGBA;
    texProps.imgFormat = TextureProperties::ImageFormat::RGBA;
    texProps.imgDataType = TextureProperties::ImageDataType::UNSIGNED_BYTE;

    // load background texture
    Texture backgroundTex = LoadTexture(UseResPath("spaceshooter/space_background.png").c_str(), texProps, TextureMaterialType::DIFFUSE, false);
    // load player tex
    Texture playerTex = LoadTexture(UseResPath("spaceshooter/enemyBlue1.png").c_str(), texProps, TextureMaterialType::DIFFUSE, false);
    // load enemy tex
    Texture enemyTex = LoadTexture(UseResPath("spaceshooter/enemyRed3.png").c_str(), texProps, TextureMaterialType::DIFFUSE, false);
    // load laser tex
    Texture laserTex = LoadTexture(UseResPath("spaceshooter/laserRed04.png").c_str(), texProps, TextureMaterialType::DIFFUSE, false);

    // load Sprites
    gs.background = Sprite(backgroundTex);
    gs.player.sprite = Sprite(playerTex);
    for (Entity& enemy : gs.enemies) {
        enemy.sprite = Sprite(enemyTex);
    }
    for (Entity& laser : gs.lasers) {
        laser.sprite = Sprite(laserTex);
    }

    if (!gs.scoreText) {
        gs.scoreText = gltCreateText();
    }
    gltSetText(gs.scoreText, "Score: 0");
    if (!gs.endGameText) {
        gs.endGameText = gltCreateText();
        gltSetText(gs.endGameText, "Game Over!");
    }
}



namespace Spaceshooter {


void drawSpaceshooter(GameState& gs) {
    Camera& cam = gs.camera;
    Entity& player = gs.player;

    // UI --------------------------------
    gltBeginDraw();
    gltColor(1.0f, 1.0f, 1.0f, 1.0f);
    const char* currentScoreTxt = ("Score: " + std::to_string(gs.score)).c_str();
    gltSetText(gs.scoreText, currentScoreTxt);
    gltDrawText2D(gs.scoreText, 5.0f, 5.0f, 2.0f);

    if (gs.isGameOver) {
        gltDrawText2D(gs.endGameText, 150.0f, 150.0f, 5.0f);
    }
    gltEndDraw();
    // ------------------------------------


    // Game Objects --------------------------
    player.Draw(cam);

    for (Entity& laser : gs.lasers) {
        if (laser.active) laser.Draw(cam);
    }
    for (Entity& enemy : gs.enemies) {
        if (enemy.active) enemy.Draw(cam);
    }
    // ---------------------------------------
    

    // Background -----------------------------
    gs.background.DrawSprite(gs.camera, vec2(0.0, 0.0), vec2(gs.camera.screenWidth, gs.camera.screenHeight));
    //std::cout << "----- END FRAME -----\n";
}

void initSpaceshooter(GameState& gs) {
    gs.isGameOver = false;
    gs.score = 0;
    initSpaceshooterSprites(gs);

    gs.player.position = glm::vec3(gs.camera.screenWidth / 2.0f, gs.camera.screenHeight - 75.0f, 0.0);
    gs.player.size = glm::vec2(50.0f, 50.0f);
    gs.player.name = "Player";

    for (Entity& laser : gs.lasers) {
        laser.name = "Laser";
        laser.active = false;
    }
    for (Entity& enemy : gs.enemies) {
        enemy.name = "Enemy";
        enemy.active = false;
        enemy.size = glm::vec2(50.0f, 50.0f);
    }
}

void updateSpaceshooterGame(GameState& gs, const UserInput& inputs) {
    if (gs.isGameOver) {
        if (inputs.GetKeyDown(GLFW_KEY_SPACE)) {
            initSpaceshooter(gs);
        }
        return;
    }

    Player::updatePlayer(gs.player, inputs);

    for (Entity& laser : gs.lasers) {
        if (laser.active) Laser::updateLaser(laser);
    }

    for (Entity& enemy : gs.enemies) {
        if (enemy.active) Enemy::updateEnemy(enemy);
    }

    EnemySpawner::tickEnemySpawner(&gs.enemies[0], ARRAY_SIZE(gs.enemies));
    
}


}

#endif