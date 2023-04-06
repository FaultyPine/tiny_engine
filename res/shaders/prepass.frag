#version 330 core

out vec4 finalColor;

in vec3 fragNormalOS;

void main() {
    float depth = gl_FragCoord.z;
    vec4 col = vec4(fragNormalOS, depth);
    finalColor = col;
}