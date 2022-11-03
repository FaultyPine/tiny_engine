#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;
uniform sampler2D mainTex;
uniform float screenWidth;
uniform float screenHeight;
uniform float time;

vec2 center = vec2(0.5, 0.5);

void main()
{
    vec2 uv = vec2(TexCoords.x, 1-TexCoords.y);
    vec3 screenTex = texture(mainTex, uv).rgb;
    
    vec3 col = screenTex;

    // distort
    //float distToCenter = 1-pow(1-distance(uv, center), 36);
    //col = texture(mainTex, uv * distToCenter).rgb ;

    FragColor = vec4(col, 1.0);
}