struct PSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord: TEXCOORD0;
    float4 color: COLOR0;
};

struct PSOutput
{
    float4 color: SV_Target0;
};

sampler Sampler : register(s0);

Texture2D Texture : register(t0);

PSOutput main(PSInput input)
{
    PSOutput output = (PSOutput)0;
    float4 texel = Texture.Sample(Sampler, input.texcoord);
    output.color = texel;
    return output;
}
