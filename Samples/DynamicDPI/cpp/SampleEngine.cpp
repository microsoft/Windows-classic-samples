//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "SampleEngine.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace D2D1;

CSampleEngine::CSampleEngine()
{
    m_pointerOver = nullptr;
}

CSampleEngine::~CSampleEngine()
{
    ReleaseElementResources();
    m_elements.clear();
    m_deviceResources.reset();
}

void CSampleEngine::ReleaseElementResources()
{
    for_each(begin(m_elements), end(m_elements), [&](const SPSampleElementBase& item)
    {
        item->ReleaseDeviceResources();
        item->ReleaseDeviceIndependentResources();
    });
}

void CSampleEngine::Init()
{
}

void CSampleEngine::Update()
{
}

template <class T> std::shared_ptr<T> CSampleEngine::CreateElement()
{
    SPSceneElement newElement(new T(this));
    return newElement;
}

void CSampleEngine::AddElement(SPSampleElementBase element)
{
    m_elementlock.Enter();
    m_elements.push_back(element);
    m_elementlock.Exit();
}

void CSampleEngine::Draw()
{
    D2D1_RECT_F rect;
    auto factory = m_deviceResources->GetD2DFactory();

    m_elementlock.Enter();
    for_each(begin(m_elements),end(m_elements), [&](const SPSampleElementBase& item)
    {
        item->Tick();

        item->DoDraw();

        rect.top = item->Top();
        rect.left = item->Left();
        rect.right = item->Left() + item->GetWidth();
        rect.bottom = item->Top() + item->GetHeight();

        if (nullptr == item->m_bounds)
        {
            DX::ThrowIfFailed(factory->CreateRectangleGeometry(rect, item->m_bounds.ReleaseAndGetAddressOf()));
        }
    });

    m_elementlock.Exit();
}

CSampleElementBase* CSampleEngine::FindElement(float x, float y)
{
    CSampleElementBase* found = nullptr;
    m_elementlock.Enter();
    for (std::vector<SPSampleElementBase>::iterator it = m_elements.begin(); it != m_elements.end(); ++it)
    {
        auto item = it->get();
        BOOL contains;
        DX::ThrowIfFailed(item->m_bounds->FillContainsPoint(Point2F(x, y), Matrix3x2F::Identity(), &contains));
        if (contains)
        {
            found = item;
            break;
        }
    }
    m_elementlock.Exit();
    return found;
}

float CSampleEngine::GetPointerX()
{
    return m_pointerCurrentX;
}

float CSampleEngine::GetPointerY()
{
    return m_pointerCurrentY;
}

void CSampleEngine::PointerDown(float x, float y)
{
    CSampleElementBase* found = FindElement(x,y);
    if (found != nullptr)
    {
        found->OnPointerDown(x, y);
    }
}

void CSampleEngine::PointerUpdate(float x, float y)
{
    m_pointerCurrentX = x;
    m_pointerCurrentY = y;

    CSampleElementBase* found = FindElement(x, y);

    if ((m_pointerOver != nullptr && m_pointerOver != found))
    {
        m_pointerOver->OnPointerLeave();
        m_pointerOver = nullptr;
    }

    if (found != nullptr)
    {
        if (m_pointerOver == nullptr)
        {
            m_pointerOver = found;
            found->OnPointerEnter();
        }
        found->OnPointerUpdate(x, y);
    }
}

void CSampleEngine::PointerUp(float x, float y)
{
    CSampleElementBase* found = FindElement(x, y);
    if (found != nullptr)
    {
        found->OnPointerUp(x, y);
    }
}

void CSampleEngine::CreateDeviceIndependentResources(const std::shared_ptr<DeviceResources>& deviceResources)
{
    m_deviceResources = deviceResources;
    m_elementlock.Enter();
    for (std::vector<SPSampleElementBase>::iterator it = m_elements.begin(); it != m_elements.end(); ++it)
    {
        auto item = it->get();
        item->CreateDeviceIndependentResources(deviceResources);
    }
    m_elementlock.Exit();
}

void CSampleEngine::ReleaseDeviceIndependentResources()
{
    if (m_elements.size() > 0)
    {
        m_elementlock.Enter();
        for (std::vector<SPSampleElementBase>::iterator it = m_elements.begin(); it != m_elements.end(); ++it)
        {
            auto item = it->get();
            item->ReleaseDeviceIndependentResources();
        }
        m_elementlock.Exit();
    }
}

void CSampleEngine::CreateDeviceResources()
{
    m_elementlock.Enter();
    for (std::vector<SPSampleElementBase>::iterator it = m_elements.begin(); it != m_elements.end(); ++it)
    {
        auto item = it->get();
        item->CreateDeviceResources();
    }
    m_elementlock.Exit();
}

void CSampleEngine::ReleaseDeviceResources()
{
    if (m_elements.size() > 0)
    {
        m_elementlock.Enter();
        for (std::vector<SPSampleElementBase>::iterator it = m_elements.begin(); it != m_elements.end(); ++it)
        {
            auto item = it->get();
            item->ReleaseDeviceResources();
        }
        m_elementlock.Exit();
    }
}
