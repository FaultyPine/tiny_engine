//#include "pch.h"
#include "shader.h"
#include "texture.h"
#include "cubemap.h"
#include "tiny_fs.h"
#include "tiny_ogl.h"
#include "tiny_engine.h"
#include <unordered_map>
#include <set>
#include <sstream>
#include <string>
#include <charconv>
#include <xstring>

#include "camera.h"
#include "tiny_lights.h"
#include "tiny_profiler.h"
#include "tiny_material.h"
#include "shader_buffer.h"

enum UniformDataType : s32
{
    FLOAT,
    UINT,
    SINT,
    VEC2,
    VEC3,
    VEC4,
    MAT3,
    MAT4,
    NUM_DATA_TYPES,
};
static const u32 UniformDataTypeSizes[NUM_DATA_TYPES] = 
{
    sizeof(f32),
    sizeof(u32),
    sizeof(s32),
    sizeof(f32) * 2, // vec2
    sizeof(f32) * 3, // vec3
    sizeof(f32) * 4, // vec4
    sizeof(f32) * 9, // mat3
    sizeof(f32) * 16, // mat4
};



struct UniformData
{
    // pointer into the dedicated uniform memory block given to us by the engine
    void* uniformData = 0;
    u32 uniformSize = 0;
    UniformDataType dataType;
    s32 uniformLocation = 0;
    // TODO: dirty flag and only set when we actually change the data
};


struct ShaderInternal
{
    u32 oglShaderProgram = 0;
    ShaderLocation filepaths = {};
    std::vector<Texture> samplerIDs = {};
    std::unordered_map<std::string, UniformData> cachedUniforms = {};
};

struct GlobalShaderState
{
    ShaderBufferGlobals globals = {};
    std::unordered_map<u32, ShaderInternal> shaderMap = {};
    Arena globalShaderMem = {};
};

static GlobalShaderState& GetGSS()
{
    return *GetEngineCtx().shaderSubsystem;
}


void InitializeShaderSystem(Arena* arena)
{
    GlobalShaderState*& gss = GetEngineCtx().shaderSubsystem;
    gss = (GlobalShaderState*)arena_alloc(arena, sizeof(GlobalShaderState));
    new(&gss->shaderMap) std::unordered_map<u32, ShaderInternal>();

    u32 shaderMemBlockSize = Math::PercentOf(get_free_space(arena), 10);;
    void* globalShaderMem = arena_alloc(arena, shaderMemBlockSize);
    gss->globalShaderMem = arena_init(globalShaderMem, shaderMemBlockSize);
    InitializeUBOs(gss->globals);
}

const ShaderLocation& GetShaderPaths(const Shader& shader)
{
    GlobalShaderState& gss = GetGSS();
    return gss.shaderMap.at(shader.ID).filepaths;
}

void ShaderSystemPreDraw()
{
    // passes through to shader_buffer impl
    ShaderSystemPreDraw(GetGSS().globals);
}


s32 GetShaderErrorLineNum(s8* infoLog, u32 infoLogSize)
{
    std::string str = std::string(infoLog, infoLogSize);
    size_t errorLineStrEnd = str.find(") : error ", 0); // exclusive
    size_t errorLineStrStart = str.rfind("(", errorLineStrEnd)+1; // inclusive
    std::string errorLineNum = std::string(infoLog + errorLineStrStart, errorLineStrEnd - errorLineStrStart);
    //s32 lineNum = -1;
    //auto result = std::from_chars(errorLineNum.data(), errorLineNum.data() + errorLineNum.size(), lineNum);
    char** end = 0;
    s32 lineNum = strtol(errorLineNum.data(), end, 10);
    //if (result.ec == std::errc::invalid_argument) {
        //LOG_WARN("Could not convert.");
    //}
    return lineNum;
}

void LogShaderLinesAroundError(s8* infoLog, u32 infoLogSize, const s8* shaderSource)
{
    s32 errorLineNum = GetShaderErrorLineNum(infoLog, infoLogSize);
    if (errorLineNum != -1)
    {
        s32 currentLineNum = 0;
        for (s32 i = 0; i < strlen(shaderSource); i++)
        {
            if (shaderSource[i] == '\n')
            {
                currentLineNum++;
                constexpr s32 errorLinesPrintRage = 1; // num lines to print around the erroneous line
                if (Math::isInRange(currentLineNum, errorLineNum-errorLinesPrintRage, errorLineNum+errorLinesPrintRage))
                {
                    s32 col = 1;
                    for (; col < 500 && shaderSource[i+col] != '\n'; col++) {}
                    const char* errorLineStr = shaderSource + i;
                    LOG_ERROR("%.*s", col, errorLineStr);
                }
            }
        }
    }
}

bool CreateAndCompileShader(u32 shaderType, const s8* shaderSource, u32& shaderOut) 
{
    u32 shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &shaderSource, NULL);
    glCompileShader(shaderID);
    s32 successCode;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &successCode);
    if (!successCode) {
        const u32 infoLogSize = 1024;
        s8 infoLog[infoLogSize];
        glGetShaderInfoLog(shaderID, infoLogSize, NULL, infoLog);
        LogShaderLinesAroundError(infoLog, infoLogSize, shaderSource);
        LOG_ERROR("%s shader compilation failed. shaderID = %i\n%s", (shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment"), shaderID, infoLog);
        return false;
    }
    shaderOut = shaderID;
    return true;
}


std::string ShaderPreprocessIncludes(
    const s8* shaderSource, 
    const std::string& includeIdentifier, 
    const std::string& includeSearchDir, 
    std::set<std::string>& alreadyIncluded) 
{
    static bool isRecursiveCall = false;
    std::string fullSourceCode = "";
    std::string lineBuffer;
    std::istringstream stream = std::istringstream(shaderSource);
    // TODO: custom strings
    while (std::getline(stream, lineBuffer)) {
        // if include is in this line
        if (lineBuffer.find(includeIdentifier) != lineBuffer.npos && lineBuffer.find("//") == lineBuffer.npos) {
            // Remove the include identifier, this will cause the path to remain
            lineBuffer.erase(0, includeIdentifier.size());
            lineBuffer = lineBuffer.erase(0, 1);
            lineBuffer.erase(lineBuffer.size() - 1, 1); // remove ""
            // The include path is relative to the current shader file path
            std::string path = includeSearchDir + lineBuffer;
            // if we haven't already included this in the compilation unit
            if (alreadyIncluded.count(path) == 0)
            {
                std::string nextFile;
                if (ReadEntireFile(path.c_str(), nextFile)) {
                    alreadyIncluded.insert(path);
                    // recursively process included file
                    isRecursiveCall = true;
                    std::string recursiveShaderSource = ShaderPreprocessIncludes(nextFile.c_str(), includeIdentifier, includeSearchDir, alreadyIncluded);
                    fullSourceCode += recursiveShaderSource;
                }
                else {
                    LOG_ERROR("Failed to open shader include: %s", path.c_str());
                    TINY_ASSERT(false);
                }
            }

            // don't add the actual "#include blah.blah" line in the final source
            continue;
        }

        fullSourceCode += lineBuffer + '\n';
    }

    // null terminate the very end of the file
    if (!isRecursiveCall)
        fullSourceCode += '\0';

    return fullSourceCode;
}

enum ShaderCompileFlags: u32
{
    VERTEX_SHADER =   (1 << 0),
    FRAGMENT_SHADER = (1 << 1),
    COMPUTE_SHADER =  (1 << 2),
};

std::string ShaderSourcePreprocess(const s8* shaderSource, u32 flags) 
{
    // take in original source code and run some procedure(s) on it and return the "processed" source code
    std::string ret;

    ret += "#version 440 core\n";

    if ((flags & ShaderCompileFlags::VERTEX_SHADER) != 0)
    {
        ret += "#define VERTEX_SHADER\n";
    }
    else if ((flags & ShaderCompileFlags::FRAGMENT_SHADER) != 0)
    {
        ret += "#define FRAGMENT_SHADER\n";
    }
    else if ((flags & ShaderCompileFlags::COMPUTE_SHADER) != 0)
    {
        ret += "#define COMPUTE_SHADER\n";
    }

    std::string includeSearchDir = ResPath("shaders/");
    static const char* includeIdentifier = "#include "; // space after it so #include"hi.bye" is invalid. Must be #include "hi.bye"
    std::set<std::string> alreadyIncluded = {};
    ret += ShaderPreprocessIncludes(shaderSource, "#include ", includeSearchDir, alreadyIncluded);

    return ret;
}


static const char* fallbackFragShader = R"shad(
out vec4 outColor;
void main()
{
    outColor = vec4(1.0, 0.0, 0.5, 1.0);
}
)shad";
static const char* fallbackVertShader = R"shad(
layout (location = 0) in vec3 vertPos;
void main()
{
    gl_Position = vec4(vertPos, 1.0);
}
)shad";


u32 CreateShaderProgramFromStr(
    const s8* vsSource, 
    const s8* fsSource, 
    const std::string& vertPath = "", 
    const std::string& fragPath = "") 
{
    // preprocess both vert and frag shader source code before compiling
    std::string vsSourcePreprocessed = ShaderSourcePreprocess(vsSource, ShaderCompileFlags::VERTEX_SHADER);
    std::string fsSourcePreprocessed = ShaderSourcePreprocess(fsSource, ShaderCompileFlags::FRAGMENT_SHADER);

    u32 vertexShader = 0;
    if (!CreateAndCompileShader(GL_VERTEX_SHADER, vsSourcePreprocessed.c_str(), vertexShader) && !vertPath.empty())
    {
        LOG_ERROR("%s failed to compile", vertPath.c_str());
    }
    u32 fragShader = 0;
    if (!CreateAndCompileShader(GL_FRAGMENT_SHADER, fsSourcePreprocessed.c_str(), fragShader) && !fragPath.empty())
    {
        LOG_ERROR("%s failed to compile", fragPath.c_str());
    }

    u32 shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    s32 success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        const u32 infoLogSize = 1024;
        s8 infoLog[infoLogSize];
        glGetProgramInfoLog(shaderProgram, infoLogSize, NULL, infoLog);
        //LOG_ERROR("Vertex shader source: %s\n", vsSourcePreprocessed);
        //LOG_ERROR("Fragment shader source: %s\n", fsSourcePreprocessed);
        LOG_ERROR("shader linking failed. vs = %s fs = %s\n%s", vertPath.c_str(), fragPath.c_str(), infoLog);
        TINY_ASSERT(false);
        return CreateShaderProgramFromStr(fallbackVertShader, fallbackFragShader, vertPath, fragPath);
    }
    // delete vert/frag shader after we've linked them to the program object
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
    HandleShaderUBOInit(shaderProgram);
    return shaderProgram;
}

u32 CreateShaderFromFiles(const std::string& vertexPath, const std::string& fragmentPath) 
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    if (!ReadEntireFile(vertexPath.c_str(), vertexCode))
    {
        LOG_ERROR("Failed to read vertex shader file: %s", vertexPath.c_str());
        vertexCode = fallbackVertShader;
    }
    if (!ReadEntireFile(fragmentPath.c_str(), fragmentCode))
    {
        LOG_ERROR("Failed to read fragment shader file: %s", fragmentPath.c_str());
        fragmentCode = fallbackFragShader;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();
    u32 shaderProgram = CreateShaderProgramFromStr(vShaderCode, fShaderCode, vertexPath, fragmentPath);
    return shaderProgram;
}


static u32 GetOpenGLProgramID(u32 shaderID)
{
    return GetGSS().shaderMap.at(shaderID).oglShaderProgram;
}

u32 InitShaderFromProgramID(u32 shaderProgram, const std::string& vertexPath = "", const std::string& fragmentPath = "") 
{
    GlobalShaderState& gss = GetGSS();
    // ID is the index into the loadedShaders list that contains the OGL shader id
    u32 shaderID = gss.shaderMap.size();
    gss.shaderMap[shaderID].oglShaderProgram = shaderProgram;
    gss.shaderMap[shaderID].filepaths = std::make_pair(vertexPath, fragmentPath);
    return shaderID;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) 
{
    u32 shaderProgram = CreateShaderFromFiles(vertexPath, fragmentPath);
    ID = InitShaderFromProgramID(shaderProgram, vertexPath, fragmentPath);
}

Shader Shader::CreateShaderFromStr(const s8* vsCodeStr, const s8* fsCodeStr) 
{
    Shader shader;
    shader.ID = InitShaderFromProgramID(CreateShaderProgramFromStr(vsCodeStr, fsCodeStr));
    return shader;
}

void Shader::Delete() 
{
    TINY_ASSERT(false && "Proper shader deletion is currently unimplemented!");
    GlobalShaderState& gss = GetGSS();
    glDeleteShader(this->ID);
    gss.shaderMap.erase(this->ID);
    this->ID = U32_INVALID_ID;
}

void RefreshShaderUniformLocations(u32 shaderID, u32 oglShaderProgram)
{
    GlobalShaderState& gss = GetGSS();
    for (auto& [uniformName, uniformData] : gss.shaderMap[shaderID].cachedUniforms)
    {
        s32 loc = glGetUniformLocation(oglShaderProgram, uniformName.c_str());
        uniformData.uniformLocation = loc;
    }
}

void ReloadShader(u32 shaderID) 
{ // shader "id" (not opengl shader program)
    GlobalShaderState& gss = GetGSS();
    ShaderInternal& shaderInternal = gss.shaderMap[shaderID];
    u32 oglShaderProgram = shaderInternal.oglShaderProgram;
    const ShaderLocation& shaderLocations = shaderInternal.filepaths;
    // if shader locations are blank, this shader probably came from a string (cant reload)
    if (shaderLocations.first.empty() || shaderLocations.second.empty()) return;

    // TODO: check if (either frag/vert) file timestamp changed
    bool wasShaderFilesChanged = true;
    if (wasShaderFilesChanged) {
        u32 newShaderProgram = CreateShaderFromFiles(shaderLocations.first, shaderLocations.second);
        LOG_INFO("New reloaded shader %i", newShaderProgram);
        glDeleteProgram(oglShaderProgram);
        shaderInternal.oglShaderProgram = newShaderProgram;
        // new ogl shader, old uniform locations are now invalid
        RefreshShaderUniformLocations(shaderID, newShaderProgram);
    }
}
void Shader::Reload() const 
{
    ReloadShader(this->ID);
}
void Shader::ReloadShaders() {
    GlobalShaderState& gss = GetGSS();
    for (auto& [shaderID, shaderInternal] : gss.shaderMap) {
        ReloadShader(shaderID);
    }
    LOG_INFO("Reloaded shaders!");
}



void ActivateSamplers(u32 shaderID) 
{
    PROFILE_FUNCTION();
    GlobalShaderState& gss = GetGSS();
    const std::vector<Texture>& shaderSamplers = gss.shaderMap[shaderID].samplerIDs;
    for (s32 i = 0; i < shaderSamplers.size(); i++) {
        const Texture& tex = shaderSamplers.at(i);
        tex.bindUnit(i);
    }
}

void Shader::TryAddSampler(const Texture& texture, const char* uniformName) const 
{
    GlobalShaderState& gss = GetGSS();
    if (!texture.isValid())
    {
        //LOG_WARN("Tried to add sampler on invalid texture");
        return;
    }
    std::vector<Texture>& shaderSamplers = gss.shaderMap[ID].samplerIDs;
    s32 samplerIdx = -1;
    for (s32 i = 0; i < shaderSamplers.size(); i++)
    {
        if (shaderSamplers[i].id == texture.id)
        {
            samplerIdx = i;
            break;
        }
    }
    if (samplerIdx == -1) 
    {
        samplerIdx = shaderSamplers.size();
        shaderSamplers.push_back(texture);
    }
    // when we pass texture id to our shader, it needs to use the glUniform1i (<- signed) which is why this should be s32 not u32
    setUniform(uniformName, samplerIdx);
}
void Shader::TryAddSampler(const Cubemap& texture, const char* uniformName) const
{
    Texture cubemapTex = Texture::FromGPUTex(texture.id, texture.width, texture.height, GL_TEXTURE_CUBE_MAP);
    this->TryAddSampler(cubemapTex, uniformName);
}

void updateUniformData(
    u32 ID, 
    const std::string& uniformName, 
    void* uniformData, 
    u32 uniformSize, 
    UniformDataType dataType) 
{
    PROFILE_FUNCTION();
    GlobalShaderState& gss = GetGSS();
    TINY_ASSERT(gss.globalShaderMem.backing_mem_size > 0 && "Make sure to call InitializeShaderSystem before doing any shader calls!");
    std::unordered_map<std::string, UniformData>& cachedUniforms = gss.shaderMap[ID].cachedUniforms;
    // if we already have uniform - simply update cached values. If we don't, allocate more mem in our uniform mem block
    // NOTE/TODO: would like to be able to free uniform mem. Rn we just arena alloc new mem and never free it up
    // maybe use our fixed block allocator and have all uniforms be sizeof(mat4) so we can easily free/reuse chunks in the middle
    if (cachedUniforms.count(uniformName)) 
    {
        UniformData& uniform = cachedUniforms[uniformName];
        TINY_ASSERT(dataType == uniform.dataType && uniformSize == uniform.uniformSize);
        TMEMCPY(uniform.uniformData, uniformData, uniformSize);
    }
    else {
        // new uniform, cache it
        u32 oglShaderID = GetOpenGLProgramID(ID);
        s32 loc = glGetUniformLocation(oglShaderID, uniformName.c_str());
        if (loc != -1) {
            void* uniformDataPersistent = arena_alloc(&gss.globalShaderMem, uniformSize);
            TMEMCPY(uniformDataPersistent, uniformData, uniformSize);
            UniformData uniform = {};
            uniform.uniformLocation = loc;
            uniform.uniformData = uniformDataPersistent;
            uniform.dataType = dataType;
            uniform.uniformSize = uniformSize;
            cachedUniforms[uniformName] = uniform;
        }
        else {
            //LOG_WARN("Shader uniform %s either isn't defined or is unused", uniformName);
        }
    }
}

void TransferUniforms(const Shader& src, const Shader& dst)
{
    GlobalShaderState& gss = GetGSS();
    const auto& uniformMap = gss.shaderMap[src.ID].cachedUniforms;
    for (const auto& [uniformName, uniformData] : uniformMap)
    {
        updateUniformData(dst.ID, uniformName, uniformData.uniformData, uniformData.uniformSize, uniformData.dataType);
    }
}

void SetOglUniformFromBuffer(const char* uniformName, const UniformData& uniform);

void UseShaderAndSetUniforms(const Shader& shaderIDToReceive, const Shader& shaderIDForUniforms)
{
    PROFILE_FUNCTION();
    GlobalShaderState& gss = GetGSS();
    u32 oglShaderID = GetOpenGLProgramID(shaderIDToReceive.ID);
    glUseProgram(oglShaderID); 
    if (shaderIDToReceive != shaderIDForUniforms)
    {
        TransferUniforms(shaderIDForUniforms, shaderIDToReceive);
    }
    ActivateSamplers(shaderIDForUniforms.ID);
    const auto& uniformMap = gss.shaderMap[shaderIDToReceive.ID].cachedUniforms;
    for (const auto& [uniformName, uniformData] : uniformMap)
    {
        SetOglUniformFromBuffer(uniformName.c_str(), uniformData);
    }
}

void Shader::use() const 
{
    TINY_ASSERT("Invalid shader ID!" && isValid());
    UseShaderAndSetUniforms(*this, *this);
}



// ========= Shader setUniform overloads =============================

#define SET_UNIFORM_IMPL(uniformName, uniformType, uniformDataCopyArr) \
u32 uniformSize = UniformDataTypeSizes[uniformType] * ARRAY_SIZE(uniformDataCopyArr); \
updateUniformData(this->ID, uniformName, uniformDataCopyArr, uniformSize, uniformType)
// -------------------


void Shader::setUniform(const s8* uniformName, f32 val) const
{
    f32 uniformData[1] = {val};
    SET_UNIFORM_IMPL(uniformName, FLOAT, uniformData);
}
void Shader::setUniform(const s8* uniformName, f32 val, f32 val2) const
{
    f32 uniformData[2] = {val, val2};
    SET_UNIFORM_IMPL(uniformName, FLOAT, uniformData);
}
void Shader::setUniform(const s8* uniformName, f32 val, f32 val2, f32 val3) const
{
    f32 uniformData[3] = {val, val2, val3};
    SET_UNIFORM_IMPL(uniformName, FLOAT, uniformData);
}
void Shader::setUniform(const s8* uniformName, f32 val, f32 val2, f32 val3, f32 val4) const
{
    f32 uniformData[4] = {val, val2, val3, val4};
    SET_UNIFORM_IMPL(uniformName, FLOAT, uniformData);
}

void Shader::setUniform(const s8* uniformName, s32 val) const
{
    s32 uniformData[1] = {val};
    SET_UNIFORM_IMPL(uniformName, SINT, uniformData);
}
void Shader::setUniform(const s8* uniformName, s32 val, s32 val2) const
{
    s32 uniformData[2] = {val, val2};
    SET_UNIFORM_IMPL(uniformName, SINT, uniformData);
}
void Shader::setUniform(const s8* uniformName, s32 val, s32 val2, s32 val3) const
{
    s32 uniformData[3] = {val, val2, val3};
    SET_UNIFORM_IMPL(uniformName, SINT, uniformData);
}
void Shader::setUniform(const s8* uniformName, s32 val, s32 val2, s32 val3, s32 val4) const
{
    s32 uniformData[4] = {val, val2, val3, val4};
    SET_UNIFORM_IMPL(uniformName, SINT, uniformData);
}

void Shader::setUniform(const s8* uniformName, u32 val) const
{
    u32 uniformData[1] = {val};
    SET_UNIFORM_IMPL(uniformName, UINT, uniformData);
}
void Shader::setUniform(const s8* uniformName, u32 val, u32 val2) const
{
    u32 uniformData[2] = {val, val2};
    SET_UNIFORM_IMPL(uniformName, UINT, uniformData);
}
void Shader::setUniform(const s8* uniformName, u32 val, u32 val2, u32 val3) const
{
    u32 uniformData[3] = {val, val2, val3};
    SET_UNIFORM_IMPL(uniformName, UINT, uniformData);
}
void Shader::setUniform(const s8* uniformName, u32 val, u32 val2, u32 val3, u32 val4) const
{
    u32 uniformData[4] = {val, val2, val3, val4};
    SET_UNIFORM_IMPL(uniformName, UINT, uniformData);
}

void Shader::setUniform(const s8* uniformName, glm::mat4 mat4, bool transpose) const {
    glm::mat4 uniformData[1] = {mat4};
    SET_UNIFORM_IMPL(uniformName, MAT4, uniformData);
}
void Shader::setUniform(const s8* uniformName, glm::mat3 mat3, bool transpose ) const {
    glm::mat3 uniformData[1] = {mat3};
    SET_UNIFORM_IMPL(uniformName, MAT3, uniformData);
}










// set actual opengl uniforms
void OglSetUniform(const s8* uniformName, s32 loc, f32 val)
{
    if (loc != -1) GLCall(glUniform1f(loc, val));
}
void OglSetUniform(const s8* uniformName, s32 loc, f32 val, f32 val2)
{
    if (loc != -1) GLCall(glUniform2f(loc, val, val2));
}
void OglSetUniform(const s8* uniformName, s32 loc, f32 val, f32 val2, f32 val3)
{
    if (loc != -1) GLCall(glUniform3f(loc, val, val2, val3));
}
void OglSetUniform(const s8* uniformName, s32 loc, f32 val, f32 val2, f32 val3, f32 val4)
{
    if (loc != -1) GLCall(glUniform4f(loc, val, val2, val3, val4));
}
void OglSetUniform(const s8* uniformName, s32 loc, s32 val)
{
    if (loc != -1) GLCall(glUniform1i(loc, val));
}
void OglSetUniform(const s8* uniformName, s32 loc, s32 val, s32 val2)
{
    if (loc != -1) GLCall(glUniform2i(loc, val, val2));
}
void OglSetUniform(const s8* uniformName, s32 loc, s32 val, s32 val2, s32 val3)
{
    if (loc != -1) GLCall(glUniform3i(loc, val, val2, val3));
}
void OglSetUniform(const s8* uniformName, s32 loc, s32 val, s32 val2, s32 val3, s32 val4)
{
    if (loc != -1) GLCall(glUniform4i(loc, val, val2, val3, val4));
}
void OglSetUniform(const s8* uniformName, s32 loc, u32 val)
{
    if (loc != -1) GLCall(glUniform1ui(loc, val));
}
void OglSetUniform(const s8* uniformName, s32 loc, u32 val, u32 val2)
{
    if (loc != -1) GLCall(glUniform2ui(loc, val, val2));
}
void OglSetUniform(const s8* uniformName, s32 loc, u32 val, u32 val2, u32 val3)
{
    if (loc != -1) GLCall(glUniform3ui(loc, val, val2, val3));
}
void OglSetUniform(const s8* uniformName, s32 loc, u32 val, u32 val2, u32 val3, u32 val4)
{
    if (loc != -1) GLCall(glUniform4ui(loc, val, val2, val3, val4));
}
void OglSetUniform(const s8* uniformName, s32 loc, glm::mat4 mat4) {
    if (loc != -1) GLCall(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat4)));
}
void OglSetUniform(const s8* uniformName, s32 loc, glm::mat3 mat3) {
    if (loc != -1) GLCall(glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(mat3)));
}



void SetOglUniformFromBuffer(const char* uniformName, const UniformData& uniform)
{
    PROFILE_FUNCTION();
    #define ONE_VAR(type) OglSetUniform(uniformName, uniform.uniformLocation, ((type*)uniform.uniformData)[0]);
    #define TWO_VAR(type) OglSetUniform(uniformName, uniform.uniformLocation, ((type*)uniform.uniformData)[0], ((type*)uniform.uniformData)[1]);
    #define THREE_VAR(type) OglSetUniform(uniformName, uniform.uniformLocation, ((type*)uniform.uniformData)[0], ((type*)uniform.uniformData)[1], ((type*)uniform.uniformData)[2]);
    #define FOUR_VAR(type) OglSetUniform(uniformName, uniform.uniformLocation, ((type*)uniform.uniformData)[0], ((type*)uniform.uniformData)[1], ((type*)uniform.uniformData)[2], ((type*)uniform.uniformData)[3]);

    #define SET_FROM_TYPE(type, numElements) \
        switch (numElements) \
        { \
            case 1: \
            { \
                ONE_VAR(type) \
            } break; \
            case 2:\
            {\
                TWO_VAR(type)\
            } break;\
            case 3:\
            {   \
                THREE_VAR(type) \
            } break; \
            case 4: \
            { \
                FOUR_VAR(type) \
            } break; \
            default: \
            { \
                LOG_WARN("Attempted to set uniform with a weird number of elements: %i", numElements); \
            } break; \
        }

    u32 uniformSize = uniform.uniformSize;
    u32 uniformTypeSize = UniformDataTypeSizes[uniform.dataType];
    if (uniformSize % uniformTypeSize != 0 || uniformSize == 0 || uniformTypeSize == 0)
    {
        LOG_WARN("Tried to activate a shader before setting uniforms! Ignoring...");
        return;
    }
    u32 uniformNumElements = uniformSize / uniformTypeSize;

    switch (uniform.dataType)
    {
        case UniformDataType::FLOAT:
        {
            SET_FROM_TYPE(f32, uniformNumElements)
        } break;
        case UniformDataType::UINT:
        {
            SET_FROM_TYPE(u32, uniformNumElements)
        } break;
        case UniformDataType::SINT:
        {
            SET_FROM_TYPE(s32, uniformNumElements)
        } break;
        case UniformDataType::VEC2:
        {
            SET_FROM_TYPE(f32, uniformNumElements)
        } break;
        case UniformDataType::VEC3:
        {
            SET_FROM_TYPE(f32, uniformNumElements)
        } break;
        case UniformDataType::VEC4:
        {
            SET_FROM_TYPE(f32, uniformNumElements)
        } break;
        case UniformDataType::MAT4:
        {
            OglSetUniform(uniformName, uniform.uniformLocation, *(glm::mat4*)uniform.uniformData);
        } break;
        case UniformDataType::MAT3:
        {
            OglSetUniform(uniformName, uniform.uniformLocation, *(glm::mat3*)uniform.uniformData);
        } break;
        default:
        {
            LOG_WARN("Attempted to set uniform data with unknown type %i", uniform.dataType);
        } break;
    }
}