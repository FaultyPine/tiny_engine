 
out vec4 FragColor;

// TODO:
// https://www.patreon.com/posts/making-stylized-27402644

in vec3 TexCoords;

uniform samplerCube skybox;
#include "lighting.glsl"

uniform vec3 topGradientCol = vec3(1.0, 0.3, 0.2);
uniform vec3 bottomGradientCol = vec3(0.3, 0.3, 1.0);
uniform vec3 horizonCol = vec3(1.0, 1.0, 1.0);

void main()
{
    vec3 sun = -sunlight.direction.xyz;
    float sunMask = dot(normalize(sun), normalize(TexCoords)) / 2.0 + 0.5; // 1 directly in sun, 0 directly opposite of sun
    
    sunMask = smoothstep(0.98,1.0,sunMask)*2;

    //vec3 skyboxCol = texture(skybox, TexCoords).rgb;
    //vec3 col = vec3(sunMask*0.4) + (skyboxCol);
    vec3 col = vec3(sunMask*0.4);
    
    vec3 gradient = mix(bottomGradientCol, topGradientCol, TexCoords.y);
    float horizon = (1.0 - (abs(TexCoords.y) * 2.5)) * clamp(TexCoords.y, 0.0,1.0);
    vec3 horizonGlow = clamp(horizon * horizonCol, 0.0,1.0);
    col += gradient + horizonGlow;
    //vec3 col = vec3(pow(gradient, 32), pow(gradient, 48) / 2.0 + 0.5, gradient / 4.0 + 0.75);
    
    FragColor = vec4(col, 1.0);
}