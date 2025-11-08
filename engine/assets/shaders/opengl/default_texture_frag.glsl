#version 410 core
out vec4 FragColor;

in vec2 frag_texcoord;

uniform sampler2D texture_diffuse1;

void main()
{    
    FragColor = texture(texture_diffuse1, frag_texcoord);
}