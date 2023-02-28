#include "pch.h"
#include "tetris_main.h"

#include "tiny_engine/tiny_engine.h" // for CloseGameWindow
#include "tiny_engine/input.h" // to poll for keyboard input
#include "tiny_engine/tiny_fs.h" // to get the "resource" path...(ResPath)
#include "tiny_engine/camera.h" // to get screen height/width

// called once when the game boots
void tetris_init() {
    TetrisGameState& gs = TetrisGameState::Get();
    gs.testSprite = Sprite(LoadTexture(ResPath("other/awesomeface.png")));
    // init the sprite at position (20,0) with a size of 50
    gs.testSpriteTF = Transform2D({20.0, 0.0}, glm::vec2(50)); 

    // Text
    gs.testText = CreateText("Hello world!");
}

// get input vector based on WASD
glm::vec2 PollInputs () {
    glm::vec2 inputs = glm::vec2(0);
    // NOTE: Holding down and right doesn't go diagonally...fix this?
    if (Keyboard::isKeyDown(GLFW_KEY_W)) {
        // origin is top-left of the screen...
        // which means Y positive is going down the screen and Y negative is going up
        inputs.y -= 1;
    }
    else if (Keyboard::isKeyDown(GLFW_KEY_S)) {
        inputs.y += 1;
    }
    else if (Keyboard::isKeyDown(GLFW_KEY_A)) {
        inputs.x -= 1;
    }
    else if (Keyboard::isKeyDown(GLFW_KEY_D)) {
        inputs.x += 1;
    }
    return inputs;
}

// called every frame
void tetris_tick() {
    TetrisGameState& gs = TetrisGameState::Get();
    if (Keyboard::isKeyDown(GLFW_KEY_ESCAPE)) {
        CloseGameWindow();
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_SPACE)) {
        // reset sprite position when the space key is pressed
        gs.testSpriteTF.position = glm::vec2(10, 10);
    }

    // move sprite based on inputs
    gs.testSpriteTF.position += PollInputs();
    
    // TODO: we keep track of the sprite and it's transform *seperately* in the GameState...
    // but a sprite will generally never exist without a position onscreen.
    // how can we package them together (sprite and transform) to make things a bit cleaner?
    gs.testSprite.DrawSprite(gs.testSpriteTF);

    // draw text 100 pixels away from the bottom of the screen
    DrawText(gs.testText, 0.0, Camera::GetScreenHeight() - 100.0, 2.0);
}

// called when game shuts down
void tetris_terminate() {
    TetrisGameState& gs = TetrisGameState::Get();
    // unload our text
    UnloadText(gs.testText);
    // unload our sprite
    gs.testSprite.Delete();
}