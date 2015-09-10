//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "SampleElementBase.h"

struct CSceneGraphicsBitmap
{
    UINT ImageWidth, ImageHeight;
    double ImageDPIX, ImageDPIY;
    Microsoft::WRL::ComPtr<IWICFormatConverter> Image;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> Bitmap;
};

class CSampleMultiDPIImage : public CSampleElementBase
{
private:
    std::shared_ptr<CSceneGraphicsBitmap> m_bitmap100;
    std::shared_ptr<CSceneGraphicsBitmap> m_bitmap125;
    std::shared_ptr<CSceneGraphicsBitmap> m_bitmap150;
    std::shared_ptr<CSceneGraphicsBitmap> m_bitmap200;
    float m_currentWidth;

    std::shared_ptr<CSceneGraphicsBitmap> LoadImage(std::wstring name);

public:
    CSampleMultiDPIImage();
    CSampleMultiDPIImage(float left, float top);
    ~CSampleMultiDPIImage(void);
    virtual void DoDraw();
    virtual void CreateDeviceIndependentResources(const std::shared_ptr<DeviceResources>& deviceResources);
    virtual void CreateDeviceResources();
    virtual void ReleaseDeviceResources();
    virtual float GetWidth();
};
