#ifndef SHADER_H
#define SHADER_H

//#include "pch.h"
#include "tiny_defines.h"
//#include "tiny_ogl.h"
#include "tiny_math.h"

struct Shader {
    // ID is not necessarily the OpenGL shader id!
    // it is an index into a list of OGL shader ids which we can
    // change to facilitate shader hot reloading
    u32 ID = 0;
    bool valid = false;

    Shader() = default;
    TAPI Shader(u32 id);
    TAPI Shader(const std::string& vertexPath, const std::string& fragmentPath);
    TAPI static Shader CreateShaderFromStr(const s8* vsCodeStr, const s8* fsCodeStr);

    TAPI void Delete() const;
    bool isValid() const { return valid; }
    TAPI u32 GetOpenGLProgramID();

    /// Takes vertex/fragment shader code (as a string)
    TAPI void ActivateSamplers() const;
    /// attempts to add the texture to the sampler list. If the texture id alrady exists, does nothing
    TAPI void TryAddSampler(s32 texture, const char* uniformName) const;

    // use/activate the shader
    TAPI void use() const;
    TAPI s32 getLoc(const std::string& uniformName) const;

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