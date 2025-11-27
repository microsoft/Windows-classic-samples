// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "OutputManager.h"
using namespace DirectX;

//
// Constructor NULLs out all pointers & sets appropriate var vals
//
OutputManager::OutputManager() : m_SwapChain(nullptr),
                                 m_Device(nullptr),
                                 m_Factory(nullptr),
                                 m_DeviceContext(nullptr),
                                 m_RTV(nullptr),
                                 m_SamplerLinear(nullptr),
                                 m_BlendState(nullptr),
                                 m_VertexShader(nullptr),
                                 m_PixelShader(nullptr),
                                 m_InputLayout(nullptr),
                                 m_SharedSurf(nullptr),
                                 m_KeyMutex(nullptr),
                                 m_WindowHandle(nullptr),
                                 m_NeedsResize(false),
                                 m_OcclusionCookie(0)
{
}

//
// Destructor which calls CleanRefs to release all references and memory.
//
OutputManager::~OutputManager()
{
    CleanRefs();
}

//
// Indicates that window has been resized.
//
void OutputManager::WindowResize()
{
    m_NeedsResize = true;
}

//
// Initialize all state
//
DUPL_RETURN OutputManager::InitOutput(HWND Window, INT SingleOutput, _Out_ UINT* OutCount, _Out_ RECT* DeskBounds)
{
    HRESULT hr;

    // Store window handle
    m_WindowHandle = Window;

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

    // Create device
    for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
    {
        hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, FeatureLevels, NumFeatureLevels,
        D3D11_SDK_VERSION, m_Device.put(), &FeatureLevel, m_DeviceContext.put());
        if (SUCCEEDED(hr))
        {
            // Device creation succeeded, no need to loop anymore
            break;
        }
    }
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Device creation in OutputManager failed", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Get DXGI factory
    winrt::com_ptr<IDXGIDevice> DxgiDevice = m_Device.try_as<IDXGIDevice>();
    if (!DxgiDevice)
    {
        return ProcessFailure(nullptr, L"Failed to QI for DXGI Device", L"Error", E_NOTIMPL, nullptr);
    }

    winrt::com_ptr<IDXGIAdapter> DxgiAdapter;
    hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), DxgiAdapter.put_void());
    DxgiDevice = nullptr;
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to get parent DXGI Adapter", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    hr = DxgiAdapter->GetParent(__uuidof(IDXGIFactory2), m_Factory.put_void());
    DxgiAdapter = nullptr;
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to get parent DXGI Factory", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Register for occlusion status windows message
    hr = m_Factory->RegisterOcclusionStatusWindow(Window, OCCLUSION_STATUS_MSG, &m_OcclusionCookie);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to register for occlusion message", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Get window size
    RECT WindowRect;
    GetClientRect(m_WindowHandle, &WindowRect);
    UINT Width = WindowRect.right - WindowRect.left;
    UINT Height = WindowRect.bottom - WindowRect.top;

    // Create swapchain for window
    DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};

    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.Width = Width;
    SwapChainDesc.Height = Height;
    SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.SampleDesc.Quality = 0;
    hr = m_Factory->CreateSwapChainForHwnd(m_Device.get(), Window, &SwapChainDesc, nullptr, nullptr, m_SwapChain.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create window swapchain", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Disable the ALT-ENTER shortcut for entering full-screen mode
    hr = m_Factory->MakeWindowAssociation(Window, DXGI_MWA_NO_ALT_ENTER);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to make window association", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Create shared texture
    DUPL_RETURN Return = CreateSharedSurf(SingleOutput, OutCount, DeskBounds);
    if (Return != DUPL_RETURN_SUCCESS)
    {
        return Return;
    }

    // Make new render target view
    Return = MakeRTV();
    if (Return != DUPL_RETURN_SUCCESS)
    {
        return Return;
    }

    // Set view port
    SetViewPort(Width, Height);

    // Create the sample state
    D3D11_SAMPLER_DESC SampDesc = {};
    SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SampDesc.MinLOD = 0;
    SampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = m_Device->CreateSamplerState(&SampDesc, m_SamplerLinear.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create sampler state in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Create the blend state
    D3D11_BLEND_DESC BlendStateDesc = {};
    BlendStateDesc.AlphaToCoverageEnable = FALSE;
    BlendStateDesc.IndependentBlendEnable = FALSE;
    BlendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    BlendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = m_Device->CreateBlendState(&BlendStateDesc, m_BlendState.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create blend state in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Initialize shaders
    Return = InitShaders();
    if (Return != DUPL_RETURN_SUCCESS)
    {
        return Return;
    }

    GetWindowRect(m_WindowHandle, &WindowRect);
    MoveWindow(m_WindowHandle, WindowRect.left, WindowRect.top, (DeskBounds->right - DeskBounds->left) / 2, (DeskBounds->bottom - DeskBounds->top) / 2, TRUE);

    return Return;
}

//
// Recreate shared texture
//
DUPL_RETURN OutputManager::CreateSharedSurf(INT SingleOutput, _Out_ UINT* OutCount, _Out_ RECT* DeskBounds)
{
    HRESULT hr;

    // Get DXGI resources
    winrt::com_ptr<IDXGIDevice> DxgiDevice = m_Device.try_as<IDXGIDevice>();
    if (!DxgiDevice)
    {
        return ProcessFailure(nullptr, L"Failed to QI for DXGI Device", L"Error", E_NOTIMPL);
    }

    winrt::com_ptr<IDXGIAdapter> DxgiAdapter;
    hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), DxgiAdapter.put_void());
    DxgiDevice = nullptr;
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to get parent DXGI Adapter", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Set initial values so that we always catch the right coordinates
    DeskBounds->left = INT_MAX;
    DeskBounds->right = INT_MIN;
    DeskBounds->top = INT_MAX;
    DeskBounds->bottom = INT_MIN;

    winrt::com_ptr<IDXGIOutput> DxgiOutput;

    // Figure out right dimensions for full size desktop texture and # of outputs to duplicate
    UINT OutputCount;
    if (SingleOutput < 0)
    {
        hr = S_OK;
        for (OutputCount = 0; SUCCEEDED(hr); ++OutputCount)
        {
            DxgiOutput = nullptr;
            hr = DxgiAdapter->EnumOutputs(OutputCount, DxgiOutput.put());
            if (DxgiOutput && (hr != DXGI_ERROR_NOT_FOUND))
            {
                DXGI_OUTPUT_DESC DesktopDesc;
                DxgiOutput->GetDesc(&DesktopDesc);

                DeskBounds->left = min(DesktopDesc.DesktopCoordinates.left, DeskBounds->left);
                DeskBounds->top = min(DesktopDesc.DesktopCoordinates.top, DeskBounds->top);
                DeskBounds->right = max(DesktopDesc.DesktopCoordinates.right, DeskBounds->right);
                DeskBounds->bottom = max(DesktopDesc.DesktopCoordinates.bottom, DeskBounds->bottom);
            }
        }

        --OutputCount;
    }
    else
    {
        hr = DxgiAdapter->EnumOutputs(SingleOutput, DxgiOutput.put());
        if (FAILED(hr))
        {
            DxgiAdapter = nullptr;
            return ProcessFailure(m_Device.get(), L"Output specified to be duplicated does not exist", L"Error", hr);
        }
        DXGI_OUTPUT_DESC DesktopDesc;
        DxgiOutput->GetDesc(&DesktopDesc);
        *DeskBounds = DesktopDesc.DesktopCoordinates;

        DxgiOutput = nullptr;

        OutputCount = 1;
    }

    DxgiAdapter = nullptr;

    // Set passed in output count variable
    *OutCount = OutputCount;

    if (OutputCount == 0)
    {
        // We could not find any outputs, the system must be in a transition so return expected error
        // so we will attempt to recreate
        return DUPL_RETURN_ERROR_EXPECTED;
    }

    // Create shared texture for all duplication threads to draw into
    D3D11_TEXTURE2D_DESC DeskTexD = {};
    DeskTexD.Width = DeskBounds->right - DeskBounds->left;
    DeskTexD.Height = DeskBounds->bottom - DeskBounds->top;
    DeskTexD.MipLevels = 1;
    DeskTexD.ArraySize = 1;
    DeskTexD.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    DeskTexD.SampleDesc.Count = 1;
    DeskTexD.Usage = D3D11_USAGE_DEFAULT;
    DeskTexD.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    DeskTexD.CPUAccessFlags = 0;
    DeskTexD.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

    hr = m_Device->CreateTexture2D(&DeskTexD, nullptr, m_SharedSurf.put());
    if (FAILED(hr))
    {
        if (OutputCount != 1)
        {
            // If we are duplicating the complete desktop we try to create a single texture to hold the
            // complete desktop image and blit updates from the per output DDA interface.  The GPU can
            // always support a texture size of the maximum resolution of any single output but there is no
            // guarantee that it can support a texture size of the desktop.
            // The sample only use this large texture to display the desktop image in a single window using DX
            // we could revert back to using GDI to update the window in this failure case.
            return ProcessFailure(m_Device.get(), L"Failed to create DirectX shared texture - we are attempting to create a texture the size of the complete desktop and this may be larger than the maximum texture size of your GPU.  Please try again using the -output command line parameter to duplicate only 1 monitor or configure your computer to a single monitor configuration", L"Error", hr, SystemTransitionsExpectedErrors);
        }
        else
        {
            return ProcessFailure(m_Device.get(), L"Failed to create shared texture", L"Error", hr, SystemTransitionsExpectedErrors);
        }
    }

    // Get keyed mutex
    m_KeyMutex = m_SharedSurf.try_as<IDXGIKeyedMutex>();
    if (!m_KeyMutex)
    {
        return ProcessFailure(m_Device.get(), L"Failed to query for keyed mutex in OutputManager", L"Error", E_NOTIMPL);
    }

    return DUPL_RETURN_SUCCESS;
}

//
// Present to the application window
//
DUPL_RETURN OutputManager::UpdateApplicationWindow(_In_ PointerInfo* PointerInfo, _Inout_ bool* Occluded)
{
    // In a typical desktop duplication application there would be an application running on one system collecting the desktop images
    // and another application running on a different system that receives the desktop images via a network and display the image. This
    // sample contains both these aspects into a single application.
    // This routine is the part of the sample that displays the desktop image onto the display

    // Try and acquire sync on common display buffer
    HRESULT hr = m_KeyMutex->AcquireSync(1, 100);
    if (hr == static_cast<HRESULT>(WAIT_TIMEOUT))
    {
        // Another thread has the keyed mutex so try again later
        return DUPL_RETURN_SUCCESS;
    }
    else if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to acquire Keyed mutex in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Got mutex, so draw
    DUPL_RETURN Ret = DrawFrame();
    if (Ret == DUPL_RETURN_SUCCESS)
    {
        // We have keyed mutex so we can access the mouse info
        if (PointerInfo->Visible)
        {
            // Draw mouse into texture
            Ret = DrawMouse(PointerInfo);
        }
    }

    // Release keyed mutex
    hr = m_KeyMutex->ReleaseSync(0);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to Release Keyed mutex in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Present to window if all worked
    if (Ret == DUPL_RETURN_SUCCESS)
    {
        // Present to window
        hr = m_SwapChain->Present(1, 0);
        if (FAILED(hr))
        {
            return ProcessFailure(m_Device.get(), L"Failed to present", L"Error", hr, SystemTransitionsExpectedErrors);
        }
        else if (hr == DXGI_STATUS_OCCLUDED)
        {
            *Occluded = true;
        }
    }

    return Ret;
}

//
// Returns shared handle
//
HANDLE OutputManager::GetSharedHandle()
{
    HANDLE Hnd = nullptr;

    // QI IDXGIResource interface to synchronized shared surface.
    winrt::com_ptr<IDXGIResource> DXGIResource = m_SharedSurf.try_as<IDXGIResource>();

    if (DXGIResource)
    {
        // Obtain handle to IDXGIResource object.
        DXGIResource->GetSharedHandle(&Hnd);
    }

    return Hnd;
}

//
// Draw frame into backbuffer
//
DUPL_RETURN OutputManager::DrawFrame()
{
    HRESULT hr;

    // If window was resized, resize swapchain
    if (m_NeedsResize)
    {
        DUPL_RETURN Ret = ResizeSwapChain();
        if (Ret != DUPL_RETURN_SUCCESS)
        {
            return Ret;
        }
        m_NeedsResize = false;
    }

    // Vertices for drawing whole texture
    Vertex Vertices[NUMVERTICES] =
    {
        {XMFLOAT3(-1.0f, -1.0f, 0), XMFLOAT2(0.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
        {XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
        {XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
        {XMFLOAT3(1.0f, 1.0f, 0), XMFLOAT2(1.0f, 0.0f)},
    };

    D3D11_TEXTURE2D_DESC FrameDesc;
    m_SharedSurf->GetDesc(&FrameDesc);

    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderDesc;
    ShaderDesc.Format = FrameDesc.Format;
    ShaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ShaderDesc.Texture2D.MostDetailedMip = FrameDesc.MipLevels - 1;
    ShaderDesc.Texture2D.MipLevels = FrameDesc.MipLevels;

    // Create new shader resource view
    winrt::com_ptr<ID3D11ShaderResourceView> ShaderResource;
    hr = m_Device->CreateShaderResourceView(m_SharedSurf.get(), &ShaderDesc, ShaderResource.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create shader resource when drawing a frame", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Set resources
    UINT Stride = sizeof(Vertex);
    UINT Offset = 0;
    FLOAT blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
    ID3D11RenderTargetView* RTVs[] = { m_RTV.get() };
    ID3D11ShaderResourceView* ShaderResources[] = { ShaderResource.get() };
    ID3D11SamplerState* Samplers[] = { m_SamplerLinear.get() };
    m_DeviceContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
    m_DeviceContext->OMSetRenderTargets(1, RTVs, nullptr);
    m_DeviceContext->VSSetShader(m_VertexShader.get(), nullptr, 0);
    m_DeviceContext->PSSetShader(m_PixelShader.get(), nullptr, 0);
    m_DeviceContext->PSSetShaderResources(0, 1, ShaderResources);
    m_DeviceContext->PSSetSamplers(0, 1, Samplers);
    m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.ByteWidth = sizeof(Vertex) * NUMVERTICES;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = Vertices;

    winrt::com_ptr<ID3D11Buffer> VertexBuffer;

    // Create vertex buffer
    hr = m_Device->CreateBuffer(&BufferDesc, &InitData, VertexBuffer.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create vertex buffer when drawing a frame", L"Error", hr, SystemTransitionsExpectedErrors);
    }
    ID3D11Buffer* Buffers[] = { VertexBuffer.get() };
    m_DeviceContext->IASetVertexBuffers(0, 1, Buffers, &Stride, &Offset);

    // Draw textured quad onto render target
    m_DeviceContext->Draw(NUMVERTICES, 0);

    return DUPL_RETURN_SUCCESS;
}

//
// Process both masked and monochrome pointers
//
DUPL_RETURN OutputManager::ProcessMonoMask(bool IsMono, _Inout_ PointerInfo* PtrInfo, _Out_ INT* PtrWidth, _Out_ INT* PtrHeight, _Out_ INT* PtrLeft, _Out_ INT* PtrTop, _Inout_ std::vector<BYTE>& InitBuffer, _Out_ D3D11_BOX* Box)
{
    // Desktop dimensions
    D3D11_TEXTURE2D_DESC FullDesc;
    m_SharedSurf->GetDesc(&FullDesc);
    INT DesktopWidth = FullDesc.Width;
    INT DesktopHeight = FullDesc.Height;

    // Pointer position
    INT GivenLeft = PtrInfo->Position.x;
    INT GivenTop = PtrInfo->Position.y;

    // Figure out if any adjustment is needed for out of bound positions
    if (GivenLeft < 0)
    {
        *PtrWidth = GivenLeft + static_cast<INT>(PtrInfo->ShapeInfo.Width);
    }
    else if ((GivenLeft + static_cast<INT>(PtrInfo->ShapeInfo.Width)) > DesktopWidth)
    {
        *PtrWidth = DesktopWidth - GivenLeft;
    }
    else
    {
        *PtrWidth = static_cast<INT>(PtrInfo->ShapeInfo.Width);
    }

    if (IsMono)
    {
        PtrInfo->ShapeInfo.Height = PtrInfo->ShapeInfo.Height / 2;
    }

    if (GivenTop < 0)
    {
        *PtrHeight = GivenTop + static_cast<INT>(PtrInfo->ShapeInfo.Height);
    }
    else if ((GivenTop + static_cast<INT>(PtrInfo->ShapeInfo.Height)) > DesktopHeight)
    {
        *PtrHeight = DesktopHeight - GivenTop;
    }
    else
    {
        *PtrHeight = static_cast<INT>(PtrInfo->ShapeInfo.Height);
    }

    if (IsMono)
    {
        PtrInfo->ShapeInfo.Height = PtrInfo->ShapeInfo.Height * 2;
    }

    *PtrLeft = (GivenLeft < 0) ? 0 : GivenLeft;
    *PtrTop = (GivenTop < 0) ? 0 : GivenTop;

    // Staging buffer/texture
    D3D11_TEXTURE2D_DESC CopyBufferDesc;
    CopyBufferDesc.Width = *PtrWidth;
    CopyBufferDesc.Height = *PtrHeight;
    CopyBufferDesc.MipLevels = 1;
    CopyBufferDesc.ArraySize = 1;
    CopyBufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    CopyBufferDesc.SampleDesc.Count = 1;
    CopyBufferDesc.SampleDesc.Quality = 0;
    CopyBufferDesc.Usage = D3D11_USAGE_STAGING;
    CopyBufferDesc.BindFlags = 0;
    CopyBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    CopyBufferDesc.MiscFlags = 0;

    winrt::com_ptr<ID3D11Texture2D> CopyBuffer;
    HRESULT hr = m_Device->CreateTexture2D(&CopyBufferDesc, nullptr, CopyBuffer.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed creating staging texture for pointer", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Copy needed part of desktop image
    Box->left = *PtrLeft;
    Box->top = *PtrTop;
    Box->right = *PtrLeft + *PtrWidth;
    Box->bottom = *PtrTop + *PtrHeight;
    m_DeviceContext->CopySubresourceRegion(CopyBuffer.get(), 0, 0, 0, 0, m_SharedSurf.get(), 0, Box);

    // QI for IDXGISurface
    winrt::com_ptr<IDXGISurface> CopySurface = CopyBuffer.try_as<IDXGISurface>();
    CopyBuffer = nullptr;
    if (!CopySurface)
    {
        return ProcessFailure(nullptr, L"Failed to QI staging texture into IDXGISurface for pointer", L"Error", E_NOTIMPL, SystemTransitionsExpectedErrors);
    }

    // Map pixels
    DXGI_MAPPED_RECT MappedSurface;
    hr = CopySurface->Map(&MappedSurface, DXGI_MAP_READ);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to map surface for pointer", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // New mouseshape buffer
    try
    {
        InitBuffer.resize(*PtrWidth * *PtrHeight * BPP);
    }
    catch (std::bad_alloc)
    {
        return ProcessFailure(nullptr, L"Failed to allocate memory for new mouse shape buffer.", L"Error", E_OUTOFMEMORY);
    }

    UINT* InitBuffer32 = reinterpret_cast<UINT*>(InitBuffer.data());
    UINT* Desktop32 = reinterpret_cast<UINT*>(MappedSurface.pBits);
    UINT  DesktopPitchInPixels = MappedSurface.Pitch / sizeof(UINT);

    // What to skip (pixel offset)
    UINT SkipX = (GivenLeft < 0) ? (-1 * GivenLeft) : (0);
    UINT SkipY = (GivenTop < 0) ? (-1 * GivenTop) : (0);

    if (IsMono)
    {
        for (INT Row = 0; Row < *PtrHeight; ++Row)
        {
            // Set mask
            BYTE Mask = 0x80;
            Mask = Mask >> (SkipX % 8);
            for (INT Col = 0; Col < *PtrWidth; ++Col)
            {
                // Get masks using appropriate offsets
                BYTE AndMask = PtrInfo->PtrShapeBuffer[((Col + SkipX) / 8) + ((Row + SkipY) * (PtrInfo->ShapeInfo.Pitch))] & Mask;
                BYTE XorMask = PtrInfo->PtrShapeBuffer[((Col + SkipX) / 8) + ((Row + SkipY + (PtrInfo->ShapeInfo.Height / 2)) * (PtrInfo->ShapeInfo.Pitch))] & Mask;
                UINT AndMask32 = (AndMask) ? 0xFFFFFFFF : 0xFF000000;
                UINT XorMask32 = (XorMask) ? 0x00FFFFFF : 0x00000000;

                // Set new pixel
                InitBuffer32[(Row * *PtrWidth) + Col] = (Desktop32[(Row * DesktopPitchInPixels) + Col] & AndMask32) ^ XorMask32;

                // Adjust mask
                if (Mask == 0x01)
                {
                    Mask = 0x80;
                }
                else
                {
                    Mask = Mask >> 1;
                }
            }
        }
    }
    else
    {
        UINT* Buffer32 = reinterpret_cast<UINT*>(PtrInfo->PtrShapeBuffer.data());

        // Iterate through pixels
        for (INT Row = 0; Row < *PtrHeight; ++Row)
        {
            for (INT Col = 0; Col < *PtrWidth; ++Col)
            {
                // Set up mask
                UINT MaskVal = 0xFF000000 & Buffer32[(Col + SkipX) + ((Row + SkipY) * (PtrInfo->ShapeInfo.Pitch / sizeof(UINT)))];
                if (MaskVal)
                {
                    // Mask was 0xFF
                    InitBuffer32[(Row * *PtrWidth) + Col] = (Desktop32[(Row * DesktopPitchInPixels) + Col] ^ Buffer32[(Col + SkipX) + ((Row + SkipY) * (PtrInfo->ShapeInfo.Pitch / sizeof(UINT)))]) | 0xFF000000;
                }
                else
                {
                    // Mask was 0x00
                    InitBuffer32[(Row * *PtrWidth) + Col] = Buffer32[(Col + SkipX) + ((Row + SkipY) * (PtrInfo->ShapeInfo.Pitch / sizeof(UINT)))] | 0xFF000000;
                }
            }
        }
    }

    // Done with resource
    hr = CopySurface->Unmap();
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to unmap surface for pointer", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    return DUPL_RETURN_SUCCESS;
}

//
// Draw mouse provided in buffer to backbuffer
//
DUPL_RETURN OutputManager::DrawMouse(_In_ PointerInfo* PtrInfo)
{
    // Vars to be used
    winrt::com_ptr<ID3D11Texture2D> MouseTex;
    winrt::com_ptr<ID3D11ShaderResourceView> ShaderRes;
    winrt::com_ptr<ID3D11Buffer> VertexBufferMouse;
    D3D11_SUBRESOURCE_DATA InitData;
    D3D11_TEXTURE2D_DESC Desc;
    D3D11_SHADER_RESOURCE_VIEW_DESC SDesc;

    // Position will be changed based on mouse position
    Vertex Vertices[NUMVERTICES] =
    {
        {XMFLOAT3(-1.0f, -1.0f, 0), XMFLOAT2(0.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
        {XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
        {XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
        {XMFLOAT3(1.0f, 1.0f, 0), XMFLOAT2(1.0f, 0.0f)},
    };

    D3D11_TEXTURE2D_DESC FullDesc;
    m_SharedSurf->GetDesc(&FullDesc);
    INT DesktopWidth = FullDesc.Width;
    INT DesktopHeight = FullDesc.Height;

    // Center of desktop dimensions
    INT CenterX = (DesktopWidth / 2);
    INT CenterY = (DesktopHeight / 2);

    // Clipping adjusted coordinates / dimensions
    INT PtrWidth = 0;
    INT PtrHeight = 0;
    INT PtrLeft = 0;
    INT PtrTop = 0;

    // Buffer used if necessary (in case of monochrome or masked pointer)
    std::vector<BYTE> InitBuffer;

    // Used for copying pixels
    D3D11_BOX Box;
    Box.front = 0;
    Box.back = 1;

    Desc.MipLevels = 1;
    Desc.ArraySize = 1;
    Desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    Desc.SampleDesc.Count = 1;
    Desc.SampleDesc.Quality = 0;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    Desc.CPUAccessFlags = 0;
    Desc.MiscFlags = 0;

    // Set shader resource properties
    SDesc.Format = Desc.Format;
    SDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SDesc.Texture2D.MostDetailedMip = Desc.MipLevels - 1;
    SDesc.Texture2D.MipLevels = Desc.MipLevels;

    switch (PtrInfo->ShapeInfo.Type)
    {
        case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
        {
            PtrLeft = PtrInfo->Position.x;
            PtrTop = PtrInfo->Position.y;

            PtrWidth = static_cast<INT>(PtrInfo->ShapeInfo.Width);
            PtrHeight = static_cast<INT>(PtrInfo->ShapeInfo.Height);

            break;
        }

        case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
        {
            ProcessMonoMask(true, PtrInfo, &PtrWidth, &PtrHeight, &PtrLeft, &PtrTop, InitBuffer, &Box);
            break;
        }

        case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
        {
            ProcessMonoMask(false, PtrInfo, &PtrWidth, &PtrHeight, &PtrLeft, &PtrTop, InitBuffer, &Box);
            break;
        }

        default:
            break;
    }

    // Vertex creation
    Vertices[0].Pos.x = (PtrLeft - CenterX) / (FLOAT)CenterX;
    Vertices[0].Pos.y = -1 * ((PtrTop + PtrHeight) - CenterY) / (FLOAT)CenterY;
    Vertices[1].Pos.x = (PtrLeft - CenterX) / (FLOAT)CenterX;
    Vertices[1].Pos.y = -1 * (PtrTop - CenterY) / (FLOAT)CenterY;
    Vertices[2].Pos.x = ((PtrLeft + PtrWidth) - CenterX) / (FLOAT)CenterX;
    Vertices[2].Pos.y = -1 * ((PtrTop + PtrHeight) - CenterY) / (FLOAT)CenterY;
    Vertices[3].Pos.x = Vertices[2].Pos.x;
    Vertices[3].Pos.y = Vertices[2].Pos.y;
    Vertices[4].Pos.x = Vertices[1].Pos.x;
    Vertices[4].Pos.y = Vertices[1].Pos.y;
    Vertices[5].Pos.x = ((PtrLeft + PtrWidth) - CenterX) / (FLOAT)CenterX;
    Vertices[5].Pos.y = -1 * (PtrTop - CenterY) / (FLOAT)CenterY;

    // Set texture properties
    Desc.Width = PtrWidth;
    Desc.Height = PtrHeight;

    // Set up init data
    InitData.pSysMem = (PtrInfo->ShapeInfo.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR) ? PtrInfo->PtrShapeBuffer.data() : InitBuffer.data();
    InitData.SysMemPitch = (PtrInfo->ShapeInfo.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR) ? PtrInfo->ShapeInfo.Pitch : PtrWidth * BPP;
    InitData.SysMemSlicePitch = 0;

    // Create mouseshape as texture
    HRESULT hr = m_Device->CreateTexture2D(&Desc, &InitData, MouseTex.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create mouse pointer texture", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Create shader resource from texture
    hr = m_Device->CreateShaderResourceView(MouseTex.get(), &SDesc, ShaderRes.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create shader resource from mouse pointer texture", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    D3D11_BUFFER_DESC BDesc = {};
    BDesc.Usage = D3D11_USAGE_DEFAULT;
    BDesc.ByteWidth = sizeof(Vertex) * NUMVERTICES;
    BDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BDesc.CPUAccessFlags = 0;

    InitData = {};
    InitData.pSysMem = Vertices;

    // Create vertex buffer
    hr = m_Device->CreateBuffer(&BDesc, &InitData, VertexBufferMouse.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create mouse pointer vertex buffer in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Set resources
    FLOAT BlendFactor[4] = {0.f, 0.f, 0.f, 0.f};
    UINT Stride = sizeof(Vertex);
    UINT Offset = 0;
    ID3D11RenderTargetView* RTVs[] = { m_RTV.get() };
    ID3D11ShaderResourceView* ShaderResources[] = { ShaderRes.get() };
    ID3D11SamplerState* Samplers[] = { m_SamplerLinear.get() };
    ID3D11Buffer* Buffers[] = { VertexBufferMouse.get() };

    m_DeviceContext->IASetVertexBuffers(0, 1, Buffers, &Stride, &Offset);
    m_DeviceContext->OMSetBlendState(m_BlendState.get(), BlendFactor, 0xFFFFFFFF);
    m_DeviceContext->OMSetRenderTargets(1, RTVs, nullptr);
    m_DeviceContext->VSSetShader(m_VertexShader.get(), nullptr, 0);
    m_DeviceContext->PSSetShader(m_PixelShader.get(), nullptr, 0);
    m_DeviceContext->PSSetShaderResources(0, 1, ShaderResources);
    m_DeviceContext->PSSetSamplers(0, 1, Samplers);

    // Draw
    m_DeviceContext->Draw(NUMVERTICES, 0);

    return DUPL_RETURN_SUCCESS;
}

//
// Initialize shaders for drawing to screen
//
DUPL_RETURN OutputManager::InitShaders()
{
    HRESULT hr;

    UINT Size = ARRAYSIZE(g_VS);
    hr = m_Device->CreateVertexShader(g_VS, Size, nullptr, m_VertexShader.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create vertex shader in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    D3D11_INPUT_ELEMENT_DESC Layout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    UINT NumElements = ARRAYSIZE(Layout);
    hr = m_Device->CreateInputLayout(Layout, NumElements, g_VS, Size, m_InputLayout.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create input layout in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
    }
    m_DeviceContext->IASetInputLayout(m_InputLayout.get());

    Size = ARRAYSIZE(g_PS);
    hr = m_Device->CreatePixelShader(g_PS, Size, nullptr, m_PixelShader.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create pixel shader in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    return DUPL_RETURN_SUCCESS;
}

//
// Reset render target view
//
DUPL_RETURN OutputManager::MakeRTV()
{
    // Get backbuffer
    winrt::com_ptr<ID3D11Texture2D> BackBuffer;
    HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), BackBuffer.put_void());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to get backbuffer for making render target view in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Create a render target view
    hr = m_Device->CreateRenderTargetView(BackBuffer.get(), nullptr, m_RTV.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create render target view in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Set new render target
    ID3D11RenderTargetView* RTVs[] = { m_RTV.get() };
    m_DeviceContext->OMSetRenderTargets(1, RTVs, nullptr);

    return DUPL_RETURN_SUCCESS;
}

//
// Set new viewport
//
void OutputManager::SetViewPort(UINT Width, UINT Height)
{
    D3D11_VIEWPORT VP;
    VP.Width = static_cast<FLOAT>(Width);
    VP.Height = static_cast<FLOAT>(Height);
    VP.MinDepth = 0.0f;
    VP.MaxDepth = 1.0f;
    VP.TopLeftX = 0;
    VP.TopLeftY = 0;
    m_DeviceContext->RSSetViewports(1, &VP);
}

//
// Resize swapchain
//
DUPL_RETURN OutputManager::ResizeSwapChain()
{
    m_RTV = nullptr;

    RECT WindowRect;
    GetClientRect(m_WindowHandle, &WindowRect);
    UINT Width = WindowRect.right - WindowRect.left;
    UINT Height = WindowRect.bottom - WindowRect.top;

    // Resize swapchain
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    m_SwapChain->GetDesc(&SwapChainDesc);
    HRESULT hr = m_SwapChain->ResizeBuffers(SwapChainDesc.BufferCount, Width, Height, SwapChainDesc.BufferDesc.Format, SwapChainDesc.Flags);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to resize swapchain buffers in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Make new render target view
    DUPL_RETURN Ret = MakeRTV();
    if (Ret != DUPL_RETURN_SUCCESS)
    {
        return Ret;
    }

    // Set new viewport
    SetViewPort(Width, Height);

    return Ret;
}

//
// Releases all references
//
void OutputManager::CleanRefs()
{
    m_VertexShader = nullptr;
    m_PixelShader = nullptr;
    m_InputLayout = nullptr;
    m_RTV = nullptr;
    m_SamplerLinear = nullptr;
    m_BlendState = nullptr;
    m_DeviceContext = nullptr;
    m_Device = nullptr;
    m_SwapChain = nullptr;
    m_SharedSurf = nullptr;
    m_KeyMutex = nullptr;

    if (m_Factory)
    {
        if (m_OcclusionCookie)
        {
            m_Factory->UnregisterOcclusionStatus(m_OcclusionCookie);
            m_OcclusionCookie = 0;
        }
        m_Factory = nullptr;
    }
}
