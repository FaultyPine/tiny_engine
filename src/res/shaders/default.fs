#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec2 texCoord;
in vec3 color;


// texture samplers
uniform sampler2D tex_diffuse1;
uniform sampler2D tex_diffuse2;
//uniform float time;

void main()
{
    vec4 tex1 = texture(tex_diffuse1, texCoord);
    vec4 tex2 = texture(tex_diffuse2, texCoord);
    

    // linearly interpolate between both textures (80% container, 20% awesomeface)
	//FragColor = mix(tex1, tex2, (sin(time*5.0)+1) / 2);
    FragColor = vec4(normal, 1.0);
}