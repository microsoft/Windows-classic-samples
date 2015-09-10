//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "SampleElementBase.h"
#include "SampleElementAnimation.h"

class CSampleHoverElement : public CSampleElementBase
{
private:
    float m_hoverAlpha;
    std::shared_ptr<CSampleElementAnimation> m_hoverFade;

protected:
    virtual void OnPointerEnter();
    virtual void OnPointerLeave();
    virtual void DoDraw();
    virtual void CreateDeviceResources();
    virtual void ReleaseDeviceResources();
    virtual void Tick();
    void SetHoverAlpha(float alpha);

public:
    CSampleHoverElement(void);
    CSampleHoverElement(float left, float top, float width, float height);
    ~CSampleHoverElement(void);
};

