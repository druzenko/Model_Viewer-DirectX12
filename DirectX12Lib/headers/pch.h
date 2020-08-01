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

#include <d3dx12.h>
//#include <d3dx12Residency.h>

#include <iostream>

#include "Utility.h"

#include <DirectXTex.h>

#endif //PCH_H
