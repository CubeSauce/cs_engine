#version 450

uniform sampler2D SPIRV_Cross_CombinedTextureSampler;

layout(location = 2) in vec2 input_texcoord;
layout(location = 0) out vec4 _entryPointOutput_color;

void main()
{
    _entryPointOutput_color = texture(SPIRV_Cross_CombinedTextureSampler, input_texcoord);
}

