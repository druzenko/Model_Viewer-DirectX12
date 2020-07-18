#include <GraphicsCommon.h>

namespace Graphics
{
	D3D12_RASTERIZER_DESC RasterizerDefault;

    D3D12_BLEND_DESC BlendNoColorWrite;
	D3D12_BLEND_DESC BlendDisable;

	void InitializeCommonState(void)
	{
        RasterizerDefault.FillMode = D3D12_FILL_MODE_SOLID;
        RasterizerDefault.CullMode = D3D12_CULL_MODE_BACK;
        RasterizerDefault.FrontCounterClockwise = TRUE;
        RasterizerDefault.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        RasterizerDefault.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        RasterizerDefault.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        RasterizerDefault.DepthClipEnable = TRUE;
        RasterizerDefault.MultisampleEnable = FALSE;
        RasterizerDefault.AntialiasedLineEnable = FALSE;
        RasterizerDefault.ForcedSampleCount = 0;
        RasterizerDefault.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        D3D12_BLEND_DESC alphaBlend = {};
        alphaBlend.IndependentBlendEnable = FALSE;
        alphaBlend.RenderTarget[0].BlendEnable = FALSE;
        alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        alphaBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        alphaBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        alphaBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        alphaBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        alphaBlend.RenderTarget[0].RenderTargetWriteMask = 0;
        BlendNoColorWrite = alphaBlend;

        alphaBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        BlendDisable = alphaBlend;
	}
}