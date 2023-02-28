#ifndef TETRIS_MAIN_H
#define TETRIS_MAIN_H

#include "tiny_engine/sprite.h" // to create/draw sprites
#include "tiny_engine/tiny_text.h" // to create/render text

struct TetrisGameState {
    Transform2D testSpriteTF;
    Sprite testSprite;
    GLTtext* testText;

    // Singleton!
    static TetrisGameState& Get() {
        static TetrisGameState gs;
        return gs;
    }
};




void tetris_init();
void tetris_tick();
void tetris_terminate();


#endif