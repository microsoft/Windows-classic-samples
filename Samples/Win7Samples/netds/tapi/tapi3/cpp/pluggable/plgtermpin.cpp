/*++

Copyright (c) 1999-2001 Microsoft Corporation

Module Name:

    PlgTstPin.cpp

Abstract:

    Implementation of the Input pin of the pluggable filter



--*/

#include "stdafx.h"
#include "PlgTermPin.h"
#include <initguid.h>
#include "GUIDs.h"       

// --- Construction/Destruction ---
CPlgPin::CPlgPin(
        CPlgFilter*  pFilter,
        HRESULT*    phr,
        LPCWSTR     pPinName)
        : CBaseInputPin(_T("CPlgPin"), pFilter, pFilter, phr, pPinName)
{
    LOG((MSP_TRACE, "CPlgPin::CPlgPin"));
}

CPlgPin::~CPlgPin()
{
    LOG((MSP_TRACE, "CPlgPin::~CPlgPin"));
}


// -- CBasePin ---

/*++
CheckMediaType

Description:
    Abstract method from CBasePin
    Check if the pin can support this specific proposed type&format
--*/
HRESULT CPlgPin::CheckMediaType(const CMediaType * pmt)
{
    LOG((MSP_TRACE, "CPlgPin::CheckMediaType - enter"));

    //
    // Validates parameters
    //

    if (!pmt)
    {
        LOG((MSP_ERROR, "CPlgPin::CheckMediaType - "
            "invalid pointer, return E_POINTER"));
        return E_POINTER;
    }


    if ( !pmt->IsValid() )
    {

        LOG((MSP_ERROR,
            "CPlgPin::CheckMediaType - media type invalid. "
            "E_INVALIDARG"));
        
        return VFW_E_INVALIDMEDIATYPE;
    }

    CAutoLock lock((CPlgFilter*)m_pFilter);

    //
    // reject non-Audio type
    //

    if (pmt->majortype  != MEDIATYPE_Audio ||
        pmt->formattype != FORMAT_WaveFormatEx)
    {
        LOG((MSP_ERROR, "CPlgPin::CheckMediaType - "
            "non-Audio type, return VFW_E_INVALIDMEDIATYPE"));
        return VFW_E_INVALIDMEDIATYPE;
    }

    LOG((MSP_TRACE, "CPlgPin::CheckMediaType - succeeded"));
    return S_OK;
}


/*++
SetMediaType

Description:
    Method from CBasePin not necessary to be rewritten
    CBasePin, start using this mediatype
--*/
HRESULT CPlgPin::SetMediaType(
     IN     const CMediaType*       pmt
     )
{
    LOG((MSP_TRACE, "CPlgPin::SetMediaType - enter"));

    LOG((MSP_TRACE,"SetMediaType %d bit %d channel %dHz",
        ((LPWAVEFORMATEX)(pmt->pbFormat))->wBitsPerSample,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nChannels,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nSamplesPerSec));

    CAutoLock lock((CPlgFilter*)m_pFilter);

    // Pass the call up to my base class

    HRESULT hr = CBaseInputPin::SetMediaType(pmt);
    if (SUCCEEDED(hr))
    {

        WAVEFORMATEX *pwf = (WAVEFORMATEX *) pmt->Format();

   }
    // We assume this format has been checked out already and is OK
    LOG((MSP_TRACE, "CPlgPin::SetMediaType - exit 0x%08x", hr));
    return hr;
}

/*++
GetMediaType

Description:
    Method from CBasePin should be rewrote
    Enumerate supported input types
--*/
HRESULT CPlgPin::GetMediaType(
     OUT    CMediaType*             pmt
     )
{
    LOG((MSP_TRACE, "CPlgPin::GetMediaType - enter"));
    //
    // Validates parameters
    //

    if (!pmt)
    {
        LOG((MSP_ERROR, "CPlgPin::GetMediaType - "
            "invalid pointer, return E_POINTER"));
        return E_POINTER;
    }

    if(NULL == m_pFilter)
    {
        return E_UNEXPECTED;
    }

    CAutoLock lock((CPlgFilter*)m_pFilter);

    HRESULT hr = InitMediaType();
    if (FAILED (hr))
    {
        LOG((MSP_ERROR, "CPlgPin::GetMediaType - "
            "InitMediaType failed, return 0x%08x", hr));
        return hr;
    }

    CPlgFilter* pFilter = (CPlgFilter*)m_pFilter;

    //
    //create media type to return
    //
    hr = CreateAudioMediaType(pFilter->m_lpWaveFormatEx, pmt, TRUE);

    LOG((MSP_TRACE, "CPlgPin::GetMediaType - exit 0x%08x", hr));
    return hr;
}

/*++
Active

Description;
    Implements the remaining IMemInputPin virtual methods
--*/
HRESULT CPlgPin::Active(void)
{
    LOG((MSP_TRACE, "CPlgPin::Active - enter"));

    if(NULL == m_pFilter)
    {
        return E_UNEXPECTED;
    }


    CAutoLock lock((CPlgFilter*)m_pFilter);

    HRESULT hr = CBasePin::Active();

	if(((CPlgFilter*)m_pFilter)->m_pPlgEventSink != NULL)
	{
		((CPlgFilter*)m_pFilter)->m_pPlgEventSink->FireEvent(1);
	}

    LOG((MSP_TRACE, "CPlgPin::Active - exit 0x%08x", hr));
    return hr;
}

/*++
Inactive

Description:
    Called when the filter is stopped
--*/
HRESULT CPlgPin::Inactive(void)
{
    LOG((MSP_TRACE, "CPlgPin::Inactive - enter"));

    if(NULL == m_pFilter)
    {
        return E_UNEXPECTED;
    }


    CAutoLock lock((CPlgFilter*)m_pFilter);

    HRESULT hr = CBasePin::Inactive();

    if(((CPlgFilter*)m_pFilter)->m_pPlgEventSink != NULL)
    {
        ((CPlgFilter*)m_pFilter)->m_pPlgEventSink->FireEvent(0);
    }
    
    LOG((MSP_TRACE, "CPlgPin::Inactive - exit 0x%08x", hr));
    return hr;
}

/*++
Receive

Description:
    Here's the next block of data from the stream
--*/
HRESULT CPlgPin::Receive(
    IN  IMediaSample*       pSample
    )
{
    LOG((MSP_TRACE, "CPlgPin::Receive - enter"));
    //
    //validate parameters
    //
    if (!pSample)
    {
        LOG((MSP_ERROR, "CPlgPin::Receive - exit E_POINTER"));
        return E_POINTER;
    }
    LOG((MSP_TRACE, "CPlgPin::Receive - sample size 0x%08x", pSample->GetSize()));

    if(NULL == m_pFilter)
    {
        return E_UNEXPECTED;
    }

    CAutoLock lock((CPlgFilter*)m_pFilter);

    // If we're stopped, then reject this call
    // (the filter graph may be in mid-change)
    if (((CPlgFilter*)m_pFilter)->m_State == State_Stopped)
    {
        LOG((MSP_ERROR, "CPlgPin::Receive - "
            "the filter is stoped, return E_FAIL"));
        return E_FAIL;
    }

    // Check all is well with the base class
    HRESULT hr = CBaseInputPin::Receive(pSample);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CPlgPin::Receive - "
            "base Received failed, return 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CPlgPin::Receive - ended 0x%08x", hr));
    return hr;

}



// --- Helper methods ---

/*++
InitMediaType

Description:
    Initialize WAVEFORMATEX member of CTGFilter
    Is called by GetMediaType
--*/
HRESULT CPlgPin::InitMediaType(void)
{
    LOG((MSP_TRACE, "CPlgPin::InitMediaType - enter"));
  
    CPlgFilter* pFilter = (CPlgFilter*)m_pFilter;

    if(NULL == m_pFilter)
    {
        return E_UNEXPECTED;
    }

    //
    // Is already initialized ?
    //
    if(pFilter->m_lpWaveFormatEx != NULL)
    {
        LOG((MSP_TRACE, "CTGPin::InitMediaType - succeeded,"
            "WAVEFORMATEX already initialize"));    
        return S_OK;
    }

    //
    // Create a new WAVEFORMATEX
    //

    pFilter->m_lpWaveFormatEx = (LPWAVEFORMATEX) CoTaskMemAlloc(sizeof(WAVEFORMATEX));
    if (!pFilter->m_lpWaveFormatEx)
    {
        LOG((MSP_ERROR, "CTGPin::InitMediaType - "
            "cannot allocate WAVEFORMATEX, return E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }
                
    // reset the memory
    ZeroMemory(pFilter->m_lpWaveFormatEx, sizeof (WAVEFORMATEX));
    
    // sets the fields
    pFilter->m_lpWaveFormatEx->wFormatTag      = WAVE_FORMAT_PCM;
    pFilter->m_lpWaveFormatEx->wBitsPerSample  = 16;    
    pFilter->m_lpWaveFormatEx->nChannels       = 1;
    pFilter->m_lpWaveFormatEx->nSamplesPerSec  = 8000;
    pFilter->m_lpWaveFormatEx->nBlockAlign     = 1 * ((16 + 7)/8);
    pFilter->m_lpWaveFormatEx->nAvgBytesPerSec = 8000 * 
                                               pFilter->m_lpWaveFormatEx->nBlockAlign;
    pFilter->m_lpWaveFormatEx->cbSize          = 0;
    
    LOG((MSP_TRACE, "CPlgPin::InitMediaType exit - S_OK"));    
    return S_OK;
}

