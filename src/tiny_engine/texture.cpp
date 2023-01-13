#include "texture.h"
#include "shader.h"


std::string GetTexMatTypeString(TextureMaterialType type) {
    switch (type) {
        case DIFFUSE: return "tex_diffuse";
        case SPECULAR: return "tex_specular";
        case AMBIENT: return "tex_ambient";
        case NORMAL: return "tex_normal";
        case ROUGHNESS: return "tex_roughness";
        case EMISSION: return "tex_emission";
        case OTHER: return "tex_other";

        default: return "tex_unknown";
    }
}

// https://registry.khronos.org/OpenGL-Refpages/es3.0/html/glPixelStorei.xhtml
void SetPixelReadSettings(s32 width, s32 offsetX, s32 offsetY, s32 alignment) {
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, offsetX);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, offsetY);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
}


u8* LoadImageData(const char* imgPath, s32* width, s32* height, s32* numChannels, bool shouldFlipVertically) {
    stbi_set_flip_vertically_on_load(shouldFlipVertically);
    u8* data = stbi_load(imgPath, width, height, numChannels, 0);
    return data;
}


Texture GenTextureFromImg(u8* imgData, u32 width, u32 height, TextureProperties props) {
    u32 texture = 0;
    if (imgData) {
        GLCall(glGenTextures(1, &texture));
        GLCall(glBindTexture(GL_TEXTURE_2D, texture));
        // wrapping mode
        //              tex target     tex wrap axis      tex wrap mode
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (s32)props.texWrapMode));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (s32)props.texWrapMode));
        // filter mode
        //               apply taget to minified or magnified textures
        //              tex target         ^                  tex filter mode & mipmap filter mode
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (s32)props.minFilter));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (s32)props.magFilter)); // don't specify mipmap filtering on upscaling, since it only happens when you downscale
        // tex target, mipmap level, tex format, width, height, 0 (legacy stuff), format of src img, datatype of src img, actual img data
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, (s32)props.texFormat, width, height, 0, (s32)props.imgFormat, (s32)props.imgDataType, imgData));
        GLCall(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else {
        ASSERT(false && "Failed to load texture. Invalid image data passed!\n");
    }
    Texture ret;
    ret.id = texture;
    ret.height = (u32)height;
    ret.width = (u32)width;
    return ret;
}

// Note: May crash with invalid path/image. Should this return an optional?
Texture LoadTexture(const std::string& imgPath, 
                    TextureProperties props, 
                    bool flipVertically) {
    s32 width, height, numChannels;
    u8* data = LoadImageData(imgPath.c_str(), &width, &height, &numChannels, flipVertically);
    // if we didn't specify texture properties, fetch them automatically
    if (props.isNone) {
        props = numChannels == 3 ? TextureProperties::RGB_LINEAR() : TextureProperties::RGBA_LINEAR();
    }
    Texture ret = GenTextureFromImg(data, width, height, props);
    ret.texpath = imgPath;
    stbi_image_free(data);
    std::cout << "Loaded texture " << ret.texpath << " channels: " << numChannels << "\n";
    return ret;
}


void Material::SetShaderUniforms(const Shader& shader, u32 matIdx) const {
    #define MAT_DIFFUSE   0
    #define MAT_AMBIENT   1
    #define MAT_SPECULAR  2
    #define MAT_NORMAL    3
    shader.use();
    s32 matPropIdx = 0;
    /*#define SET_MATERIAL_UNIFORMS(matType, matVar) \
        shader.setUniform(TextFormat("materials[%i].%s.useSampler", matIdx, #matVar), matVar.hasTexture); \
        matPropIdx = (matType * matIdx) + matType; \
        if (matVar.hasTexture) { \
            matVar.texture.bindUnit(matPropIdx); \
        } \
        shader.setUniform(TextFormat("materials[%i].%s.tex", matIdx, #matVar), matPropIdx); \
        shader.setUniform(TextFormat("materials[%i].%s.color", matIdx, #matVar), matVar.color)
    */
   #define SET_MATERIAL_UNIFORMS(matType, matVar) \
        shader.setUniform(TextFormat("materials[%i].%s.useSampler", matIdx, #matVar), matVar.hasTexture); \
        if (matVar.hasTexture) { \
            shader.TryAddSampler(matVar.texture.id, TextFormat("materials[%i].%s.tex", matIdx, #matVar)); \
        } \
        shader.setUniform(TextFormat("materials[%i].%s.color", matIdx, #matVar), matVar.color)

    SET_MATERIAL_UNIFORMS(MAT_DIFFUSE, diffuseMat);
    SET_MATERIAL_UNIFORMS(MAT_AMBIENT, ambientMat);
    SET_MATERIAL_UNIFORMS(MAT_SPECULAR, specularMat);
    SET_MATERIAL_UNIFORMS(MAT_NORMAL, normalMat);
    shader.setUniform(TextFormat("materials[%i].shininess", matIdx), shininess);
    shader.setUniform("useNormalMap", normalMat.hasTexture);
}