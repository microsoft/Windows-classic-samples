//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "LinearEaseInOutAnimation.h"
#include "SampleElementBase.h"
#include "SampleElementAnimation.h"
#include "SlimRWLock.h"

class CSampleEngine
{
private:
    void ReleaseElementResources();

    std::vector<SPSampleElementBase> m_elements;
    CSlimRWLock m_elementlock;

    float m_pointerCurrentX;
    float m_pointerCurrentY;

protected:
    std::shared_ptr<DeviceResources> m_deviceResources;

    CSampleElementBase* FindElement(float x, float y);
    CSampleElementBase* m_pointerOver;

public:
    CSampleEngine(void);
    ~CSampleEngine(void);
    virtual void Draw();
    void Init();
    void Update();
    void AddElement(SPSampleElementBase);
    D2D1_SIZE_F GetEffectiveSize();
    float GetPointerX();
    float GetPointerY();
    template <class T> std::shared_ptr<T> CreateElement();
    virtual void PointerDown(float x, float y);
    virtual void PointerUpdate(float x, float y);
    virtual void PointerUp(float x, float y);
    virtual void CreateDeviceIndependentResources(const std::shared_ptr<DeviceResources>& deviceResources);
    virtual void ReleaseDeviceIndependentResources();
    virtual void CreateDeviceResources();
    virtual void ReleaseDeviceResources();
};





