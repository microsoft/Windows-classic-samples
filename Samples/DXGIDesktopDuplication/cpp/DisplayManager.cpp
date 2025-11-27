// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "DisplayManager.h"
#include "AdvancedColorTonemapper.h"
using namespace DirectX;

//
// Constructor NULLs out vars
//
DisplayManager::DisplayManager() : m_Device(nullptr),
                                   m_DeviceContext(nullptr),
                                   m_MoveSurf(nullptr),
                                   m_VertexShader(nullptr),
                                   m_PixelShader(nullptr),
                                   m_InputLayout(nullptr),
                                   m_RTV(nullptr),
                                   m_SamplerLinear(nullptr)
{
}

//
// Destructor calls CleanRefs to destroy everything
//
DisplayManager::~DisplayManager()
{
    CleanRefs();
}

//
// Initialize D3D variables
//
void DisplayManager::InitD3D(DxResources* Data, const DXGI_OUTPUT_DESC1& DesktopDesc)
{
    m_Device = Data->Device;
    m_DeviceContext = Data->Context;
    m_VertexShader = Data->VertexShader;
    m_PixelShader = Data->PixelShader;
    m_InputLayout = Data->InputLayout;
    m_SamplerLinear = Data->SamplerLinear;

    m_Tonemapper = std::make_unique<AdvancedColorTonemapper>(DesktopDesc, m_Device);
}

//
// Process a given frame and its metadata
//
DUPL_RETURN DisplayManager::ProcessFrame(_In_ FrameData* Data, _Inout_ ID3D11Texture2D* SharedSurf, INT OffsetX, INT OffsetY, _In_ DXGI_OUTPUT_DESC1* DeskDesc)
{
    DUPL_RETURN Ret = DUPL_RETURN_SUCCESS;

    // Process dirties and moves
    if (Data->FrameInfo.TotalMetadataBufferSize)
    {
        D3D11_TEXTURE2D_DESC Desc;
        Data->Frame->GetDesc(&Desc);

        if (Data->MoveCount)
        {
            Ret = CopyMove(SharedSurf, std::span<DXGI_OUTDUPL_MOVE_RECT>(reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(Data->MetaData), Data->MoveCount), OffsetX, OffsetY, DeskDesc, Desc.Width, Desc.Height);
            if (Ret != DUPL_RETURN_SUCCESS)
            {
                return Ret;
            }
        }

        if (Data->DirtyCount)
        {
            std::span<RECT> DirtyRects(reinterpret_cast<RECT*>(Data->MetaData + (Data->MoveCount * sizeof(DXGI_OUTDUPL_MOVE_RECT))), Data->DirtyCount);

            if ((DeskDesc->ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) || (Desc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT))
            {
                // For HDR outputs, tonemap the screen image to SDR while copying the dirty rects
                Ret = m_Tonemapper->CopyDirtyWithTonemapping(Data->Frame, SharedSurf, DirtyRects, OffsetX, OffsetY, DeskDesc);
            }
            else
            {
                Ret = CopyDirty(Data->Frame, SharedSurf, DirtyRects, OffsetX, OffsetY, DeskDesc);
            }
        }
    }

    return Ret;
}

//
// Returns D3D device being used
//
ID3D11Device* DisplayManager::GetDevice()
{
    return m_Device.get();
}

//
// Set appropriate source and destination rects for move rects
//
void DisplayManager::SetMoveRect(_Out_ RECT* SrcRect, _Out_ RECT* DestRect, _In_ DXGI_OUTPUT_DESC1* DeskDesc, _In_ DXGI_OUTDUPL_MOVE_RECT* MoveRect, INT TexWidth, INT TexHeight)
{
    switch (DeskDesc->Rotation)
    {
        case DXGI_MODE_ROTATION_UNSPECIFIED:
        case DXGI_MODE_ROTATION_IDENTITY:
        {
            SrcRect->left = MoveRect->SourcePoint.x;
            SrcRect->top = MoveRect->SourcePoint.y;
            SrcRect->right = MoveRect->SourcePoint.x + MoveRect->DestinationRect.right - MoveRect->DestinationRect.left;
            SrcRect->bottom = MoveRect->SourcePoint.y + MoveRect->DestinationRect.bottom - MoveRect->DestinationRect.top;

            *DestRect = MoveRect->DestinationRect;
            break;
        }
        case DXGI_MODE_ROTATION_ROTATE90:
        {
            SrcRect->left = TexHeight - (MoveRect->SourcePoint.y + MoveRect->DestinationRect.bottom - MoveRect->DestinationRect.top);
            SrcRect->top = MoveRect->SourcePoint.x;
            SrcRect->right = TexHeight - MoveRect->SourcePoint.y;
            SrcRect->bottom = MoveRect->SourcePoint.x + MoveRect->DestinationRect.right - MoveRect->DestinationRect.left;

            DestRect->left = TexHeight - MoveRect->DestinationRect.bottom;
            DestRect->top = MoveRect->DestinationRect.left;
            DestRect->right = TexHeight - MoveRect->DestinationRect.top;
            DestRect->bottom = MoveRect->DestinationRect.right;
            break;
        }
        case DXGI_MODE_ROTATION_ROTATE180:
        {
            SrcRect->left = TexWidth - (MoveRect->SourcePoint.x + MoveRect->DestinationRect.right - MoveRect->DestinationRect.left);
            SrcRect->top = TexHeight - (MoveRect->SourcePoint.y + MoveRect->DestinationRect.bottom - MoveRect->DestinationRect.top);
            SrcRect->right = TexWidth - MoveRect->SourcePoint.x;
            SrcRect->bottom = TexHeight - MoveRect->SourcePoint.y;

            DestRect->left = TexWidth - MoveRect->DestinationRect.right;
            DestRect->top = TexHeight - MoveRect->DestinationRect.bottom;
            DestRect->right = TexWidth - MoveRect->DestinationRect.left;
            DestRect->bottom =  TexHeight - MoveRect->DestinationRect.top;
            break;
        }
        case DXGI_MODE_ROTATION_ROTATE270:
        {
            SrcRect->left = MoveRect->SourcePoint.x;
            SrcRect->top = TexWidth - (MoveRect->SourcePoint.x + MoveRect->DestinationRect.right - MoveRect->DestinationRect.left);
            SrcRect->right = MoveRect->SourcePoint.y + MoveRect->DestinationRect.bottom - MoveRect->DestinationRect.top;
            SrcRect->bottom = TexWidth - MoveRect->SourcePoint.x;

            DestRect->left = MoveRect->DestinationRect.top;
            DestRect->top = TexWidth - MoveRect->DestinationRect.right;
            DestRect->right = MoveRect->DestinationRect.bottom;
            DestRect->bottom =  TexWidth - MoveRect->DestinationRect.left;
            break;
        }
        default:
        {
            DestRect = {};
            SrcRect = {};
            break;
        }
    }
}

//
// Copy move rectangles
//
DUPL_RETURN DisplayManager::CopyMove(_Inout_ ID3D11Texture2D* SharedSurf, std::span<DXGI_OUTDUPL_MOVE_RECT> MoveRects, INT OffsetX, INT OffsetY, _In_ DXGI_OUTPUT_DESC1* DeskDesc, INT TexWidth, INT TexHeight)
{
    D3D11_TEXTURE2D_DESC FullDesc;
    SharedSurf->GetDesc(&FullDesc);

    // Make new intermediate surface to copy into for moving
    if (!m_MoveSurf)
    {
        D3D11_TEXTURE2D_DESC MoveDesc = FullDesc;
        MoveDesc.Width = DeskDesc->DesktopCoordinates.right - DeskDesc->DesktopCoordinates.left;
        MoveDesc.Height = DeskDesc->DesktopCoordinates.bottom - DeskDesc->DesktopCoordinates.top;
        MoveDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
        MoveDesc.MiscFlags = 0;
        HRESULT hr = m_Device->CreateTexture2D(&MoveDesc, nullptr, m_MoveSurf.put());
        if (FAILED(hr))
        {
            return ProcessFailure(m_Device.get(), L"Failed to create staging texture for move rects", L"Error", hr, SystemTransitionsExpectedErrors);
        }
    }

    for (UINT i = 0; i < MoveRects.size(); ++i)
    {
        RECT SrcRect;
        RECT DestRect;

        SetMoveRect(&SrcRect, &DestRect, DeskDesc, &(MoveRects[i]), TexWidth, TexHeight);

        // Copy rect out of shared surface
        D3D11_BOX Box = {};
        Box.left = SrcRect.left + DeskDesc->DesktopCoordinates.left - OffsetX;
        Box.top = SrcRect.top + DeskDesc->DesktopCoordinates.top - OffsetY;
        Box.front = 0;
        Box.right = SrcRect.right + DeskDesc->DesktopCoordinates.left - OffsetX;
        Box.bottom = SrcRect.bottom + DeskDesc->DesktopCoordinates.top - OffsetY;
        Box.back = 1;
        m_DeviceContext->CopySubresourceRegion(m_MoveSurf.get(), 0, SrcRect.left, SrcRect.top, 0, SharedSurf, 0, &Box);

        // Copy back to shared surface
        Box.left = SrcRect.left;
        Box.top = SrcRect.top;
        Box.front = 0;
        Box.right = SrcRect.right;
        Box.bottom = SrcRect.bottom;
        Box.back = 1;
        m_DeviceContext->CopySubresourceRegion(SharedSurf, 0, DestRect.left + DeskDesc->DesktopCoordinates.left - OffsetX, DestRect.top + DeskDesc->DesktopCoordinates.top - OffsetY, 0, m_MoveSurf.get(), 0, &Box);
    }

    return DUPL_RETURN_SUCCESS;
}

//
// Sets up vertices for dirty rects for rotated desktops
//
#pragma warning(push)
#pragma warning(disable:__WARNING_USING_UNINIT_VAR) // false positives in SetDirtyVert due to tool bug

void DisplayManager::SetDirtyVert(_Out_writes_(NUMVERTICES) Vertex* Vertices, _In_ RECT* Dirty, INT OffsetX, INT OffsetY, _In_ DXGI_OUTPUT_DESC1* DeskDesc, _In_ D3D11_TEXTURE2D_DESC* FullDesc, _In_ D3D11_TEXTURE2D_DESC* ThisDesc)
{
    INT CenterX = FullDesc->Width / 2;
    INT CenterY = FullDesc->Height / 2;

    INT Width = DeskDesc->DesktopCoordinates.right - DeskDesc->DesktopCoordinates.left;
    INT Height = DeskDesc->DesktopCoordinates.bottom - DeskDesc->DesktopCoordinates.top;

    // Rotation compensated destination rect
    RECT DestDirty = *Dirty;

    // Set appropriate coordinates compensated for rotation
    switch (DeskDesc->Rotation)
    {
        case DXGI_MODE_ROTATION_ROTATE90:
        {
            DestDirty.left = Width - Dirty->bottom;
            DestDirty.top = Dirty->left;
            DestDirty.right = Width - Dirty->top;
            DestDirty.bottom = Dirty->right;

            Vertices[0].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[1].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[2].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[5].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
            break;
        }
        case DXGI_MODE_ROTATION_ROTATE180:
        {
            DestDirty.left = Width - Dirty->right;
            DestDirty.top = Height - Dirty->bottom;
            DestDirty.right = Width - Dirty->left;
            DestDirty.bottom = Height - Dirty->top;

            Vertices[0].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[1].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[2].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[5].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
            break;
        }
        case DXGI_MODE_ROTATION_ROTATE270:
        {
            DestDirty.left = Dirty->top;
            DestDirty.top = Height - Dirty->right;
            DestDirty.right = Dirty->bottom;
            DestDirty.bottom = Height - Dirty->left;

            Vertices[0].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[1].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[2].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[5].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
            break;
        }
        default:
            assert(false); // drop through

            [[fallthrough]]
        case DXGI_MODE_ROTATION_UNSPECIFIED:
        case DXGI_MODE_ROTATION_IDENTITY:
        {
            Vertices[0].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[1].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[2].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
            Vertices[5].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
            break;
        }
    }

    // Set positions
    Vertices[0].Pos = XMFLOAT3((DestDirty.left + DeskDesc->DesktopCoordinates.left - OffsetX - CenterX) / static_cast<FLOAT>(CenterX),
                             -1 * (DestDirty.bottom + DeskDesc->DesktopCoordinates.top - OffsetY - CenterY) / static_cast<FLOAT>(CenterY),
                             0.0f);
    Vertices[1].Pos = XMFLOAT3((DestDirty.left + DeskDesc->DesktopCoordinates.left - OffsetX - CenterX) / static_cast<FLOAT>(CenterX),
                             -1 * (DestDirty.top + DeskDesc->DesktopCoordinates.top - OffsetY - CenterY) / static_cast<FLOAT>(CenterY),
                             0.0f);
    Vertices[2].Pos = XMFLOAT3((DestDirty.right + DeskDesc->DesktopCoordinates.left - OffsetX - CenterX) / static_cast<FLOAT>(CenterX),
                             -1 * (DestDirty.bottom + DeskDesc->DesktopCoordinates.top - OffsetY - CenterY) / static_cast<FLOAT>(CenterY),
                             0.0f);
    Vertices[3].Pos = Vertices[2].Pos;
    Vertices[4].Pos = Vertices[1].Pos;
    Vertices[5].Pos = XMFLOAT3((DestDirty.right + DeskDesc->DesktopCoordinates.left - OffsetX - CenterX) / static_cast<FLOAT>(CenterX),
                             -1 * (DestDirty.top + DeskDesc->DesktopCoordinates.top - OffsetY - CenterY) / static_cast<FLOAT>(CenterY),
                             0.0f);

    Vertices[3].TexCoord = Vertices[2].TexCoord;
    Vertices[4].TexCoord = Vertices[1].TexCoord;
}

#pragma warning(pop) // re-enable __WARNING_USING_UNINIT_VAR

//
// Copies dirty rectangles
//
DUPL_RETURN DisplayManager::CopyDirty(_In_ ID3D11Texture2D* SrcSurface, _Inout_ ID3D11Texture2D* SharedSurf, std::span<RECT> DirtyRects, INT OffsetX, INT OffsetY, _In_ DXGI_OUTPUT_DESC1* DeskDesc)
{
    HRESULT hr;

    D3D11_TEXTURE2D_DESC FullDesc;
    SharedSurf->GetDesc(&FullDesc);

    D3D11_TEXTURE2D_DESC ThisDesc;
    SrcSurface->GetDesc(&ThisDesc);

    if (!m_RTV)
    {
        hr = m_Device->CreateRenderTargetView(SharedSurf, nullptr, m_RTV.put());
        if (FAILED(hr))
        {
            return ProcessFailure(m_Device.get(), L"Failed to create render target view for dirty rects", L"Error", hr, SystemTransitionsExpectedErrors);
        }
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderDesc;
    ShaderDesc.Format = ThisDesc.Format;
    ShaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ShaderDesc.Texture2D.MostDetailedMip = ThisDesc.MipLevels - 1;
    ShaderDesc.Texture2D.MipLevels = ThisDesc.MipLevels;

    // Create new shader resource view
    winrt::com_ptr<ID3D11ShaderResourceView> ShaderResource;
    hr = m_Device->CreateShaderResourceView(SrcSurface, &ShaderDesc, ShaderResource.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create shader resource view for dirty rects", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    FLOAT BlendFactor[4] = {0.f, 0.f, 0.f, 0.f};
    ID3D11SamplerState* Samplers[] = { m_SamplerLinear.get() };
    ID3D11RenderTargetView* RTVs[] = { m_RTV.get() };
    ID3D11ShaderResourceView* SRVs[] = { ShaderResource.get() };

    m_DeviceContext->OMSetBlendState(nullptr, BlendFactor, 0xFFFFFFFF);
    m_DeviceContext->OMSetRenderTargets(1, RTVs, nullptr);
    m_DeviceContext->VSSetShader(m_VertexShader.get(), nullptr, 0);
    m_DeviceContext->PSSetShader(m_PixelShader.get(), nullptr, 0);
    m_DeviceContext->PSSetShaderResources(0, 1, SRVs);
    m_DeviceContext->PSSetSamplers(0, 1, Samplers);
    m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create space for vertices for the dirty rects if the current space isn't large enough
    UINT BytesNeeded = sizeof(Vertex) * NUMVERTICES * DirtyRects.size();
    if (BytesNeeded > m_DirtyVertexBuffer.size())
    {
        try
        {
            m_DirtyVertexBuffer.resize(BytesNeeded);
        }
        catch (std::bad_alloc)
        {
            return ProcessFailure(nullptr, L"Failed to allocate memory for dirty vertex buffer.", L"Error", E_OUTOFMEMORY);
        }
    }

    // Fill them in
    Vertex* DirtyVertex = reinterpret_cast<Vertex*>(m_DirtyVertexBuffer.data());
    for (UINT i = 0; i < DirtyRects.size(); ++i, DirtyVertex += NUMVERTICES)
    {
        SetDirtyVert(DirtyVertex, &(DirtyRects[i]), OffsetX, OffsetY, DeskDesc, &FullDesc, &ThisDesc);
    }

    // Create vertex buffer
    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.ByteWidth = BytesNeeded;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = m_DirtyVertexBuffer.data();

    winrt::com_ptr<ID3D11Buffer> VertBuf;
    hr = m_Device->CreateBuffer(&BufferDesc, &InitData, VertBuf.put());
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device.get(), L"Failed to create vertex buffer in dirty rect processing", L"Error", hr, SystemTransitionsExpectedErrors);
    }
    UINT Stride = sizeof(Vertex);
    UINT Offset = 0;
    ID3D11Buffer* Buffers[] = { VertBuf.get() };
    m_DeviceContext->IASetVertexBuffers(0, 1, Buffers, &Stride, &Offset);

    D3D11_VIEWPORT VP;
    VP.Width = static_cast<FLOAT>(FullDesc.Width);
    VP.Height = static_cast<FLOAT>(FullDesc.Height);
    VP.MinDepth = 0.0f;
    VP.MaxDepth = 1.0f;
    VP.TopLeftX = 0.0f;
    VP.TopLeftY = 0.0f;
    m_DeviceContext->RSSetViewports(1, &VP);

    m_DeviceContext->Draw(NUMVERTICES * DirtyRects.size(), 0);

    return DUPL_RETURN_SUCCESS;
}

//
// Clean all references
//
void DisplayManager::CleanRefs()
{
    m_DeviceContext = nullptr;
    m_Device = nullptr;
    m_MoveSurf = nullptr;
    m_VertexShader = nullptr;
    m_PixelShader = nullptr;
    m_InputLayout = nullptr;
    m_SamplerLinear = nullptr;
    m_RTV = nullptr;

    if (m_Tonemapper)
    {
        m_Tonemapper->Cleanup();
        m_Tonemapper.reset();
    }
}
