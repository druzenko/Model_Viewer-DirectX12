#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h>

#if defined (min)
#undef min
#endif

#if defined (max)
#undef max
#endif

#if defined (CreateWindow)
#undef CreateWindow
#endif

#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "VectorMath.h"

#include <d3dx12.h>
//#include <d3dx12Residency.h>

#include <iostream>

#include "Utility.h"

#include <DirectXTex.h>

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#endif //PCH_H
