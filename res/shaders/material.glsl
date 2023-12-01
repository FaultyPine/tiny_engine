

struct MaterialProperty 
{
    vec4 color;
    int useSampler;
    sampler2D tex;
};

struct Material 
{
    MaterialProperty diffuseMat;
    // TODO: don't use ambient material, ambient should always just be a color. 
    // If you really want an ambient "texture", use diffuse
    MaterialProperty ambientMat;
    MaterialProperty specularMat;
    MaterialProperty normalMat;
    MaterialProperty shininessMat;
    MaterialProperty emissiveMat;
    MaterialProperty opacityMat;
};

uniform Material material;

vec4 GetMaterialColor(MaterialProperty mat, vec2 uv) 
{
    int shouldUseSampler = mat.useSampler;
    vec4 color = (1-shouldUseSampler) * mat.color;
    vec4 tex =   shouldUseSampler     * texture(mat.tex, uv);
    // if useSampler is true, color is 0, if it's false, tex is 0
    return color + tex;
}
vec4 GetDiffuseMaterial(vec2 uv) 
{
    return GetMaterialColor(material.diffuseMat, uv);
}
vec4 GetAmbientMaterial(vec2 uv) 
{
    return GetMaterialColor(material.ambientMat, uv);
}
vec4 GetSpecularMaterial(vec2 uv)
{
    return GetMaterialColor(material.specularMat, uv);
}
vec4 GetNormalMaterial(vec2 uv) 
{
    return GetMaterialColor(material.normalMat, uv);
}
vec4 GetShininessMaterial(vec2 uv) 
{
    return GetMaterialColor(material.shininessMat, uv);
}
vec4 GetEmissiveMaterial(vec2 uv) 
{
    return GetMaterialColor(material.emissiveMat, uv);
}
vec4 GetOpacityMaterial(vec2 uv)
{
    return GetMaterialColor(material.opacityMat, uv);
}

vec3 GetNormals(vec3 fragNormalOS, vec3 fragTangentOS, vec2 fragTexCoord) 
{
    vec3 normal = vec3(0);
    bool shouldUseSampler = material.normalMat.useSampler == 1;
    shouldUseSampler = true;
    if (shouldUseSampler)
    {
        // normal from map TODO: but TBN in vert shader
        fragTangentOS = (fragTangentOS - dot(fragTangentOS, fragNormalOS) * fragNormalOS);
        vec3 bitangentOS = cross(fragNormalOS, fragTangentOS);
        mat3 TBN = mat3(fragTangentOS, bitangentOS, fragNormalOS);

        vec3 localNormal = 2.0 * GetNormalMaterial(fragTexCoord).rgb - 1.0;
        normal = normalize(TBN * localNormal); // world space normal
    }
    else
    {
        normal = normalize(fragNormalOS);
    }
    return normal;
}