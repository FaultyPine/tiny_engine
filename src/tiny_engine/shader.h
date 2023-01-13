#ifndef SHADER_H
#define SHADER_H

#include "pch.h"
#include "texture.h"

// map of shader id -> list of sampler ids
// putting this outside the shader object to keep Shaders a wrapper around an ID
static std::unordered_map<u32, std::vector<s32>> samplerIDs = {};

struct Shader {
    u32 ID = 0;

    Shader() { ID = 0; }
    Shader(u32 id) { ID = id; }
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    void Delete() const { glDeleteProgram(ID); }
    bool isValid() const { return ID != 0; }
    /// Takes vertex/fragment shader code (as a string)
    static Shader CreateShaderFromStr(const s8* vsCodeStr, const s8* fsCodeStr) {
        return Shader(CreateShaderProgFromStr(vsCodeStr, fsCodeStr));
    }
    inline void ActivateSamplers() const {
        const std::vector<s32>& shaderSamplers = samplerIDs[ID];
        for (s32 i = 0; i < shaderSamplers.size(); i++) {
            Texture::bindUnit(i, shaderSamplers.at(i));
        }
    }
    /// attempts to add the texture to the sampler list. If the texture id alrady exists, does nothing
    inline bool TryAddSampler(s32 texture, const char* uniformName) const {
        std::vector<s32>& shaderSamplers = samplerIDs[ID];
        // don't add if this texture is already tracked
        if (std::find(shaderSamplers.begin(), shaderSamplers.end(), texture) != shaderSamplers.end()) return false;
        // this sampler needs to be added
        shaderSamplers.push_back(texture);
        use();
        setUniform(uniformName, (s32)shaderSamplers.size()-1);
        return true;
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
    static u32 CreateAndCompileShader(u32 shaderType, const s8* shaderSource);
    static u32 CreateShaderProgFromStr(const s8* vsSource, const s8* fsSource);
};


#endif