//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "SampleMultiDPIImage.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace D2D1;

CSampleMultiDPIImage::CSampleMultiDPIImage() : CSampleElementBase(0.0F, 0.0F)
{
}

CSampleMultiDPIImage::CSampleMultiDPIImage(float left, float top) : CSampleElementBase(left, top)
{
}

CSampleMultiDPIImage::~CSampleMultiDPIImage(void)
{
    ReleaseDeviceResources();
    ReleaseDeviceIndependentResources();
}

void CSampleMultiDPIImage::DoDraw()
{
    float dpi = m_deviceResources->GetDpi();

    shared_ptr<CSceneGraphicsBitmap> toDraw;

    switch ((INT)dpi)
    {
    case 192:
        toDraw = m_bitmap200;
        break;
    case 144:
        toDraw = m_bitmap150;
        break;
    case 120:
        toDraw = m_bitmap125;
        break;
    case 96:
    default:
        toDraw = m_bitmap100;
        break;
    }

    m_currentWidth = static_cast<float>(toDraw->ImageWidth);

    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    if (toDraw->Bitmap)
    {
        d2dContext->SetUnitMode(D2D1_UNIT_MODE::D2D1_UNIT_MODE_PIXELS);  // Disable D2D DPI scaling

        d2dContext->DrawBitmap(
            toDraw->Bitmap.Get(),
            RectF(
                m_left,
                m_top,
                static_cast<float>(toDraw->ImageWidth) + m_left,
                static_cast<float>(toDraw->ImageHeight) + m_top
                ),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
            );

        d2dContext->SetUnitMode(D2D1_UNIT_MODE::D2D1_UNIT_MODE_DIPS);    // Reenable D2D scaling
    }
}

shared_ptr<CSceneGraphicsBitmap> CSampleMultiDPIImage::LoadImage(std::wstring name)
{
    auto bitmap = make_shared<CSceneGraphicsBitmap>();

    auto factory = m_deviceResources->GetWicImagingFactory();

    PCWSTR uri = name.c_str();

    auto attr = GetFileAttributes(uri);
    if (0xFFFFFFFF!=attr)
    {
        ComPtr<IWICBitmapDecoder> decoder;
        DX::ThrowIfFailed(
            factory->CreateDecoderFromFilename(
                uri,
                nullptr,
                GENERIC_READ,
                WICDecodeMetadataCacheOnDemand,
                decoder.GetAddressOf()
                )
            );

        ComPtr<IWICBitmapFrameDecode> source;
        DX::ThrowIfFailed(decoder->GetFrame(0, source.GetAddressOf()));

        source->GetSize(&bitmap->ImageWidth,&bitmap->ImageHeight);
        source->GetResolution(&bitmap->ImageDPIX, &bitmap->ImageDPIY);

        DX::ThrowIfFailed(factory->CreateFormatConverter(bitmap->Image.GetAddressOf()));
        DX::ThrowIfFailed(
            bitmap->Image->Initialize(
                source.Get(),
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0,
                WICBitmapPaletteTypeMedianCut
                )
            );
    }
    return bitmap;
}


void CSampleMultiDPIImage::CreateDeviceIndependentResources(const std::shared_ptr<DeviceResources>& deviceResources)
{
    CSampleElementBase::CreateDeviceIndependentResources(deviceResources);

    WCHAR szPath[MAX_PATH];
    DX::ThrowIfFailed(GetModuleFileName(NULL, szPath, MAX_PATH) == 0 ? E_FAIL : S_OK);
    auto path = wstring(szPath);
    auto found = path.find_last_of('\\');
    auto dirPath = path.substr(0,found);

    auto bitmap100path = dirPath;
    bitmap100path.append(L"\\png100.png");

    auto bitmap125path = dirPath;
    bitmap125path.append(L"\\png125.png");

    auto bitmap150path = dirPath;
    bitmap150path.append(L"\\png150.png");

    auto bitmap200path = dirPath;
    bitmap200path.append(L"\\png200.png");

    m_bitmap100 = LoadImage(bitmap100path.c_str());
    m_bitmap125 = LoadImage(bitmap125path.c_str());
    m_bitmap150 = LoadImage(bitmap150path.c_str());
    m_bitmap200 = LoadImage(bitmap200path.c_str());
}

void CSampleMultiDPIImage::CreateDeviceResources()
{
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    if (m_bitmap100->Image)
    {
        DX::ThrowIfFailed(
            d2dContext->CreateBitmapFromWicBitmap(
                m_bitmap100->Image.Get(),
                m_bitmap100->Bitmap.GetAddressOf()
                )
            );
    }

    if (m_bitmap125->Image) 
    {
        DX::ThrowIfFailed(
            d2dContext->CreateBitmapFromWicBitmap(
                m_bitmap125->Image.Get(),
                m_bitmap125->Bitmap.GetAddressOf()
                )
            );
    }

    if (m_bitmap150->Image)
    {
        DX::ThrowIfFailed(
            d2dContext->CreateBitmapFromWicBitmap(
                m_bitmap150->Image.Get(),
                m_bitmap150->Bitmap.GetAddressOf()
                )
            );
    }

    if (m_bitmap200->Image)
    {
        DX::ThrowIfFailed(
            d2dContext->CreateBitmapFromWicBitmap(
                m_bitmap200->Image.Get(),
                m_bitmap200->Bitmap.GetAddressOf()
                )
            );
    }
}

void CSampleMultiDPIImage::ReleaseDeviceResources()
{
    m_bitmap100.reset();
    m_bitmap125.reset();
    m_bitmap150.reset();
    m_bitmap200.reset();
}


float CSampleMultiDPIImage::GetWidth()
{
    return m_currentWidth;
}
