//#include <iostream>
#include <d3d12.h>
#include <wrl/client.h>
//#include <pch.h>
#include <Core.h>
#include <Graphics.h>
#include <Utility.h>
#include <chrono>

class ModelViewer : public Core::IApp
{
private:

    // Pipeline objects.
    D3D12_VIEWPORT m_viewport = { 0.0f, 0.0f, static_cast<float>(Graphics::g_DisplayWidth), static_cast<float>(Graphics::g_DisplayHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
    D3D12_RECT m_scissorRect = { 0, 0, LONG_MAX, LONG_MAX };
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

    // App resources.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    DirectX::XMMATRIX m_ModelMatrix;
    DirectX::XMMATRIX m_ViewMatrix;
    DirectX::XMMATRIX m_ProjectionMatrix;

    float m_FoV = 45.0f;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    void WaitForPreviousFrame();
    void PopulateCommandList();

public:

	ModelViewer() {}

	void Startup() override;
	void Cleanup() override;
	void Update(float deltaT) override;
	void RenderScene() override;
};

CREATE_APPLICATION(ModelViewer)

void ModelViewer::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. More advanced samples 
    // illustrate how to use fences for efficient resource usage.

    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    ASSERT_SUCCEEDED(Graphics::m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
        ASSERT_SUCCEEDED(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_frameIndex = ((IDXGISwapChain3*)Graphics::s_SwapChain1.Get())->GetCurrentBackBufferIndex();
}

void ModelViewer::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ASSERT_SUCCEEDED(Graphics::m_commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ASSERT_SUCCEEDED(m_commandList->Reset(Graphics::m_commandAllocator.Get(), m_pipelineState.Get()));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    D3D12_RESOURCE_BARRIER barrier1 = {};
    barrier1.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier1.Transition.pResource = Graphics::g_DisplayPlane[m_frameIndex].Get();
    barrier1.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier1.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier1.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier1.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

    m_commandList->ResourceBarrier(1, &barrier1);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {};
    rtvHandle.ptr = SIZE_T(INT64(Graphics::m_rtvHeap->GetCPUDescriptorHandleForHeapStart().ptr)) + INT64(m_frameIndex) * INT64(Graphics::m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->IASetIndexBuffer(&m_indexBufferView);

    // Update the MVP matrix
    DirectX::XMMATRIX mvpMatrix = XMMatrixMultiply(m_ModelMatrix, m_ViewMatrix);
    mvpMatrix = XMMatrixMultiply(mvpMatrix, m_ProjectionMatrix);
    m_commandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / 4, &mvpMatrix, 0);


    //m_commandList->DrawInstanced(3, 1, 0, 0);
    m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

    // Indicate that the back buffer will now be used to present.
    D3D12_RESOURCE_BARRIER barrier2 = {};
    barrier2.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier2.Transition.pResource = Graphics::g_DisplayPlane[m_frameIndex].Get();
    barrier2.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier2.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier2.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier2.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

    m_commandList->ResourceBarrier(1, &barrier2);

    ASSERT_SUCCEEDED(m_commandList->Close());
}

void ModelViewer::Startup()
{
    // Create a root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
        if (Graphics::g_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)) < 0)
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        // Allow input layout and deny unnecessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

        D3D12_ROOT_PARAMETER1 rootParameter;
        rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rootParameter.Constants.Num32BitValues = sizeof(DirectX::XMMATRIX) / sizeof(float);
        rootParameter.Constants.RegisterSpace = 0;
        rootParameter.Constants.ShaderRegister = 0;

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Version = featureData.HighestVersion;
        rootSignatureDesc.Desc_1_1.NumParameters = 1;
        rootSignatureDesc.Desc_1_1.pParameters = &rootParameter;
        rootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
        rootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
        rootSignatureDesc.Desc_1_1.Flags = rootSignatureFlags;

        Microsoft::WRL::ComPtr<ID3DBlob> signature;
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        ASSERT_SUCCEEDED(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));
        ASSERT_SUCCEEDED(Graphics::g_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ASSERT_SUCCEEDED(D3DReadFileToBlob(L"vertex_simple.cso", &vertexShader));
        ASSERT_SUCCEEDED(D3DReadFileToBlob(L"pixel_simple.cso", &pixelShader));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
        psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
        psoDesc.RasterizerState = D3D12_RASTERIZER_DESC(Graphics::RasterizerDefault);
        psoDesc.BlendState = D3D12_BLEND_DESC(Graphics::BlendDisable);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R10G10B10A2_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ASSERT_SUCCEEDED(Graphics::g_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    // Create the command list.
    ASSERT_SUCCEEDED(Graphics::g_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Graphics::m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    ASSERT_SUCCEEDED(m_commandList->Close());

    // Create the vertex buffer.
    {
        // Define the geometry for a triangle.
        struct Vertex
        {
            float position[3];
            float color[4];
        };

        Vertex triangleVertices[] =
        {
            { { -1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { -1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
            { { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
            { { -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } },
            { { -1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
            { { 1.0f, -1.0f, -1.0f }, {1.0f, 1.0f, 0.0f, 1.0f } },
            { { 1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        };

        UINT16 indices[] = {0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7, 3, 6, 2, 3, 7, 6, 0, 1, 5, 4, 0, 5, 4, 3, 0, 4, 7, 3, 1, 2, 5, 2, 6, 5};

        const UINT vertexBufferSize = sizeof(triangleVertices);
        const UINT indexBufferSize = sizeof(indices);

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        D3D12_HEAP_PROPERTIES HeapProps;
        HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProps.CreationNodeMask = 1;
        HeapProps.VisibleNodeMask = 1;
        HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC ResourceDesc;
        ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        ResourceDesc.Alignment = 0;
        ResourceDesc.Height = 1;
        ResourceDesc.DepthOrArraySize = 1;
        ResourceDesc.MipLevels = 1;
        ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        ResourceDesc.SampleDesc.Count = 1;
        ResourceDesc.SampleDesc.Quality = 0;
        ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        ResourceDesc.Width = vertexBufferSize;
        ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        ASSERT_SUCCEEDED(Graphics::g_Device->CreateCommittedResource(
            &HeapProps,
            D3D12_HEAP_FLAG_NONE,
            &ResourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        D3D12_RANGE readRange = { 0, 0 };        // We do not intend to read from this resource on the CPU.
        ASSERT_SUCCEEDED(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, triangleVertices, vertexBufferSize);
        m_vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;

        ResourceDesc.Width = indexBufferSize;
        ASSERT_SUCCEEDED(Graphics::g_Device->CreateCommittedResource(
            &HeapProps,
            D3D12_HEAP_FLAG_NONE,
            &ResourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_indexBuffer)));

        // Copy the indices data to the index buffer.
        UINT8* pIndexDataBegin;
        ASSERT_SUCCEEDED(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
        memcpy(pIndexDataBegin, indices, indexBufferSize);
        m_indexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        m_indexBufferView.SizeInBytes = indexBufferSize;
    }

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ASSERT_SUCCEEDED(Graphics::g_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ASSERT_SUCCEEDED(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }
}

void ModelViewer::Cleanup()
{
    WaitForPreviousFrame();
    CloseHandle(m_fenceEvent);
}

void ModelViewer::Update(float deltaT)
{
    static float totalTime = 0.0f;
    
    std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
    static std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::duration deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    deltaT = deltaTime.count() * 1e-9;
    totalTime += deltaT;

    // Update the model matrix.
    float angle = static_cast<float>(totalTime * 90.0);
    const DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);
    m_ModelMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));

    // Update the view matrix.
    const DirectX::XMVECTOR eyePosition = DirectX::XMVectorSet(0, 0, -10, 1);
    const DirectX::XMVECTOR focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
    const DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
    m_ViewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

    // Update the projection matrix.
    float aspectRatio = Graphics::g_DisplayWidth / static_cast<float>(Graphics::g_DisplayHeight);
    m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_FoV), aspectRatio, 0.1f, 100.0f);
}

void ModelViewer::RenderScene()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    Graphics::m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ASSERT_SUCCEEDED(Graphics::s_SwapChain1->Present(1, 0));

    WaitForPreviousFrame();
}
