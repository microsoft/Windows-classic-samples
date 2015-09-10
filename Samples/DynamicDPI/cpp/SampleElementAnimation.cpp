//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "SampleElementAnimation.h"

using namespace std;

CSampleElementAnimation::CSampleElementAnimation(void)
{
}

CSampleElementAnimation::~CSampleElementAnimation(void)
{
}

CSampleElementAnimation::CSampleElementAnimation(CSampleElementBase* element, shared_ptr<CLinearEaseInOutAnimation> animation)
{
    m_element = element;
    m_animation = animation;
}

void CSampleElementAnimation::Tick()
{
    m_animation->Tick();

    if (m_element)
    {
        m_element->Left(m_animation->GetCurrentValue());
    }

    if (m_update)
    {
        m_update(m_animation->GetCurrentValue(),-1);
    }
}

void CSampleElementAnimation::Tick(int val)
{
    m_animation->Tick();

    if (m_update)
    {
        m_update(m_animation->GetCurrentValue(),val);
    }
}

bool CSampleElementAnimation::IsComplete()
{
    return m_animation->IsComplete();
}

void CSampleElementAnimation::SetSetter(function<void (float,int)> val)
{
    m_update = val;
}