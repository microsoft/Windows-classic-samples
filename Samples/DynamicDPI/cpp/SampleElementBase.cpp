//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "SampleElementBase.h"

CSampleElementBase::CSampleElementBase(float left, float top)
{
    m_top = top;
    m_left = left;
}

CSampleElementBase::CSampleElementBase(float left, float top, float width, float height)
{
    m_top = top;
    m_left = left;
    m_width = width;
    m_height  =height;
}

CSampleElementBase::~CSampleElementBase(void)
{
    ReleaseDeviceResources();
    ReleaseDeviceIndependentResources();
}

void CSampleElementBase::DoDraw()
{
    m_deviceResources->GetD2DDeviceContext()->FillRectangle(
        D2D1::RectF(m_left, m_top, m_left + m_width, m_top + m_height),
        m_defaultBrush.Get()
        );
}

void CSampleElementBase::CreateDeviceIndependentResources(const std::shared_ptr<DeviceResources>& deviceResources)
{
    m_deviceResources = deviceResources;
}

void CSampleElementBase::ReleaseDeviceIndependentResources()
{
}

void CSampleElementBase::CreateDeviceResources()
{
    DX::ThrowIfFailed(m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_defaultBrush));
}

void CSampleElementBase::ReleaseDeviceResources()
{
    m_defaultBrush.Reset();
}

void CSampleElementBase::SetPosition(float left, float top)
{
    m_top = top;
    m_left = left;
}

float CSampleElementBase::GetWidth()
{
    return m_width;
}

float CSampleElementBase::GetHeight()
{
    return m_height;
}