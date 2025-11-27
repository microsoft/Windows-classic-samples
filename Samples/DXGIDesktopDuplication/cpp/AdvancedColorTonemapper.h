#pragma once

#include "CommonTypes.h"
#include <span>

class AdvancedColorTonemapper
{
public:
    AdvancedColorTonemapper(DXGI_OUTPUT_DESC1 outputDesc, const winrt::com_ptr<ID3D11Device>& device);
    ~AdvancedColorTonemapper();

    DUPL_RETURN CopyDirtyWithTonemapping(_In_ ID3D11Texture2D* SrcSurface, _In_ ID3D11Texture2D* SharedSurf, std::span<RECT> DirtyBuffers, INT OffsetX, INT OffsetY, _In_ DXGI_OUTPUT_DESC1* DeskDesc);
    void Cleanup();

private:
    winrt::Windows::Graphics::Display::DisplayInformation m_outputDisplayInfo{ nullptr };
    winrt::Windows::Graphics::Display::AdvancedColorInfo m_outputAdvancedColorInfo{ nullptr };

    winrt::com_ptr<ID2D1Factory7> m_d2dFactory;
    winrt::com_ptr<ID2D1DeviceContext6> m_d2dContext;
    winrt::com_ptr<ID2D1Device6> m_d2dDevice;
    winrt::com_ptr<ID2D1Effect> m_hdrTonemappingEffect;
    winrt::com_ptr<ID2D1Effect> m_whitelevelAdjustmentEffect;
    winrt::com_ptr<ID2D1Effect> m_colorManagementEffect;
    winrt::com_ptr<ID2D1ColorContext1> m_srcColorContext;
    winrt::com_ptr<ID2D1ColorContext1> m_destColorContext;
};

