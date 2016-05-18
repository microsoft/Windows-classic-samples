/*++

Copyright (c) 1998-1999 Microsoft Corporation

Module Name:

    msptrmvc.cpp

Abstract:

    MSP base classes: implementation of video capture terminal.

--*/


#include "precomp.h"
#pragma hdrstop

#include <amvideo.h>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CVideoCaptureTerminal::~CVideoCaptureTerminal()
{
    LOG((MSP_TRACE, "CVideoCaptureTerminal::~CVideoCaptureTerminal() finished"));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

HRESULT CVideoCaptureTerminal::CreateTerminal(
    IN    CComPtr<IMoniker>    pMoniker,
    IN    MSP_HANDLE           htAddress,
    OUT   ITTerminal         **ppTerm
    )
{
    USES_CONVERSION;

    LOG((MSP_TRACE, "CVideoCaptureTerminal::CreateTerminal - enter"));

    //
    // Validate the parameters
    //

    if ( !ppTerm)
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal : "
            "bad terminal pointer; returning E_POINTER"));
        return E_POINTER;
    }

    if ( !pMoniker)
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal : "
            "bad moniker pointer; returning E_POINTER"));
        return E_POINTER;
    }

    *ppTerm = NULL;
    HRESULT hr;

    //
    // Get the name for this filter out of the property bag.
    //
    CComPtr<IPropertyBag> pBag;
    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, 
            "CVideoCaptureTerminal::CreateTerminal (BindToStorage) - returning  %8x", hr));
        return hr;
    }

    VARIANT var;
    var.vt = VT_BSTR;
    hr = pBag->Read(L"FriendlyName", &var, 0);
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal (IPropertyBag::Read) - returning  %8x", hr));
        return hr;
    }

    //
    // Create the actual terminal object
    //
    CMSPComObject<CVideoCaptureTerminal> *pLclTerm = new CMSPComObject<CVideoCaptureTerminal>;
    if (pLclTerm == NULL)
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    //
    // Save some stuff we'll need later
    //
    pLclTerm->m_pMoniker = pMoniker;
    wcsncpy_s(pLclTerm->m_szName,MAX_PATH, OLE2T(var.bstrVal), MAX_PATH);

    SysFreeString(var.bstrVal);

    //
    // Finally get the ITTerminal interface that we were asked for.
    //
    hr = pLclTerm->_InternalQueryInterface(IID_ITTerminal, (void**)ppTerm);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal - "
            "Internal QI failed; returning 0x%08x", hr));

        delete pLclTerm;
        return hr;
    }

    //
    // Finish initializing the terminal.
    //

    hr = pLclTerm->Initialize(CLSID_VideoInputTerminal,
                              TAPIMEDIATYPE_VIDEO,
                              TD_CAPTURE,
                              htAddress
                             );
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateTerminal - "
            "Initialize failed; returning 0x%08x", hr));

        (*ppTerm)->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CVideoCaptureTerminal::CreateTerminal - exit S_OK"));
    return S_OK;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Create the filters used by this terminal

HRESULT CVideoCaptureTerminal::CreateFilters()
{
    LOG((MSP_TRACE, "CVideoCaptureTerminal::CreateFilters() - enter"));

    //
    // If we already have the filter, just return S_OK.
    //

    if ( m_pIFilter != NULL )
    {
        LOG((MSP_TRACE, "CVideoCaptureTerminal::CreateFilters() - "
            "already have a filter - exit S_OK"));

        return S_OK;
    }

    //
    // Now make the filter.
    //

    HRESULT hr = m_pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pIFilter);

    if ( FAILED(hr) )
    {
        // reset the filer interface - it's a CComPointer so this releases it

        m_pIFilter = NULL;
 
        LOG((MSP_ERROR, "CVideoCaptureTerminal::CreateFilters() - "
            "BindToObject failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CVideoCaptureTerminal::CreateFilters() - exit S_OK"));

    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

HRESULT 
CVideoCaptureTerminal::FindCapturePin(
    )
{
    LOG((MSP_TRACE, "CVideoCaptureTerminal::FindCapturePin() - enter"));

    if ( m_pIPin != NULL )
    {
        LOG((MSP_INFO, "CVideoCaptureTerminal::FindCapturePin() - "
            "already called, so we already have a pin - exit S_OK"));

        return S_OK;
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
            "CVideoCaptureTerminal::FindCapturePin - can't enum pins %8x",
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
                "CVideoCaptureTerminal::FindCapturePin - can't get a pin %8x",
                hr));
            return (hr == S_FALSE) ? E_FAIL : hr;
        }

        if (0 == cFetched)
        {
            LOG((MSP_ERROR, "CVideoCaptureTerminal::FindCapturePin - got zero pins"));
            return E_FAIL;
        }

        PIN_DIRECTION dir;

        if (FAILED(hr = pIPin->QueryDirection(&dir)))
        {
            LOG((MSP_ERROR, 
                "CVideoCaptureTerminal::FindCapturePin - can't query pin direction %8x",
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

    LOG((MSP_TRACE, "CVideoCaptureTerminal::FindCapturePin - exit S_OK"));
  
    return S_OK;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

HRESULT 
CVideoCaptureTerminal::AddFiltersToGraph(
    )
{
    LOG((MSP_TRACE, "CVideoCaptureTerminal::AddFiltersToGraph called"));
    
    if (m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::AddFiltersToGraph - "
            "no graph - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    HRESULT hr;

    hr = CreateFilters();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::AddFiltersToGraph - "
            "CreateFilters failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Add the filter to the graph.
    //
    // A word about names:
    // If a filter has already been added with the same name (which will
    // happen if we have more than one video capture terminal in the same
    // graph) then that will return VFW_S_DUPLICATE_NAME, which is not
    // a failure.
    //

    try 
    {
        USES_CONVERSION;
        hr = m_pGraph->AddFilter(m_pIFilter, T2CW(m_szName));
    }
    catch (...)
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::AddFiltersToGraph - T2CW threw an exception - "
            "return E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::AddFiltersToGraph - "
            "AddFilter failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // set the terminal's capture pin (output pin)
    //

    hr = FindCapturePin();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoCaptureTerminal::AddFiltersToGraph - "
            "FindCapturePin failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CVideoCaptureTerminal::AddFiltersToGraph succeeded"));
    return S_OK;
}

// eof
