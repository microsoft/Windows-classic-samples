/*++

Copyright (c) 1998-1999  Microsoft Corporation

Module Name:

    MSPCall.cpp 

Abstract:

    This module contains implementation of CMSPCall.

--*/

#include "precomp.h"
#pragma hdrstop

/////////////////////////////////////////////////////////////////////////////
// CMSPCallBase
/////////////////////////////////////////////////////////////////////////////

CMSPCallBase::CMSPCallBase()
    : m_pMSPAddress(NULL),
      m_htCall(NULL),
      m_dwMediaType(0)
{
    LOG((MSP_TRACE, "CMSPCallBase::CMSPCallBase[%p] entered.", this));

    LOG((MSP_TRACE, "CMSPCallBase::CMSPCallBase exited."));
}

    
CMSPCallBase::~CMSPCallBase()
{
    LOG((MSP_TRACE, "CMSPCallBase::~CMSPCallBase[%p] entered.", this));


    // We wait until destructor to release the address because
    // they might be used by calls from the stream. If the last stream
    // has released its reference, this pointer will not be used again.

    // If the MSPAddress had a refcount on the call, it should have been
    // released in the ShutdownMSPCall() method. 

    // release the address 
    if (m_pMSPAddress != NULL)
    {
        LOG((MSP_TRACE, "CMSPCallBase::~CMSPCallBase releasing address [%p].", m_pMSPAddress));    

        m_pMSPAddress->MSPAddressRelease();
    }

    LOG((MSP_TRACE, "CMSPCallBase::~CMSPCallBase exited."));
}

// ITStreamControl methods, called by the app.
STDMETHODIMP CMSPCallBase::EnumerateStreams(
    OUT     IEnumStream **      ppEnumStream
    )
{
    LOG((MSP_TRACE, 
        "EnumerateStreams entered. ppEnumStream:%x", ppEnumStream));

    //
    // Check parameters.
    //

    if (!ppEnumStream)
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStreams - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

    //
    // First see if this call has been shut down.
    // acquire the lock before accessing the stream object list.
    //

    CLock lock(m_lock);

    if (m_Streams.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStreams - "
            "call appears to have been shut down - exit E_UNEXPECTED"));

        // This call has been shut down.
        return E_UNEXPECTED;
    }

    //
    // Create an enumerator object.
    //

    typedef _CopyInterface<ITStream> CCopy;
    typedef CSafeComEnum<IEnumStream, &IID_IEnumStream, 
                ITStream *, CCopy> CEnumerator;

    HRESULT hr;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);
    if (pEnum == NULL)
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStreams - "
            "Could not create enumerator object, %x", hr));

        return hr;
    }

    //
    // query for the IID_IEnumStream i/f
    //

    hr = pEnum->_InternalQueryInterface(IID_IEnumStream, (void**)ppEnumStream);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStreams - "
            "query enum interface failed, %x", hr));

        delete pEnum;
        return hr;
    }

    //
    // Init the enumerator object. The CSafeComEnum can handle zero-sized
    // array.
    //

    hr = pEnum->Init(
        m_Streams.GetData(),                        // the begin itor
        m_Streams.GetData() + m_Streams.GetSize(),  // the end itor, 
        NULL,                                       // IUnknown
        AtlFlagCopy                                 // copy the data.
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStreams - "
            "init enumerator object failed, %x", hr));

        (*ppEnumStream)->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CMSPCallBase::EnumerateStreams - exit S_OK"));

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// return a VB collection of streams
//

STDMETHODIMP CMSPCallBase::get_Streams(
    OUT     VARIANT *           pVariant
    )
{
    LOG((MSP_TRACE, "CMSPCallBase::get_Streams - enter"));

    //
    // Check parameters.
    //

    if ( !pVariant)
    {
        LOG((MSP_ERROR, "CMSPCallBase::get_Streams - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

    //
    // See if this call has been shut down. Acquire the lock before accessing
    // the stream object list.
    //

    CLock lock(m_lock);

    if (m_Streams.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CMSPCallBase::get_Streams - "
            "call appears to have been shut down - exit E_UNEXPECTED"));

        // This call has been shut down.
        return E_UNEXPECTED;
    }

    //
    // create the collection object - see mspcoll.h
    //

    typedef CTapiIfCollection< ITStream * > StreamCollection;
    CComObject<StreamCollection> * pCollection;
    HRESULT hr = CComObject<StreamCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPCallBase::get_Streams - "
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
        LOG((MSP_ERROR, "CMSPCallBase::get_Streams - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

    //
    // Init the collection using an iterator -- pointers to the beginning and
    // the ending element plus one.
    //

    hr = pCollection->Initialize( m_Streams.GetSize(),
                                  m_Streams.GetData(),
                                  m_Streams.GetData() + m_Streams.GetSize() );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPCallBase::get_Streams - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();
        return hr;
    }

    //
    // put the IDispatch interface pointer into the variant
    //

    LOG((MSP_INFO, "CMSPCallBase::get_Streams - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPCallBase::get_Streams - exit S_OK"));
 
    return S_OK;
}

// methods called by the MSPstream object.
HRESULT CMSPCallBase::HandleStreamEvent(
    IN      MSPEVENTITEM *     pEventItem
    ) const
{
    _ASSERTE(!!pEventItem);

    pEventItem->MSPEventInfo.hCall = m_htCall;
    return m_pMSPAddress->PostEvent(pEventItem);
}

STDMETHODIMP CMSPCallBase::CreateStream(
    IN      long                lMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    IN OUT  ITStream **         ppStream
    )
/*++

Routine Description:


Arguments:

  
Return Value:

S_OK

E_POINTER
E_OUTOFMEMORY
TAPI_E_INVALIDMEDIATYPE
TAPI_E_INVALIDTERMINALDIRECTION
TAPI_E_INVALIDTERMINALCLASS

--*/
{
    LOG((MSP_TRACE, 
        "CreateStream--dwMediaType:%x, Direction:%x, ppStream %x",
        lMediaType, Direction, ppStream
        ));
 
    if ( ! IsValidSingleMediaType( (DWORD) lMediaType, m_dwMediaType) )
    {
        LOG((MSP_ERROR, 
            "wrong media type:%x, call media type:%x",
            lMediaType, m_dwMediaType
            ));

        return TAPI_E_INVALIDMEDIATYPE;
    }

    if (!ppStream)
    {
        LOG((MSP_ERROR, "Bad pointer, ppStream:%x",ppStream));

        return E_POINTER;
    }

    return InternalCreateStream( (DWORD) lMediaType, Direction, ppStream);
}

HRESULT CMSPCallBase::ReceiveTSPCallData(
        IN      PBYTE               pBuffer,
        IN      DWORD               dwSize
        )
/*++

Routine Description:

  Base class receive TSP call data method... does nothing in base class.
  Implemented so that MSP's that only communicate per-address don't have
  to override it.

Arguments:

  
Return Value:

S_OK

--*/

{
    LOG((MSP_TRACE, "CMSPCallBase::ReceiveTSPCallData - enter"));
    LOG((MSP_TRACE, "CMSPCallBase::ReceiveTSPCallData - exit S_OK"));

    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// Debugging utilities
/////////////////////////////////////////////////////////////////////////////

#ifdef DBGGRAPH

HRESULT
SetGraphLogFile(
    IN IGraphBuilder *pIGraphBuilder
    )
/*++

Routine Description:

    Set the log file for the filter graph.

Arguments:
    
    pIGraphBuilder - The filter graph.

Return Value:

    HRESULT.

--*/
{
    const TCHAR GRAPHLOGPATH[] = _T("c:\\temp\\graph.log");

    HANDLE hFile = CreateFile(
        GRAPHLOGPATH,
        GENERIC_WRITE,
        FILE_SHARE_READ, // sharing
        NULL, // no security
        OPEN_ALWAYS,
        0,    // no attributes, no flags
        NULL  // no template
        );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        LOG((MSP_ERROR, 
            "Can not open graph log file: %s, %x", 
            GRAPHLOGPATH, 
            GetLastError()
            ));
        return S_FALSE;
    }
    
    SetFilePointer(hFile, 0, NULL, FILE_END);
    HRESULT hr = pIGraphBuilder->SetLogFile(hFile);

    return hr;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CMSPCallMultiGraph
/////////////////////////////////////////////////////////////////////////////

CMSPCallMultiGraph::CMSPCallMultiGraph()
    : CMSPCallBase()
{
    LOG((MSP_TRACE, "CMSPCallMultiGraph::CMSPCallMultiGraph entered."));
    LOG((MSP_TRACE, "CMSPCallMultiGraph::CMSPCallMultiGraph exited."));
}
    
CMSPCallMultiGraph::~CMSPCallMultiGraph()
{
    LOG((MSP_TRACE, "CMSPCallMultiGraph::~CMSPCallMultiGraph entered."));

    LOG((MSP_TRACE, "CMSPCallMultiGraph::~CMSPCallMultiGraph exited."));
}

// methods called by the MSPAddress object.
HRESULT CMSPCallMultiGraph::Init(
    IN      CMSPAddress *       pMSPAddress,
    IN      MSP_HANDLE          htCall,
    IN      DWORD               dwReserved,
    IN      DWORD               dwMediaType
    )
/*++

Routine Description:

This method is called by CMSPAddress when the call is first created. 
It creates a filter graph for the streams. It gets the event handle from 
the graph and posts it to the thread pool. The derived method is supposed
to create its own streams based one the mediatypes.

Arguments:
    

Return Value:

    HRESULT.

--*/
{
    LOG((MSP_TRACE, 
        "MSP call %x initialize entered, pMSPAddress:%x",
        this, pMSPAddress));

    // No need to acquire locks on this call because it is called only
    // once when the object is created. No other calls can be made on
    // this object at this point.
    _ASSERTE(m_pMSPAddress == NULL);

    // initialize the stream array so that the array is not NULL.
    if (!m_Streams.Grow())
    {
        return E_OUTOFMEMORY;
    }

    pMSPAddress->MSPAddressAddRef();
    m_pMSPAddress   = pMSPAddress;
    m_htCall        = htCall;
    m_dwMediaType   = dwMediaType;

    return S_OK;   
}


HRESULT CMSPCallMultiGraph::ShutDown()
/*++

Routine Description:

Cancel the event waiting and then call the base impelmentaion. Call the
shutdown on the stream objects. Release the references on all the stream 
objects. Acquires the lock in the function.

Arguments:
    
    pIGraphBuilder - The filter graph.

Return Value:

    HRESULT.

--*/

{
    LOG((MSP_TRACE, "MSP call %x is shutting down", this));

    // acquire the lock on the terminal data because we are writing to it.
    m_lock.Lock();

    // release all the streams
    for (int i = m_Streams.GetSize() - 1; i >= 0; i --)
    {
        UnregisterWaitEvent(i);

        ((CMSPStream*)m_Streams[i])->ShutDown();
        m_Streams[i]->Release();
    }
    m_Streams.RemoveAll();
    m_ThreadPoolWaitBlocks.RemoveAll();

    m_lock.Unlock();

    return S_OK;
}

HRESULT CMSPCallMultiGraph::InternalCreateStream(
    IN      DWORD               dwMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    IN OUT  ITStream **         ppStream
    )
/*++

Routine Description:


Arguments:

  
Return Value:

S_OK

E_POINTER
E_OUTOFMEMORY
TAPI_E_INVALIDMEDIATYPE
TAPI_E_INVALIDTERMINALDIRECTION
TAPI_E_INVALIDTERMINALCLASS

--*/
{
    // Create a filter graph and get the media event interface.
    CComPtr <IMediaEvent> pIMediaEvent;
    HRESULT hr = CoCreateInstance(
            CLSID_FilterGraph,     
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IMediaEvent,
            (void **) &pIMediaEvent
            );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "create filter graph %x", hr));
        return hr;
    }

    ITStream * pITStream;
    hr = CreateStreamObject(
        dwMediaType, 
        Direction, 
        pIMediaEvent, 
        &pITStream
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateStreamObject returned:%x",hr));
        return hr;
    }

    // Add the stream into our list of streams.
    m_lock.Lock();
    if (!m_Streams.Add(pITStream))
    {
        ((CMSPStream*)pITStream)->ShutDown();
        pITStream->Release();
        
        m_lock.Unlock();

        LOG((MSP_ERROR, "out of memory is adding a stream."));
        return E_OUTOFMEMORY;
    }

    // register the new graph and stream to the thread pool for graph events.
    hr = RegisterWaitEvent(pIMediaEvent, pITStream);

    if (FAILED(hr))
    {
        ((CMSPStream*)pITStream)->ShutDown();
        pITStream->Release();

        m_Streams.Remove(pITStream);

        m_lock.Unlock();

        LOG((MSP_ERROR, "Register wait returned %x.", hr));
        return hr;
    }
    m_lock.Unlock();

    // AddRef the interface pointer and return it.
    pITStream->AddRef(); 
    *ppStream = pITStream;

    return S_OK;
}

HRESULT CMSPCallMultiGraph::RegisterWaitEvent(
    IN  IMediaEvent *   pIMediaEvent,
    IN  ITStream *      pITStream
    )
{
    // This function should only be called within a critical section 
    // on the object.

    HANDLE hEvent;
    HRESULT hr = pIMediaEvent->GetEventHandle((OAEVENT*)&hEvent);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Can not get the event handle. %x", hr));
        return hr;
    }

    THREADPOOLWAITBLOCK WaitBlock;
    ZeroMemory(&WaitBlock, sizeof WaitBlock);

    WaitBlock.pContext = (MSPSTREAMCONTEXT *)malloc(sizeof(MSPSTREAMCONTEXT));
    if (WaitBlock.pContext == NULL)
    {
        LOG((MSP_ERROR, "out of memory for the context."));
        return E_OUTOFMEMORY;
    }

    if (!m_ThreadPoolWaitBlocks.Add(WaitBlock))
    {
        free(WaitBlock.pContext);

        LOG((MSP_ERROR, "out of memory adding the waitblock."));
        return E_OUTOFMEMORY;
    }

    // increment the ref count before posting the callback to the thread pool.
    // but for the call, use our special inner object addref.

    this->MSPCallAddRef();
    pITStream->AddRef();
    pIMediaEvent->AddRef();

    WaitBlock.pContext->pMSPCall        = this;
    WaitBlock.pContext->pITStream       = pITStream;
    WaitBlock.pContext->pIMediaEvent    = pIMediaEvent;

    //
    // post the event to the thread pool to wait on.
    //

    HANDLE hWaitHandle = NULL;
    
    BOOL fSuccess = RegisterWaitForSingleObject(
        & hWaitHandle,          // pointer to the returned handle
        hEvent,                 // the event handle to wait for.
        DispatchGraphEvent,     // the callback function.
        WaitBlock.pContext,     // the context for the callback.
        INFINITE,               // wait forever.
        WT_EXECUTEINWAITTHREAD  // use the wait thread to call the callback.
        );

    if ( ( ! fSuccess ) || (hWaitHandle == NULL) )
    {
        LOG((MSP_ERROR, 
            "Register wait call back failed. %x", GetLastError()));

        // decrement the ref count if the posting failed.
        this->MSPCallRelease();
        pITStream->Release();
        pIMediaEvent->Release();

        // Free the context block;
        free(WaitBlock.pContext);
        m_ThreadPoolWaitBlocks.Remove(WaitBlock);

        return E_FAIL;
    }

    // If register succeeded, save the wait handle. We know it is the last one.
    m_ThreadPoolWaitBlocks[m_ThreadPoolWaitBlocks.GetSize() - 1].hWaitHandle 
        = hWaitHandle;

    return S_OK;
}

STDMETHODIMP CMSPCallMultiGraph::RemoveStream(
    IN      ITStream *         pStream
    )
/*++

Routine Description:


Arguments:

  
Return Value:

S_OK

E_INVALIDARG

--*/
{
    LOG((MSP_TRACE, "CMSPCallMultiGraph::RemoveStream - pStream %x", pStream));

    // acquire the lock before accessing the stream object list.
    CLock lock(m_lock);

    int index = m_Streams.Find(pStream);
    if (index < 0)
    {
        LOG((MSP_ERROR, "CMSPCallMultiGraph::RemoveStream - Stream %x is not found.", pStream));
        return E_INVALIDARG;
    }

    UnregisterWaitEvent(index);    

    ((CMSPStream*)m_Streams[index])->ShutDown();
    m_Streams[index]->Release();

    m_Streams.RemoveAt(index);

    LOG((MSP_TRACE, "CMSPCallMultiGraph::RemoveStream - exit S_OK"));

    return S_OK;
}

HRESULT CMSPCallMultiGraph::UnregisterWaitEvent(
    IN  int     index
    )
{
    if (index >= m_ThreadPoolWaitBlocks.GetSize())
    {
        // the call must have been disconnected.
        return E_UNEXPECTED;
    }

    THREADPOOLWAITBLOCK &WaitBlock = m_ThreadPoolWaitBlocks[index];

    // These pointers should never be NULL.
    _ASSERTE(WaitBlock.hWaitHandle != NULL);
    _ASSERTE(WaitBlock.pContext != NULL);

    // Cancel the wait posted to the thread pool.
    BOOL fRes = ::UnregisterWaitEx(WaitBlock.hWaitHandle, (HANDLE)-1);
    if (!fRes)
    {
        // we should never get here, UnregisterWaitEx will block until success
        
        LOG((MSP_ERROR, 
            "UnregisterWait failed. %x", GetLastError()));

        // just remove it from the list. keep the data so that it won't AV.
        m_ThreadPoolWaitBlocks.RemoveAt(index);
        return E_FAIL;
    }

    // We need to decrement the refcount because it was incremented 
    // before we post the wait.
    (WaitBlock.pContext->pMSPCall)->MSPCallRelease();
    (WaitBlock.pContext->pITStream)->Release();
    (WaitBlock.pContext->pIMediaEvent)->Release();

    // Free the context block;
    free(WaitBlock.pContext);

    m_ThreadPoolWaitBlocks.RemoveAt(index);

    return S_OK;
}

// methods called by the thread pool
VOID NTAPI CMSPCallMultiGraph::DispatchGraphEvent(
    IN      VOID *              pContext,
    IN      BOOLEAN             bFlag
    )
{
    LOG((MSP_EVENT, 
        "DispatchGraphEvent:pContext:%x, bFlag:%u", 
        pContext, bFlag));

    // the pContext is a pointer to a call, since it carries a ref count,
    // the call should still be alive.
    _ASSERTE( ! !pContext);

    MSPSTREAMCONTEXT * pEventContext = (MSPSTREAMCONTEXT *)pContext;
    pEventContext->pMSPCall->HandleGraphEvent(pEventContext);
}

VOID CMSPCallMultiGraph::HandleGraphEvent(
    IN  MSPSTREAMCONTEXT * pContext
    )
{
    long     lEventCode;
    LONG_PTR lParam1, lParam2; // win64 fix

    HRESULT hr = pContext->pIMediaEvent->GetEvent(&lEventCode, &lParam1, &lParam2, 0);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Can not get the actual event. %x", hr));
        return;
    }

    LOG((MSP_EVENT, "ProcessGraphEvent, code:%d param1:%x param2:%x",
        lEventCode, lParam1, lParam2));

    //
    // Create an event data structure that we will pass to the worker thread.
    //

    MULTI_GRAPH_EVENT_DATA * pData;
    pData = new MULTI_GRAPH_EVENT_DATA;
    
    if (pData == NULL)
    {
        pContext->pIMediaEvent->FreeEventParams(lEventCode, lParam1, lParam2);

        LOG((MSP_ERROR, "Out of memory for event data."));
        return;
    }
    
    pData->pCall      = this;
    pData->pITStream  = pContext->pITStream;
    pData->lEventCode = lEventCode;
    pData->lParam1    = lParam1;
    pData->lParam2    = lParam2;


    //
    // also pass an addref'ed pointer to IMediaEvent, so that whoever processes
    // the message has the opportunity to free event parameters
    //

    pData->pIMediaEvent = pContext->pIMediaEvent;
    pData->pIMediaEvent->AddRef();

 
    //
    // Make sure the call and stream don't go away while we handle the event.
    // but use our special inner object addref for the call
    //

    pData->pCall->MSPCallAddRef();
    pData->pITStream->AddRef();

    //
    // Queue an async work item to call ProcessGraphEvent.
    //

    hr = g_Thread.QueueWorkItem(AsyncMultiGraphEvent,
                                (void *) pData,
                                FALSE);  // asynchronous

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "QueueWorkItem failed, return code:%x", hr));

        pData->pCall->MSPCallRelease();
        pData->pITStream->Release();


        //
        // no one is going to free event params and release the IMediaEvent
        // pointer, so do it here
        //

        pContext->pIMediaEvent->FreeEventParams(lEventCode, lParam1, lParam2);
        pData->pIMediaEvent->Release();

        delete pData;
    }
}

DWORD WINAPI AsyncMultiGraphEvent(LPVOID pVoid)
{
    MULTI_GRAPH_EVENT_DATA * pData = ( MULTI_GRAPH_EVENT_DATA * ) pVoid;

    //
    // Handle the event.
    //

    (pData->pCall)->ProcessGraphEvent(pData->pITStream,
                                      pData->lEventCode,
                                      pData->lParam1,
                                      pData->lParam2);

    //
    // These were addrefed when the event was queued.
    // but we used our special inner object addref for the call
    //

    pData->pCall->MSPCallRelease();
    pData->pITStream->Release();


    //
    // if we have IMediaEvent pointer, free event params and release the media 
    // event interface pointer
    //

    if (NULL != pData->pIMediaEvent)
    {
        pData->pIMediaEvent->FreeEventParams(pData->lEventCode,
                                             pData->lParam1,
                                             pData->lParam2);
        pData->pIMediaEvent->Release();
        pData->pIMediaEvent = NULL;
    }

    //
    // Free the event data structure.
    //

    delete pData;

    return 0;
}

HRESULT CMSPCallMultiGraph::ProcessGraphEvent(
    IN      ITStream *      pITStream,
    IN      long            lEventCode,
    IN      LONG_PTR        lParam1,
    IN      LONG_PTR        lParam2
    )
{
    CLock lock(m_lock);

    if (m_Streams.Find(pITStream) < 0)
    {
        LOG((MSP_WARN, 
            "stream %x is already removed.", 
            pITStream));
        return TAPI_E_NOITEMS;
    }

    //
    // No dynamic cast because this is our own pointer.
    //

    return ((CMSPStream*)pITStream)->
        ProcessGraphEvent(lEventCode, lParam1, lParam2);
}
