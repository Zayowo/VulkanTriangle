struct VSInput
{
    float3 inPos;
    float3 inColor;
    float2 inTexCoord;
};

struct UniformBuffer
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

ConstantBuffer<UniformBuffer> ubo;

struct VSOutput
{
    float4 pos : SV_Position;
    float3 fragColor;
    float2 fragTexCoord;
};

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    VSOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPos, 1.0))));
    // output.pos = float4(input.inPos, 0.0, 1.0);
    output.fragColor = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 fragMain(VSOutput output) : SV_Target
{
    // return float4(output.fragColor, 1.0);
    return texture.Sample(output.fragTexCoord);
    // clip(texel.a - 0.5);
    // return texel;
}