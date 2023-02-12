#include "shader.h"

// TODO: There's a lot of static tracking stuff here meant to make Shaders
// a simple wrapper on an ID... But is there a better way to structure this?

// contains vertex shader path, frag shader path
typedef std::pair<std::string, std::string> ShaderLocation;

// <abstract shader "id", <OGL shader id, shaderFilePaths>>
static std::unordered_map<u32, std::pair<u32, ShaderLocation>> loadedShaders = {};

// map of shader id -> list of sampler ids
// putting this outside the shader object to keep Shaders a wrapper around an ID
static std::unordered_map<u32, std::vector<s32>> samplerIDs = {};

//              shader id               uniform name    uniform location
static std::unordered_map<u32, std::unordered_map<std::string, s32>> cachedUniformLocs = {};

u32 CreateAndCompileShader(u32 shaderType, const s8* shaderSource) {
    u32 shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &shaderSource, NULL);
    glCompileShader(shaderID);
    s32 successCode;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &successCode);
    if (!successCode) {
        s8 infoLog[512];
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        std::cout << "[ERR] " << (shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader compilation failed. shaderID = " << shaderID << "\n" << infoLog << "\n";
    }
    return shaderID;
}
u32 CreateShaderProgramFromStr(const s8* vsSource, const s8* fsSource) {
    u32 vertexShader = CreateAndCompileShader(GL_VERTEX_SHADER, vsSource);
    u32 fragShader = CreateAndCompileShader(GL_FRAGMENT_SHADER, fsSource);
    u32 shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    s32 success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        s8 infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "shader linking failed. vs = " << vsSource << " fs = " << fsSource << "\n" << infoLog << "\n";
        ASSERT(false);
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
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();
    u32 shaderProgram = CreateShaderProgramFromStr(vShaderCode, fShaderCode);
    return shaderProgram;
}


s32 Shader::getLoc(const std::string& uniformName) const {
    u32 oglShaderID = loadedShaders.at(ID).first;
    // cache uniforms for later retrieval
    if (cachedUniformLocs[oglShaderID].count(uniformName)) return cachedUniformLocs[oglShaderID][uniformName];
    else {
        s32 loc = glGetUniformLocation(oglShaderID, uniformName.c_str());
        if (loc != -1) {
            cachedUniformLocs[oglShaderID][uniformName] = loc;
            return loc;
        }
        else {
            //std::cout << "Shader Uniform " << uniformName << " either isn't defined or is unused!\n";
            return -1;
        }
    }
}

u32 Shader::GetOpenGLProgramID() {
    return loadedShaders.at(ID).first;
}
void Shader::InitShaderFromProgramID(u32 shaderProgram, const std::string& vertexPath, const std::string& fragmentPath) {
    // ID is the index into the loadedShaders list that contains the OGL shader id
    ID = loadedShaders.size();
    loadedShaders[ID] = std::make_pair(shaderProgram, std::make_pair(vertexPath, fragmentPath));
    valid = true;
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
    //ASSERT(false && "Proper shader deletion is currently unimplemented!");
    // TODO: properly release shader...
    // if we erase the shader program from the list
    // other shaders whose IDs are indexes are now totally invalid
    
}




void Shader::use() const {
    ASSERT("Invalid shader ID!\n" && valid);
    glUseProgram(loadedShaders.at(ID).first); 
}

 
void Shader::ReloadShaders() {
    for (auto& [shaderID, oglIDAndPaths] : loadedShaders) {
        const ShaderLocation& shaderLocations = oglIDAndPaths.second;
        // if shader locations are blank, this shader probably came from a string (cant reload)
        if (!shaderLocations.first.empty() && !shaderLocations.second.empty()) {
            // TODO: check if (either frag/vert) file timestamp changed
            bool wasShaderFilesChanged = true;
            if (wasShaderFilesChanged) {
                u32 oldOGLShaderProgram = oglIDAndPaths.first;
                glDeleteProgram(oldOGLShaderProgram);
                cachedUniformLocs.erase(oldOGLShaderProgram);
                u32 newShaderProgram = CreateShaderFromFiles(shaderLocations.first, shaderLocations.second);
                oglIDAndPaths.first = newShaderProgram;                
            }
        }
    }
    std::cout << "Reloaded shaders!\n";
}



void Shader::ActivateSamplers() const {
    const std::vector<s32>& shaderSamplers = samplerIDs[ID];
    for (s32 i = 0; i < shaderSamplers.size(); i++) {
        Texture::bindUnit(i, shaderSamplers.at(i));
    }
}
void Shader::TryAddSampler(s32 texture, const char* uniformName) const {
    std::vector<s32>& shaderSamplers = samplerIDs[ID];
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