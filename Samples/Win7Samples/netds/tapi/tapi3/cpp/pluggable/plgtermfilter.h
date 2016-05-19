/*++

Copyright (c) 2000 Microsoft Corporation

Module Name:

    PlgTstFilter.h

Abstract:

    Declaration of the Plg Filter class (CPlgFilter)

--*/

#ifndef __PLGFILTER__
#define __PLGFILTER__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PlgTermPriv.h"

#include "stdafx.h"
#include <streams.h>        // DShow streams

class CPlgPin;

class CPlgFilter : 
    public CBaseFilter,
    public CCritSec
{

public:

    // --- IUnknown ---
    DECLARE_IUNKNOWN

    // --- CBaseFilter ---
    virtual int GetPinCount();
    CBasePin * GetPin(int nIndex);

    // --- CBaseStreamControl ---
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP Pause(void);
    STDMETHODIMP Stop(void);

    STDMETHOD (Initialize)( );

public:

    // --- Constructor / Desctructor ---
    CPlgFilter();
    ~CPlgFilter();

    HRESULT InitializePrivate(ITPlgPrivEventSink* pSink );

private:
    // --- Helper functions ---

    // Create the input pin
    HRESULT CreatePin( );

    // --- Members ---

    // The input pin
    CPlgPin  *m_pInputPin;

    // The waveformat supported
    LPWAVEFORMATEX m_lpWaveFormatEx;

	ITPlgPrivEventSink*		m_pPlgEventSink;

    friend class CPlgPin;
};

#endif // __PLGFILTER__
