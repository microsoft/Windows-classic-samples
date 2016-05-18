/*++

Copyright (c) 1999-2001 Microsoft Corporation

Module Name:

    PlgTermPin.h

Abstract:

    Declaration of the Input pin


--*/

#ifndef __PLGPIN__
#define __PLGPIN__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 100

#include "PlgTermFilter.h"
#include <source.h>


class CPlgPin :
    public CBaseInputPin
{
public:
    // -- Constructor / Destructor
	CPlgPin(
        CPlgFilter*  pFilter,
        HRESULT*    phr,
        LPCWSTR     pPinName
        );

	virtual ~CPlgPin();

    // --- IUnknown ---
    DECLARE_IUNKNOWN;

    // --- CBasePin ---
    HRESULT GetMediaType(CMediaType *pmt);
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);

    // --- Worker Thread fn's ---
    HRESULT Active(void);
    HRESULT Inactive(void);

    // Receive data
    STDMETHODIMP Receive(IMediaSample *pSample);


private:
    // --- Helper functions ---

    // Initialize MediaType
    HRESULT InitMediaType();

private:
    WAVEFORMATEX            m_wfxFormat;

};

#endif // __PLGPIN__
