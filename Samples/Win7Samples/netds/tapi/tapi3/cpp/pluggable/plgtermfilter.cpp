/*++

Copyright (c) 2000 Microsoft Corporation

Module Name:

    PlgTstFilter.cpp

Abstract:

    Implementation of the Plg Filter class (CPlgFilter)

--*/

#include "stdafx.h"
#include <initguid.h>

#include "PlgTermPin.h" //includes PlgTermFilter.h

#include "GUIDs.h"       


/*++
Constructor CPlgFilter
--*/
CPlgFilter::CPlgFilter(  )
    : CBaseFilter(_T("PlgFilter"), NULL, (CCritSec *) this, CLSID_PlgSampleFilter),
    m_pInputPin(NULL),
    m_lpWaveFormatEx(NULL),
    m_pPlgEventSink(NULL)
{
    LOG((MSP_TRACE, "CPlgFilter::CPlgFilter - enter"));
}

/*++
Destructor CPlgFilter
--*/
CPlgFilter::~CPlgFilter()
{
    LOG((MSP_TRACE, "CPlgFilter::~CPlgFilter - enter"));

    //
    // Delete the input pin
    //
    if(m_pInputPin)
    {
        delete m_pInputPin;
    }

    m_pPlgEventSink = NULL;

    //
    // Delete the WAVEFORMATEX
    //
    if(m_lpWaveFormatEx)
    {
        CoTaskMemFree(m_lpWaveFormatEx);
    }

    LOG((MSP_TRACE, "CPlgFilter::~CPlgFilter - exit"));

}

/*++
GetPinCount
--*/

int CPlgFilter::GetPinCount()
{
    LOG((MSP_TRACE, "CPlgFilter::GetPinCount - enter"));

    LOG((MSP_TRACE, "CPlgFilter::GetPinCount - exit"));
    return 1;
}

/*++
GetPin

Description:
    Return the pin of the filter
--*/
CBasePin* CPlgFilter::GetPin(int nPinIndex)
{
    LOG((MSP_TRACE, "CPlgFilter::GetPin - enter"));

    //
    //check the index
    //

    HRESULT hr = (0 == nPinIndex)?S_OK:E_FAIL;

    LOG((MSP_TRACE, "CPlgFilter::GetPin - exit 0x%08x", hr));
	
    return (hr == S_OK)? m_pInputPin : NULL;

}

/*++
CreatePin

Description;
    Create the output pin CPlgFilter
--*/
HRESULT CPlgFilter::CreatePin( )
{
    LOG((MSP_TRACE, "CPlgFilter::CreatePin - enter"));
    USES_CONVERSION;

    CAutoLock lock(this);

    HRESULT hr = S_OK;


    //
    // Already created?
    //

    if (m_pInputPin)
    {
        LOG((MSP_TRACE, "CPlgFilter::CreatePin - exit, the pin already created"));
        return S_OK;
    }

    //
    // Create the pin named using CLSID
    //

    LPOLESTR lpolestrPinName = NULL;
    hr = StringFromCLSID(CLSID_PlgSamplePin, &lpolestrPinName);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "StringFromCLSID 0x%08x", hr));
        return hr;
    }

    //
    //create pin 
    //
    m_pInputPin = new CPlgPin( this, &hr, OLE2W(lpolestrPinName));

    //
    //clean up
    //
    ::CoTaskMemFree(lpolestrPinName);
    
    //
    // Cannot allocate?
    //

    if (m_pInputPin == NULL)
    {
        LOG((MSP_TRACE, "CPlgFilter::CreatePin - cannot create CPlgPin; returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    //
    // Other things went wrong?
    //

    if ( FAILED(hr) )
    {
        delete m_pInputPin;
        m_pInputPin = NULL;
    }

    LOG((MSP_TRACE, "CPlgFilter::CreatePin - exit 0x%08x", hr));
    return hr;
}

/*++
Run

Description;
    Activate the pin, letting it know that we are moving to State_Running
--*/
HRESULT CPlgFilter::Run(
    IN  REFERENCE_TIME      tStart
    )
{
    LOG((MSP_TRACE, "CPlgFilter::Run - enter"));

    CAutoLock lock(this);
    HRESULT hr = NOERROR;

    //
    // This will call Pause if currently stopped
    //

    hr = CBaseFilter::Run(tStart);

    LOG((MSP_TRACE, "CPlgFilter::Run - exit 0x%08x", hr));

    return hr;
}


/*++
Pause

Description;
    Activate the pin, letting it know that Paused will
    be the next state
--*/
HRESULT CPlgFilter::Pause(void)
{
    LOG((MSP_TRACE, "CPlgFilter::Pause - enter"));

    CAutoLock lock(this);

    //
    // Check we can PAUSE given our current state
    //

    if (m_State == State_Running)
    {
    }

    //    
    // tell the pin to go active or inactive and change state
    //
    HRESULT hr = CBaseFilter::Pause();

    LOG((MSP_TRACE, "CPlgFilter::Pause - exit 0x%08x\n", hr));
    return hr;
}


/*++
Stop

Description;
    Pass the current state to the pins Inactive method
--*/
HRESULT CPlgFilter::Stop(void)
{
    LOG((MSP_TRACE, "CPlgFilter::Stop - enter"));

    CAutoLock lock(this);

    HRESULT hr = E_FAIL;

    if (m_State != State_Stopped)
    {

        // Pause the device if we were running
        if (m_State == State_Running)
        {
            hr = Pause(); 
            if (FAILED(hr))
            {
                LOG((MSP_TRACE, "CPlgFilter::Stop - exit 0x%08x\n", hr));
                return hr;
            }
        }

        // Base class changes state and tells pin to go to inactive
        // the pin Inactive method will decommit our allocator which
        // we need to do before closing the device

        hr = CBaseFilter::Stop();
    }

    LOG((MSP_TRACE, "CPlgFilter::Stop - exit 0x%08x\n", hr));
    return hr;
}


HRESULT CPlgFilter::Initialize()
{

    LOG((MSP_TRACE, "CPlgFilter::Initialize - enter"));

    HRESULT hr = CreatePin();

    LOG((MSP_TRACE, "CPlgFilter::Initialize - exit 0x%08x\n", hr));

    return hr;

}


HRESULT CPlgFilter::InitializePrivate(ITPlgPrivEventSink* pSink )
{
	m_pPlgEventSink = pSink;
	return S_OK;
}
