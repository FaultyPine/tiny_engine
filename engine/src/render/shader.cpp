//#include "pch.h"
#include "shader.h"
#include "texture.h"
#include "tiny_fs.h"
#include "tiny_ogl.h"
#include <unordered_map>
#include <set>
#include <sstream>

// TODO: There's a lot of static tracking stuff here meant to make Shaders
// a simple wrapper on an ID... But is there a better way to structure this?

// contains vertex shader path, frag shader path
typedef std::pair<std::string, std::string> ShaderLocation;

struct GlobalShaderState
{
    // <abstract shader "id", <OGL shader id, shaderFilePaths>>
    std::unordered_map<u32, std::pair<u32, ShaderLocation>> loadedShaders = {};

    // map of shader id -> list of sampler ids
    // putting this outside the shader object to keep Shaders a wrapper around an ID
    std::unordered_map<u32, std::vector<u32>> samplerIDs = {};

    //              shader id               uniform name    uniform location
    std::unordered_map<u32, std::unordered_map<std::string, s32>> cachedUniformLocs = {};
};
static GlobalShaderState gss;


u32 CreateAndCompileShader(u32 shaderType, const s8* shaderSource) {
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
    std::set<std::string> alreadyIncluded = {}) {
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

std::string ShaderSourcePreprocess(const s8* shaderSource) {
    // take in original source code and run some procedure(s) on it and return the "processed" source code
    std::string ret;

    ret += "#version 330 core\n";

    std::string includeSearchDir = ResPath("shaders/");
    static const char* includeIdentifier = "#include "; // space after it so #include"hi.bye" is invalid. Must be #include "hi.bye"
    ret += ShaderPreprocessIncludes(shaderSource, "#include ", includeSearchDir);

    return ret;
}


static const char* fallbackFragShader = R"shad(
#version 330
void main()
{
    gl_FragColor = vec4(1.0, 0.0, 0.5, 1.0);
}
)shad";
static const char* fallbackVertShader = R"shad(
#version 330
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

u32 CreateShaderProgramFromStr(const s8* vsSource, const s8* fsSource, const std::string& vertPath = "", const std::string& fragPath = "") {
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

u32 CreateShaderFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
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


s32 Shader::getLoc(const std::string& uniformName) const {
    u32 oglShaderID = gss.loadedShaders.at(ID).first;
    // cache uniforms for later retrieval
    if (gss.cachedUniformLocs[oglShaderID].count(uniformName)) return gss.cachedUniformLocs[oglShaderID][uniformName];
    else {
        s32 loc = glGetUniformLocation(oglShaderID, uniformName.c_str());
        if (loc != -1) {
            gss.cachedUniformLocs[oglShaderID][uniformName] = loc;
            return loc;
        }
        else {
            //std::cout << "Shader Uniform " << uniformName << " either isn't defined or is unused!";
            return -1;
        }
    }
}

u32 Shader::GetOpenGLProgramID() {
    return gss.loadedShaders.at(ID).first;
}
void Shader::InitShaderFromProgramID(u32 shaderProgram, const std::string& vertexPath, const std::string& fragmentPath) {
    // ID is the index into the loadedShaders list that contains the OGL shader id
    ID = gss.loadedShaders.size();
    gss.loadedShaders[ID] = std::make_pair(shaderProgram, std::make_pair(vertexPath, fragmentPath));
}
Shader::Shader(u32 shaderProgram) { 
    InitShaderFromProgramID(shaderProgram);
}
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    u32 shaderProgram = CreateShaderFromFiles(vertexPath, fragmentPath);
    InitShaderFromProgramID(shaderProgram, vertexPath, fragmentPath);
}
Shader Shader::CreateShaderFromStr(const s8* vsCodeStr, const s8* fsCodeStr) {
    return Shader(CreateShaderProgramFromStr(vsCodeStr, fsCodeStr));
}
void Shader::Delete() const {
    //TINY_ASSERT(false && "Proper shader deletion is currently unimplemented!");
    // TODO: properly release shader...
    // if we erase the shader program from the list
    // other shaders whose IDs are indexes are now totally invalid
    
}




void Shader::use() const {
    TINY_ASSERT("Invalid shader ID!" && isValid());
    glUseProgram(gss.loadedShaders.at(ID).first); 
}

 
void ReloadShader(u32 shaderID) { // shader "id" (not opengl shader program id)
    std::pair<u32, ShaderLocation>& oglIDAndPaths = gss.loadedShaders.at(shaderID);
    const ShaderLocation& shaderLocations = oglIDAndPaths.second;

    // if shader locations are blank, this shader probably came from a string (cant reload)
    if (shaderLocations.first.empty() || shaderLocations.second.empty()) return;

    // TODO: check if (either frag/vert) file timestamp changed
    bool wasShaderFilesChanged = true;
    if (wasShaderFilesChanged) {
        u32 newShaderProgram = CreateShaderFromFiles(shaderLocations.first, shaderLocations.second);
        LOG_INFO("New reloaded shader %i", newShaderProgram);

        u32 oldOGLShaderProgram = oglIDAndPaths.first;
        glDeleteProgram(oldOGLShaderProgram);
        gss.cachedUniformLocs.erase(oldOGLShaderProgram);
        oglIDAndPaths.first = newShaderProgram;                
    }
}
void Shader::Reload() const {
    ReloadShader(this->ID);
}
void Shader::ReloadShaders() {
    for (auto& [shaderID, oglIDAndPaths] : gss.loadedShaders) {
        ReloadShader(shaderID);
    }
    LOG_INFO("Reloaded shaders!");
}



void Shader::ActivateSamplers() const {
    const std::vector<u32>& shaderSamplers = gss.samplerIDs[ID];
    for (s32 i = 0; i < shaderSamplers.size(); i++) {
        Texture::bindUnit(i, shaderSamplers.at(i));
    }
}
void Shader::TryAddSampler(u32 texture, const char* uniformName) const {
    std::vector<u32>& shaderSamplers = gss.samplerIDs[ID];
    // if this texture is NOT already tracked
    if (std::find(shaderSamplers.begin(), shaderSamplers.end(), texture) == shaderSamplers.end()) {
        // this sampler needs to be added
        shaderSamplers.push_back(texture);
    }
    // find texture in sampler list and set uniform.
    // doing this here to help support shader hot-reloading. When shaders are reloaded the sampler
    // uniforms need to be re-set with the appropriate textures
    auto samplerFound = std::find(shaderSamplers.begin(), shaderSamplers.end(), texture);
    if (samplerFound != shaderSamplers.end()) {
        s32 samplerIndex = samplerFound - shaderSamplers.begin();
        use();
        setUniform(uniformName, samplerIndex);
    }
}









// ========= Shader setUniform overloads =============================


void Shader::setUniform(const s8* uniformName, f32 val) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform1f(loc, val));
}
void Shader::setUniform(const s8* uniformName, f32 val, f32 val2) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform2f(loc, val, val2));
}
void Shader::setUniform(const s8* uniformName, f32 val, f32 val2, f32 val3) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform3f(loc, val, val2, val3));
}
void Shader::setUniform(const s8* uniformName, f32 val, f32 val2, f32 val3, f32 val4) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform4f(loc, val, val2, val3, val4));
}

void Shader::setUniform(const s8* uniformName, s32 val) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform1i(loc, val));
}
void Shader::setUniform(const s8* uniformName, s32 val, s32 val2) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform2i(loc, val, val2));
}
void Shader::setUniform(const s8* uniformName, s32 val, s32 val2, s32 val3) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform3i(loc, val, val2, val3));
}
void Shader::setUniform(const s8* uniformName, s32 val, s32 val2, s32 val3, s32 val4) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform4i(loc, val, val2, val3, val4));
}

void Shader::setUniform(const s8* uniformName, u32 val) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform1ui(loc, val));
}
void Shader::setUniform(const s8* uniformName, u32 val, u32 val2) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform2ui(loc, val, val2));
}
void Shader::setUniform(const s8* uniformName, u32 val, u32 val2, u32 val3) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform3ui(loc, val, val2, val3));
}
void Shader::setUniform(const s8* uniformName, u32 val, u32 val2, u32 val3, u32 val4) const
{
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniform4ui(loc, val, val2, val3, val4));
}

void Shader::setUniform(const s8* uniformName, glm::mat4 mat4, bool transpose) const {
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniformMatrix4fv(loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat4)));
}
void Shader::setUniform(const s8* uniformName, glm::mat3 mat3, bool transpose ) const {
    s32 loc = getLoc(uniformName);
    if (loc != -1) GLCall(glUniformMatrix3fv(loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(mat3)));
}

