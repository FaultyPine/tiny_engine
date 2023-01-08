#version 330 core

// Input vertex attributes (from vertex shader)

//in vec3 fragPositionWS;
in vec2 fragTexCoord;
//in vec4 fragColor;
//in vec3 fragNormalWS;
//in vec4 fragPosLightSpace;
flat in int materialId;
//in vec3 fragPositionOS;
// Output fragment color
out vec4 finalColor;
uniform int numInstances;


#define NUM_MATERIAL_TYPES 4
#define MAX_NUM_MATERIALS 4

struct MaterialProperty {
    vec4 color;
    int useSampler;
    sampler2D tex;
};
struct Material {
    MaterialProperty diffuseMat;
    MaterialProperty ambientMat;
    MaterialProperty specularMat;
    MaterialProperty normalMat;
    float shininess;
};
uniform int useNormalMap = 0; // If unset, use vertex normals. If set, sample normal map
uniform Material materials[MAX_NUM_MATERIALS];
vec4 GetMaterialColor(MaterialProperty mat) {
    int shouldUseSampler = mat.useSampler;
    vec4 color = (1-shouldUseSampler) * mat.color;
    vec4 tex =   shouldUseSampler     * texture(mat.tex, fragTexCoord);
    // if useSampler is true, color is 0, if it's false, tex is 0
    return color + tex;
}
vec4 GetDiffuseMaterial() {
    return GetMaterialColor(materials[materialId].diffuseMat);
}




const vec3 brown = vec3(115, 72, 69);


void main() {
    float height = fragTexCoord.y;
    vec3 brown = brown/255.0;
    vec3 col = mix(vec3(0,1,0), brown, 1-height);

    col = GetDiffuseMaterial().rgb * col;

    finalColor = vec4(col, 1.0);
}