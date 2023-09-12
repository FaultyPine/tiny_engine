#ifndef SHADER_H
#define SHADER_H

#include "pch.h"


struct Shader {
    // ID is not necessarily the OpenGL shader id!
    // it is an index into a list of OGL shader ids which we can
    // change to facilitate shader hot reloading
    u32 ID = 0;
    bool valid = false;

    Shader() = default;
    Shader(u32 id);
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    static Shader CreateShaderFromStr(const s8* vsCodeStr, const s8* fsCodeStr);

    void Delete() const;
    bool isValid() const { return valid; }
    u32 GetOpenGLProgramID();

    /// Takes vertex/fragment shader code (as a string)
    void ActivateSamplers() const;
    /// attempts to add the texture to the sampler list. If the texture id alrady exists, does nothing
    void TryAddSampler(s32 texture, const char* uniformName) const;

    // use/activate the shader
    void use() const;
    s32 getLoc(const std::string& uniformName) const;

    // reloads all shaders
    static void ReloadShaders();
    // reloads this shader
    void Reload() const;

    // utility uniform functions

    #define SET_UNIFORM_MACRO_FUNC_NAME setUniform

    #define DECLARE_SET_UNIFORM_FUNC1(glFunc, type)                                                 \
    inline void SET_UNIFORM_MACRO_FUNC_NAME(const s8* uniformName, type val) const {          \
        s32 loc = getLoc(uniformName);                                 \
        if (loc != -1) GLCall(glFunc(loc, val));                                                            \
    }

    #define DECLARE_SET_UNIFORM_FUNC2(glFunc, type)                                                 \
    inline void SET_UNIFORM_MACRO_FUNC_NAME(const s8* uniformName, type val1, type val2) const {  \
        s32 loc = getLoc(uniformName);                                 \
        if (loc != -1) GLCall(glFunc(loc, val1, val2));                                                     \
    }

    #define DECLARE_SET_UNIFORM_FUNC3(glFunc, type)                                                 \
    inline void SET_UNIFORM_MACRO_FUNC_NAME(const s8* uniformName, type val1, type val2, type val3) const {    \
        s32 loc = getLoc(uniformName);                                 \
        if (loc != -1) GLCall(glFunc(loc, val1, val2, val3));                                               \
    }

    #define DECLARE_SET_UNIFORM_FUNC4(glFunc, type)                                                         \
    inline void SET_UNIFORM_MACRO_FUNC_NAME(const s8* uniformName, type val1, type val2, type val3, type val4) const { \
        s32 loc = getLoc(uniformName);                                         \
        if (loc != -1) GLCall(glFunc(loc, val1, val2, val3, val4));                                                 \
    }


    DECLARE_SET_UNIFORM_FUNC1(glUniform1f, f32)
    DECLARE_SET_UNIFORM_FUNC1(glUniform1i, s32)
    DECLARE_SET_UNIFORM_FUNC1(glUniform1ui, u32)

    DECLARE_SET_UNIFORM_FUNC2(glUniform2f, f32)
    DECLARE_SET_UNIFORM_FUNC2(glUniform2i, s32)
    DECLARE_SET_UNIFORM_FUNC2(glUniform2ui, u32)

    DECLARE_SET_UNIFORM_FUNC3(glUniform3f, f32)
    DECLARE_SET_UNIFORM_FUNC3(glUniform3i, s32)
    DECLARE_SET_UNIFORM_FUNC3(glUniform3ui, u32)

    DECLARE_SET_UNIFORM_FUNC4(glUniform4f, f32)
    DECLARE_SET_UNIFORM_FUNC4(glUniform4i, s32)
    DECLARE_SET_UNIFORM_FUNC4(glUniform4ui, u32)

    inline void setUniform(const s8* uniformName, glm::vec2 vec2) const {
        setUniform(uniformName, vec2.x, vec2.y);
    }
    inline void setUniform(const s8* uniformName, glm::vec3 vec3) const {
        setUniform(uniformName, vec3.x, vec3.y, vec3.z);
    }
    inline void setUniform(const s8* uniformName, glm::vec4 vec4) const {
        setUniform(uniformName, vec4.x, vec4.y, vec4.z, vec4.w);
    }

    inline void setUniform(const s8* uniformName, glm::mat4 mat4, bool transpose = false) const {
        s32 loc = getLoc(uniformName);
        if (loc != -1) GLCall(glUniformMatrix4fv(loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat4)));
    }
    inline void setUniform(const s8* uniformName, glm::mat3 mat3, bool transpose = false) const {
        s32 loc = getLoc(uniformName);
        if (loc != -1) GLCall(glUniformMatrix3fv(loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat3)));
    }

private:
    void InitShaderFromProgramID(u32 shaderProgram, const std::string& vertexPath = "", const std::string& fragmentPath = "");
};


#endif