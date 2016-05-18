//------------------------------------------------------------------------------
// File: VidProp.h
//
// Desc: DirectShow sample code - definition of CQualityProperties class.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef __VIDPROP__
#define __VIDPROP__

#define IDD_QUALITY             150     // Dialog resource
#define IDD_Q1                  151     // Frames played
#define IDD_Q2                  152     // Frames dropped
#define IDD_Q4                  154     // Frame rate
#define IDD_Q5                  155     // Frame jitter
#define IDD_Q6                  156     // Sync offset
#define IDD_Q7                  157     // Sync deviation
#define FIRST_Q_BUTTON          171     // First button
#define LAST_Q_BUTTON           177     // Last button
#define IDD_QDRAWN              171     // Frames played
#define IDD_QDROPPED            172     // Frames dropped
#define IDD_QAVGFRM             174     // Average frame rate achieved
#define IDD_QJITTER             175     // Average frame jitter
#define IDD_QSYNCAVG            176     // Average sync offset
#define IDD_QSYNCDEV            177     // Std dev sync offset
#define IDS_NAME                178     // Quality dialog name

// Property page built on top of a renderer IQualProp interface

class CQualityProperties : public CBasePropertyPage
{
    IQualProp *m_pQualProp;         // Interface held on the renderer
    int m_iDropped;                 // Number of frames dropped
    int m_iDrawn;                   // Count of images drawn
    int m_iSyncAvg;                 // Average sync value
    int m_iSyncDev;                 // And standard deviation
    int m_iFrameRate;               // Total frame rate average
    int m_iFrameJitter;             // Measure of frame jitter

    void SetEditFieldData();
    void DisplayStatistics(void);

public:

    CQualityProperties(LPUNKNOWN lpUnk, HRESULT *phr);
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

    // Overriden from CBasePropertyPage base class

    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();

}; // CQualityProperties

#endif // __VIDPROP__

