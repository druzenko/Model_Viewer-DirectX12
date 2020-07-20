cbuffer ModelViewProjection : register(b0)
{
    matrix MVP;
};

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(VSInput input)
{
    VSOutput result;

    result.position = mul(MVP, float4(input.position, 1.0));
    result.color = input.color;

    return result;
}
