//#include "pch.h"
#include "shader.h"
#include "texture.h"
#include "cubemap.h"
#include "tiny_fs.h"
#include "tiny_ogl.h"
#include <unordered_map>
#include <set>
#include <sstream>

// There's a lot of static tracking stuff here meant to make Shaders
// a simple wrapper on an ID... But is there a better way to structure this?


enum UniformDataType
{
    UNK = 0,
    FLOAT,
    UINT,
    SINT,
    VEC2,
    VEC3,
    VEC4,
    MAT3,
    MAT4,
};

struct UniformData
{
    // pointer into the dedicated uniform memory block given to us by the engine
    void* uniformData = 0;
    u32 uniformSize = 0;
    UniformDataType dataType = UNK;
    s32 uniformLocation = 0;
    // TODO: dirty flag and only set when we actually change the data
};

// contains vertex shader path, frag shader path
typedef std::pair<std::string, std::string> ShaderLocation;

struct ShaderInternal
{
    u32 oglShaderProgram = 0;
    ShaderLocation filepaths = {};
    std::vector<Texture> samplerIDs = {};
    std::unordered_map<std::string, UniformData> cachedUniforms = {};
};

struct GlobalShaderState
{ // TODO: use only 1 map. shader id -> {ogl id, filepaths, samplers, uniforms}

    // <abstract shader "id", <OGL shader id, shaderFilePaths>>
    //std::unordered_map<u32, std::pair<u32, ShaderLocation>> loadedShaders = {};

    // map of shader id -> list of sampler ids
    // putting this outside the shader object to keep Shaders a wrapper around an ID
    //std::unordered_map<u32, std::vector<Texture>> samplerIDs = {};

    //              shader id               uniform name    uniform
    //std::unordered_map<u32, std::unordered_map<std::string, UniformData>> cachedUniforms = {};

    std::unordered_map<u32, ShaderInternal> shaderMap = {};

    Arena uniformsDataBlock = {};
};
static GlobalShaderState gss;



void InitializeShaderSystem(Arena* arena, size_t shaderUniformDataBlockSize)
{
    void* uniformsDataBlock = arena_alloc(arena, shaderUniformDataBlockSize);
    gss.uniformsDataBlock = arena_init(uniformsDataBlock, shaderUniformDataBlockSize);
}


u32 CreateAndCompileShader(u32 shaderType, const s8* shaderSource) 
{
    u32 shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &shaderSource, NULL);
    glCompileShader(shaderID);
    s32 successCode;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &successCode);
    if (!successCode) {
        s8 infoLog[512];
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        LOG_ERROR("%s shader compilation failed. shaderID = %i\n%s", (shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment"), shaderID, infoLog);
    }
    return shaderID;
}


std::string ShaderPreprocessIncludes(
    const s8* shaderSource, 
    const std::string& includeIdentifier, 
    const std::string& includeSearchDir, 
    std::set<std::string> alreadyIncluded = {}) 
{
    static bool isRecursiveCall = false;
    std::string fullSourceCode = "";
    std::string lineBuffer;
    std::istringstream stream = std::istringstream(shaderSource);
    // TODO: custom strings
    while (std::getline(stream, lineBuffer)) {
        // if include is in this line
        if (lineBuffer.find(includeIdentifier) != lineBuffer.npos) {
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
                    LOG_ERROR("Failed to open shader include: %s", path);
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

std::string ShaderSourcePreprocess(const s8* shaderSource) 
{
    // take in original source code and run some procedure(s) on it and return the "processed" source code
    std::string ret;

    ret += "#version 330 core\n";

    std::string includeSearchDir = ResPath("shaders/");
    static const char* includeIdentifier = "#include "; // space after it so #include"hi.bye" is invalid. Must be #include "hi.bye"
    ret += ShaderPreprocessIncludes(shaderSource, "#include ", includeSearchDir);

    return ret;
}


static const char* fallbackFragShader = R"shad(
void main()
{
    gl_FragColor = vec4(1.0, 0.0, 0.5, 1.0);
}
)shad";
static const char* fallbackVertShader = R"shad(
layout (location = 0) in vec3 vertPos;
uniform mat4 mvp;
void main()
{
    gl_Position =  mvp * vec4(vertPos, 1.0);
}
)shad";

u32 GetShaderErrorLineNumber(const s8* infoLog, u32 infoLogSize)
{
    // TODO: grab error line number and print out the error line in the preprocessed string
    return 0;
}

u32 CreateShaderProgramFromStr(const s8* vsSource, const s8* fsSource, const std::string& vertPath = "", const std::string& fragPath = "") 
{
    // preprocess both vert and frag shader source code before compiling
    std::string vsSourcePreprocessed = ShaderSourcePreprocess(vsSource);
    std::string fsSourcePreprocessed = ShaderSourcePreprocess(fsSource);

    u32 vertexShader = CreateAndCompileShader(GL_VERTEX_SHADER, vsSourcePreprocessed.c_str());
    u32 fragShader = CreateAndCompileShader(GL_FRAGMENT_SHADER, fsSourcePreprocessed.c_str());

    u32 shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    s32 success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        s8 infoLog[1024];
        glGetProgramInfoLog(shaderProgram, 1024, NULL, infoLog);
        //LOG_ERROR("Vertex shader source: %s\n", vsSourcePreprocessed);
        //LOG_ERROR("Fragment shader source: %s\n", fsSourcePreprocessed);
        LOG_ERROR("shader linking failed. vs = %s fs = %s\n%s", vertPath.c_str(), fragPath.c_str(), infoLog);
        return CreateShaderProgramFromStr(fallbackVertShader, fallbackFragShader, vertPath, fragPath);
        //TINY_ASSERT(false);
    }
    // delete vert/frag shader after we've linked them to the program object
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
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
    return gss.shaderMap.at(shaderID).oglShaderProgram;
    //return gss.loadedShaders.at(shaderID).first;
}
void Shader::InitShaderFromProgramID(u32 shaderProgram, const std::string& vertexPath, const std::string& fragmentPath) 
{
    // ID is the index into the loadedShaders list that contains the OGL shader id
    //ID = gss.loadedShaders.size();
    ID = gss.shaderMap.size();
    //gss.loadedShaders[ID] = std::make_pair(shaderProgram, std::make_pair(vertexPath, fragmentPath));
    gss.shaderMap[ID].oglShaderProgram = shaderProgram;
    gss.shaderMap[ID].filepaths = std::make_pair(vertexPath, fragmentPath);
}
Shader::Shader(u32 shaderProgram) 
{ 
    InitShaderFromProgramID(shaderProgram);
}
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) 
{
    u32 shaderProgram = CreateShaderFromFiles(vertexPath, fragmentPath);
    InitShaderFromProgramID(shaderProgram, vertexPath, fragmentPath);
}
Shader Shader::CreateShaderFromStr(const s8* vsCodeStr, const s8* fsCodeStr) 
{
    return Shader(CreateShaderProgramFromStr(vsCodeStr, fsCodeStr));
}
void Shader::Delete() const 
{
    //TINY_ASSERT(false && "Proper shader deletion is currently unimplemented!");
    // TODO: properly release shader...
    // if we erase the shader program from the list
    // other shaders whose IDs are indexes are now totally invalid
    
}

void RefreshShaderUniformLocations(u32 shaderID, u32 oglShaderProgram)
{
    for (auto& [uniformName, uniformData] : gss.shaderMap[shaderID].cachedUniforms)
    //for (auto& [uniformName, uniformData] : gss.cachedUniforms[shaderID])
    {
        s32 loc = glGetUniformLocation(oglShaderProgram, uniformName.c_str());
        if (loc != -1)
        {
            uniformData.uniformLocation = loc;
        }
    }
}

void ReloadShader(u32 shaderID) 
{ // shader "id" (not opengl shader program)
    ShaderInternal& shaderInternal = gss.shaderMap[shaderID];
    u32 oglShaderProgram = shaderInternal.oglShaderProgram;
    //std::pair<u32, ShaderLocation>& oglIDAndPaths = gss.loadedShaders.at(shaderID);
    //const ShaderLocation& shaderLocations = oglIDAndPaths.second;
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
    for (auto& [shaderID, shaderInternal] : gss.shaderMap) {
        ReloadShader(shaderID);
    }
    LOG_INFO("Reloaded shaders!");
}



void ActivateSamplers(u32 shaderID) 
{
    if (gss.shaderMap.count(shaderID) == 0) return;
    //if (gss.samplerIDs.count(shaderID) == 0) return;
    const std::vector<Texture>& shaderSamplers = gss.shaderMap[shaderID].samplerIDs;
    for (s32 i = 0; i < shaderSamplers.size(); i++) {
        const Texture& tex = shaderSamplers.at(i);
        Texture::bindUnit(i, tex.id, tex.type);
    }
}

void Shader::TryAddSampler(const Texture& texture, const char* uniformName) const 
{
    TINY_ASSERT(texture.isValid());
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
    if (samplerIdx == -1) {
        samplerIdx = shaderSamplers.size();
        shaderSamplers.push_back(texture);
    }
    // when we pass texture id to our shader, it needs to use the glUniform1i (<- signed) which is why this should be s32 not u32
    setUniform(uniformName, samplerIdx);
}
void Shader::TryAddSampler(const Cubemap& texture, const char* uniformName) const
{
    Texture cubemapTex = Texture(texture.id, GL_TEXTURE_CUBE_MAP, texture.width, texture.height);
    this->TryAddSampler(cubemapTex, uniformName);
}



void updateUniformData(u32 ID, const std::string& uniformName, void* uniformData, u32 uniformSize, UniformDataType dataType) 
{
    TINY_ASSERT(gss.uniformsDataBlock.backing_mem_size > 0 && "Make sure to call InitializeShaderSystem before doing any shader calls!");
    if (gss.shaderMap.count(ID) == 0) return;
    std::unordered_map<std::string, UniformData>& cachedUniforms = gss.shaderMap[ID].cachedUniforms;
    // if we already have uniform - simply update cached values. If we don't, allocate more mem in our uniform mem block
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
            void* uniformDataPersistent = arena_alloc(&gss.uniformsDataBlock, uniformSize);
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


// might need to be able to go from buffer + size, to proper glUniformXXX call
// store & switch on gl function pointers?

// maybe specialize the template on the ogl function pointer types?


void SetOglUniformFromBuffer(const char* uniformName, const UniformData& uniform);

void Shader::use() const 
{
    TINY_ASSERT("Invalid shader ID!" && isValid());
    u32 shaderID = this->ID;
    u32 oglShaderID = GetOpenGLProgramID(shaderID);

    glUseProgram(oglShaderID); 
    ActivateSamplers(shaderID);
    // TODO: use all uniforms associated with this ID
    const auto& uniformMap = gss.shaderMap[shaderID].cachedUniforms;
    for (const auto& [uniformName, uniformData] : uniformMap)
    {
        SetOglUniformFromBuffer(uniformName.c_str(), uniformData);
    }
}


u32 UniformDataTypeToSize(UniformDataType dataType)
{
    switch (dataType)
    {
        case UniformDataType::FLOAT:
        {
            return sizeof(f32);
        } break;
        case UniformDataType::UINT:
        {
            return sizeof(u32);
        } break;
        case UniformDataType::SINT:
        {
            return sizeof(s32);
        } break;
        case UniformDataType::VEC2:
        {
            return sizeof(f32) * 2;
        } break;
        case UniformDataType::VEC3:
        {
            return sizeof(f32) * 3;
        } break;
        case UniformDataType::VEC4:
        {
            return sizeof(f32) * 4;
        } break;
        case UniformDataType::MAT4:
        {
            return sizeof(f32) * 16;
        } break;
        case UniformDataType::MAT3:
        {
            return sizeof(f32) * 9;
        } break;
        default:
        {
            return 0;
        } break;
    }
}



// ========= Shader setUniform overloads =============================

#define SET_UNIFORM_IMPL(uniformName, uniformType, uniformDataCopyArr) \
u32 uniformSize = UniformDataTypeToSize(uniformType) * ARRAY_SIZE(uniformDataCopyArr); \
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
    u32 uniformTypeSize = UniformDataTypeToSize(uniform.dataType);
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