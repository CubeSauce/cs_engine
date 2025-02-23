#version 450

layout(location = 1) in vec4 input_color;
layout(location = 0) out vec4 _entryPointOutput_color;

void main()
{
    _entryPointOutput_color = input_color;
}

