#ifndef TINY_SKYBOX_H
#define TINY_SKYBOX_H


#include "cubemap.h"
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "tiny_fs.h"


struct Skybox {
    Cubemap cubemap = {};
    Skybox(){}
    Skybox(const std::vector<const char*>& facesPaths, TextureProperties props = TextureProperties::RGB_LINEAR());
    /// Ensure this is the last thing drawn in the scene for it to have the proper effect
    void Draw();

    static Shader skyboxShader;
    static Mesh skyboxCube;
};


#endif