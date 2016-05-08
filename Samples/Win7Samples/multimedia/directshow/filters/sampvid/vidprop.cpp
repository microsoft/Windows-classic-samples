//------------------------------------------------------------------------------
// File: VidProp.cpp
//
// Desc: DirectShow sample code - implementation of CQualityProperties class.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


// This class implements a property page dialog for the video renderer. We
// expose certain statistics from the quality management implementation.
// In particular, we have two edit fields that show the number of frames we have
// actually drawn and the number of frames that we dropped. The number of
// frames we dropped does NOT represent the total number dropped in any play
// back sequence (as expressed through MCI status frames skipped) since the
// quality management protocol may have negotiated with the source filter for
// it to send fewer frames in the first place. Dropping frames in the source
// filter is nearly always a more efficient mechanism when we are flooded.

#include "sampvid.h"
#include <strsafe.h>


//
// Constructor
//
CQualityProperties::CQualityProperties(LPUNKNOWN pUnk,HRESULT *phr) :
    CBasePropertyPage(NAME("Quality Page"),pUnk,IDD_QUALITY,IDS_NAME),
    m_pQualProp(NULL)
{
    ASSERT(phr);

} // (Constructor)


//
// CreateInstance
//
// Create a quality properties object
//
CUnknown * WINAPI CQualityProperties::CreateInstance(LPUNKNOWN lpUnk, HRESULT *phr)
{
    ASSERT(phr);
    return new CQualityProperties(lpUnk, phr);

} // CreateInstance


//
// OnConnect
//
// Give us the filter to communicate with
//
HRESULT CQualityProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pQualProp == NULL);
    CheckPointer(pUnknown,E_POINTER);

    // Ask the renderer for it's IQualProp interface

    HRESULT hr = pUnknown->QueryInterface(IID_IQualProp,(void **)&m_pQualProp);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pQualProp);

    // Get quality data for our page

    m_pQualProp->get_FramesDroppedInRenderer(&m_iDropped);
    m_pQualProp->get_FramesDrawn(&m_iDrawn);
    m_pQualProp->get_AvgFrameRate(&m_iFrameRate);
    m_pQualProp->get_Jitter(&m_iFrameJitter);
    m_pQualProp->get_AvgSyncOffset(&m_iSyncAvg);
    m_pQualProp->get_DevSyncOffset(&m_iSyncDev);

    return NOERROR;

}  // OnConnect


//
// OnDisconnect
//
// Release any IQualProp interface we have
//
HRESULT CQualityProperties::OnDisconnect()
{
    // Release the interface

    if (m_pQualProp == NULL) {
        return E_UNEXPECTED;
    }

    m_pQualProp->Release();
    m_pQualProp = NULL;
    return NOERROR;

}  // OnDisconnect


//
// OnActivate
//
// Set the text fields in the property page
//
HRESULT CQualityProperties::OnActivate()
{
    SetEditFieldData();
    return NOERROR;

} // OnActivate


//
// SetEditFieldData
//
// Initialise the property page fields
//
void CQualityProperties::SetEditFieldData()
{
    ASSERT(m_pQualProp);
    TCHAR buffer[50];

    (void)StringCchPrintf(buffer,NUMELMS(buffer), TEXT("%d\0"), m_iDropped);
    SendDlgItemMessage(m_Dlg, IDD_QDROPPED, WM_SETTEXT, 0, (LPARAM) buffer);

    (void)StringCchPrintf(buffer,NUMELMS(buffer), TEXT("%d\0"), m_iDrawn);
    SendDlgItemMessage(m_Dlg, IDD_QDRAWN,   WM_SETTEXT, 0, (LPARAM) buffer);

    (void)StringCchPrintf(buffer,NUMELMS(buffer), TEXT("%d.%02d\0"), m_iFrameRate/100, m_iFrameRate%100);
    SendDlgItemMessage(m_Dlg, IDD_QAVGFRM,  WM_SETTEXT, 0, (LPARAM) buffer);

    (void)StringCchPrintf(buffer,NUMELMS(buffer), TEXT("%d\0"), m_iFrameJitter);
    SendDlgItemMessage(m_Dlg, IDD_QJITTER,  WM_SETTEXT, 0, (LPARAM) buffer);

    (void)StringCchPrintf(buffer,NUMELMS(buffer), TEXT("%d\0"), m_iSyncAvg);
    SendDlgItemMessage(m_Dlg, IDD_QSYNCAVG, WM_SETTEXT, 0, (LPARAM) buffer);

    (void)StringCchPrintf(buffer,NUMELMS(buffer), TEXT("%d\0"), m_iSyncDev);
    SendDlgItemMessage(m_Dlg, IDD_QSYNCDEV, WM_SETTEXT, 0, (LPARAM) buffer);

} // SetEditFieldData



