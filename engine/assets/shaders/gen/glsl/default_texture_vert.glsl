#version 450

uniform vec4 VS_CONSTANT_BUFFER[16];
layout(location = 0) in vec3 input_position;
layout(location = 1) in vec3 input_normal;
layout(location = 2) in vec2 input_texcoord;
layout(location = 0) out vec3 _entryPointOutput_normal;
layout(location = 1) out vec2 _entryPointOutput_texcoord;

void main()
{
    mat4 _119 = transpose(mat4(VS_CONSTANT_BUFFER[4], VS_CONSTANT_BUFFER[5], VS_CONSTANT_BUFFER[6], VS_CONSTANT_BUFFER[7]));
    gl_Position = ((vec4(input_position, 1.0) * mat4(VS_CONSTANT_BUFFER[0], VS_CONSTANT_BUFFER[1], VS_CONSTANT_BUFFER[2], VS_CONSTANT_BUFFER[3])) * mat4(VS_CONSTANT_BUFFER[8], VS_CONSTANT_BUFFER[9], VS_CONSTANT_BUFFER[10], VS_CONSTANT_BUFFER[11])) * mat4(VS_CONSTANT_BUFFER[12], VS_CONSTANT_BUFFER[13], VS_CONSTANT_BUFFER[14], VS_CONSTANT_BUFFER[15]);
    _entryPointOutput_normal = mat3(_119[0].xyz, _119[1].xyz, _119[2].xyz) * input_normal;
    _entryPointOutput_texcoord = input_texcoord;
}

