cbuffer VSConstants : register(b0) 
{
    float4x4 mvp; 
};

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
    PSInput o;
    o.pos = mul(float4(input.pos, 1.0f), mvp);
    o.normal = input.normal;
    o.uv = input.uv;
    return o;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(input.normal * 0.5 + 0.5, 1.0);
}