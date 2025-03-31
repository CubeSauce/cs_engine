#version 410
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

uniform sampler2D SPIRV_Cross_CombinedTextureSampler;

layout(location = 2) in vec2 input_texcoord;
layout(location = 0) out vec4 _entryPointOutput_color;

void main()
{
    _entryPointOutput_color = texture(SPIRV_Cross_CombinedTextureSampler, input_texcoord);
}

