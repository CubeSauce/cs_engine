struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord: TEXCOORD0;
    float4 color: COLOR0;
};

struct VSOutput
{
    float4 position: SV_Position;
    float3 normal: NORMAL;
    float4 color: COLOR0;
};

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	matrix World;
	matrix WorldInverseTranspose;
	matrix View;
	matrix Projection;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;

    output.normal = mul(input.normal, (float3x3)WorldInverseTranspose);
    
    output.position = float4(input.position, 1.0);
    output.position = mul(output.position, World);
    output.position = mul(output.position, View);
    output.position = mul(output.position, Projection);
    
    output.color = input.color;

    return output;
}
