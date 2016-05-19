// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

/******************************************************************
*                                                                 *
*  RingBuffer                                                     *
*                                                                 *
******************************************************************/

template<typename T, UINT maxElements>
class RingBuffer
{
public:
    RingBuffer()
        : m_start(0), m_count(0)
    {
    }

    void Add(T element)
    {
        m_elements[(m_start + m_count)%maxElements] = element;

        if (m_count < maxElements)
        {
            m_count++;
        }
        else
        {
            m_start = (m_start + 1) % maxElements;
        }
    }

    T GetFirst() const
    {
        Assert(m_count > 0);

        return m_elements[m_start];
    }

    T GetLast() const
    {
        Assert(m_count > 0);

        return m_elements[(m_start + m_count-1)%maxElements];
    }

    T GetCount() const
    {
        return m_count;
    }

    void Reset()
    {
        m_start = 0;
        m_count = 0;
    }

private:
    UINT m_start;
    UINT m_count;
    T m_elements[maxElements];
};
