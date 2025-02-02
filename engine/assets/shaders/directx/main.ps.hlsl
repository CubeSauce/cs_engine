struct PSInput
{
    float4 position: SV_Position;
    float3 normal: NORMAL;
};

struct PSOutput
{
    float4 color: SV_Target0;
};

PSOutput main(PSInput input)
{
    PSOutput output = (PSOutput)0;
    output.color = float4(input.normal, 1.0);
    return output;
}
