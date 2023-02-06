#include "game_main.h"

#include "tiny_engine/camera.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/shapes.h"
#include "tiny_engine/external/imgui/tiny_imgui.h"
#include "tiny_engine/tiny_fs.h"
#include "tiny_engine/tiny_profiler.h"
#include "PoissonGenerator.h"
#include "rundata.h"
#include "QuadTree.h"

void PollInputs() {
    if (Keyboard::isKeyDown(GLFW_KEY_ESCAPE)) {
        CloseGameWindow();
    }
    // Press tab to toggle locking the cursor into the window/being able to move the cursor around normally
    if (Keyboard::isKeyPressed(GLFW_KEY_TAB)) {
        // when "tabbing" in and out of the game, the cursor position jumps around weirdly
        // so here we save the last cursor pos when we tab out, and re-set it when we tab back in
        static glm::vec2 lastMousePos = glm::vec2(0);
        if (glfwGetInputMode(glob_glfw_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
            glfwSetInputMode(glob_glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPos(glob_glfw_window, lastMousePos.x, lastMousePos.y);
        }
        else {
            lastMousePos = {MouseInput::GetMouse().lastX, MouseInput::GetMouse().lastY};
            glfwSetInputMode(glob_glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  
        }
    }

}
void DrawDebug() {
    ImGuiBeginFrame();
    ImGui::Text("%.3fms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGuiEndFrame();
}


void DrawGame(GameState& gs, NonGameState& ngs) {
    PROFILE_FUNCTION();
    glm::vec2 screenSize = {Camera::GetScreenWidth(), Camera::GetScreenHeight()};
    for (auto& npc : gs.npcs) {
        //ngs.character.DrawSprite(npc.tf);
    }
    gs.tree.Draw();
    DrawDebug();
}




void game_init() {
    PROFILE_FUNCTION();
    InitImGui();
    NonGameState& ngs = Rundata::Instance().ngs;
    GameState& gs = Rundata::Instance().gs;
    u32 screenWidth = Camera::GetScreenWidth();
    u32 screenHeight = Camera::GetScreenHeight();
    gs.tree = QuadTree<NPC*>(BoundingBox2D(glm::vec2(0), {screenWidth, screenHeight}));

    ngs.character = Sprite(LoadTexture(ResPath("other/awesomeface.png")));

    u32 numChars = 20;
    // points is a vector of xyz in the range 0-1
    PoissonGenerator::DefaultPRNG PRNG;
	const auto Points = PoissonGenerator::generatePoissonPoints( numChars, PRNG );

    for (u32 i = 0; i < numChars; i++) {
        NPC ent = {};
        glm::vec2 pos = glm::vec2(Points[i].x*screenWidth, Points[i].y*screenHeight);
        ent.tf = Transform2D(pos, glm::vec2(10));
        ent.desiredPosition = glm::vec2(Points[i+1].x*screenWidth, Points[i+1].y*screenHeight);
        gs.npcs.push_back(ent);
        Node nodeToInsert = Node(pos, &gs.npcs.back());
        gs.tree.insert(nodeToInsert);
    }
    std::cout << "Finished init\n";

}

void EntitiesTick(GameState& gs) {
    PROFILE_FUNCTION();
    u32 screenWidth = Camera::GetScreenWidth();
    u32 screenHeight = Camera::GetScreenHeight();
    constexpr f32 moveSpeed = 50.0f;
    for (auto& npc : gs.npcs) {
        glm::vec2 moveDelta = glm::normalize(npc.desiredPosition - npc.tf.position) * moveSpeed;
        npc.tf.position += moveDelta * GetDeltaTime();

        if (glm::distance(npc.tf.position, npc.desiredPosition) <= 1.0f) {
            npc.desiredPosition = glm::vec2(GetRandomf(0.0, 1.0) * screenWidth, GetRandomf(0.0, 1.0) * screenHeight);
        }
    }

    { PROFILE_SCOPE("QuadTreeRefresh");
        // update quadtree with new positions
        gs.tree = QuadTree<NPC*>(BoundingBox2D(glm::vec2(0), {screenWidth, screenHeight}));
        for (auto& npc : gs.npcs) {
            gs.tree.insert(Node(npc.tf.position, &npc));
        }
    }

    const BoundingBox2D searchArea = BoundingBox2D({100, 100}, {250, 250});
    std::vector<NPC*> npcsInRange = {};
    gs.tree.search(searchArea, npcsInRange);
    for (auto& x : npcsInRange) {
        Shapes2D::DrawCircle(x->tf.position, 5.0f, glm::vec4(1, 0, 0, 1));
    }
    Shapes2D::DrawWireframeSquare(searchArea.min, searchArea.max, glm::vec4(1, 0, 0, 1), 3.0f);
}

void game_tick() {
    NonGameState& ngs = Rundata::Instance().ngs;
    GameState& gs = Rundata::Instance().gs;

    PollInputs();
    EntitiesTick(gs);
    DrawGame(gs, ngs);
}
void game_terminate() {
    ImGuiTerminate();
}