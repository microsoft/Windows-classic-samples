//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "SampleElementBase.h"

class CSampleElementTextBlock : public CSampleElementBase
{
private:
    void PrepareLayout();
    std::wstring m_text;
    float m_fontSize;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_textLayout;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_defaultbrush;

public:
    CSampleElementTextBlock(void);
    CSampleElementTextBlock(float left, float top,  float width, float height, float fontSize);

    ~CSampleElementTextBlock(void);

    virtual void DoDraw();
    virtual void CreateDeviceResources();
    virtual void ReleaseDeviceResources();

    void SetText(std::wstring);
};
