

#define MAX_NUM_MATERIALS 4

struct MaterialProperty {
    vec4 color;
    int useSampler;
    sampler2D tex;
};
struct Material {
    MaterialProperty diffuseMat;
    // TODO: don't use ambient material, ambient should always just be a color. 
    // If you really want an ambient "texture", use diffuse
    MaterialProperty ambientMat;
    MaterialProperty specularMat;
    MaterialProperty normalMat;
    MaterialProperty shininessMat;
    MaterialProperty emissiveMat;
};


vec4 GetMaterialColor(MaterialProperty mat, vec2 uv) {
    int shouldUseSampler = mat.useSampler;
    vec4 color = (1-shouldUseSampler) * mat.color;
    vec4 tex =   shouldUseSampler     * texture(mat.tex, uv);
    // if useSampler is true, color is 0, if it's false, tex is 0
    return color + tex;
}
vec4 GetDiffuseMaterial(in Material materials[MAX_NUM_MATERIALS], int materialId, vec2 uv) {
    return GetMaterialColor(materials[materialId].diffuseMat, uv);
}
vec4 GetAmbientMaterial(in Material materials[MAX_NUM_MATERIALS], int materialId, vec2 uv) {
    return GetMaterialColor(materials[materialId].ambientMat, uv);
}
vec4 GetSpecularMaterial(in Material materials[MAX_NUM_MATERIALS], int materialId, vec2 uv) {
    return GetMaterialColor(materials[materialId].specularMat, uv);
}
vec4 GetNormalMaterial(in Material materials[MAX_NUM_MATERIALS], int materialId, vec2 uv) {
    return GetMaterialColor(materials[materialId].normalMat, uv);
}
float GetShininessMaterial(in Material materials[MAX_NUM_MATERIALS], int materialId, vec2 uv) {
    return GetMaterialColor(materials[materialId].shininessMat, uv).r;
}
vec4 GetEmissiveMaterial(in Material materials[MAX_NUM_MATERIALS], int materialId, vec2 uv) {
    return GetMaterialColor(materials[materialId].emissiveMat, uv);
}