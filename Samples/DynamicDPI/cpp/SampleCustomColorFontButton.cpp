//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "SampleCustomColorFontButton.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace D2D1;

CSampleCustomColorFontButton::CSampleCustomColorFontButton(void) :
    CSampleHoverElement(),
    m_pressed(false),
    m_size(18.0F)
{
}

CSampleCustomColorFontButton::CSampleCustomColorFontButton(float left, float top, float size, CustomGlyphId id) :
    CSampleHoverElement(),
    m_pressed(false),
    m_size(size)
{
    m_left = left;
    m_top = top;

    // The custom font 'ColorToolbarIcons' defines a set code points in the private range starting at 0xE000.
    // The 'CustomGlphyId' enum provides mnemonics for the defined icons.  This code converts enum to the appropriate
    // code point.  glyphs[1] is set to 0x0000 to null terminate the string so it can properly be used to generate a wstring.
    wchar_t glyphs[2];
    glyphs[0] = 0xE000 + static_cast<wchar_t>(id);
    glyphs[1] = 0x0000;
    m_text = wstring(glyphs);
}

CSampleCustomColorFontButton::~CSampleCustomColorFontButton(void)
{
    ReleaseDeviceIndependentResources();
}

void CSampleCustomColorFontButton::SetClickHandler(function<void ()> handler)
{
    m_mouseClickedFunction = handler;
}

void CSampleCustomColorFontButton::CreateDeviceIndependentResources(const std::shared_ptr<DeviceResources>& deviceResources)
{
    CSampleHoverElement::CreateDeviceIndependentResources(deviceResources);

    auto dwriteFactory = m_deviceResources->GetDWriteFactory();

    DX::ThrowIfFailed(
        dwriteFactory->CreateTextFormat(
            L"Color Toolbar Icons",
            m_deviceResources->GetFontCollection(),
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            m_size,
            L"en-us",
            &m_textFormat
            )
        );

    ComPtr<IDWriteTextFormat1> textFormat;

    DX::ThrowIfFailed(m_textFormat.As(&textFormat));
    DX::ThrowIfFailed(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    DX::ThrowIfFailed(textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
}

void CSampleCustomColorFontButton::ReleaseDeviceIndependentResources()
{
    m_textFormat.Reset();
    m_textLayout.Reset();
    CSampleHoverElement::ReleaseDeviceIndependentResources();
}

void CSampleCustomColorFontButton::CreateDeviceResources()
{
    CSampleHoverElement::CreateDeviceResources();
    DX::ThrowIfFailed(m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_blackBrush));
}

void CSampleCustomColorFontButton::ReleaseDeviceResources()
{
    m_blackBrush.Reset();
    //Important: without this bad leaks can happen via captured sceneelements holding on to sceneEngines.
    m_mouseClickedFunction = nullptr;
    CSampleHoverElement::ReleaseDeviceResources();
}

void CSampleCustomColorFontButton::DoDraw()
{
    if (!m_textLayout)
    {
        DX::ThrowIfFailed(
            m_deviceResources->GetDWriteFactory()->CreateTextLayout(
                m_text.c_str(),
                static_cast<unsigned int>(m_text.length()),
                m_textFormat.Get(),
                m_size * m_text.length() * 2.0f,
                m_size * 2.0f,
                &m_textLayout
                )
            );

        // Set the rectangle of the button to tightly contain the text.
        DWRITE_TEXT_METRICS metrics = { 0 };
        DX::ThrowIfFailed(m_textLayout->GetMetrics(&metrics));
        m_width = metrics.width + 6.0f;
        m_height = metrics.height + 6.0f;
    }

    CSampleHoverElement::DoDraw();

    auto context = m_deviceResources->GetD2DDeviceContext();

    if (m_pressed == false)
    {
        context->DrawTextLayout(
            Point2F(m_left + 3.0f, m_top + 3.0F),
            m_textLayout.Get(),
            m_blackBrush.Get(),
            D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP | D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
            );
    }
    else
    {
        context->FillRectangle(
            D2D1::RectF(m_left, m_top, m_left + m_width, m_top + m_height),
            m_defaultBrush.Get()
            );

        context->DrawTextLayout(
            Point2F(m_left + 3.0f, m_top + 3.0F),
            m_textLayout.Get(),
            m_blackBrush.Get(),
            D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP | D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
            );
    }
}

void CSampleCustomColorFontButton::OnPointerDown(float /* x */, float /* y */)
{
    m_pressed = true;
}

void CSampleCustomColorFontButton::OnPointerUp(float /* x */, float /* y */)
{
    m_pressed = false;

    if (m_mouseClickedFunction)
    {
        m_mouseClickedFunction();
    }
}