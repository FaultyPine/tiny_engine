#ifndef SHADER_H
#define SHADER_H

#include "tiny_defines.h"
#include "math/tiny_math.h"

struct Texture;
struct Cubemap;
struct Arena;

void InitializeShaderSystem(Arena* arena);
void ShaderSystemPreDraw();


struct Shader {
    // ID is not the OpenGL shader id!
    // this is to facilitate shader hot reloading - 
    // we can keep this same ID but have it refer to a different ogl shader program
    // these IDs are deterministically generated, and will not change when this or other shaders are deleted/reloaded/etc. 
    // so storing references to these IDs is fine
    u32 ID = U32_INVALID_ID;

    Shader() = default;
    TAPI Shader(const std::string& vertexPath, const std::string& fragmentPath);
    TAPI static Shader CreateShaderFromStr(const s8* vsCodeStr, const s8* fsCodeStr);

    TAPI void Delete();
    TAPI bool isValid() const { return ID != U32_INVALID_ID; }

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
    // NOTE: these do not do any opengl calls
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

    bool operator==(const Shader& p) const { return ID == p.ID; }
    bool operator!=(const Shader& p) const { return ID != p.ID; }
};


// contains vertex shader path, frag shader path
typedef std::pair<std::string, std::string> ShaderLocation;

// takes in a shader ID that receives uniforms, and a shader ID to supply the uniforms
// in the typical case, these will both be the same shader. I.E. a shader applies it's own uniforms to itself
// this also allows us to apply ANOTHER shader's uniforms to some other shader. I.E. for prepasses where 
// uniforms should be the same for the original shader and the prepass shader
void UseShaderAndSetUniforms(const Shader& shaderIDToReceive, const Shader& shaderIDForUniforms);
void RefreshShaderUniformLocations(u32 shaderID, u32 oglShaderProgram);
const ShaderLocation& GetShaderPaths(const Shader& shader);

struct ShaderHasher {
    size_t operator()(const Shader& p) const
    {
        return p.ID;
    }
};

#endif