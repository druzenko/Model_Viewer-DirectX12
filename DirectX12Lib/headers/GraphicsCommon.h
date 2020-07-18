#pragma once

#include "pch.h"

namespace Graphics
{
	extern D3D12_RASTERIZER_DESC RasterizerDefault;

	extern D3D12_BLEND_DESC BlendNoColorWrite;        // XXX
	extern D3D12_BLEND_DESC BlendDisable;            // 1, 0

	void InitializeCommonState(void);
}
