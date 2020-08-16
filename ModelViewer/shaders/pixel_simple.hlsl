struct PSInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
    float2 uv : TEXCOORD;
};

Texture2D<float4> anteruTexture : register(t0);

SamplerState texureSampler      : register(s0);

float4 main(PSInput input) : SV_TARGET
{
    //return input.color;
    float4 color = anteruTexture.Sample(texureSampler, input.uv);
    return color;
    //return anteruTexture.Sample(texureSampler, input.uv);
}
