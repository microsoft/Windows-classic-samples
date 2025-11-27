// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "ThreadManager.h"

DWORD WINAPI DDProc(_In_ void* Param);

ThreadManager::ThreadManager()
{
}

ThreadManager::~ThreadManager()
{
    Clean();
}

//
// Clean up resources
//
void ThreadManager::Clean()
{
    m_PtrInfo = {};

    m_ThreadHandles.clear();
    m_ThreadData.clear();
    m_ThreadCount = 0;
}

//
// Start up threads for DDA
//
DUPL_RETURN ThreadManager::Initialize(INT SingleOutput, UINT OutputCount, HANDLE UnexpectedErrorEvent, HANDLE ExpectedErrorEvent, HANDLE TerminateThreadsEvent, HANDLE SharedHandle, _In_ RECT* DesktopDim)
{
    m_ThreadCount = OutputCount;
    try
    {
        m_ThreadHandles.resize(m_ThreadCount);
        m_ThreadData.resize(m_ThreadCount);
    }
    catch (std::bad_alloc)
    {
        return ProcessFailure(nullptr, L"Failed to allocate array for threads", L"Error", E_OUTOFMEMORY);
    }

    // Create appropriate # of threads for duplication
    DUPL_RETURN Ret = DUPL_RETURN_SUCCESS;
    for (UINT i = 0; i < m_ThreadCount; ++i)
    {
        m_ThreadData[i].UnexpectedErrorEvent = UnexpectedErrorEvent;
        m_ThreadData[i].ExpectedErrorEvent = ExpectedErrorEvent;
        m_ThreadData[i].TerminateThreadsEvent = TerminateThreadsEvent;
        m_ThreadData[i].Output = (SingleOutput < 0) ? i : SingleOutput;
        m_ThreadData[i].TexSharedHandle = SharedHandle;
        m_ThreadData[i].OffsetX = DesktopDim->left;
        m_ThreadData[i].OffsetY = DesktopDim->top;
        m_ThreadData[i].PtrInfo = &m_PtrInfo;

        Ret = InitializeDx(&m_ThreadData[i].DxRes);
        if (Ret != DUPL_RETURN_SUCCESS)
        {
            return Ret;
        }

        DWORD ThreadId;
        m_ThreadHandles[i] = winrt::handle(CreateThread(nullptr, 0, DDProc, &m_ThreadData[i], 0, &ThreadId));
        if (!m_ThreadHandles[i])
        {
            return ProcessFailure(nullptr, L"Failed to create thread", L"Error", E_FAIL);
        }
    }

    return Ret;
}

//
// Get DX_RESOURCES
//
DUPL_RETURN ThreadManager::InitializeDx(_Inout_ DxResources* Data)
{
    HRESULT hr = S_OK;

    // Driver types supported
    D3D_DRIVER_TYPE DriverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

    // Feature levels supported
    D3D_FEATURE_LEVEL FeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_1
    };
    UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

    D3D_FEATURE_LEVEL FeatureLevel;
    D3D11_CREATE_DEVICE_FLAG DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Create device
    for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
    {
        hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, DeviceFlags, FeatureLevels, NumFeatureLevels,
                                D3D11_SDK_VERSION, Data->Device.put(), &FeatureLevel, Data->Context.put());
        if (SUCCEEDED(hr))
        {
            // Device creation success, no need to loop anymore
            break;
        }
    }
    if (FAILED(hr))
    {
        return ProcessFailure(nullptr, L"Failed to create device in InitializeDx", L"Error", hr);
    }

    // Vertex shader
    UINT Size = ARRAYSIZE(g_VS);
    hr = Data->Device->CreateVertexShader(g_VS, Size, nullptr, Data->VertexShader.put());
    if (FAILED(hr))
    {
        return ProcessFailure(Data->Device.get(), L"Failed to create vertex shader in InitializeDx", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Input layout
    D3D11_INPUT_ELEMENT_DESC Layout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    UINT NumElements = ARRAYSIZE(Layout);
    hr = Data->Device->CreateInputLayout(Layout, NumElements, g_VS, Size, Data->InputLayout.put());
    if (FAILED(hr))
    {
        return ProcessFailure(Data->Device.get(), L"Failed to create input layout in InitializeDx", L"Error", hr, SystemTransitionsExpectedErrors);
    }
    Data->Context->IASetInputLayout(Data->InputLayout.get());

    // Pixel shader
    Size = ARRAYSIZE(g_PS);
    hr = Data->Device->CreatePixelShader(g_PS, Size, nullptr, Data->PixelShader.put());
    if (FAILED(hr))
    {
        return ProcessFailure(Data->Device.get(), L"Failed to create pixel shader in InitializeDx", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Set up sampler
    D3D11_SAMPLER_DESC SampDesc = {};
    SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SampDesc.MinLOD = 0;
    SampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Data->Device->CreateSamplerState(&SampDesc, Data->SamplerLinear.put());
    if (FAILED(hr))
    {
        return ProcessFailure(Data->Device.get(), L"Failed to create sampler state in InitializeDx", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    return DUPL_RETURN_SUCCESS;
}

//
// Getter for the PointerInfo structure
//
PointerInfo* ThreadManager::GetPointerInfo()
{
    return &m_PtrInfo;
}

//
// Waits infinitely for all spawned threads to terminate
//
void ThreadManager::WaitForThreadTermination()
{
    if (m_ThreadCount != 0)
    {
        // Copy the thread handle smart pointers to a plain handle vector
        std::vector<HANDLE> Handles;
        std::transform(m_ThreadHandles.begin(), m_ThreadHandles.end(), std::back_inserter(Handles), [](const winrt::handle& Handle) { return Handle.get(); });

        WaitForMultipleObjectsEx(m_ThreadCount, Handles.data(), TRUE, INFINITE, FALSE);
    }
}
