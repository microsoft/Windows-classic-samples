//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "LinearEaseInOutAnimation.h"

CLinearEaseInOutAnimation::CLinearEaseInOutAnimation()
{
}

CLinearEaseInOutAnimation::CLinearEaseInOutAnimation(
    _In_    float begin, 
    _In_    float change, 
    _In_    float duration, 
    _In_    bool infinite
    )
{
    m_currentTime = 0.0F;
    m_currentValue = 0.0F;
    m_goBackwards = false;
    m_begin = begin;
    m_change = change;
    m_duration = duration;
    m_complete = false;
    m_infinite = infinite;
}

CLinearEaseInOutAnimation::~CLinearEaseInOutAnimation()
{
}

// <summary>
// This function causes a linear fade in/out for animations.
// </summary>
float 
CLinearEaseInOutAnimation::LinearEaseInOut(
    _In_    float time, 
    _In_    float begin, 
    _In_    float change, 
    _In_    float duration
    )
{
    return(((time * change) / duration) + begin);
}

float 
CLinearEaseInOutAnimation::GetCurrentValue()
{
    return m_currentValue;
}

void 
CLinearEaseInOutAnimation::Tick()
{
    if (m_currentTime == m_duration)
    {
        if (m_infinite)
        {
            m_goBackwards = true;
        }
        else
        {
            m_complete = true;
        } 
    }
    else if (m_currentTime == 0.0F)
    {
        m_goBackwards = false;
    }

    if (!m_goBackwards && m_currentTime < m_duration)
    {
        m_currentTime += 1;
        m_currentValue = LinearEaseInOut(m_currentTime, m_begin ,m_change, m_duration);
    }
    else if (m_goBackwards && m_currentTime >= 0)
    {
        m_currentTime -= 1;
        m_currentValue = LinearEaseInOut(m_currentTime, m_begin, m_change, m_duration);
    }
}

bool 
CLinearEaseInOutAnimation::IsComplete()
{
    return m_complete;
}