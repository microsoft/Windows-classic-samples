/*++

Copyright (c) 1998-1999 Microsoft Corporation

Module Name:

    mspterm.cpp

Abstract:

    Implementations for the CBaseTerminal, CSingleFilterTerminal, and various
    work item / worker thread classes.

--*/

#include "precomp.h"
#pragma hdrstop

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CBaseTerminal::CBaseTerminal()
            : m_TerminalDirection(TD_CAPTURE)
            , m_TerminalType(TT_STATIC)
            , m_TerminalState(TS_NOTINUSE)
            , m_TerminalClassID(CLSID_NULL)
            , m_pFTM(NULL)
{
    LOG((MSP_TRACE, "CBaseTerminal::CBaseTerminal() called"));

    HRESULT hr = CoCreateFreeThreadedMarshaler(
            GetControllingUnknown(), &m_pFTM);

    if ( FAILED(hr) )
    {
        LOG((MSP_TRACE, "CBaseTerminal::CBaseTerminal() - create ftm failed"));
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CBaseTerminal::~CBaseTerminal()
{
    if (NULL != m_pFTM)
    {
         m_pFTM->Release();
    }
    
    LOG((MSP_TRACE, "CBaseTerminal::~CBaseTerminal() finished"));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//
// Dynamic terminals that support only one direction must override this to check
// if the direction is valid (although the terminal manager is supposed to
// ensure that the wrong direction is never passed). Dynamic terminal might want
// to override this for other reasons too (create filters now, etc.).
//
// Static terminals normally just call this in their CreateTerminal().
//
    
HRESULT CBaseTerminal::Initialize(
            IN  IID                   iidTerminalClass,
            IN  DWORD                 dwMediaType,
            IN  TERMINAL_DIRECTION    Direction,
            IN  MSP_HANDLE            htAddress
            )
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::Initialize - enter"));

    //
    // Check if the media type is supported by this terminal.
    //

    if ( ! MediaTypeSupported( (long) dwMediaType) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::Initialize - "
            "media type not supported - returning E_INVALIDARG"));
        return E_INVALIDARG;
    }

    //
    // Save this configurarion.
    //

    m_dwMediaType       = dwMediaType;
    m_TerminalDirection = Direction;
    m_TerminalClassID   = iidTerminalClass;
    m_htAddress         = htAddress;

    LOG((MSP_TRACE, "CBaseTerminal::Initialize - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CBaseTerminal::get_AddressHandle (
        OUT     MSP_HANDLE    * phtAddress
        )
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_AddressHandle - enter"));

    if ( ! phtAddress)
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_AddressHandle - returning E_POINTER")); 
        return E_POINTER;
    }

    *phtAddress = m_htAddress;

    LOG((MSP_TRACE, "CBaseTerminal::get_AddressHandle - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CBaseTerminal::get_Name(BSTR * pbsName)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_Name - enter"));

    if ( ! pbsName)
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_Name - "
            "bad BSTR passed in - returning E_POINTER")); 

        return E_POINTER;
    }

    *pbsName = SysAllocString(m_szName);

    if ( *pbsName == NULL )
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_Name - "
            "can't sysallocstring - returning E_OUTOFMEMORY")); 

        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CBaseTerminal::get_Name - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CBaseTerminal::get_State(TERMINAL_STATE * pVal)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_State - enter"));

    if ( ! pVal)
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_State - returning E_POINTER")); 
        return E_POINTER;
    }

    *pVal = m_TerminalState;

    LOG((MSP_TRACE, "CBaseTerminal::get_State - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CBaseTerminal::get_TerminalType(TERMINAL_TYPE * pVal)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_TerminalType - enter"));
    
    if ( ! pVal)
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_TerminalType - returning E_POINTER")); 
        return E_POINTER;
    }

    *pVal = m_TerminalType;

    LOG((MSP_TRACE, "CBaseTerminal::get_TerminalType - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CBaseTerminal::get_TerminalClass(BSTR * pbsClassID)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_TerminalClass - enter"));

    if ( ! pbsClassID)
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_TerminalClass - returning E_POINTER")); 
        return E_POINTER;
    }

    //
    // Convert the CLSID to an OLE string.
    //

    LPOLESTR lposClass = NULL;
    HRESULT hr = StringFromCLSID(m_TerminalClassID, &lposClass);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_TerminalClass (StringFromCLSID) - returning  %8x", hr));
        return hr;
    }

    //
    // Put the string in a BSTR.
    //

    *pbsClassID = ::SysAllocString(lposClass);

    //
    // Free the OLE string.
    //

    ::CoTaskMemFree(lposClass);

    if (*pbsClassID == NULL)
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_TerminalClass - returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CBaseTerminal::get_TerminalClass - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CBaseTerminal::get_Direction(
    OUT  TERMINAL_DIRECTION *pDirection
    )
{   
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_Direction - enter"));

    if ( ! pDirection)
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_Direction - returning E_POINTER"));
        return E_POINTER;
    }

    *pDirection = m_TerminalDirection;

    LOG((MSP_TRACE, "CBaseTerminal::get_Direction - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// enters each of the internal filters into the filter graph
// connects the internal filters together (if applicable)
// and returns all the filters to be used as connection points
STDMETHODIMP CBaseTerminal::ConnectTerminal(
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwTerminalDirection,
        IN OUT  DWORD          * pdwNumPins,
        OUT     IPin          ** ppPins
        )
{
    LOG((MSP_TRACE, "CBaseTerminal::ConnectTerminal - enter"));
    
    //
    // Check parameters.
    //

    if ( !pGraph)
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "bad graph pointer; exit E_POINTER"));
        
        return E_POINTER;
    }

    if ( !pdwNumPins)
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "bad numpins pointer; exit E_POINTER"));

        return E_POINTER;
    }

    //
    // Find out how many pins we expose. For most terminals this is
    // straightforward but we pass in the graph pointer in case they
    // need to do something funky to figure this out.
    //

    DWORD dwActualNumPins;

    HRESULT hr;

    hr = GetNumExposedPins(pGraph, &dwActualNumPins);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "GetNumExposedPins failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // If ppPins is NULL, just return the number of pins and don't try to
    // connect the terminal.
    //

    if ( ppPins == NULL )
    {
        LOG((MSP_TRACE, "CBaseTerminal::ConnectTerminal - "
            "returned number of exposed pins - exit S_OK"));

        *pdwNumPins = dwActualNumPins;
        
        return S_OK;
    }

    //
    // Otherwise, we have a pin return buffer. Check that the purported buffer
    // size is big enough and that the buffer is actually writable to the size
    // we need.
    //

    if ( *pdwNumPins < dwActualNumPins )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "not enough space to place pins; exit TAPI_E_NOTENOUGHMEMORY"));

        *pdwNumPins = dwActualNumPins;
        
        return TAPI_E_NOTENOUGHMEMORY;
    }

    if ( !ppPins)
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "bad pins array pointer; exit E_POINTER"));

        return E_POINTER;
    }

    //
    // Check if we're already connected, and if so, change our state to
    // connected. Note that this makes sense for both core static terminals
    // and dynamic terminals. Also note that we need to protect this with
    // a critical section, but after this we can let go of the lock because
    // anyone who subsequently enters the critical section will bail at this
    // point.
    //

    {
        CLock lock(m_CritSec);

        //
        // check if already connected
        //

        if (TS_INUSE == m_TerminalState)
        {
            LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
                "terminal already in use; exit TAPI_E_TERMINALINUSE"));

            return TAPI_E_TERMINALINUSE;
        }

        //
        // Save important state.
        //

        m_pGraph        = pGraph;
        m_TerminalState = TS_INUSE;
    }


    // add filters to the filter graph
    hr = AddFiltersToGraph();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "can't add filters to graph"));
        goto disconnect_terminal;
    }

    // Give the terminal a chance to do any preconnection
    hr = ConnectFilters();
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "can't do internal filter connection"));
        goto disconnect_terminal;
    }

    //
    // Get the pins that this filter exposes. No need to pass in
    // the filter graph because we already saved the graph pointer.
    //

    *pdwNumPins = dwActualNumPins;
    hr = GetExposedPins(ppPins);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "can't get exposed pins"));
        goto disconnect_terminal;
    }

    LOG((MSP_TRACE, "CBaseTerminal::ConnectTerminal success"));
    return S_OK;

disconnect_terminal:

    //
    // best effort attempt to disconnect - ignore error code
    //

    DisconnectTerminal(pGraph, 0);

    //
    // Release our reference to the graph and set ourselves to notinuse state.
    // DisconnectTerminal does this on success, but we need to make sure this
    // cleanup happens even if DisconnectTerminal failed.
    //

    m_pGraph        = NULL;          // this releases the CComPtr
    
    m_TerminalState = TS_NOTINUSE;

    LOG((MSP_TRACE, "CBaseTerminal::ConnectTerminal - exit 0x%08x", hr));
    return hr;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP 
CBaseTerminal::CompleteConnectTerminal(void)
{
    LOG((MSP_TRACE, "CBaseTerminal::CompleteConnectTerminal - enter"));
    LOG((MSP_TRACE, "CBaseTerminal::CompleteConnectTerminal - exit S_OK"));
    return S_OK;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// disconnects the internal filters from each other (if applicable)
// and removes them from the filter graph (thus breaking connections to
// the stream).
// Filter graph parameter is used for validation, to make sure the terminal
// is disconnected from the same graph that it was originally connected to.


STDMETHODIMP 
CBaseTerminal::DisconnectTerminal(
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwReserved
        )
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::DisconnectTerminal called"));

    //
    // If not in use, then there is nothing to be done.
    //

    if ( TS_INUSE != m_TerminalState ) 
    {
        _ASSERTE(m_pGraph == NULL);

        LOG((MSP_TRACE, "CBaseTerminal::DisconnectTerminal success; not in use"));

        return S_OK;
    }

    //
    // Check that we are being disconnected from the correct graph.
    //
    if ( m_pGraph != pGraph )
    {
        LOG((MSP_TRACE, "CBaseTerminal::DisconnectTerminal - "
            "wrong graph; returning E_INVALIDARG"));
        
        return E_INVALIDARG;
    }

    //
    // Extra sanity check.
    //

    if ( m_pGraph == NULL )
    {
        LOG((MSP_TRACE, "CBaseTerminal::DisconnectTerminal - "
            "no graph; returning E_UNEXPECTED"));
        
        return E_UNEXPECTED;
    }

    HRESULT hr;

    //
    // Remove filters from the graph
    //

    hr = RemoveFiltersFromGraph();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::DisconnectTerminal - "
            "remove filters from graph failed; returning 0x%08x", hr));

        return hr;
    }

    //
    // Release our reference to the graph and set ourselves to notinuse state.
    //

    m_pGraph        = NULL;          // this releases the CComPtr
    
    m_TerminalState = TS_NOTINUSE;

    LOG((MSP_TRACE, "CBaseTerminal::DisconnectTerminal success"));

    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

STDMETHODIMP CBaseTerminal::get_MediaType(long * plMediaType)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_MediaType - enter"));

    if ( !plMediaType)
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_MediaType - returning E_POINTER"));
        return E_POINTER;
    }
    
    *plMediaType = (long) m_dwMediaType;

    LOG((MSP_TRACE, "CBaseTerminal::get_MediaType - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOL CBaseTerminal::MediaTypeSupported(long lMediaType)
{
    return IsValidSingleMediaType( (DWORD) lMediaType,
                                   GetSupportedMediaTypes() );
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// CSingleFilterTerminal                                                   //
//                                                                         //
// This is a base class for a terminal with a single filter and pin. The   //
// terminal could be any direction or media type, and it could be static   //
// or dynamic.                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


HRESULT CSingleFilterTerminal::GetNumExposedPins(
        IN   IGraphBuilder * pGraph,
        OUT  DWORD         * pdwNumPins)
{
    LOG((MSP_TRACE, "CSingleFilterTerminal::GetNumExposedPins - enter"));

    //
    // We ignote pGraph because we don't need to do anything special to find
    // out how many pins we have.
    //

    *pdwNumPins = 1;
    
    LOG((MSP_TRACE, "CSingleFilterTerminal::GetNumExposedPins - exit S_OK"));

    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

HRESULT CSingleFilterTerminal::GetExposedPins(
        OUT    IPin  ** ppPins
        )
{
    LOG((MSP_TRACE, "CSingleFilterTerminal::GetExposedPins - enter"));

    _ASSERTE( ! !ppPins);

    //
    // Return our single pin.
    //

    *ppPins = m_pIPin;
    (*ppPins)->AddRef();

    LOG((MSP_TRACE, "CSingleFilterTerminal::GetExposedPins - exit S_OK"));
    return S_OK;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// stops the rightmost render filter in the terminal
// (needed for dynamic filter graphs)
STDMETHODIMP CSingleFilterTerminal::RunRenderFilter(void)
{
    // check that we're really a render filter

    // tell our single filter to run

    return E_NOTIMPL;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// stops the rightmost render filter in the terminal
// (needed for dynamic filter graphs)
STDMETHODIMP CSingleFilterTerminal::StopRenderFilter(void)
{
    // check that we're really a render filter

    // tell our single filter to stop

    return E_NOTIMPL;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

HRESULT 
CSingleFilterTerminal::RemoveFiltersFromGraph(void)
{
    LOG((MSP_TRACE, "CSingleFilterTerminal::RemoveFiltersFromGraph - enter"));

    if (m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CSingleFilterTerminal::RemoveFiltersFromGraph - "
            "no graph; returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    if (m_pIFilter == NULL)
    {
        LOG((MSP_ERROR, "CSingleFilterTerminal::RemoveFiltersFromGraph - "
            "no filter; returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    //
    // Remove the filter from the graph. This also disconnects any connections
    // the filter may have.
    //

    HRESULT hr = m_pGraph->RemoveFilter(m_pIFilter);

    LOG((MSP_TRACE, "CSingleFilterTerminal::RemoveFiltersFromGraph - exit 0x%08x", hr));
    return hr;
}

HRESULT 
CSingleFilterStaticTerminal::CompareMoniker(
                                             IMoniker *pMoniker
                                           )
{
    IMoniker    *pReducedMoniker;
    IMoniker    *pReducedNewMoniker;
    IBindCtx    *pbc; 
    HRESULT     hr;

    hr = CreateBindCtx( 0, &pbc ); 

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CSingleFilterStaticTerminal::CompareMoniker - "
            "unable to create bind context"));
        return hr;
    }

    hr = m_pMoniker->Reduce(pbc ,MKRREDUCE_ALL, NULL, &pReducedMoniker);
    
    if (FAILED(hr) || !pReducedMoniker)
    {
        LOG((MSP_ERROR, "CSingleFilterStaticTerminal::CompareMoniker - "
            "unable to reduce moniker"));
        pbc->Release();  // release the bind context              
        return hr;
    }

    hr = pMoniker->Reduce(pbc ,MKRREDUCE_ALL, NULL, &pReducedNewMoniker);
    
    if (FAILED(hr) || !pReducedNewMoniker)
    {
        LOG((MSP_ERROR, "CSingleFilterStaticTerminal::CompareMoniker - "
            "unable to reduce moniker"));
        pbc->Release();  // release the bind context
        pReducedMoniker->Release();   // release the reduced moniker
        return hr;
    }

    pbc->Release();  // release the bind context
   
    if (pReducedMoniker->IsEqual(pReducedNewMoniker) == S_OK)
    {
        LOG((MSP_TRACE, "CSingleFilterStaticTerminal::CompareMoniker - "
            "exit - return S_OK"));

        pReducedMoniker->Release();   // release the reduced monikers
        pReducedNewMoniker->Release();  
        return S_OK;
    }

    pReducedMoniker->Release();   // release the reduced monikers
    pReducedNewMoniker->Release();

    LOG((MSP_TRACE, "CSingleFilterStaticTerminal::CompareMoniker - "
            "exit - return S_FALSE"));
    return S_FALSE;
}

// eof
