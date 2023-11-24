

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

uniform int useNormalMap = 0; // If unset, use vertex normals. If set, sample normal map
uniform Material material;

vec4 GetMaterialColor(MaterialProperty mat, vec2 uv) {
    int shouldUseSampler = mat.useSampler;
    vec4 color = (1-shouldUseSampler) * mat.color;
    vec4 tex =   shouldUseSampler     * texture(mat.tex, uv);
    // if useSampler is true, color is 0, if it's false, tex is 0
    return color + tex;
}
vec4 GetDiffuseMaterial(vec2 uv) {
    return GetMaterialColor(material.diffuseMat, uv);
}
vec4 GetAmbientMaterial(vec2 uv) {
    return GetMaterialColor(material.ambientMat, uv);
}
vec4 GetSpecularMaterial(vec2 uv) {
    return GetMaterialColor(material.specularMat, uv);
}
vec4 GetNormalMaterial(vec2 uv) {
    return GetMaterialColor(material.normalMat, uv);
}
float GetShininessMaterial(vec2 uv) {
    return GetMaterialColor(material.shininessMat, uv).r;
}
vec4 GetEmissiveMaterial(vec2 uv) {
    return GetMaterialColor(material.emissiveMat, uv);
}

vec3 GetNormals(vec3 fragNormalWS, vec2 fragTexCoord) {
    vec3 vertNormals = (1-useNormalMap) * normalize(fragNormalWS);
    vec3 normalMapNormals = (useNormalMap) * GetNormalMaterial(fragTexCoord).rgb;
    return vertNormals + normalMapNormals;
}