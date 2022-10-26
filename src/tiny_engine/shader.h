#ifndef SHADER_H
#define SHADER_H

#include "pch.h"

// cachedUniformLocs is a map with keys being the shader ID and value being a
// map of uniform names mapped to uniform locations

// the alternative to this is having each shader carry it's cached uniforms map
// to prevent uniform name collisions, but i'd like to keep the shader class as a
// fancy wrapper on just the u32 ID



struct Shader {
    u32 ID;

    Shader() { ID = 0; }
    Shader(u32 id) { ID = id; }


    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    void Unload() const { glDeleteProgram(ID); }
    bool isValid() const { return ID != 0; }
    static u32 CreateAndCompileShader(u32 shaderType, const s8* shaderSource);
    static u32 CreateShaderProgFromStr(const s8* vsSource, const s8* fsSource);
    static Shader CreateShaderFromSources(const s8* vsSource, const s8* fsSource) {
        return Shader(CreateShaderProgFromStr(vsSource, fsSource));
    }

    // use/activate the shader
    inline void use() const {
        ASSERT("Invalid shader ID!\n" && ID);
        glUseProgram(ID); 
    }
    s32 getLoc(const std::string& uniformName) const;


    // utility uniform functions

    #define SET_UNIFORM_MACRO_FUNC_NAME setUniform

    #define DECLARE_SET_UNIFORM_FUNC1(glFunc, type)                                                 \
    inline void SET_UNIFORM_MACRO_FUNC_NAME(const s8* uniformName, type val) const {          \
        s32 loc = getLoc(uniformName);                                 \
        if (loc != -1) glFunc(loc, val);                                                            \
    }

    #define DECLARE_SET_UNIFORM_FUNC2(glFunc, type)                                                 \
    inline void SET_UNIFORM_MACRO_FUNC_NAME(const s8* uniformName, type val1, type val2) const {  \
        s32 loc = getLoc(uniformName);                                 \
        if (loc != -1) glFunc(loc, val1, val2);                                                     \
    }

    #define DECLARE_SET_UNIFORM_FUNC3(glFunc, type)                                                 \
    inline void SET_UNIFORM_MACRO_FUNC_NAME(const s8* uniformName, type val1, type val2, type val3) const {    \
        s32 loc = getLoc(uniformName);                                 \
        if (loc != -1) glFunc(loc, val1, val2, val3);                                               \
    }

    #define DECLARE_SET_UNIFORM_FUNC4(glFunc, type)                                                         \
    inline void SET_UNIFORM_MACRO_FUNC_NAME(const s8* uniformName, type val1, type val2, type val3, type val4) const { \
        s32 loc = getLoc(uniformName);                                         \
        if (loc != -1) glFunc(loc, val1, val2, val3, val4);                                                 \
    }

    // TODO: Decide whether or not to include an implementation for
    // f64. As of now, there'll be a weird looking error if you try to
    // pass a double to setUniform. Could generate a normal glUniformXf
    // impl for it, but that would implicit cast and lose precision without
    // anyone knowing.....

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
        s32 loc = getLoc(uniformName);
        if (loc != -1) glUniform2f(loc, vec2.x, vec2.y);
    }
    inline void setUniform(const s8* uniformName, glm::vec3 vec3) const {
        s32 loc = getLoc(uniformName);
        if (loc != -1) glUniform3f(loc, vec3.x, vec3.y, vec3.z);
    }
    inline void setUniform(const s8* uniformName, glm::vec4 vec4) const {
        s32 loc = getLoc(uniformName);
        if (loc != -1) glUniform4f(loc, vec4.x, vec4.y, vec4.z, vec4.w);
    }

    inline void setUniform(const s8* uniformName, glm::mat4 mat4, bool transpose = false) const {
        s32 loc = getLoc(uniformName);
        if (loc != -1) glUniformMatrix4fv(loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat4));
    }


};


#endif