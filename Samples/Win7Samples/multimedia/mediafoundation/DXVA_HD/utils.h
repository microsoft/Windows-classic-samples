//////////////////////////////////////////////////////////////////////
// 
// Miscellaneous helper classes.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#pragma once

void DBGMSG(PCWSTR format, ...);

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}



// DWM function pointers

typedef HRESULT (WINAPI * PFNDWMISCOMPOSITIONENABLED)(
    __out BOOL* pfEnabled
    );

typedef HRESULT (WINAPI * PFNDWMGETCOMPOSITIONTIMINGINFO)(
    __in HWND hwnd,
    __out DWM_TIMING_INFO* pTimingInfo
    );

typedef HRESULT (WINAPI * PFNDWMSETPRESENTPARAMETERS)(
    __in HWND hwnd,
    __inout DWM_PRESENT_PARAMETERS* pPresentParams
    );


//-------------------------------------------------------------------
//
// Timer class
//
// Used to control the frame rate of the output video.
//
//-------------------------------------------------------------------

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
            DBGMSG(L"CreateWaitableTimer failed with error %d.\n", GetLastError());
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
            DBGMSG(L"SetWaitableTimer failed with error %d.\n", GetLastError());
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


//-------------------------------------------------------------------
// DwmHelper Class
//
// Manages DWM queuing.
//-------------------------------------------------------------------

struct DwmHelper
{
    HMODULE hDwmApiDLL;
    
    PFNDWMISCOMPOSITIONENABLED      pfnDwmIsCompositionEnabled;
    PFNDWMGETCOMPOSITIONTIMINGINFO  pfnDwmGetCompositionTimingInfo;
    PFNDWMSETPRESENTPARAMETERS      pfnDwmSetPresentParameters;

    BOOL                            m_bDwmQueuing;

    DwmHelper() :
        hDwmApiDLL(NULL),
        pfnDwmIsCompositionEnabled(NULL),
        pfnDwmGetCompositionTimingInfo(NULL),
        pfnDwmSetPresentParameters(NULL),
        m_bDwmQueuing(FALSE)
    {
    }


    BOOL Initialize()
    {
        hDwmApiDLL = LoadLibrary(L"dwmapi.dll");

        if (!hDwmApiDLL)
        {
            DBGMSG(L"LoadLibrary(dwmapi.dll) failed with error %d.\n", GetLastError());
            return FALSE;
        }

        pfnDwmIsCompositionEnabled = (PFNDWMISCOMPOSITIONENABLED)GetProcAddress(hDwmApiDLL, "DwmIsCompositionEnabled");

        if (!pfnDwmIsCompositionEnabled)
        {
            DBGMSG(L"GetProcAddress(DwmIsCompositionEnabled) failed with error %d.\n", GetLastError());
            return FALSE;
        }

        pfnDwmGetCompositionTimingInfo = (PFNDWMGETCOMPOSITIONTIMINGINFO)GetProcAddress(hDwmApiDLL, "DwmGetCompositionTimingInfo");

        if (!pfnDwmGetCompositionTimingInfo)
        {
            DBGMSG(L"GetProcAddress(DwmGetCompositionTimingInfo) failed with error %d.\n", GetLastError());
            return FALSE;
        }

        pfnDwmSetPresentParameters = (PFNDWMSETPRESENTPARAMETERS)GetProcAddress(hDwmApiDLL, "DwmSetPresentParameters");

        if (!pfnDwmSetPresentParameters)
        {
            DBGMSG(L"GetProcAddress(DwmSetPresentParameters) failed with error %d.\n", GetLastError());
            return FALSE;
        }

        return TRUE;
    }

    BOOL EnableDwmQueuing(HWND hwnd)
    {
        HRESULT hr;

        if (!hDwmApiDLL)
        {
            // DWM is not available.
            return TRUE;
        }

        // Check to see if DWM is currently enabled.

        BOOL bDWM = FALSE;

        hr = pfnDwmIsCompositionEnabled(&bDWM);

        if (FAILED(hr))
        {
            DBGMSG(L"DwmIsCompositionEnabled failed with error 0x%x.\n", hr);
            return FALSE;
        }

        if (!bDWM)
        {
            // If DWM is disabled, DWM queuing is also disabled.
            m_bDwmQueuing = FALSE;
            return TRUE;
        }

        if (m_bDwmQueuing)
        {
            // DWM queuing is enabled already.
            return TRUE;
        }

        // Get the DWM refresh count of the last v-sync.

        DWM_TIMING_INFO dwmti = {0};

        dwmti.cbSize = sizeof(dwmti);

        hr = pfnDwmGetCompositionTimingInfo(NULL, &dwmti);

        if (FAILED(hr))
        {
            DBGMSG(L"DwmGetCompositionTimingInfo failed with error 0x%x.\n", hr);
            return FALSE;
        }

        // Enable DWM queuing, starting from the next refresh.

        DWM_PRESENT_PARAMETERS dwmpp = {0};

        dwmpp.cbSize             = sizeof(dwmpp);
        dwmpp.fQueue             = TRUE;
        dwmpp.cRefreshStart      = dwmti.cRefresh + 1;
        dwmpp.cBuffer            = DWM_BUFFER_COUNT;
        dwmpp.fUseSourceRate     = FALSE;
        dwmpp.cRefreshesPerFrame = 1;
        dwmpp.eSampling          = DWM_SOURCE_FRAME_SAMPLING_POINT;

        hr = pfnDwmSetPresentParameters(hwnd, &dwmpp);

        if (FAILED(hr))
        {
            DBGMSG(L"DwmSetPresentParameters failed with error 0x%x.\n", hr);
            return FALSE;
        }

        // DWM queuing is enabled.
        m_bDwmQueuing = TRUE;

        return TRUE;
    }

};





