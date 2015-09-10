/******************************Module*Header*******************************\
* Module Name: display.cpp
*
* Support for DDraw device on Multiple Monitors.
*
*
* Created: Mon 01/24/2000
* Author:  Stephen Estrop [StEstrop]
* Mod:     KirtD - First integration into MF renderer model
*
* Copyright (c) Microsoft Corporation
\**************************************************************************/
#include <windows.h>
#include <unknwn.h>
#include "display.h"
#include <strsafe.h>


#ifndef DEFAULT_DENSITY_LIMIT
#define DEFAULT_DENSITY_LIMIT       60
#endif
#ifndef WIDTH
#define WIDTH(x) ((x)->right - (x)->left)
#endif
#ifndef HEIGHT
#define HEIGHT(x) ((x)->bottom - (x)->top)
#endif


/* -------------------------------------------------------------------------
** Structure use to pass info to the DDrawEnumEx callback
** -------------------------------------------------------------------------
*/

struct DDRAWINFO
{
    DWORD               dwCount;
    DWORD               dwPmiSize;
    HRESULT             hrCallback;
    const GUID*         pGUID;
    CAMDDrawMonitorInfo* pmi;
    HWND                hwnd;
};

/******************************Public*Routine******************************\
* TermDDrawMonitorInfo
*
*
*
* History:
* 01-17-2000 - StEstrop - Created
*
\**************************************************************************/
void CMonitorArray::TermDDrawMonitorInfo(
    _Inout_ CAMDDrawMonitorInfo* pmi
    )
{


    ZeroMemory(pmi, sizeof(CAMDDrawMonitorInfo));
}

/*****************************Private*Routine******************************\
* GetAMDDrawMonitorInfo
*
*
*
* History:
* Tue 08/17/1999 - StEstrop - Created
*
\**************************************************************************/
BOOL CMonitorArray::GetAMDDrawMonitorInfo(
    UINT uDevID,
    _Out_ CAMDDrawMonitorInfo* lpmi,
    _In_ HMONITOR hm
    )
{
    MONITORINFOEX miInfoEx;
    miInfoEx.cbSize = sizeof(miInfoEx);

    lpmi->hMon = NULL;
    lpmi->uDevID = 0;
    lpmi->physMonDim.cx = 0;
    lpmi->physMonDim.cy = 0;
    lpmi->dwRefreshRate = DEFAULT_DENSITY_LIMIT;

    if (GetMonitorInfo(hm, &miInfoEx))
    {
        HRESULT hr = StringCchCopy(lpmi->szDevice, sizeof(lpmi->szDevice)/sizeof(lpmi->szDevice[0]), miInfoEx.szDevice);

        if ( FAILED( hr ) )
        {
            return FALSE;
        }

        lpmi->hMon = hm;
        lpmi->uDevID = uDevID;
        lpmi->physMonDim.cx = WIDTH(&miInfoEx.rcMonitor);
        lpmi->physMonDim.cy = HEIGHT(&miInfoEx.rcMonitor);

        int j = 0;
        DISPLAY_DEVICE ddMonitor;

        ddMonitor.cb = sizeof(ddMonitor);
        while (EnumDisplayDevices(lpmi->szDevice, j, &ddMonitor, 0))
        {
            if (ddMonitor.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
            {
                DEVMODE     dm;

                ZeroMemory(&dm, sizeof(dm));
                dm.dmSize = sizeof(dm);
                if (EnumDisplaySettings(lpmi->szDevice, ENUM_CURRENT_SETTINGS, &dm))
                {
                    lpmi->dwRefreshRate = dm.dmDisplayFrequency == 0 ? lpmi->dwRefreshRate : dm.dmDisplayFrequency;
                }

                // Remove registry snooping for monitor dimensions, as this is not supported by LDDM.
                // if (!FindMonitorDimensions(ddMonitor.DeviceID, &lpmi->physMonDim.cx, &lpmi->physMonDim.cy))
                {
                    lpmi->physMonDim.cx = WIDTH(&miInfoEx.rcMonitor);
                    lpmi->physMonDim.cy = HEIGHT(&miInfoEx.rcMonitor);
                }
            }
            j++;
        }

        return TRUE;
    }

    return FALSE;
}


BOOL CMonitorArray::InitMonitor(
    _In_ HMONITOR hMon,
    BOOL fXclMode
    )
{


    if (GetAMDDrawMonitorInfo(m_dwNumMonitors, &m_DDMon[m_dwNumMonitors], hMon))
    {
        m_DDMon[m_dwNumMonitors].pDD = (IUnknown*)1; // make checks for pDD succeed.
        m_dwNumMonitors++;
    }

    if (EVR_MAX_MONITORS >= m_dwNumMonitors)
    {
        // don't exceed array bounds
        return TRUE;
    }

    return FALSE;
}


BOOL CALLBACK CMonitorArray::MonitorEnumProc(
    _In_ HMONITOR hMon,
    _In_opt_ HDC hDC,
    _In_ LPRECT pRect,
    LPARAM dwData
    )
{
    MonitorEnumProcInfo* info = (MonitorEnumProcInfo*)dwData;

    if (!info)
    {
        return TRUE;
    }

    return info->pMonArray->InitMonitor(hMon, FALSE);
}

/*****************************Private*Routine******************************\
* InitializeDisplaySystem
*
*
*
* History:
* Mon 01/24/2000 - StEstrop - Created
*
\**************************************************************************/
HRESULT CMonitorArray::InitializeDisplaySystem(
    _In_ HWND hwnd
    )
{
    HRESULT hr = S_OK;

    MonitorEnumProcInfo info;

    info.hwnd = hwnd;
    info.pMonArray = this;

    EnumDisplayMonitors(NULL, NULL, &MonitorEnumProc, (LPARAM)&info);

    if (m_dwNumMonitors == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return(hr);
    }

    return(hr);
}

/*****************************Private*Routine******************************\
* FindMonitor
*
* find the current monitor
*
* History:
* Fri 04/25/2000 - GlennE - Created
*
\**************************************************************************/
CAMDDrawMonitorInfo* CMonitorArray::FindMonitor(
    _In_ HMONITOR hMon
    )
{


    for (DWORD i = 0; i < m_dwNumMonitors; i++)
    {
        if (hMon == m_DDMon[i].hMon)
        {
            return &m_DDMon[i];
        }
    }

    return NULL;
}

/*****************************Private*Routine******************************\
* MatchGUID
*
*
*
* History:
* Fri 04/25/2000 - GlennE - Created
*
\**************************************************************************/
HRESULT CMonitorArray::MatchGUID(
    UINT uDevID,
    _Out_ DWORD* pdwMatchID
    )
{
    HRESULT hr = S_OK;

    *pdwMatchID = 0;

    for (DWORD i = 0; i < m_dwNumMonitors; i++)
    {

        UINT uMonDevID = m_DDMon[i].uDevID;

        if (uDevID == uMonDevID)
        {

            *pdwMatchID = i;
            hr = S_OK;
            return( hr );
        }
    }

    hr = S_FALSE;
    return( hr );
}


/*****************************Private*Routine******************************\
* TerminateDisplaySystem
*
*
*
* History:
* Mon 01/24/2000 - StEstrop - Created
*
\**************************************************************************/
void CMonitorArray::TerminateDisplaySystem()
{


    for (DWORD i = 0; i < m_dwNumMonitors; i++)
    {
        TermDDrawMonitorInfo(&m_DDMon[i]);
    }
    m_dwNumMonitors = 0;
}


/******************************Public*Routine******************************\
* Constructor and destructor
*
*
*
* History:
* Fri 11/17/2001 - StEstrop - Created
*
\**************************************************************************/
CMonitorArray::CMonitorArray()
    : m_dwNumMonitors(0)
{


    ZeroMemory(m_DDMon, sizeof(m_DDMon));
}


CMonitorArray::~CMonitorArray()
{

    TerminateDisplaySystem();
}

