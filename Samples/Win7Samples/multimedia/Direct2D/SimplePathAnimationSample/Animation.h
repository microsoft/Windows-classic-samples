// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <math.h>

/******************************************************************
*                                                                 *
* Base Animation Class                                            *
*                                                                 *
******************************************************************/

template <class T>
class Animation
{
public:
    Animation(T start, T end, T duration) :
        m_Start(start),
        m_End(end),
        m_Duration(duration)
    {
    }

    void SetStart(T start)
    {
        m_Start = start;
    }

    T GetStart()
    {
        return m_Start;
    }

    void SetEnd(T end)
    {
        m_End = end;
    }

    T GetEnd()
    {
        return m_End;
    }

    void SetDuration(T duration)
    {
        m_Duration = max(0, duration);
    }

    T GetDuration()
    {
        return m_Duration;
    }

    T GetValue(T time)
    {
        time = min(max(time, 0), m_Duration);
        return ComputeValue(time);
    }

protected:
    virtual T ComputeValue(T time) = 0;

    T m_Duration;
    T m_Start;
    T m_End;
};

/******************************************************************
*                                                                 *
* Linearly Interpolate Between Start and End                      *
*                                                                 *
******************************************************************/

template <class T>
class LinearAnimation : public Animation<T>
{
public:
    LinearAnimation(T start=0, T end=0, T duration=0) :
        Animation(start, end, duration)
    {
    }
protected:
    virtual T ComputeValue(T time)
    {
        return m_Start + ((m_End - m_Start) * (time / m_Duration));
    }
};

/******************************************************************
*                                                                 *
*                                                                 *
*                                                                 *
******************************************************************/

template <class T>
class EaseInExponentialAnimation : public Animation<T>
{
public:
    EaseInExponentialAnimation(T start=0, T end=0, T duration=0) :
        Animation(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
        return m_Start + (m_End - m_Start) * pow(2, 10 * (time/m_Duration - 1));
    }
};

/******************************************************************
*                                                                 *
*                                                                 *
*                                                                 *
******************************************************************/

template <class T>
class EaseOutExponentialAnimation : public Animation<T>
{
public:
    EaseOutExponentialAnimation(T start=0, T end=0, T duration=0) :
        Animation(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
        return m_Start + (m_End - m_Start) * (-pow(2, -10 * time/m_Duration) + 1);
    }
};

/******************************************************************
*                                                                 *
*                                                                 *
*                                                                 *
******************************************************************/

template <class T>
class EaseInOutExponentialAnimation : public Animation<T>
{
public:
    EaseInOutExponentialAnimation(T start=0, T end=0, T duration=0) :
        Animation(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
        //compute the current time relative to the midpoint
        time = time / (m_Duration / 2);
        //if we haven't reached the midpoint, we want to do the ease-in portion
        if (time < 1)
        {
            return m_Start + (m_End - m_Start)/2 * pow(2, 10 * (time - 1));
        }
        //otherwise, do the ease-out portion
        return m_Start + (m_End - m_Start)/2 * (-pow(2, -10 * --time) + 2);
    }
};

