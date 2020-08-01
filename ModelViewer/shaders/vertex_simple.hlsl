cbuffer ModelViewProjection : register(b0, space0)
{
    matrix MVP;
};

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

VSOutput main(VSInput input)
{
    VSOutput result;

    result.position = mul(MVP, float4(input.position, 1.0));
    result.color = input.color;
    result.uv = input.uv;

    return result;
}
