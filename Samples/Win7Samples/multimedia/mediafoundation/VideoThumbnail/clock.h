//////////////////////////////////////////////////////////////////////////
//
// Simple timer class. 
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

class Timer
{
    HANDLE  m_hTimer;
    BOOL    m_bBeginPeriod;
    DWORD   m_StartSysTime;
    DWORD   m_PreviousTime;
    LONG    m_lPeriodMsec;

public:

    Timer() : 
        m_hTimer(NULL), 
        m_bBeginPeriod(FALSE), 
        m_StartSysTime(0),
        m_lPeriodMsec(0),
        m_PreviousTime(0)
    {
    }

    const HANDLE& Handle() const
    {
        return m_hTimer;
    }

    BOOL InitializeTimer(LONG lPeriodMsec)
    {
        m_hTimer = CreateWaitableTimer(NULL, FALSE, NULL);

        if (!m_hTimer)
        {
            return FALSE;
        }

        LARGE_INTEGER li = {0};

        if (!SetWaitableTimer(
            m_hTimer,
            &li,
            lPeriodMsec,
            NULL,
            NULL,
            FALSE
            ))
        {
            return FALSE;
        }

        m_bBeginPeriod = (timeBeginPeriod(1) == TIMERR_NOERROR);

        m_StartSysTime = timeGetTime();

        m_lPeriodMsec = lPeriodMsec;

        return TRUE;
    }

    ~Timer()
    {
        if (m_bBeginPeriod)
        {
            timeEndPeriod(1);
            m_bBeginPeriod = FALSE;
        }

        if (m_hTimer)
        {
            CloseHandle(m_hTimer);
            m_hTimer = NULL;
        }
    }

    DWORD GetFrameNumber()
    {
        DWORD currentTime;
        DWORD currentSysTime = timeGetTime();

        if (m_StartSysTime > currentSysTime)
        {
            currentTime = currentSysTime + (0xFFFFFFFF - m_StartSysTime);
        }
        else
        {
            currentTime = currentSysTime - m_StartSysTime;
        }

        DWORD frame = currentTime / m_lPeriodMsec;
        DWORD delta = (currentTime - m_PreviousTime) / m_lPeriodMsec;

        if (delta > 0)
        {
            m_PreviousTime = currentTime;
        }

        return frame;
    }

};
