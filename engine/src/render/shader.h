#ifndef SHADER_H
#define SHADER_H

//#include "pch.h"
#include "tiny_defines.h"
//#include "tiny_ogl.h"
#include "math/tiny_math.h"
#include "tiny_alloc.h"

void InitializeShaderSystem(Arena* arena, size_t shaderUniformDataBlockSize);

struct Texture;
struct Cubemap;

struct Shader {
    // ID is not the OpenGL shader id!
    // this is to facilitate shader hot reloading - 
    // we can keep this same ID but have it refer to a different ogl shader program
    u32 ID = 0xDEADBEEF;

    Shader() = default;
    TAPI Shader(u32 id);
    TAPI Shader(const std::string& vertexPath, const std::string& fragmentPath);
    TAPI static Shader CreateShaderFromStr(const s8* vsCodeStr, const s8* fsCodeStr);

    TAPI void Delete() const;
    bool isValid() const { return ID != 0xDEADBEEF; }

    // adds a sampler to this shader
    TAPI void TryAddSampler(const Texture& texture, const char* uniformName) const;
    TAPI void TryAddSampler(const Cubemap& texture, const char* uniformName) const;

    // use/activate the shader
    TAPI void use() const;

    // reloads all shaders
    TAPI static void ReloadShaders();
    // reloads this shader
    TAPI void Reload() const;

    // utility uniform functions
    TAPI void setUniform(const s8* uniformName, f32 val) const;
    TAPI void setUniform(const s8* uniformName, f32 val, f32 val2) const;
    TAPI void setUniform(const s8* uniformName, f32 val, f32 val2, f32 val3) const;
    TAPI void setUniform(const s8* uniformName, f32 val, f32 val2, f32 val3, f32 val4) const;
    
    TAPI void setUniform(const s8* uniformName, s32 val) const;
    TAPI void setUniform(const s8* uniformName, s32 val, s32 val2) const;
    TAPI void setUniform(const s8* uniformName, s32 val, s32 val2, s32 val3) const;
    TAPI void setUniform(const s8* uniformName, s32 val, s32 val2, s32 val3, s32 val4) const;
    
    TAPI void setUniform(const s8* uniformName, u32 val) const;
    TAPI void setUniform(const s8* uniformName, u32 val, u32 val2) const;
    TAPI void setUniform(const s8* uniformName, u32 val, u32 val2, u32 val3) const;
    TAPI void setUniform(const s8* uniformName, u32 val, u32 val2, u32 val3, u32 val4) const;

    TAPI inline void setUniform(const s8* uniformName, glm::vec2 vec2) const {
        setUniform(uniformName, vec2.x, vec2.y);
    }
    TAPI inline void setUniform(const s8* uniformName, glm::vec3 vec3) const {
        setUniform(uniformName, vec3.x, vec3.y, vec3.z);
    }
    TAPI inline void setUniform(const s8* uniformName, glm::vec4 vec4) const {
        setUniform(uniformName, vec4.x, vec4.y, vec4.z, vec4.w);
    }

    TAPI void setUniform(const s8* uniformName, glm::mat4 mat4, bool transpose = false) const;
    TAPI void setUniform(const s8* uniformName, glm::mat3 mat3, bool transpose = false) const;


private:
    void InitShaderFromProgramID(u32 shaderProgram, const std::string& vertexPath = "", const std::string& fragmentPath = "");
};


#endif