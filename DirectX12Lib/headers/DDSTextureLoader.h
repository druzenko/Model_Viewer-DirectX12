//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//--------------------------------------------------------------------------------------
//
// Functions for loading a DDS texture and creating a Direct3D runtime resource for it
//
// Note these functions are useful as a light-weight runtime loader for DDS files. For
// a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#pragma once

#include <d3d12.h>
#include <DDS.h>
#pragma warning(push)
#pragma warning(disable : 4005)
#include <stdint.h>
#pragma warning(pop)

HRESULT __cdecl CreateDDSTextureFromMemory(_In_ ID3D12Device* d3dDevice,
    _In_reads_bytes_(ddsDataSize) const uint8_t* ddsData,
    _In_ size_t ddsDataSize,
    _In_ size_t maxsize,
    _In_ bool forceSRGB,
    _Outptr_opt_ ID3D12Resource** texture,
    _In_ D3D12_CPU_DESCRIPTOR_HANDLE textureView,
    _Out_opt_ DirectX::DDS_ALPHA_MODE* alphaMode = nullptr
);

HRESULT __cdecl CreateDDSTextureFromFile(_In_ ID3D12Device* d3dDevice,
    _In_z_ const wchar_t* szFileName,
    _In_ size_t maxsize,
    _In_ bool forceSRGB,
    _Outptr_opt_ ID3D12Resource** texture,
    _In_ D3D12_CPU_DESCRIPTOR_HANDLE textureView,
    _Out_opt_ DirectX::DDS_ALPHA_MODE* alphaMode = nullptr
);
