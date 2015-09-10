//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "DeviceResources.h"
#include "DirectXHelper.h"

class CSampleElementBase
{
    friend class CSampleEngine;

public:
    CSampleElementBase(float left, float top);
    CSampleElementBase(float left, float top, float width, float height);

    ~CSampleElementBase(void);
    virtual void DoDraw();
    virtual void CreateDeviceIndependentResources(const std::shared_ptr<DeviceResources>& deviceResources);
    virtual void ReleaseDeviceIndependentResources();
    virtual void CreateDeviceResources();
    virtual void ReleaseDeviceResources();
    virtual float GetWidth();
    virtual float GetHeight();
    virtual void OnPointerUp(float /* x */, float /* y */) {};
    virtual void OnPointerDown(float /* x */, float /* y */) {};
    virtual void OnPointerUpdate(float /* x */, float /* y */) {};
    virtual void OnPointerEnter() {};
    virtual void OnPointerLeave() {};
    virtual void OnEnterSizeMove() {};
    virtual void OnExitSizeMove() {};
    virtual void OnDpiChange(int /* Dpi */) {};
    virtual void Tick() {};

    float Left() const      { return m_left;};
    float Top() const       { return m_top; };
    void Left(float left)   { m_left = left; };
    void Top(float top)     { m_top = top; };

    void SetPosition(float left, float top);

protected:
    float m_top;
    float m_left;
    float m_width;
    float m_height;

    // Cached pointer to device resources.
    std::shared_ptr<DeviceResources> m_deviceResources;

    // Device resources.
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_defaultBrush;
    Microsoft::WRL::ComPtr<ID2D1RectangleGeometry> m_bounds;

};

typedef std::shared_ptr<CSampleElementBase> SPSampleElementBase;
