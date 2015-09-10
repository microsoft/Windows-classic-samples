//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "SampleElementTextBlock.h"

using namespace std;

CSampleElementTextBlock::CSampleElementTextBlock(float left, float top,  float width, float height, float fontSize) : CSampleElementBase(left, top, width, height)
{
    m_fontSize = fontSize;
}

CSampleElementTextBlock::CSampleElementTextBlock(void) : CSampleElementBase(0.0F, 0.0F)
{
}

CSampleElementTextBlock::~CSampleElementTextBlock(void)
{
    ReleaseDeviceResources();
}

void CSampleElementTextBlock::DoDraw()
{
    ASSERT(m_defaultbrush);
    PrepareLayout();
    m_deviceResources->GetD2DDeviceContext()->DrawTextLayout(
        D2D1::Point2F(m_left, m_top),
        m_textLayout.Get(),
        m_defaultbrush.Get(),
        D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP
        );
}

void CSampleElementTextBlock::CreateDeviceResources()
{
    CSampleElementBase::CreateDeviceResources();
    DX::ThrowIfFailed(m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_defaultbrush));
}

void CSampleElementTextBlock::ReleaseDeviceResources()
{
    m_defaultbrush.Reset();
    CSampleElementBase::ReleaseDeviceResources();
}

void CSampleElementTextBlock::SetText(wstring text)
{
    m_text = text;
    m_textLayout.Reset();
}

void CSampleElementTextBlock::PrepareLayout()
{
    if (!m_textFormat)
    {
        DX::ThrowIfFailed(
            m_deviceResources->GetDWriteFactory()->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                DWRITE_FONT_WEIGHT_REGULAR,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                m_fontSize,
                L"en-us",
                m_textFormat.GetAddressOf()
                )
            );

        m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    }

    if (nullptr == m_textLayout)
    {
        DX::ThrowIfFailed(
            m_deviceResources->GetDWriteFactory()->CreateTextLayout(
                m_text.c_str(),
                static_cast<unsigned int>(m_text.length()),
                m_textFormat.Get(),
                m_width,
                m_height,
                &m_textLayout
                )
            );
    }
}
