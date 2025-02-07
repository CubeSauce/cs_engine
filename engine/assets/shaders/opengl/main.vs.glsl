#version 330 core

layout (location = 0) in vec3 vertex_location;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_texture_coordinate;

out vec3 fragment_position;
out vec3 fragment_normal;

uniform struct {
	mat4 World;
	mat4 WorldInverseTranspose;
	mat4 View;
	mat4 Projection;
} ubo;

void main()
{
    fragment_normal = mat3(ubo.WorldInverseTranspose) * vertex_normal;
    gl_Position = ubo.Projection * ubo.View * ubo.World * vec4(vertex_location, 1.0);
    fragment_position = gl_Position.xyz;
}