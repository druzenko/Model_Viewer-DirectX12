#pragma once

#include "GraphicsCommon.h"

namespace Graphics {

	extern uint32_t g_DisplayWidth;
	extern uint32_t g_DisplayHeight;

    void Initialize(void);
    void Terminate(void);
    void Shutdown(void);
    void Present(void);

    // Returns the number of elapsed frames since application start
    uint64_t GetFrameCount(void);

    // The amount of time elapsed during the last completed frame.  The CPU and/or
    // GPU may be idle during parts of the frame.  The frame time measures the time
    // between calls to present each frame.
    float GetFrameTime(void);

    // The total number of frames per second
    float GetFrameRate(void);

    //temporary extern variables that will be replace with appropriate abstractions
    extern Microsoft::WRL::ComPtr <ID3D12Device> g_Device;
    extern Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    extern Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    extern Microsoft::WRL::ComPtr <IDXGISwapChain1> s_SwapChain1;
    extern Microsoft::WRL::ComPtr<ID3D12Resource> g_DisplayPlane[3];
    extern uint32_t m_rtvDescriptorSize;
    extern Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
}