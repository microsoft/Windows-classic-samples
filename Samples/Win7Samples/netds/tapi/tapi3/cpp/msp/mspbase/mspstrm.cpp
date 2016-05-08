/*++

Copyright (c) 1997-1999  Microsoft Corporation

Module Name:

    mspstrm.cpp

Abstract:

    This module contains implementation of CMSPStream. The object represents
    one stream in the filter graph.

--*/

#include "precomp.h"
#pragma hdrstop

/////////////////////////////////////////////////////////////////////////////
// CMSPStream
/////////////////////////////////////////////////////////////////////////////

CMSPStream::CMSPStream()
    : m_dwState(STRM_INITIAL),
      m_dwMediaType(0),
      m_pFTM(NULL),
      m_hAddress(NULL),
      m_pMSPCall(NULL),
      m_pIGraphBuilder(NULL),
      m_pIMediaControl(NULL),

#if ((WINVER > 0x501) && (_WIN32_WINNT > 0x501))
      m_pPTEventSink( NULL ),
      m_lMyPersonalRefcount(0),
      m_bFirstAddRef(TRUE)
#else
      m_pPTEventSink( NULL )
#endif // ((WINVER > 0x501) && (_WIN32_WINNT > 0x501))
{
    LOG((MSP_TRACE, "CMSPStream::CMSPStream - enter"));
    LOG((MSP_TRACE, "CMSPStream::CMSPStream - exit"));
}

CMSPStream::~CMSPStream()
{
    LOG((MSP_TRACE, "CMSPStream::~CMSPStream - enter"));
    
    ReleaseSink();

    LOG((MSP_TRACE, "CMSPStream::~CMSPStream - exit"));
}

STDMETHODIMP CMSPStream::get_MediaType(
    OUT     long *                  plTapiMediaType
    )
{
    LOG((MSP_TRACE, "CMSPStream::get_MediaType - enter"));

    if (!plTapiMediaType)
    {
        LOG((MSP_ERROR, "CMSPStream::get_MediaType - exit E_POINTER"));

        return E_POINTER;
    }

    
    CLock lock(m_lock);

    *plTapiMediaType = m_dwMediaType;

    LOG((MSP_TRACE, "CMSPStream::get_MediaType - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPStream::get_Direction(
    OUT     TERMINAL_DIRECTION *    pTerminalDirection
    )
{
    LOG((MSP_TRACE, "CMSPStream::get_Direction - enter"));

    if (!pTerminalDirection)
    {
        LOG((MSP_ERROR, "CMSPStream::get_Direction - exit E_POINTER"));

        return E_POINTER;
    }


    CLock lock(m_lock);


    *pTerminalDirection = m_Direction;
    
    LOG((MSP_TRACE, "CMSPStream::get_Direction - exit S_OK"));
    
    return S_OK;
}

STDMETHODIMP CMSPStream::SelectTerminal(
    IN      ITTerminal *            pTerminal
    )
/*++

Routine Description:

Arguments:
    

Return Value:

S_OK

E_POINTER
E_OUTOFMEMORY
TAPI_E_MAXTERMINALS
TAPI_E_INVALIDTERMINAL

--*/
{
    LOG((MSP_TRACE, "CMSPStream::SelectTerminal - enter"));

    //
    // Check parameter.
    //

    if ( !pTerminal)
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - exit E_POINTER"));

        return E_POINTER;
    }

    HRESULT hr;
    ITTerminalControl *pTerminalControl;

    //
    // Get the private interface from this terminal.
    //

    hr = pTerminal->QueryInterface(IID_ITTerminalControl, 
                                   (void **) &pTerminalControl);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - "
            "can't get ITTerminalControl - exit TAPI_E_INVALIDTERMINAL"));

        return TAPI_E_INVALIDTERMINAL;
    }

    //
    // Get the address handle and release the private interface.
    //

    MSP_HANDLE hAddress;
    hr = pTerminalControl->get_AddressHandle(&hAddress);

    pTerminalControl->Release();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - "
            "can't get address handle - exit TAPI_E_INVALIDTERMINAL"));

        return TAPI_E_INVALIDTERMINAL;
    }

    //
    // Find out if the terminal belongs to this address. Reject it if it belongs
    // to another address, but accept it if it is an application-provided terminal
    // (NULL address handle).
    //

    if ( ( hAddress != NULL ) && ( hAddress != (MSP_HANDLE) m_hAddress ) )
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - "
            "terminal from another address - exit TAPI_E_INVALIDTERMINAL"));

        return TAPI_E_INVALIDTERMINAL;
    }

    //
    // Find out if the terminal is already in our list.
    //

    CLock lock(m_lock);
    
    if (m_Terminals.Find(pTerminal) >= 0)
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - "
            "terminal already selected - exit TAPI_E_INVALIDTERMINAL"));

        return TAPI_E_INVALIDTERMINAL;
    }
    
    //
    // Add the new terminal into our list and addref it.
    //

    if (!m_Terminals.Add(pTerminal))
    {
        LOG((MSP_ERROR, "CMSPStream::SelectTerminal - "
            "exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    pTerminal->AddRef();

    hr = RegisterPluggableTerminalEventSink( pTerminal );
    if( FAILED(hr) )
    {
        LOG((MSP_TRACE, "CMSPStream::SelectTerminal - "
            "something wrong in RegisterPluggableTerminalEventSink"));
    }

    LOG((MSP_TRACE, "CMSPStream::SelectTerminal - exit S_OK"));
    
    return S_OK;
}

STDMETHODIMP CMSPStream::UnselectTerminal(
    IN     ITTerminal *             pTerminal
    )
{
    LOG((MSP_TRACE, "CMSPStream::UnselectTerminal - enter"));

    //
    // find out if the terminal is in our list.
    //

    CLock lock(m_lock);
    int index;
    
    if ((index = m_Terminals.Find(pTerminal)) < 0)
    {
        LOG((MSP_ERROR, "CMSPStream::UnselectTerminal - "
            "exit TAPI_E_INVALIDTERMINAL"));
    
        return TAPI_E_INVALIDTERMINAL;
    }

    //
    // Unregister the PTEventSink object
    //

    HRESULT hr = E_FAIL; 
    hr = UnregisterPluggableTerminalEventSink( pTerminal );

    if( FAILED(hr) )
    {
        LOG((MSP_TRACE, "CMSPStream::UnselectTerminal - "
            "something wrong in UnregisterPluggableTerminalEventSink"));
    }
    
    //
    // remove the terminal from our list and release it.
    //

    if (!m_Terminals.RemoveAt(index))
    {
        LOG((MSP_ERROR, "CMSPStream::UnselectTerminal - "
            "exit E_UNEXPECTED"));
    
        return E_UNEXPECTED;
    }

    pTerminal->Release();

    LOG((MSP_TRACE, "CMSPStream::UnselectTerminal - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPStream::EnumerateTerminals(
    OUT     IEnumTerminal **        ppEnumTerminal
    )
{
    LOG((MSP_TRACE, 
        "EnumerateTerminals entered. ppEnumTerminal:%x", ppEnumTerminal));

    if (!ppEnumTerminal)
    {
        LOG((MSP_ERROR, "ppEnumTerminal is a bad pointer"));
        return E_POINTER;
    }

    // acquire the lock before accessing the Terminal object list.
    CLock lock(m_lock);

    if (m_Terminals.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CMSPStream::EnumerateTerminals - "
            "stream appears to have been shut down - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    typedef _CopyInterface<ITTerminal> CCopy;
    typedef CSafeComEnum<IEnumTerminal, &IID_IEnumTerminal, 
                ITTerminal *, CCopy> CEnumerator;

    HRESULT hr;

    CMSPComObject<CEnumerator> *pEnum = NULL;

    hr = CMSPComObject<CEnumerator>::CreateInstance(&pEnum);
    if (pEnum == NULL)
    {
        LOG((MSP_ERROR, "Could not create enumerator object, %x", hr));
        return hr;
    }

    // query for the IID_IEnumTerminal i/f
    hr = pEnum->_InternalQueryInterface(IID_IEnumTerminal, (void**)ppEnumTerminal);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "query enum interface failed, %x", hr));
        delete pEnum;
        return hr;
    }

    // The CSafeComEnum can handle zero-sized array.
    hr = pEnum->Init(
        m_Terminals.GetData(),                        // the begin itor
        m_Terminals.GetData() + m_Terminals.GetSize(),  // the end itor, 
        NULL,                                       // IUnknown
        AtlFlagCopy                                 // copy the data.
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "init enumerator object failed, %x", hr));
        (*ppEnumTerminal)->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CMSPStream::EnumerateTerminals - exit S_OK"));

    return hr;
}

STDMETHODIMP CMSPStream::get_Terminals(
    OUT     VARIANT *               pVariant
    )
{
    LOG((MSP_TRACE, "CMSPStream::get_Terminals - enter"));

    //
    // Check parameters.
    //

    if ( !pVariant)
    {
        LOG((MSP_ERROR, "CMSPStream::get_Terminals - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

    //
    // See if this stream has been shut down. Acquire the lock before accessing
    // the terminal object list.
    //

    CLock lock(m_lock);

    if (m_Terminals.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CMSPStream::get_Terminals - "
            "stream appears to have been shut down - exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }


    //
    // create the collection object - see mspcoll.h
    //

    HRESULT hr;
    typedef CTapiIfCollection< ITTerminal * > TerminalCollection;
    CComObject<TerminalCollection> * pCollection;
    hr = CComObject<TerminalCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPStream::get_Terminals - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

    //
    // get the Collection's IDispatch interface
    //

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(IID_IDispatch,
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPStream::get_Terminals - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

    //
    // Init the collection using an iterator -- pointers to the beginning and
    // the ending element plus one.
    //

    hr = pCollection->Initialize( m_Terminals.GetSize(),
                                  m_Terminals.GetData(),
                                  m_Terminals.GetData() + m_Terminals.GetSize() );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPStream::get_Terminals - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();
        return hr;
    }

    //
    // put the IDispatch interface pointer into the variant
    //

    LOG((MSP_TRACE, "CMSPStream::get_Terminals - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPStream::get_Terminals - exit S_OK"));
 
    return S_OK;
}

STDMETHODIMP CMSPStream::StartStream()
{
    LOG((MSP_TRACE, "CMSPStream - RUNNING GRAPH"));

    HRESULT hr = m_pIMediaControl->Run();
    
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "graph doesn't run, %x", hr));
    }
    return hr;
}

STDMETHODIMP CMSPStream::PauseStream()
{
    LOG((MSP_TRACE, "CMSPStream - PAUSING GRAPH"));

    HRESULT hr = m_pIMediaControl->Pause();
    
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "graph doesn't pause, %x", hr));
    }
    return hr;
}

STDMETHODIMP CMSPStream::StopStream()
{
    LOG((MSP_TRACE, "CMSPStream - STOPPING GRAPH"));

    HRESULT hr = m_pIMediaControl->Stop();
    
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "graph doesn't stop, %x", hr));
    }
    return hr;
}

// methods called by the MSPCall object.
HRESULT CMSPStream::Init(
    IN     HANDLE                   hAddress,
    IN     CMSPCallBase *           pMSPCall,
    IN     IMediaEvent *            pGraph,
    IN     DWORD                    dwMediaType,
    IN     TERMINAL_DIRECTION       Direction
    )
{
    LOG((MSP_TRACE, "CMSPStream::Init - enter"));

    
    CLock lock(m_lock);


    // This method is called only once when the object is created. No other
    // method will be called until this function succeeds. No need to lock.
    _ASSERTE(m_hAddress == NULL);

    // initialize the terminal array so that the array is not NULL. Used for
    // generating an empty enumerator if no terminal is selected.
    if (!m_Terminals.Grow())
    {
        LOG((MSP_ERROR, "CMSPStream::Init - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }
    
    HRESULT hr;
    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), &m_pFTM);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "create marshaler failed, %x", hr));
        return hr;
    }

    // Get the media control interface on the graph.
    IMediaControl *pMC;
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **) &pMC);
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "get IMediaControl interface, %x", hr));
        return hr;
    }

    // Get the graph builder interface on the graph.
    IGraphBuilder *pGB;
    hr = pGraph->QueryInterface(IID_IGraphBuilder, (void **) &pGB);
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "get IGraphBuilder interface, %x", hr));
        pMC->Release();
        return hr;
    }

    m_hAddress          = hAddress;

    m_pMSPCall          = pMSPCall;
    m_pMSPCall->MSPCallAddRef();

    m_pIMediaControl    = pMC;
    // no addref because QI addrefs for us above
    
    m_pIGraphBuilder    = pGB;
    // no addref because QI addrefs for us above

    m_dwMediaType       = dwMediaType;
    m_Direction         = Direction;

    LOG((MSP_TRACE, "CMSPStream::Init - exit S_OK"));

    return S_OK;
}

HRESULT CMSPStream::ShutDown()
{
    LOG((MSP_TRACE, "CMSPStream::Shutdown - enter"));

    CLock lock(m_lock);

    //
    // We are shut down, so the call is now NULL.
    //

    m_pMSPCall->MSPCallRelease();
    m_pMSPCall = NULL;

    //
    // Unselect all the terminals. Rather than just removing all the
    // terminals, we call UnselectTerminal on each one to give the derived
    // class a chance to do whatever else it needs to do when a terminal
    // is unselected.
    //
    // We walk the list in reverse order because the list shrinks with
    // each iteration (see msputils.h).
    //

    for ( int i = m_Terminals.GetSize() - 1; i >= 0; i-- )
    {
        UnselectTerminal(m_Terminals[i]);
    }
 
    //
    // At this point the derive class should have removed and released
    // all of the terminals from the list. If that's not the case, the
    // derived class is buggy.
    //

    _ASSERTE( m_Terminals.GetSize() == 0 );


    //
    // no longer need the sink
    //

    ReleaseSink();

    LOG((MSP_TRACE, "CMSPStream::Shutdown - exit S_OK"));

    return S_OK;
}

void CMSPStream::FinalRelease()
{
    LOG((MSP_TRACE, "CMSPStream::FinalRelease - enter"));

    // release the two interface pointers to the graph.
    if (m_pIMediaControl)
    {
        m_pIMediaControl->Release();
    }

    if (m_pIGraphBuilder)
    {
        m_pIGraphBuilder->Release();
    }

    if (m_pFTM)
    {
        m_pFTM->Release();
    }

    LOG((MSP_TRACE, "CMSPStream::FinalRelease - exit"));
}

HRESULT CMSPStream::HandleTSPData(
    IN     BYTE *                   pData,
    IN     DWORD                    dwSize
    )
{
    LOG((MSP_TRACE, "CMSPStream::HandleTSPData - enter"));
    LOG((MSP_TRACE, "CMSPStream::HandleTSPData - exit S_OK"));

    return S_OK;
}

HRESULT CMSPStream::ProcessGraphEvent(
    IN  long lEventCode,
    IN  LONG_PTR lParam1,
    IN  LONG_PTR lParam2
    )
{
    LOG((MSP_TRACE, "CMSPStream::ProcessGraphEvent - enter"));
    LOG((MSP_TRACE, "CMSPStream::ProcessGraphEvent - exit S_OK"));

    return S_OK;
}

/*++
RegisterPluggableTerminalEventSink

Parameters:
 The terminla interface

Description;
 Is called by SelectTerminal if is a dynamic terminal
--*/
HRESULT CMSPStream::RegisterPluggableTerminalEventSink(
    IN  ITTerminal* pTerminal
    )
{
    LOG((MSP_TRACE, "CMSPStream::RegisterPluggableTerminalEventSink - enter"));

    //
    // Validates arguments
    //

    if( ! pTerminal)
    {
        LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
            "pTerminal invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Get the type of the terminal
    //

    TERMINAL_TYPE nTerminalType;
    HRESULT hr = E_FAIL;

    hr = pTerminal->get_TerminalType( &nTerminalType );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
            "get_TerminalType failed, exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    //
    // The terminal should by a dynamic terminal
    //

    if( TT_DYNAMIC != nTerminalType)
    {
        LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
            "terminal is not dynamic, exit E_INVALIDARG"));
        return E_INVALIDARG;
    }

    
    CLock lock(m_lock);


    //
    // Create the sink if we don't have one already
    //

    if(NULL == m_pPTEventSink)
    {
        //Create a PTEventSink object
        CComObject<CPTEventSink>* pPTEventSink;
        hr = CComObject<CPTEventSink>::CreateInstance(&pPTEventSink);

        if( FAILED(hr) )
        {

            LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
                "CreateInstance failed, returns E_OUTOFMEMORY"));
            return E_OUTOFMEMORY;
        }

        
        // tell sink that we are ready to be processing its events

        hr = pPTEventSink->SetSinkStream(this);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
                "event sink refused to accept sink stream. hr = %lx", hr));
            
            delete pPTEventSink;

            return hr;
        }


        // Get ITPluggableTerminalEventSink interface from the sink

        hr = pPTEventSink->QueryInterface(IID_ITPluggableTerminalEventSink, (void**)&m_pPTEventSink);

        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
                "QI for ITPluggableTerminalEventSink failed, returns E_UNEXPECTED"));


            //
            // ok, the sink is no good. get rid of it.
            //

            pPTEventSink->SetSinkStream(NULL);
            delete pPTEventSink;
            pPTEventSink = NULL;


            //
            // sink does not expose IID_ITPluggableTerminalEventSink interface? 
            // something is seriously wrong
            //

            return E_UNEXPECTED;
        }


    }

    
    // Get the ITDTEventHandler interface
    ITPluggableTerminalEventSinkRegistration*   pEventRegistration = NULL;

    hr = pTerminal->QueryInterface( IID_ITPluggableTerminalEventSinkRegistration, 
        (void**)&pEventRegistration
        );

    if( FAILED(hr) )
    {
        // The dynamic terminal doesn't implement ITPluggableTerminalEventSinkRegistration
        // This is bad! We cannot use the new event stuff
        LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
           "QI for ITPluggableTerminalEventSinkregistration failed, returns S_FALSE"));

        //
        // no need to keep the sink
        //

        ReleaseSink();

        return S_FALSE;
    }

    // pass the sink to the terminal
    hr = pEventRegistration->RegisterSink(
        m_pPTEventSink
        );


    // Clean up, anyway
    pEventRegistration->Release();

    if( FAILED(hr) )
    {

        LOG((MSP_ERROR, "CMSPStream::RegisterPluggableTerminalEventSink "
           "RegisterSink failed, returns E_FAIL"));


        //
        // no need to keep the sink
        //

        ReleaseSink();

        return E_FAIL;
    }

    LOG((MSP_TRACE, "CMSPStream::RegisterPluggableTerminalEventSink - exit S_OK"));
    return S_OK;
}

/*++
UnregisterPluggableTerminalEventSink

Parameters:
 The terminal interface

Description;
 Is called by UnselectTerminal if is a dynamic terminal
--*/
HRESULT CMSPStream::UnregisterPluggableTerminalEventSink(
    IN  ITTerminal* pTerminal
    )
{
    LOG((MSP_TRACE, "CMSPStream::UnregisterPluggableTerminalEventSink - enter"));

        //
    // Validates arguments
    //

    if( ! pTerminal)
    {
        LOG((MSP_ERROR, "CMSPStream::UnregisterPluggableTerminalEventSink "
            "pTerminal invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Get the type of the terminal
    //

    TERMINAL_TYPE nTerminalType;
    HRESULT hr = E_FAIL;

    hr = pTerminal->get_TerminalType( &nTerminalType );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPStream::UnregisterPluggableTerminalEventSink "
            "get_TerminalType failed, exit E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    //
    // The terminal should be a dynamic terminal
    //

    if( TT_DYNAMIC != nTerminalType)
    {
        LOG((MSP_ERROR, "CMSPStream::UnregisterPluggableTerminalEventSink "
            "terminal is not dynamic, exit E_INVALIDARG"));
        return E_INVALIDARG;
    }


    CLock lock(m_lock);


    //
    // Have we an EventSink object
    //

    if(NULL == m_pPTEventSink)
    {
        LOG((MSP_TRACE, "CMSPStream::UnregisterPluggableTerminalEventSink - "
            "No EventSink - exit S_OK"));
        return S_OK;
    }

    //
    // Get the ITPluggableTemrinalEventSinkRegistration interface
    //
    ITPluggableTerminalEventSinkRegistration*   pEventRegistration = NULL;

    hr = pTerminal->QueryInterface( IID_ITPluggableTerminalEventSinkRegistration, 
        (void**)&pEventRegistration
        );

    if( FAILED(hr) )
    {
        //
        // The pluggable terminal doesn't implement ITPluggableTerminalEventSinkRegistration
        // This is bad!

        LOG((MSP_ERROR, "CMSPStream::UnregisterPluggableTerminalEventSink "
           "QI for ITPluggableTerminalEventSinkRegistration failed, returns E_NOTIMPL"));
        return E_NOTIMPL;
    }


    hr = pEventRegistration->UnregisterSink( );

    //
    // Clean up, anyway
    //

    pEventRegistration->Release();

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPStream::UnregisterPluggableTerminalEventSink "
           "UnregisterSink failed, returns E_FAIL"));
        return E_FAIL;
    }

    
    //
    // no longer need this sink
    //

    ReleaseSink();


    LOG((MSP_TRACE, "CMSPStream::UnregisterPluggableTerminalEventSink - exit S_OK"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////
//
// CMSPStream::HandleSinkEvent
//
//
// CPTEventSink calls this method when it has an event for us to process
// HandleSinkEvent delegates event processing to the call, if we have one
//
//////////////////////////////////////////////////////////////////////////////

HRESULT CMSPStream::HandleSinkEvent(MSPEVENTITEM *pEventItem)
{
    LOG((MSP_TRACE, "CMSPStream::HandleSinkEvent - enter"));

    HRESULT hr = TAPI_E_CALLUNAVAIL;


    CLock lock(m_lock);


    if (NULL != m_pMSPCall)
    {

        //
        // we have a call. ask it to process the event
        //

        hr = m_pMSPCall->HandleStreamEvent(pEventItem);
    }
    else
    {

        LOG((MSP_WARN,
            "CMSPStream::HandleSinkEvent - there is no call to pass event to"));
    }


    LOG((MSP_(hr), "CMSPStream::HandleSinkEvent - exit hr = %lx", hr));
    
    return hr;
}


//////////////////////////////////////////////////////////////////////////////
//
// CMSPStream::ReleaseSink
//
//
// this is a helper function that lets go of sink when it no longer needed
//
//////////////////////////////////////////////////////////////////////////////


HRESULT CMSPStream::ReleaseSink()
{
    LOG((MSP_TRACE, "CMSPStream::ReleaseSink - enter"));

    
    HRESULT hr = S_OK;


    CLock lock(m_lock);


    //
    // if sink is present, let it know that we will no longer be available to 
    // process its events and release it
    //

    if( m_pPTEventSink)
    {

        CPTEventSink *pSinkObject = static_cast<CPTEventSink *>(m_pPTEventSink);


        HRESULT hr = pSinkObject->SetSinkStream(NULL);

        if (FAILED(hr))
        {
            
            LOG((MSP_ERROR, 
                "CMSPStream::ReleaseSink - pSinkObject->SetSinkStream failed. hr - %lx", 
                hr));
        }


        m_pPTEventSink->Release();
        m_pPTEventSink = NULL;
    }

    LOG((MSP_(hr), "CMSPStream::ReleaseSink - exit. hr - %lx", hr));

    return hr;
}

#if ((WINVER > 0x501) && (_WIN32_WINNT > 0x501))

//////////////////////////////////////////////////////////////////////////////
//
// CMSPStream::InternalAddRef
//
//
// this is a helper function that lets go of sink when it no longer needed
//
//////////////////////////////////////////////////////////////////////////////

ULONG CMSPStream::InternalAddRef()
{
    LOG((MSP_TRACE, "CMSPStream::InternalAddRef - enter"));

    m_lockRefCount.Lock();


    //
    // if the refcount is zero, return 1. this is the indication that the 
    // addref was called while the object is in its finalrelease or entering
    // destructor. Note that the only time this could happen is when event sink
    // attempts to send an event and addrefs the stream object after the stream
    // objects received its last Release() but before the stream object told 
    // the sink to stop using it (which takes place in stream's destructor).
    //
    // we also need to be able to tell if refcount was 0 because it's a new 
    // object or if the addref was called on an object whose refcount became
    // zero because of a release.
    //

    if ( !m_bFirstAddRef && (0 == m_lMyPersonalRefcount) )
    {

        //
        // the caller (event sink logic) should detect this condition (by the 
        // return value of 1) and not expect that the stream is going to 
        // continue to be valid.
        //

        LOG((MSP_WARN, "CMSPStream::InternalAddRef - current refcount is zero... finalrelease/destructor is probably in progress"));

        m_lockRefCount.Unlock();

        return 1;
    }


    //
    // we have made a transition from non-zero refcount to zero. set the 
    // flag, so that future addrefs know to return 1 in ths case when refcount 
    // is 0
    //

    m_bFirstAddRef = FALSE;


    //
    // since we are inside a lock, no need to use interlocked api
    //

    long lNewRefcountValue = (++m_lMyPersonalRefcount);


    m_lockRefCount.Unlock();

    LOG((MSP_TRACE, "CMSPStream::InternalAddRef - finish. %ld", lNewRefcountValue));

    return lNewRefcountValue;
}


ULONG CMSPStream::InternalRelease()
{
    LOG((MSP_TRACE, "CMSPStream::InternalRelease - enter"));

    m_lockRefCount.Lock();

    // try to catch over-releases
    _ASSERTE(m_lMyPersonalRefcount > 0);


    // we are inside a lock, no need to use interlocked api 
    long lNewRefcount = (--m_lMyPersonalRefcount);


    m_lockRefCount.Unlock();

    LOG((MSP_TRACE, "CMSPStream::InternalRelease - finish. %ld", lNewRefcount));

    return lNewRefcount;
}

#endif // ((WINVER > 0x501) && (_WIN32_WINNT > 0x501))


// eof
