#include "shader.h"


s32 Shader::getLoc(const std::string& uniformName) const {
    //              shader id               uniform name    uniform location
    static std::unordered_map<u32, std::unordered_map<std::string, s32>> cachedUniformLocs;

    // cache uniforms for later retrieval
    if (cachedUniformLocs[ID].count(uniformName)) return cachedUniformLocs[ID][uniformName];
    else {
        s32 loc = glGetUniformLocation(ID, uniformName.c_str());
        if (loc != -1) {
            cachedUniformLocs[ID][uniformName] = loc;
            return loc;
        }
        else {
            //std::cout << "Shader Uniform " << uniformName << " either isn't defined or is unused!\n";
            return -1;
        }
    }
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
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
    u32 shaderProgram = CreateShaderProgFromStr(vShaderCode, fShaderCode);
    ID = shaderProgram;
}

u32 Shader::CreateAndCompileShader(u32 shaderType, const s8* shaderSource) {
    u32 shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &shaderSource, NULL);
    glCompileShader(shaderID);
    s32 successCode;
    s8 infoLog[512];
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &successCode);
    if (!successCode) {
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        std::cout << "[ERR] shader compilation failed. shaderID = " << shaderID << "\n" << infoLog << "\n";
    }

    return shaderID;
}
u32 Shader::CreateShaderProgFromStr(const s8* vsSource, const s8* fsSource) {
    u32 vertexShader = CreateAndCompileShader(GL_VERTEX_SHADER, vsSource);
    u32 fragShader = CreateAndCompileShader(GL_FRAGMENT_SHADER, fsSource);
    u32 shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    s32 success;
    s8 infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "shader linking failed. vs = " << vertexShader << " fs = " << fragShader << "\n" << infoLog << std::endl;
    }
    // delete vert/frag shader after we've linked them to the program object
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
    return shaderProgram;
}