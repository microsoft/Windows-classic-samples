/*++

Copyright (c) 1998-1999 Microsoft Corporation

Module Name:

    msptrmar.cpp

Abstract:

    MSP base classes: implementation of audio render terminal.

--*/

#include "precomp.h"
#pragma hdrstop

#include <mmsystem.h>

// Filter volume level ranges
const long AX_MIN_VOLUME = -9640; // -10000;
const long AX_MAX_VOLUME = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CAudioRenderTerminal::CAudioRenderTerminal()
{
    m_TerminalDirection = TD_RENDER;
    m_TerminalType = TT_STATIC;

    m_szName[0] = L'\0'; // real name is copied in on creation

    m_bResourceReserved = false;

    LOG((MSP_TRACE, "CAudioRenderTerminal::CAudioRenderTerminal() finished"));
}

CAudioRenderTerminal::~CAudioRenderTerminal()
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::~CAudioRenderTerminal() finished"));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// This function determines if the terminal associated with a given
// moniker is "good". A good terminal returns S_OK; a bad terminal returns an
// error.
//
// A good terminal has the following properties:
//      * has a friendly name
//      * is not a WAVE_MAPPER terminal
//      * is not a DirectSound terminal (unless USE_DIRECT_SOUND is pound-defined)
//

static inline HRESULT TerminalAllowed(IMoniker * pMoniker)
{
    HRESULT hr;
    CComPtr<IPropertyBag> pBag;
    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "audio render TerminalAllowed (IMoniker::BindToStorage) "
                            "- returning  %8x", hr));
        return hr;
    }

    VARIANT var;

    // we make sure creation is not going to fail on
    // account of a nonexistent friendly name
    VariantInit(&var);
    var.vt = VT_BSTR;
    hr = pBag->Read(L"FriendlyName", &var, 0);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "audio render TerminalAllowed "
            "(IPropertyBag::Read on FriendlyName) - got  %8x; skipping terminal", hr));
        return hr;
    }

    // Fix for memory leak!
    SysFreeString(var.bstrVal);

     // NOTE: Magic code selects only wave devices
    VariantInit(&var);
    var.vt = VT_I4;
    hr = pBag->Read(L"WaveOutId", &var, 0);

    if (hr != S_OK)
    {
        #ifndef USE_DIRECT_SOUND

            // This is most likely a DirectSound terminal
            LOG((MSP_WARN, "audio render TerminalAllowed - "
                "this is a DirectSound terminal "
                "so we are skipping it - note that this is a routine "
                "occurance - returning  %8x", hr));

        #else  // we do use DirectSound
            return S_OK;
        #endif
    }
    else if (var.lVal == WAVE_MAPPER)
    {
        // hack: if the value is equal to WAVE_MAPPER then don't use it....    
        hr = E_FAIL; // random failure code :)

        LOG((MSP_WARN, "audio render TerminalAllowed - "
            "this is a WAVE_MAPPER terminal "
            "so we are skipping it - note that this is a routine "
            "occurance - returning  %8x", hr));
    }

    return hr;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

HRESULT CAudioRenderTerminal::CreateTerminal(
    IN  CComPtr<IMoniker>   pMoniker,
    IN  MSP_HANDLE          htAddress,
    OUT ITTerminal        **ppTerm
    )
{
    // Enable ATL string conversion macros.
    USES_CONVERSION;

    LOG((MSP_TRACE, "CAudioRenderTerminal::CreateTerminal - enter"));

    //
    // Validate the parameters
    //

    if ( !ppTerm)
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal : "
            "bad terminal pointer; returning E_POINTER"));
        return E_POINTER;
    }

    if ( !pMoniker)
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal : "
            "bad moniker pointer; returning E_POINTER"));
        return E_POINTER;
    }

    //
    // We return a NULL terminal if we fail.
    //

    *ppTerm = NULL;
    HRESULT hr;

    // Refuse to work with DirectSound or WAVE_MAPPER terminals.
    // or if we can't read the friendlyName...
    if (FAILED(hr = TerminalAllowed(pMoniker))) return hr;

    //
    // Get the name for this filter out of the property bag.
    //

    CComPtr<IPropertyBag> pBag;
    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal (IMoniker::BindToStorage) - returning  %8x", hr));
        return hr;
    }

    VARIANT var;
    VariantInit(&var);

    var.vt = VT_BSTR;
    hr = pBag->Read(L"FriendlyName", &var, 0);
    if (FAILED(hr)) 
    {
        LOG((MSP_WARN, "CAudioRenderTerminal::CreateTerminal "
            "(IPropertyBag::Read) - got  %8x - we are therefore skipping "
            "this terminal; note that this is fairly routine", hr));

        return hr;
    }

    //
    // Create the terminal.
    //

    CMSPComObject<CAudioRenderTerminal> *pLclTerm = new CMSPComObject<CAudioRenderTerminal>;
    if (pLclTerm == NULL) 
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal - returning E_OUTOFMEMORY"));
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
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal - "
            "Internal QI failed; returning 0x%08x", hr));

        delete pLclTerm;
        *ppTerm = NULL; // just in case

        return hr;
    }

    //
    // Finish initializing the terminal.
    //

    hr = pLclTerm->Initialize(CLSID_SpeakersTerminal,
                              TAPIMEDIATYPE_AUDIO,
                              TD_RENDER,
                              htAddress
                             );
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateTerminal - "
            "Initialize failed; returning 0x%08x", hr));

        (*ppTerm)->Release();
        *ppTerm = NULL; // just in case

        return hr;
    }

    LOG((MSP_TRACE, "CAudioRenderTerminal::CreateTerminal - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Create the filters used by this terminal

HRESULT CAudioRenderTerminal::CreateFilters()
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::CreateFilters - enter"));

    HRESULT hr;

    //
    // We used to recreate the audio render filter every time, but we don't
    // have any real reason for doing so. Just return S_OK if the filter
    // has already been created.
    //

    if ( m_pIFilter != NULL )
    {
        _ASSERTE( m_pIPin != NULL );

        LOG((MSP_TRACE, "CAudioRenderTerminal::CreateFilters - "
            "filter already created - exit S_OK"));

        return S_OK;
    }

    _ASSERTE ( m_pIBasicAudio == NULL );
    _ASSERTE ( m_pIPin == NULL );

    //
    // Sanity checks.
    //
    if ( m_pMoniker == NULL )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateFilters - "
            "no moniker present - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    //
    // Create a new instance of the filter.    
    //
    hr = m_pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pIFilter);
 
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateFilters - "
            "BindToObject failed; returning  %8x", hr));
        return hr;
    }

    //
    // Get the basic audio interface for the filter. If it doesn't exist, we
    // can live with that, but all our IBasicAudio methods will fail.
    //

    hr = m_pIFilter->QueryInterface(IID_IBasicAudio,
                                       (void **) &m_pIBasicAudio);
    if ( FAILED(hr) ) 
    {
        LOG((MSP_WARN, "CAudioRenderTerminal::CreateFilters - "
            "QI for IBasicAudio failed: %8x", hr)); 
    }

    hr = FindTerminalPin();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CreateFilters - "
            "FindTerminalPin failed; returning  0x%08x", hr));

        m_pIFilter = NULL; // does an implicit release

        if ( m_pIBasicAudio )
        {
            m_pIBasicAudio = NULL; // does an implicit release
        }
        
        return hr;
    }

    LOG((MSP_TRACE, "CAudioRenderTerminal::CreateFilters - exit S_OK"));

    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

HRESULT 
CAudioRenderTerminal::FindTerminalPin(
    )
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::FindTerminalPin - enter"));

    //
    // Sanity checks so we don't AV.
    //

    if (m_pIPin != NULL)
    {
        LOG((MSP_TRACE, "CAudioRenderTerminal::FindTerminalPin - "
            "we've already got a pin; exit E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    if (m_pIFilter == NULL)
    {
        LOG((MSP_TRACE, "CAudioRenderTerminal::FindTerminalPin - "
            "we don't have a filter; exit E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr;
    CComPtr<IEnumPins> pIEnumPins;
    ULONG cFetched;
    
    //
    // Find the render pin for the filter.
    //

    hr = m_pIFilter->EnumPins(&pIEnumPins);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CAudioRenderTerminal::FindTerminalPin - can't enum pins 0x%08x",
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
                "CAudioRenderTerminal::FindTerminalPin - can't get a pin %8x",
                hr));
            return (hr == S_FALSE) ? E_FAIL : hr;
        }

        if (0 == cFetched)
        {
            LOG((MSP_ERROR, "CAudioRenderTerminal::FindTerminalPin - got zero pins"));
            return E_FAIL;
        }

        PIN_DIRECTION dir;

        if (FAILED(hr = pIPin->QueryDirection(&dir)))
        {
            LOG((MSP_ERROR, 
                "CAudioRenderTerminal::FindTerminalPin - can't query pin direction %8x",
                hr));
            pIPin->Release();
            return hr;
        }

        if (PINDIR_INPUT == dir)
        {
            break;
        }

        pIPin->Release();
    }

    m_pIPin = pIPin;

    LOG((MSP_TRACE, "CAudioRenderTerminal::FindTerminalPin - exit S_OK"));
  
    return S_OK;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

HRESULT CAudioRenderTerminal::AddFiltersToGraph(
    )
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::AddFiltersToGraph - enter"));

    if (m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::AddFiltersToGraph - "
            "haven't got a filter graph; return E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    //
    // Create the filters if this is the first connection with this terminal.
    //

    HRESULT hr = CreateFilters();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::AddFiltersToGraph - "
            "CreateFilters failed; returning  0x%08x", hr)); 
        return hr; 
    }

    //
    // Add the filter to the graph.
    //
    // A word about names:
    // If a filter has already been added with the same name (which will
    // happen if we have more than one audio render terminal in the same
    // graph) then that will return VFW_S_DUPLICATE_NAME, which is not
    // a failure.
    //

    hr = m_pGraph->AddFilter(m_pIFilter, WAVEOUT_NAME);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::AddFiltersToGraph - "
            "returning  0x%08x", hr)); 
        return hr; 
    }

    LOG((MSP_TRACE, "CAudioRenderTerminal::AddFiltersToGraph - exit S_OK"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////
// we override this here so we can do some stuff
// right after the filter gets connected

STDMETHODIMP CAudioRenderTerminal::CompleteConnectTerminal(void)
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::CompleteConnectTerminal - enter"));

    // By default, we need not unreserve later.
    m_bResourceReserved = false;

    // Don't clobber the base class' machinations (currently nothing...)
    HRESULT hr = CSingleFilterTerminal::CompleteConnectTerminal();

    if (FAILED(hr))
    {
        LOG((MSP_TRACE, "CAudioRenderTerminal::CompleteConnectTerminal: "
                "CSingleFilterTerminal method failed"));
        return hr;
    }

    // So here we are, after our filter has been added to the filter graph and connected up.
    // We need to use the filter's
    // IAMResourceControl::Reserve method to make sure the filter opens the waveOut device
    // now (and keeps it open).

    //////////////////////////////////////////////////////////////////////////
    // we must inform the filter that we want it to grab the wave device.
    // We do this after connecting because the filter needs to negotiate the
    // media type before it can open a wave device.

    CComPtr <IAMResourceControl> pIResource;

    hr = m_pIFilter->QueryInterface(IID_IAMResourceControl, (void **) &pIResource);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CompleteConnectTerminal - QI failed: %8x", hr)); 
        
        // This is a nonesential operation so we do not fail.
        return S_OK;
    }

    // The QueryInterface didn't fail...

    hr = pIResource->Reserve(AMRESCTL_RESERVEFLAGS_RESERVE, NULL);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::CompleteConnectTerminal - "
                            "device reservation failed: %8x", hr));
        return hr;
    }
    else if (hr == S_FALSE)
    {
        // Well, in this case either another application is already using the wave out device,
        // or we are running half-duplex and we've got both wavein and waveout terminals
        // selected.

        LOG((MSP_ERROR, "CAudioRenderTerminal::CompleteConnectTerminal - "
                "device already in use: %8x", hr));
        return hr;

    } // {if the driver is half-duplex}

    // We have succeeded in reserving, so we will want to unreserve later.
    m_bResourceReserved = true;

    LOG((MSP_TRACE, "CAudioRenderTerminal::CompleteConnectTerminal - exit S_OK"));
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////
// We override this here so we can unreserve the resource when we are done.
// removes filters from the filter graph and resets member variables
// Disconnect may be called anytime after Connect succeeds (it need not be called
// if CompleteConnect fails)

STDMETHODIMP CAudioRenderTerminal::DisconnectTerminal(
            IN      IGraphBuilder  * pGraph,
            IN      DWORD            dwReserved
            )
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::DisconnectTerminal - enter"));

    HRESULT hr;

    //
    // First call the base class method, to make sure we validate everything
    // and don't mess with our resource reservation unless this is a valid
    // disconnection (e.g., the filter graph pointersmatch).
    //

    hr = CSingleFilterTerminal::DisconnectTerminal(pGraph, dwReserved);

    if (FAILED(hr))
    {
        LOG((MSP_TRACE, "CAudioRenderTerminal::DisconnectTerminal : "
                "CSingleFilterTerminal method failed; hr = %d", hr));
        return hr;
    }

    // if we need to release the resource
    if (m_bResourceReserved)
    {
        CComPtr <IAMResourceControl> pIResource;

        hr = m_pIFilter->QueryInterface(IID_IAMResourceControl, (void **) &pIResource);
        if (FAILED(hr)) 
        {
            LOG((MSP_ERROR, "CAudioRenderTerminal::DisconnectTerminal - QI failed: %8x", hr)); 
        
            // This is a nonesential operation so we do not "return hr;" here.
        }
        else
        {
            // QueryInterface didn't fail, and we have reserved WaveOut, so we must
            // unreserve now.

            hr = pIResource->Reserve(AMRESCTL_RESERVEFLAGS_UNRESERVE, NULL);
            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "CAudioRenderTerminal::DisconnectTerminal - "
                                    "device unreservation failed: %8x", hr));
                // no reason to completely die at this point, so we just continue
            }

            // if other things fail we may be called again, but we should not try
            // to unreserve again.
            m_bResourceReserved = false;

        } // {if QI succeeded}
    } // {if need to release resource}

    LOG((MSP_TRACE, "CAudioRenderTerminal::DisconnectTerminal - exit S_OK"));

    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// private helper method:

static HRESULT RangeConvert(long   lInput,
                            long   lInputMin,
                            long   lInputMax,
                            long * plOutput,
                            long   lOutputMin,
                            long   lOutputMax)
{
    _ASSERTE( lInputMin  < lInputMax );
    _ASSERTE( lOutputMin < lOutputMax );
    _ASSERTE( ! !plOutput);

    if (lInput < lInputMin)
    {
        LOG((MSP_ERROR, "RangeConvert - value out of range - "
            "%d < %d; returning E_INVALIDARG",
            lInput, lInputMin));

        return E_INVALIDARG;
    }

    if (lInput > lInputMax)
    {
        LOG((MSP_ERROR, "RangeConvert - value out of range - "
            "%d > %d; returning E_INVALIDARG",
            lInput, lInputMax));

        return E_INVALIDARG;
    }

    // This is how much we are going to expand the range of the input.    
    double dRangeWidthRatio = (double) (lOutputMax - lOutputMin) /
                              (double) (lInputMax  - lInputMin);

    *plOutput = (long) ((lInput - lInputMin) * dRangeWidthRatio) + lOutputMin;

    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CAudioRenderTerminal::get_Volume(long * plVolume)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CAudioRenderTerminal::get_Volume - enter"));

    //
    // Parameter checks.
    //

    if ( !plVolume)
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_Volume - "
            "bad pointer argument"));
        return E_POINTER;
    }

    if (m_pIBasicAudio == NULL)
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_Volume - "
            "don't have necessary interface - exit E_FAIL"));
        return E_FAIL;
    }

    //
    // Let the filter do the work.
    //

    HRESULT hr = m_pIBasicAudio->get_Volume(plVolume);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_Volume - "
            "filter call failed: %08x", hr));
        return hr;
    }

    //
    // Asjust the range of the value returned to match the range specified
    // by the TAPI APIs.
    //

    hr = RangeConvert(*plVolume, AX_MIN_VOLUME, AX_MAX_VOLUME,
                      plVolume,  0,             0xFFFF);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_Volume - "
            "RangeConvert call failed: %08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CAudioRenderTerminal::get_Volume - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CAudioRenderTerminal::put_Volume(long lVolume)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CAudioRenderTerminal::put_Volume - enter"));

    if (m_pIBasicAudio == NULL)
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::put_Volume - "
            "don't have necessary interface - exit E_FAIL"));
        return E_FAIL;
    }

    //
    // Asjust the range of the value returned to match the range needed
    // by the WaveOut filter.
    //

    HRESULT hr = RangeConvert(lVolume,  0,             0xFFFF,
                              &lVolume, AX_MIN_VOLUME, AX_MAX_VOLUME);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::put_Volume - "
            "RangeConvert call failed: %08x", hr));
        return hr;
    }

    //
    // Let the filter do the work.
    //

    hr = m_pIBasicAudio->put_Volume(lVolume);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::put_Volume - "
            "filter call failed: %08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CAudioRenderTerminal::put_Volume - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CAudioRenderTerminal::get_Balance(long * plBalance)
{
    HRESULT hr = E_NOTIMPL;

    LOG((MSP_TRACE, "CAudioRenderTerminal::get_Balance - enter"));
    LOG((MSP_TRACE, "CAudioRenderTerminal::get_Balance - exit 0x%08x", hr));

    return hr;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CAudioRenderTerminal::put_Balance(long lBalance)
{
    HRESULT hr = E_NOTIMPL;

    LOG((MSP_TRACE, "CAudioRenderTerminal::put_Balance - enter"));
    LOG((MSP_TRACE, "CAudioRenderTerminal::put_Balance - exit 0x%08x", hr));

    return hr;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP
CAudioRenderTerminal::get_WaveId(
    OUT long * plWaveId
    )
{
    LOG((MSP_TRACE, "CAudioRenderTerminal::get_WaveId - enter"));

    CLock lock(m_CritSec);

    //
    // Parameter checks.
    //

    if ( !plWaveId)
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_WaveId - "
            "bad pointer argument"));

        return E_POINTER;
    }

    //
    // Check the moniker pointer.
    //

    if ( ! m_pMoniker)
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_WaveId - "
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
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_WaveId - "
            "can't get property bag - exit 0x%08x", hr));

        return hr;
    }

    //
    // Get the ID from the property bag.
    //

    VARIANT var;

    var.vt = VT_I4;

    hr = pBag->Read(
        L"WaveOutId",
        &var,
        0);

    pBag->Release();

    if ( FAILED(hr) ) 
    {
        LOG((MSP_ERROR, "CAudioRenderTerminal::get_WaveId - "
            "can't read wave ID - exit 0x%08x", hr));

        return hr;
    }

    *plWaveId = (long) var.lVal;

    LOG((MSP_TRACE, "CAudioRenderTerminal::get_WaveId - exit S_OK"));

    return S_OK;
}

