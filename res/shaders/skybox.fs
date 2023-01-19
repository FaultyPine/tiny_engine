#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform vec3 sunDirection = vec3(0,1,0);
void main()
{
    vec3 sun = -sunDirection;
    vec3 eyeVector = TexCoords;
    float sunMask = dot(normalize(sun), normalize(eyeVector)) / 2.0 + 0.5; // 1 directly in sun, 0 directly opposite of sun
    
    sunMask = smoothstep(0.98,1,sunMask)*2;

    vec3 skyboxCol = texture(skybox, TexCoords).rgb;
    vec3 col = vec3(sunMask*0.4) + (skyboxCol);
    //vec3 col = vec3(pow(gradient, 32), pow(gradient, 48) / 2.0 + 0.5, gradient / 4.0 + 0.75);
    
    FragColor = vec4(col, 1.0);
}