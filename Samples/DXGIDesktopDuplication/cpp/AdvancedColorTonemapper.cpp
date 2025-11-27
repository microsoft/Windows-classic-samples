#include "AdvancedColorTonemapper.h"
#include <windows.graphics.display.interop.h>
#include <dispatcherqueue.h>

static D2D1_POINT_2F ToPoint(winrt::Windows::Foundation::Point point)
{
    return { point.X, point.Y };
}

AdvancedColorTonemapper::AdvancedColorTonemapper(DXGI_OUTPUT_DESC1 outputDesc, const winrt::com_ptr<ID3D11Device>& device)
{
    // Create a DisplayInformation object to track the current display settings for the monitor we're capturing
    auto interopFactory = winrt::get_activation_factory<winrt::Windows::Graphics::Display::DisplayInformation, IDisplayInformationStaticsInterop>();
    m_outputDisplayInfo = winrt::capture<winrt::Windows::Graphics::Display::DisplayInformation>(interopFactory.get(), &IDisplayInformationStaticsInterop::GetForMonitor, outputDesc.Monitor);
    m_outputAdvancedColorInfo = m_outputDisplayInfo.GetAdvancedColorInfo();

    winrt::com_ptr<IDXGIDevice> dxgiDevice = device.as<IDXGIDevice>();

    // Wrap the D3D device with a new D2D device
    winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dFactory.put()));
    winrt::check_hresult(m_d2dFactory->CreateDevice(dxgiDevice.get(), m_d2dDevice.put()));
    winrt::check_hresult(m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_d2dContext.put()));

    // Create an SDR whitelevel adjustment effect
    winrt::check_hresult(m_d2dContext->CreateEffect(CLSID_D2D1WhiteLevelAdjustment, m_whitelevelAdjustmentEffect.put()));
    winrt::check_hresult(m_whitelevelAdjustmentEffect->SetValue<float>(D2D1_WHITELEVELADJUSTMENT_PROP_OUTPUT_WHITE_LEVEL, D2D1_SCENE_REFERRED_SDR_WHITE_LEVEL));
    winrt::check_hresult(m_whitelevelAdjustmentEffect->SetValue<float>(D2D1_WHITELEVELADJUSTMENT_PROP_INPUT_WHITE_LEVEL, D2D1_SCENE_REFERRED_SDR_WHITE_LEVEL));

    // Create an HDR tonemapping effect with its output set to SDR @ standard scene-referred level (80 nits)
    winrt::check_hresult(m_d2dContext->CreateEffect(CLSID_D2D1HdrToneMap, m_hdrTonemappingEffect.put()));
    winrt::check_hresult(m_hdrTonemappingEffect->SetValue<float>(D2D1_HDRTONEMAP_PROP_OUTPUT_MAX_LUMINANCE, D2D1_SCENE_REFERRED_SDR_WHITE_LEVEL));
    winrt::check_hresult(m_hdrTonemappingEffect->SetValue<D2D1_HDRTONEMAP_DISPLAY_MODE>(D2D1_HDRTONEMAP_PROP_DISPLAY_MODE, D2D1_HDRTONEMAP_DISPLAY_MODE_SDR));

    m_hdrTonemappingEffect->SetInputEffect(0, m_whitelevelAdjustmentEffect.get());

    // Configure the color management effect to perform an absolute colorimetric conversion from scRGB to sRGB
    winrt::check_hresult(m_d2dContext->CreateColorContextFromDxgiColorSpace(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709, m_srcColorContext.put()));
    winrt::check_hresult(m_d2dContext->CreateColorContextFromDxgiColorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709, m_destColorContext.put()));

    // Create the color management effect with the color contexts
    winrt::check_hresult(m_d2dContext->CreateEffect(CLSID_D2D1ColorManagement, m_colorManagementEffect.put()));
    winrt::check_hresult(m_colorManagementEffect->SetValue(D2D1_COLORMANAGEMENT_PROP_DESTINATION_COLOR_CONTEXT, m_destColorContext.get()));
    winrt::check_hresult(m_colorManagementEffect->SetValue(D2D1_COLORMANAGEMENT_PROP_SOURCE_COLOR_CONTEXT, m_srcColorContext.get()));
    winrt::check_hresult(m_colorManagementEffect->SetValue(D2D1_COLORMANAGEMENT_PROP_QUALITY, D2D1_COLORMANAGEMENT_QUALITY_BEST));
    winrt::check_hresult(m_colorManagementEffect->SetValue(D2D1_COLORMANAGEMENT_PROP_DESTINATION_RENDERING_INTENT, D2D1_COLORMANAGEMENT_RENDERING_INTENT_ABSOLUTE_COLORIMETRIC));

    m_colorManagementEffect->SetInputEffect(0, m_hdrTonemappingEffect.get());
}

AdvancedColorTonemapper::~AdvancedColorTonemapper()
{
}

DUPL_RETURN AdvancedColorTonemapper::CopyDirtyWithTonemapping(_In_ ID3D11Texture2D* SrcSurface, _In_ ID3D11Texture2D* SharedSurf, std::span<RECT> DirtyBuffers, INT OffsetX, INT OffsetY, _In_ DXGI_OUTPUT_DESC1* DeskDesc)
{
    try
    {
        winrt::com_ptr<ID3D11Texture2D> srcSurface;
        srcSurface.copy_from(SrcSurface);
        winrt::com_ptr<ID3D11Texture2D> sharedSurf;
        sharedSurf.copy_from(SharedSurf);

        // Wrap the SrcBitmap
        winrt::com_ptr<ID2D1Bitmap1> srcBitmap;
        winrt::check_hresult(m_d2dContext->CreateBitmapFromDxgiSurface(srcSurface.as<IDXGISurface>().get(), nullptr, srcBitmap.put()));

        // Wrap the SharedSurf
        D2D1_BITMAP_PROPERTIES1 sharedSurfBitmapProps = {};
        sharedSurfBitmapProps.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
        winrt::com_ptr<ID2D1Bitmap1> sharedSurfBitmap;
        winrt::check_hresult(m_d2dContext->CreateBitmapFromDxgiSurface(sharedSurf.as<IDXGISurface>().get(), nullptr, sharedSurfBitmap.put()));

        // Configure the HDR tonemapping effect
        winrt::Windows::Graphics::Display::AdvancedColorInfo outputAdvancedColorInfo{ nullptr };
        m_outputAdvancedColorInfo = m_outputDisplayInfo.GetAdvancedColorInfo();
        outputAdvancedColorInfo = m_outputAdvancedColorInfo;

        // CONSIDER: Consider having a message loop thread listen to DisplayInformation::AdvancedColorInfoChanged and update the resulting tonemapping pipeline only when it changes for further optimization

        // The first and last effects in the graph differ by advanced color mode
        winrt::com_ptr<ID2D1Image> renderImage;
        winrt::com_ptr<ID2D1Effect> firstEffect;

        try
        {
            switch (outputAdvancedColorInfo.CurrentAdvancedColorKind())
            {
            case winrt::Windows::Graphics::Display::AdvancedColorKind::StandardDynamicRange:
            {
                // No processing is necessary
                renderImage = srcBitmap;
                break;
            }
            case winrt::Windows::Graphics::Display::AdvancedColorKind::HighDynamicRange:
            {
                // The pipeline goes whitelevel adjustment -> tonemapping -> color management
                firstEffect = m_whitelevelAdjustmentEffect;
                m_whitelevelAdjustmentEffect->SetInput(0, srcBitmap.get());
                m_hdrTonemappingEffect->SetInputEffect(0, m_whitelevelAdjustmentEffect.get());
                m_colorManagementEffect->SetInputEffect(0, m_hdrTonemappingEffect.get());
                m_colorManagementEffect->GetOutput(renderImage.put());

                // The tonemapper tends to look best with around a 200 nit whitelevel
                constexpr float TargetWhiteLevelInNits = 200.0f;

                // The luminance will be scaled by the factor we are using for the SDR whitelevel adjustment
                winrt::check_hresult(m_hdrTonemappingEffect->SetValue<float>(D2D1_HDRTONEMAP_PROP_INPUT_MAX_LUMINANCE, outputAdvancedColorInfo.MaxLuminanceInNits() * TargetWhiteLevelInNits / outputAdvancedColorInfo.SdrWhiteLevelInNits()));
                winrt::check_hresult(m_whitelevelAdjustmentEffect->SetValue<float>(D2D1_WHITELEVELADJUSTMENT_PROP_OUTPUT_WHITE_LEVEL, outputAdvancedColorInfo.SdrWhiteLevelInNits()));
                winrt::check_hresult(m_whitelevelAdjustmentEffect->SetValue<float>(D2D1_WHITELEVELADJUSTMENT_PROP_INPUT_WHITE_LEVEL, TargetWhiteLevelInNits));

                break;
            }
            case winrt::Windows::Graphics::Display::AdvancedColorKind::WideColorGamut:
            {
                // Only perform color management from scRGB -> sRGB
                firstEffect = m_colorManagementEffect;
                m_colorManagementEffect->SetInput(0, srcBitmap.get());
                m_colorManagementEffect->GetOutput(renderImage.put());
                break;
            }
            }

            m_d2dContext->BeginDraw();
            m_d2dContext->SetRenderingControls(D2D1_RENDERING_CONTROLS{ D2D1_BUFFER_PRECISION_16BPC_FLOAT });
            m_d2dContext->SetTarget(sharedSurfBitmap.get());

            for (UINT dirtyRectIndex = 0; dirtyRectIndex < DirtyBuffers.size(); dirtyRectIndex++)
            {
                const RECT& dirtyRect = DirtyBuffers[dirtyRectIndex];

                D2D1_POINT_2F targetOffset{ dirtyRect.left + OffsetX + DeskDesc->DesktopCoordinates.left, dirtyRect.top + OffsetY + DeskDesc->DesktopCoordinates.top };
                D2D1_RECT_F imageRect{ dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom };
                m_d2dContext->DrawImage(renderImage.get(), &targetOffset, &imageRect, D2D1_INTERPOLATION_MODE_LINEAR, D2D1_COMPOSITE_MODE_SOURCE_OVER);
            }

            winrt::check_hresult(m_d2dContext->EndDraw());
        }
        catch (...)
        {
            // Clear the bitmap from any saved objects
            m_d2dContext->SetTarget(nullptr);
            if (firstEffect)
            {
                firstEffect->SetInput(0, nullptr);
            }
            throw;
        }
    }
    catch (winrt::hresult_error& error)
    {
        return ProcessFailure(nullptr, error.message().c_str(), L"Error", error.code());
    }

    return DUPL_RETURN_SUCCESS;
}

void AdvancedColorTonemapper::Cleanup()
{
    m_outputDisplayInfo = nullptr;
    m_outputAdvancedColorInfo = nullptr;
}


