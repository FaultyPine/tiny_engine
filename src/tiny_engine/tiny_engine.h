#ifndef OPENGL_WRAPPER_H
#define OPENGL_WRAPPER_H

#include "pch.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void CloseGameWindow();


inline std::string Vec3ToStr(const glm::vec3& vec) {
    return std::to_string(vec.x) + " " + std::to_string(vec.y) + " " + std::to_string(vec.z);
}
inline std::string Vec2ToStr(const glm::vec2& vec) {
    return std::to_string(vec.x) + " " + std::to_string(vec.y);
}

// returns the current GLFW time
double GetTime();
long long GetTimeSinceEpoch();

void InitializeRandomSeed(u64 seed);
u32 GetRandom(u32 start = 0, u32 end = 1);
long long GetCPUCycles();

f32 GetDeltaTime();

u32 GetFrameCount();

/// Game loop should be while(!ShouldCloseWindow())
bool ShouldCloseWindow();

void TerminateGame();

void InitGame(u32 windowWidth, u32 windowHeight, const s8* windowName);


void EnableWireframeDrawing();


#endif