//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "SampleElementBase.h"
#include "LinearEaseInOutAnimation.h"

class CSampleElementAnimation
{
private:
    CSampleElementBase* m_element;
    std::shared_ptr<CLinearEaseInOutAnimation> m_animation;
    std::function<void(float, int)> m_update;

public:
    CSampleElementAnimation(void);
    CSampleElementAnimation(CSampleElementBase*, std::shared_ptr<CLinearEaseInOutAnimation> );
    ~CSampleElementAnimation(void);

    void Tick();
    void Tick(int);
    bool IsComplete();
    void SetSetter(std::function<void (float, int)>);
};

