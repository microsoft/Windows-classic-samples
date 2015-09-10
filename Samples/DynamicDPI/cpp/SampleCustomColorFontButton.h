//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "SampleHoverElement.h"

enum class CustomGlyphId
{
    Bold = 0,
    Underline = 1,
    Italics = 2,
    IncreaseSize = 3,
    DecreaseSize = 4,
    Highlight = 5,
    SetColor = 6,
    BulletList = 7,
    NumberList = 8,
    Indent = 9,
    Outdent = 10,
    LeftJustify = 11,
    CenterJustify = 12,
    RightJustify = 13
};

class CSampleCustomColorFontButton : public CSampleHoverElement
{
public:
    CSampleCustomColorFontButton(void);
    CSampleCustomColorFontButton(float left, float top, float size, CustomGlyphId id);
    ~CSampleCustomColorFontButton(void);

    void SetClickHandler(std::function<void ()>);

    virtual void CreateDeviceIndependentResources(const std::shared_ptr<DeviceResources>& deviceResources);
    virtual void ReleaseDeviceIndependentResources();
    virtual void CreateDeviceResources();
    virtual void ReleaseDeviceResources();
    virtual void DoDraw();

protected:
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_blackBrush;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_textLayout;
    std::wstring m_text;
    float m_size;
    bool m_pressed;

private:
    std::function<void ()> m_mouseClickedFunction;
    virtual void OnPointerDown(float x, float y);
    virtual void OnPointerUp(float x, float y); 
};
