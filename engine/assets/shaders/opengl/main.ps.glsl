#version 330 core

out vec4 Color;

in vec3 fragment_position;
in vec3 fragment_normal;

void main()
{    
    Color = vec4(fragment_normal, 1.0);
}