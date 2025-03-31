#version 410
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

layout(binding = 0, std140) uniform VS_CONSTANT_BUFFER
{
    layout(row_major) mat4 World;
    layout(row_major) mat4 WorldInverseTranspose;
    layout(row_major) mat4 View;
    layout(row_major) mat4 Projection;
} _29;

layout(location = 0) in vec3 input_position;
layout(location = 1) in vec3 input_normal;
layout(location = 2) in vec2 input_texcoord;
layout(location = 0) out vec3 _entryPointOutput_normal;
layout(location = 1) out vec2 _entryPointOutput_texcoord;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = spvWorkaroundRowMajor(_29.Projection) * (spvWorkaroundRowMajor(_29.View) * (spvWorkaroundRowMajor(_29.World) * vec4(input_position, 1.0)));
    _entryPointOutput_normal = mat3(spvWorkaroundRowMajor(_29.WorldInverseTranspose)[0].xyz, spvWorkaroundRowMajor(_29.WorldInverseTranspose)[1].xyz, spvWorkaroundRowMajor(_29.WorldInverseTranspose)[2].xyz) * input_normal;
    _entryPointOutput_texcoord = input_texcoord;
}

