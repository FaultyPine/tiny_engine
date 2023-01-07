#include "title_screen_scene.h"

#include "controller_setup_scene.h"
#include "tiny_engine/tiny_fs.h"

void TitleScreenSceneInit(GameState& gs) {
    gs.scene = PotpScene::TITLE;
    if (!gs.titleScreenSplash.isValid()) {
        TextureProperties texProps = TextureProperties::RGB_NEAREST();
        gs.titleScreenSplash = Sprite(LoadTexture(ResPath("potp/title_screen.jpg").c_str(), texProps));
    }

    if (!gs.titleScreenText)
        gs.titleScreenText = CreateText("Press Tab to start");
}
void TitleScreenSceneTick(GameState& gs, UserInput& inputs) {
    for (s32 i = 0; i < MAX_NUM_PLAYERS; i++) {
        bool isStartPressed = pollRawInput(i, ControllerType::KEYBOARD, ButtonValues::START) || pollRawInput(i, ControllerType::CONTROLLER, ButtonValues::START);
        if (isStartPressed) {
            ChangeScene(PotpScene::CONTROLLER_SETUP, gs);
        }
    }
}
void TitleScreenSceneDraw(const GameState& gs, const UserInput& inputs) {
    Camera& cam = Camera::GetMainCamera();
    gs.titleScreenSplash.DrawSprite(cam, {0.0, 0.0}, {Camera::GetScreenWidth(), Camera::GetScreenHeight()}, 0.0, {0.0, 0.0, 1.0}, {1.0, 1.0, 1.0, 1.0});
    DrawText(gs.titleScreenText, Camera::GetScreenWidth() - 250.0, Camera::GetScreenHeight() -50.0, 1.5, 1.0, 1.0, 1.0, 1.0);
}
void TitleScreenSceneEnd(GameState& gs) {

}