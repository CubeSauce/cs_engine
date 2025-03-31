#version 410
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

layout(location = 1) in vec4 input_color;
layout(location = 0) out vec4 _entryPointOutput_color;

void main()
{
    _entryPointOutput_color = input_color;
}

