/*++

Copyright (c) 1998-1999 Microsoft Corporation

Module Name:

    msptrmac.cpp

Abstract:

    MSP base classes: implementation of audio capture terminal.

--*/

#include "precomp.h"
#pragma hdrstop

#define MAX_LONG 0xefffffff

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CAudioCaptureTerminal::CAudioCaptureTerminal()
{
    m_TerminalDirection = TD_CAPTURE;
    m_TerminalType = TT_STATIC;

    m_bResourceReserved = false;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::CAudioCaptureTerminal() finished"));
}

CAudioCaptureTerminal::~CAudioCaptureTerminal()
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::~CAudioCaptureTerminal() finished"));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


HRESULT CAudioCaptureTerminal::CreateTerminal(
    IN  CComPtr<IMoniker>   pMoniker,
    IN  MSP_HANDLE          htAddress,
    OUT ITTerminal        **ppTerm
    )
{
    // Enable ATL string conversion macros.
    USES_CONVERSION;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::CreateTerminal : enter"));

    //
    // Validate the parameters
    //

    if ( !ppTerm)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal : "
            "bad terminal pointer; returning E_POINTER"));
        return E_POINTER;
    }

    if ( !pMoniker)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal : "
            "bad moniker pointer; returning E_POINTER"));
        return E_POINTER;
    }

    //
    // We return a NULL terminal if there is an error.
    //

    *ppTerm = NULL;
    HRESULT hr;

    //
    // Bind the moniker to storage as a property bag.
    //

    CComPtr<IPropertyBag> pBag;
    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal (IMoniker::BindToStorage) - returning  %8x", hr));
        return hr;
    }

    VARIANT var;

    //
    // Get the wave ID from the property bag.
    // Skip this terminal if it doesn't have a wave ID.
    // (Needed because WDM devices don't work, and we don't want more than one
    // terminal per device.)
    //

    var.vt = VT_I4;
    hr = pBag->Read(L"WaveInId", &var, 0);

    if (FAILED(hr)) 
    {
        LOG((MSP_INFO, "CAudioCaptureTerminal::CreateTerminal - "
            "IPropertyBag::Read failed on WaveID - "
            "skipping terminal (not cause for alarm) - "
            "returning  0x%08x", hr));

        return hr;
    }

    //
    // Get the name for this filter out of the property bag.
    // Skip this terminal if it doesn't have a name.
    //

    var.vt = VT_BSTR;
    hr = pBag->Read(L"FriendlyName", &var, 0);

    if (FAILED(hr)) 
    {
        LOG((MSP_INFO, "CAudioCaptureTerminal::CreateTerminal - "
            "IPropertyBag::Read failed on FriendlyName - "
            "skipping terminal (not cause for alarm) - "
            "returning  0x%08x", hr));

        return hr;
    }

    //
    // Create the filter.
    //

    CMSPComObject<CAudioCaptureTerminal> *pLclTerm = new CMSPComObject<CAudioCaptureTerminal>;

    if (pLclTerm == NULL) 
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal - returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    //
    // Save some stuff in the terminal.
    //

    pLclTerm->m_pMoniker = pMoniker;
    
    wcsncpy_s(pLclTerm->m_szName,MAX_PATH, OLE2T(var.bstrVal), MAX_PATH);

    SysFreeString(var.bstrVal);

    //
    // Get the ITTerminal interface that we were asked for.
    //
    hr = pLclTerm->_InternalQueryInterface(IID_ITTerminal, (void**)ppTerm);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal - "
            "Internal QI failed; returning 0x%08x", hr));

        delete pLclTerm;
        *ppTerm = NULL; // just in case        
        
        return hr;
    }

    //
    // Finish initializing the terminal.
    //

    hr = pLclTerm->Initialize(CLSID_MicrophoneTerminal,
                              TAPIMEDIATYPE_AUDIO,
                              TD_CAPTURE,
                              htAddress
                             );
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateTerminal - "
            "Initialize failed; returning 0x%08x", hr));

        (*ppTerm)->Release();
        *ppTerm = NULL;

        return hr;
    }


    LOG((MSP_TRACE, "CAudioCaptureTerminal::CreateTerminal - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Create the filters used by this terminal

HRESULT CAudioCaptureTerminal::CreateFilters(void)
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::CreateFilters() called"));

    //
    // This should only be called atmost once in the lifetime of this instance
    //

    if ( (m_pIFilter            != NULL) ||
         (m_pIPin               != NULL) ||
         (m_pIAMAudioInputMixer != NULL) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateFilters() : we've already been called; returning E_FAIL"));  
        return E_FAIL;
    }

    //
    // Create the filter.
    //

    HRESULT hr = m_pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pIFilter);

    if ( FAILED(hr) || (m_pIFilter == NULL) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateFilters() : BindToObject failed 0x%08x", hr));  

        m_pIFilter = NULL; // we are being extra careful...
        return hr;
    }

    //
    // Get the basic audio (mixer) interface for the filter.
    //

    hr = m_pIFilter->QueryInterface(IID_IAMAudioInputMixer,
                                       (void **) &m_pIAMAudioInputMixer);

    if ( FAILED(hr) || (m_pIAMAudioInputMixer == NULL) )
    {
        //
        // The filter doesn't support the mixer interface. This is not catastrophic;
        // all it means is that subsequent mixer operations on the terminal will fail.
        //

        LOG((MSP_WARN, "CAudioCaptureTerminal::CreateFilters() : mixer QI failed 0x%08x", hr));  
        m_pIAMAudioInputMixer = NULL;
    }

    //
    // Find the output pin (this is a private method on the terminal).
    //

    hr = FindTerminalPin();

    if ( FAILED(hr) || (m_pIPin == NULL) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CreateFilters() : FindTerminalPin failed 0x%08x", hr));  

        //
        // Clean up our mess.
        //

        if (m_pIAMAudioInputMixer != NULL)
        {
            m_pIAMAudioInputMixer = NULL; // implicit release
        }

        m_pIFilter = NULL; // implicit release
        
        return hr;
    }

    LOG((MSP_TRACE, "CAudioCaptureTerminal::CreateFilters() succeeded"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

HRESULT 
CAudioCaptureTerminal::FindTerminalPin(
    )
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::FindTerminalPin - enter"));
    
    // We must not do CreateFiltersIfRequired here because that would
    // result in a recursive call to us again.

    if (m_pIPin != NULL)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::FindTerminalPin - "
            "we've alread got a pin; returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr;
    CComPtr<IEnumPins> pIEnumPins;
    ULONG cFetched;

    //
    // Find the capture pin for the filter.
    //

    if (FAILED(hr = m_pIFilter->EnumPins(&pIEnumPins)))
    {
        LOG((MSP_ERROR, 
            "CAudioCaptureTerminal::FindTerminalPin - can't enum pins %8x",
            hr));
        return hr;
    }

    IPin * pIPin;

    // Enumerate all the pins and break on the 
    // first pin that meets requirement.
    for (;;)
    {
        if (pIEnumPins->Next(1, &pIPin, &cFetched) != S_OK)
        {
            LOG((MSP_ERROR, 
                "CAudioCaptureTerminal::FindTerminalPin - can't get a pin %8x",
                hr));
            return (hr == S_FALSE) ? E_FAIL : hr;
        }

        if (0 == cFetched)
        {
            LOG((MSP_ERROR, "CAudioCaptureTerminal::FindTerminalPin - got zero pins"));
            return E_FAIL;
        }

        PIN_DIRECTION dir;

        if (FAILED(hr = pIPin->QueryDirection(&dir)))
        {
            LOG((MSP_ERROR, 
                "CAudioCaptureTerminal::FindTerminalPin - can't query pin direction %8x",
                hr));
            pIPin->Release();
            return hr;
        }

        if (PINDIR_OUTPUT == dir)
        {
            break;
        }

        pIPin->Release();
    }

    m_pIPin = pIPin;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::FindTerminalPin - exit S_OK"));
  
    return S_OK;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

HRESULT CAudioCaptureTerminal::AddFiltersToGraph(
    )
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::AddFiltersToGraph - enter"));

    if (m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::AddFiltersToGraph - "
            "we don't have a filter graph; returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr = CreateFiltersIfRequired();
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::AddFiltersToGraph - "
            "CreateFiltersIfRequired failed; returning hr = 0x%08x", hr));
        return hr;
    }

    if (m_pIFilter == NULL)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::AddFiltersToGraph - "
            "we don't have a filter; returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    //
    // Add the filter to the graph.
    //
    // A word about names:
    // If a filter has already been added with the same name (which will
    // happen if we have more than one audio capture terminal in the same
    // graph) then that will return VFW_S_DUPLICATE_NAME, which is not
    // a failure.
    //

    hr = m_pGraph->AddFilter(m_pIFilter, WAVEIN_NAME);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::AddFiltersToGraph - "
            "AddFilter failed; returning hr = 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CAudioCaptureTerminal::AddFiltersToGraph - exit S_OK"));
    return S_OK;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

HRESULT CAudioCaptureTerminal::CompleteConnectTerminal(void)
{
    CLock lock(m_CritSec);
    
    LOG((MSP_TRACE, "CAudioCaptureTerminal::CompleteConnectTerminal - enter"));

    // By default, we need not unreserve later.
    m_bResourceReserved = false;

    HRESULT hr = CSingleFilterTerminal::CompleteConnectTerminal();
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::CompleteConnectTerminal: "
                "CSingleFilterTerminal method failed"));
        return hr;
    }

    // So here we are, after our filter has been added to the filter graph and connected up, but before
    // the MSP has told it to run.

    //////////////////////////////////////////////////////////////////////////
    // connect-time device reservation:
    // we must inform the filter that we want it to grab the wave device.
    // We do this after connecting because the filter needs to negotiate the
    // media type before it can open a wave device.

    CComPtr <IAMResourceControl> pIResource;
 
    hr = m_pIFilter->QueryInterface(IID_IAMResourceControl, (void **) &pIResource);
    if (FAILED(hr)) 
    {
        LOG((MSP_WARN, "CAudioCaptureTerminal::CompleteConnectTerminal - QI failed: %8x", hr)); 
        
        // This is a nonesential operation so we do not "return hr;" here.
    }
    else // QueryInterface didn't fail
    {
        hr = pIResource->Reserve(AMRESCTL_RESERVEFLAGS_RESERVE, NULL);

        if (hr != S_OK)
        {
            LOG((MSP_ERROR, "CAudioCaptureTerminal::CompleteConnectTerminal - "
                    "device reservation failed: %8x", hr));
            return hr;
        }

        // We have succeeded in reserving, so we will want to unreserve later.
        m_bResourceReserved = true;
    }

    LOG((MSP_TRACE, "CAudioCaptureTerminal::CompleteConnectTerminal - exit S_OK"));
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// We override this here so we can unreserve the resource when we are done.
// removes filters from the filter graph and resets member variables
// Disconnect may be called anytime after Connect succeeds (it need not be called
// if CompleteConnect fails)

STDMETHODIMP CAudioCaptureTerminal::DisconnectTerminal(
            IN      IGraphBuilder  * pGraph,
            IN      DWORD            dwReserved
            )
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::DisconnectTerminal - enter"));

    HRESULT hr;

    //
    // First call the base class method, to make sure we validate everything
    // and don't mess with our resource reservation unless this is a valid
    // disconnection (e.g., the filter graph pointersmatch).
    //

    hr = CSingleFilterTerminal::DisconnectTerminal(pGraph, dwReserved);

    if (FAILED(hr))
    {
        LOG((MSP_TRACE, "CAudioCaptureTerminal::DisconnectTerminal : "
                "CSingleFilterTerminal method failed; hr = %d", hr));
        return hr;
    }

    // if we need to unreserve the resource now
    if (m_bResourceReserved)
    {
        CComPtr <IAMResourceControl> pIResource;

        hr = m_pIFilter->QueryInterface(IID_IAMResourceControl,
                                        (void **) &pIResource);
        if (FAILED(hr)) 
        {
            LOG((MSP_WARN, "CAudioCaptureTerminal::DisconnectTerminal - "
                                 "QI failed: %8x", hr)); 
        
            // This is a nonesential operation so we do not "return hr;" here.
        }
        else
        {
            // QueryInterface didn't fail, so UNRESERVE now

            hr = pIResource->Reserve(AMRESCTL_RESERVEFLAGS_UNRESERVE, NULL);
            if (hr != S_OK)
            {
                LOG((MSP_WARN, "CAudioCaptureTerminal::DisconnectTerminal - "
                                     "device unreservation failed: %8x", hr));
                // no reason to completely die at this point, so we just continue
            }

            // if other things fail we may be called again, but we should not try
            // to unreserve again.
            m_bResourceReserved = false;

        } // {if QI succeeded}
    } // {if we need to release}

    LOG((MSP_TRACE, "CAudioCaptureTerminal::DisconnectTerminal - exit S_OK"));

    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//***************************************************************************//
//*                                                                         *//
//* NOTE: The input filter does not support IBasicAudio so we need to masage*//
//*       the parameters for the basic audio methods so that the will work  *//
//*       for IAMAudioInputMixer.                                           *//
//*                                                                         *//    
//*****************************************************************************
///////////////////////////////////////////////////////////////////////////////

static const long   TMGR_MIN_API_VOLUME     = 0;      // our terminal semantics
static const long   TMGR_MAX_API_VOLUME     = 0xFFFF;
static const double TMGR_MIN_CAPTURE_VOLUME = 0.0;    // capture filter semantics
static const double TMGR_MAX_CAPTURE_VOLUME = 1.0;

STDMETHODIMP CAudioCaptureTerminal::get_Volume(long * plVolume)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_Volume - enter"));

    //
    // Check parameters.
    //

    if ( !plVolume)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume - "
            "invalid pointer passed in - returning E_POINTER"));
        return E_POINTER;
    }

    //
    // Create the filters if required. This protects us if the creation fails.
    //

    HRESULT hr = CreateFiltersIfRequired();
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume - "
            "CreateFiltersIfRequired failed; returning hr = 0x%08x", hr));
        return hr;
    }

    //
    // Check if CreateFiltersIfRequired was able to get us the mixer interface.
    // If not, we must fail.
    //

    if (m_pIAMAudioInputMixer == NULL)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume - "
            "filter does not support mixer interface - returning E_FAIL"));
        return E_FAIL;
    }

    //
    // Perform the call on the filter.
    //

    double dVolume;
    hr = m_pIAMAudioInputMixer->get_MixLevel(&dVolume);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume "
            "(get_MixLevel) - returning  %8x", hr));
        return hr;
    }


    //
    // Massage ranges to convert between disparate semantics.
    //

    
    // Our argument is a pointer to a long in the range 0 - 0xFFFF.
    // We need to output that based on a double ranging from 0.0 to 1.0.

    if (dVolume < TMGR_MIN_CAPTURE_VOLUME)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume - observed "
            "volume %d < %d; returning E_INVALIDARG",
            dVolume, TMGR_MIN_CAPTURE_VOLUME));

        return E_INVALIDARG;
    }

    if (dVolume > TMGR_MAX_CAPTURE_VOLUME)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_Volume - observed "
            "volume %d > %d; returning E_INVALIDARG",
            dVolume, TMGR_MAX_CAPTURE_VOLUME));

        return E_INVALIDARG;
    }

    // Convert the volume from whatever range of doubles the filter uses
    // to the range 0 - 1. Right now this does nothing but makes the code more general.
    dVolume = ( dVolume                 - TMGR_MIN_CAPTURE_VOLUME )
            / ( TMGR_MAX_CAPTURE_VOLUME - TMGR_MIN_CAPTURE_VOLUME );

    // Convert the volume from the range 0 - 1 to the API's range.
    *plVolume = TMGR_MIN_API_VOLUME +
        (long) (( TMGR_MAX_API_VOLUME - TMGR_MIN_API_VOLUME ) * dVolume);

    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_Volume - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CAudioCaptureTerminal::put_Volume(long lVolume)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CAudioCaptureTerminal::put_Volume - enter"));
    
    //
    // Create the filters if required. This protects us if the creation fails.
    //

    HRESULT hr = CreateFiltersIfRequired();
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::put_Volume - "
            "CreateFiltersIfRequired failed; returning hr = 0x%08x", hr));
        return hr;
    }

    //
    // Check if CreateFiltersIfRequired was able to get us the mixer interface.
    // If not, we must fail.
    //

    if (m_pIAMAudioInputMixer == NULL)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::put_Volume - "
            "filter does not support mixer interface - returning E_FAIL"));
        return E_FAIL;
    }

    //
    // Massage ranges to convert between disparate semantics.
    //

    // Our argument is a long in the range 0 - 0xFFFF. We need to convert it
    // to a double ranging from 0.0 to 1.0.

    if (lVolume < TMGR_MIN_API_VOLUME)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::put_Volume - requested "
            "volume %d < %d; returning E_INVALIDARG",
            lVolume, TMGR_MIN_API_VOLUME));

        return E_INVALIDARG;
    }

    if (lVolume > TMGR_MAX_API_VOLUME)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::put_Volume - requested "
            "volume %d > %d; returning E_INVALIDARG",
            lVolume, TMGR_MAX_API_VOLUME));

        return E_INVALIDARG;
    }

    // Convert to the range 0 to 1.
    double dVolume =
               ( (double) ( lVolume             - TMGR_MIN_API_VOLUME ) )
             / ( (double) ( TMGR_MAX_API_VOLUME - TMGR_MIN_API_VOLUME ) );

    // Convert the volume to whatever range of doubles the filter uses
    // from the range 0 - 1. Right now this does nothing but makes the code
    // more general.

    dVolume = TMGR_MIN_CAPTURE_VOLUME +
        ( TMGR_MAX_CAPTURE_VOLUME - TMGR_MIN_CAPTURE_VOLUME ) * dVolume;

    hr = m_pIAMAudioInputMixer->put_MixLevel(dVolume);

    LOG((MSP_TRACE, "CAudioCaptureTerminal::put_Volume - exit 0x%08x", hr));
    return hr;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CAudioCaptureTerminal::get_Balance(long * plBalance)
{
    
    HRESULT hr = E_NOTIMPL;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_Balance - enter"));
    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_Balance - exit 0x%08x", hr));

    return hr;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CAudioCaptureTerminal::put_Balance(long lBalance)
{
    HRESULT hr = E_NOTIMPL;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::put_Balance - enter"));
    LOG((MSP_TRACE, "CAudioCaptureTerminal::put_Balance - exit 0x%08x", hr));

    return hr;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP
CAudioCaptureTerminal::get_WaveId(
    OUT long * plWaveId
    )
{
    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_WaveId - enter"));

    CLock lock(m_CritSec);

    //
    // Parameter checks.
    //

    if ( !plWaveId)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_WaveId - "
            "bad pointer argument"));

        return E_POINTER;
    }

    //
    // Check the moniker pointer.
    //

    if ( ! m_pMoniker)
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_WaveId - "
            "bad moniker pointer - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    //
    // Get a property bag from the moniker.
    //

    IPropertyBag * pBag;

    HRESULT hr = m_pMoniker->BindToStorage(0,
                                           0,
                                           IID_IPropertyBag,
                                           (void **) &pBag);
    
    if ( FAILED(hr) ) 
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_WaveId - "
            "can't get property bag - exit 0x%08x", hr));

        return hr;
    }

    //
    // Get the ID from the property bag.
    //

    VARIANT var;

    var.vt = VT_I4;

    hr = pBag->Read(
        L"WaveInId",
        &var,
        0);

    pBag->Release();

    if ( FAILED(hr) ) 
    {
        LOG((MSP_ERROR, "CAudioCaptureTerminal::get_WaveId - "
            "can't read wave ID - exit 0x%08x", hr));

        return hr;
    }

    *plWaveId = (long) var.lVal;

    LOG((MSP_TRACE, "CAudioCaptureTerminal::get_WaveId - exit S_OK"));

    return S_OK;
}
