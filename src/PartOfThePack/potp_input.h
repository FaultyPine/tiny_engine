#ifndef POTP_INPUT_H
#define POTP_INPUT_H


#include "tiny_engine/pch.h"

struct UserInput {
    enum ButtonValues {
        NONE = 0,
        UP = 1 << 0,
        DOWN = 1 << 1,
        LEFT = 1 << 2,
        RIGHT = 1 << 3,
        ACTION1 = 1 << 4, // punch
        ACTION2 = 1 << 5, // smoke
    };
    #define MAX_NUM_PLAYERS 4
    // 1 bitfield of buttons pressed for each player
    u32 buttons[MAX_NUM_PLAYERS];
    u32 prevButtons[MAX_NUM_PLAYERS];

    u32 getButtons(u32 playerIdx) const { return buttons[playerIdx]; }
    u32 getPrevButtons(u32 playerIdx) const { return prevButtons[playerIdx]; }
    inline bool isButton(u32 playerIdx, ButtonValues button) const { return getButtons(playerIdx) & button; }
    inline bool isPrevButton(u32 playerIdx, ButtonValues button) const { return getPrevButtons(playerIdx) & button; }
    inline bool isButtonJustPressed(u32 playerIdx, ButtonValues button) const { return isButton(playerIdx, button) && !isPrevButton(playerIdx, button); }
    inline bool isButtonJustReleased(u32 playerIdx, ButtonValues button) const { return !isButton(playerIdx, button) && isPrevButton(playerIdx, button); }


    bool isUp(u32 playerIdx) const {
        return isButton(playerIdx, ButtonValues::UP);
    }
    bool isDown(u32 playerIdx) const {
        return isButton(playerIdx, ButtonValues::DOWN);
    }
    bool isLeft(u32 playerIdx) const {
        return isButton(playerIdx, ButtonValues::LEFT);
    }
    bool isRight(u32 playerIdx) const {
        return isButton(playerIdx, ButtonValues::RIGHT);
    }
    bool isAction1(u32 playerIdx) const {
        return isButton(playerIdx, ButtonValues::ACTION1);
    }
    bool isAction2(u32 playerIdx) const {
        return isButton(playerIdx, ButtonValues::ACTION2);
    }

    bool isUpPressed(u32 playerIdx) const {
        return isButtonJustPressed(playerIdx, ButtonValues::UP);
    }
    bool isDownPressed(u32 playerIdx) const {
        return isButtonJustPressed(playerIdx, ButtonValues::DOWN);
    }
    bool isLeftPressed(u32 playerIdx) const {
        return isButtonJustPressed(playerIdx, ButtonValues::LEFT);
    }
    bool isRightPressed(u32 playerIdx) const {
        return isButtonJustPressed(playerIdx, ButtonValues::RIGHT);
    }
    bool isAction1Pressed(u32 playerIdx) const {
        return isButtonJustPressed(playerIdx, ButtonValues::ACTION1);
    }
    bool isAction2Pressed(u32 playerIdx) const {
        return isButtonJustPressed(playerIdx, ButtonValues::ACTION2);
    }

    static void UpdateUserInput(UserInput& input);
};


#endif