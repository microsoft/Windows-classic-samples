//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "SampleHoverElement.h"

using namespace std;
using namespace Microsoft::WRL;

CSampleHoverElement::CSampleHoverElement(void) :
    CSampleElementBase(0.0f, 0.0f),
    m_hoverAlpha(0.0F)
{
}

CSampleHoverElement::CSampleHoverElement(float left, float top, float width, float height) :
    CSampleElementBase(left, top, width, height),
    m_hoverAlpha(0.0F)
{
}

CSampleHoverElement::~CSampleHoverElement(void)
{
    ReleaseDeviceResources();
}

void CSampleHoverElement::OnPointerEnter()
{
    if (!m_hoverFade)
    {
        shared_ptr<CLinearEaseInOutAnimation> animation;
        animation = make_shared<CLinearEaseInOutAnimation>(0.0F, 1.0F, 15.0F, false);

        m_hoverFade = make_shared<CSampleElementAnimation>(nullptr, animation);
        m_hoverFade->SetSetter([this] (float value, int)
        {
            this->SetHoverAlpha(value);
        });
    }
}

void CSampleHoverElement::SetHoverAlpha(float alpha)
{
    m_hoverAlpha = alpha;
}

void CSampleHoverElement::OnPointerLeave()
{
    if (!m_hoverFade)
    {
        shared_ptr<CLinearEaseInOutAnimation> animation;
        animation = make_shared<CLinearEaseInOutAnimation>(0.0F, 1.0F, 15.0F, false);
        m_hoverFade = make_shared<CSampleElementAnimation>(nullptr, animation);
        m_hoverFade->SetSetter([this] (float value, int)
        {
            this->SetHoverAlpha(1.0F-value);
        });
    }
    else
    {
        m_hoverFade.reset();
        this->SetHoverAlpha(0.0F);
    }
}

void CSampleHoverElement::CreateDeviceResources()
{
    CSampleElementBase::CreateDeviceResources();
}

void CSampleHoverElement::ReleaseDeviceResources()
{
    CSampleElementBase::ReleaseDeviceResources();
}

void CSampleHoverElement::DoDraw()
{
    ComPtr<ID2D1SolidColorBrush> checkedBrush;
    auto context = m_deviceResources->GetD2DDeviceContext();

    DX::ThrowIfFailed(context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGray, m_hoverAlpha), &checkedBrush));
    context->FillRectangle(D2D1::RectF(m_left, m_top, m_left + m_width, m_top + m_height), checkedBrush.Get());
}

void CSampleHoverElement::Tick()
{
    if (m_hoverFade)
    {
        m_hoverFade->Tick();
        if (m_hoverFade->IsComplete())
        {
            m_hoverFade.reset();
        }
    }
}