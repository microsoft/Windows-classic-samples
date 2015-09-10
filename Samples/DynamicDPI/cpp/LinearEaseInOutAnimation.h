//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

// <summary>
// This fade in/out animation is used when the user hovers over application UI such as buttons.
// </summary>
class CLinearEaseInOutAnimation
{
private:
    bool m_complete;
    bool m_infinite;
    bool m_goBackwards;
    float m_currentTime;
    float m_currentValue;
    float m_begin;
    float m_change;
    float m_duration;

    float LinearEaseInOut(_In_ float time, _In_ float begin, _In_ float change, _In_ float duration);

public:
    CLinearEaseInOutAnimation();
    CLinearEaseInOutAnimation(_In_ float begin, _In_ float change, _In_ float duration, _In_ bool infinite);
    ~CLinearEaseInOutAnimation(void);
    float GetCurrentValue();
    void Tick();
    bool IsComplete();
};
