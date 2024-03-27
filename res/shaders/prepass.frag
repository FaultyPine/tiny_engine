 

layout(location = 0) out vec4 finalColor;

in VS_OUT 
{
    vec3 fragPositionWS;
    vec2 fragTexCoord;
    vec4 fragColor;
    vec3 fragNormal;
    vec3 fragTangent;
    vec3 fragPositionOS;
} vs_in;

void main() {
    float depth = gl_FragCoord.z;
    vec4 col = vec4(vs_in.fragNormal, depth);
    finalColor = col;
}