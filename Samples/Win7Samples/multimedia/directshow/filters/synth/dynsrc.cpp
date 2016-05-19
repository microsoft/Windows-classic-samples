//------------------------------------------------------------------------------
// File: DynSrc.cpp
//
// Desc: DirectShow sample code - implements CDynamicSource, which is a
//       Quartz source filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


// Locking Strategy.
//
// Hold the filter critical section (m_pFilter->pStateLock()) to serialise
// access to functions. Note that, in general, this lock may be held
// by a function when the worker thread may want to hold it. Therefore
// if you wish to access shared state from the worker thread you will
// need to add another critical section object. The execption is during
// the threads processing loop, when it is safe to get the filter critical
// section from within FillBuffer().

#include <windows.h>
#include <streams.h>

#include "DynSrc.h"


CDynamicSource::CDynamicSource(TCHAR *pName, LPUNKNOWN lpunk, CLSID clsid, HRESULT *phr)
    : CBaseFilter(pName, lpunk, &m_cStateLock, clsid),
      m_iPins(0),
      m_paStreams(NULL),
      m_evFilterStoppingEvent(TRUE)
{
    ASSERT(phr);
    
    // Make sure the event was successfully created.
    if( NULL == (HANDLE)m_evFilterStoppingEvent ) {
        if (phr)
            *phr = E_FAIL;
    }
}


#ifdef UNICODE
CDynamicSource::CDynamicSource(CHAR *pName, LPUNKNOWN lpunk, CLSID clsid, HRESULT *phr)
    : CBaseFilter(pName, lpunk, &m_cStateLock, clsid),
      m_iPins(0),
      m_paStreams(NULL),
      m_evFilterStoppingEvent(TRUE)
{
    ASSERT(phr);

    // Make sure the event was successfully created.
    if( NULL == (HANDLE)m_evFilterStoppingEvent ) {
        if (phr)
            *phr = E_FAIL;
    }
}
#endif


//
// CDynamicSource::Destructor
//
CDynamicSource::~CDynamicSource()
{
    /*  Free our pins and pin array */
    while (m_iPins != 0) 
    {
        // deleting the pins causes them to be removed from the array...
        delete m_paStreams[m_iPins - 1];
    }

    ASSERT(m_paStreams == NULL);
}


//
//  Add a new pin
//
HRESULT CDynamicSource::AddPin(CDynamicSourceStream *pStream)
{
    // This function holds the filter state lock because
    // it changes the number of pins.  The number of pins
    // cannot be change while another thread is calling
    // IMediaFilter::Run(), IMediaFilter::Stop(), 
    // IMediaFilter::Pause() or IBaseFilter::FindPin().
    CAutoLock lock(&m_cStateLock);

    // This function holds the pin state lock because it
    // uses m_iPins and m_paStreams.
    CAutoLock alPinStateLock(&m_csPinStateLock);

    /*  Allocate space for this pin and the old ones */
    CDynamicSourceStream **paStreams = new CDynamicSourceStream *[m_iPins + 1];
    if (paStreams == NULL)
        return E_OUTOFMEMORY;

    if (m_paStreams != NULL) 
    {
        CopyMemory((PVOID)paStreams, (PVOID)m_paStreams,
                   m_iPins * sizeof(m_paStreams[0]));

        paStreams[m_iPins] = pStream;
        delete [] m_paStreams;
    }

    m_paStreams = paStreams;
    m_paStreams[m_iPins] = pStream;
    m_iPins++;

    return S_OK;
}


//
//  Remove a pin - pStream is NOT deleted
//
HRESULT CDynamicSource::RemovePin(CDynamicSourceStream *pStream)
{
    // This function holds the filter state lock because
    // it changes the number of pins.  The number of pins
    // cannot be change while another thread is calling
    // IMediaFilter::Run(), IMediaFilter::Stop(), 
    // IMediaFilter::Pause() or IBaseFilter::FindPin().
    CAutoLock lock(&m_cStateLock);    

    // This function holds the pin state lock because it
    // uses m_iPins and m_paStreams.
    CAutoLock alPinStateLock(&m_csPinStateLock);

    for(int i = 0; i < m_iPins; i++)
    {
        if(m_paStreams[i] == pStream)
        {
            if(m_iPins == 1)
            {
                delete [] m_paStreams;
                m_paStreams = NULL;
            }
            else
            {
                /*  no need to reallocate */
                while(++i < m_iPins)
                    m_paStreams[i - 1] = m_paStreams[i];
            }

            m_iPins--;
            return S_OK;
        }
    }

    return S_FALSE;
}


//
// FindPin
//
// Set *ppPin to the IPin* that has the id Id.
// or to NULL if the Id cannot be matched.
STDMETHODIMP CDynamicSource::FindPin(LPCWSTR Id, IPin **ppPin)
{
    CAutoLock alPinStateLock(&m_csPinStateLock);

    CheckPointer(ppPin,E_POINTER);
    ValidateReadWritePtr(ppPin,sizeof(IPin *));

    // The -1 undoes the +1 in QueryId and ensures that totally invalid
    // strings (for which WstrToInt delivers 0) give a deliver a NULL pin.
    int i = WstrToInt(Id) -1;

    *ppPin = GetPin(i);
    if (*ppPin!=NULL)
    {
        (*ppPin)->AddRef();
        return NOERROR;
    }

    return VFW_E_NOT_FOUND;
}


//
// FindPinNumber
//
// return the number of the pin with this IPin* or -1 if none
int CDynamicSource::FindPinNumber(IPin *iPin) 
{

    // This function holds the pin state lock because it
    // uses m_iPins and m_paStreams.
    CAutoLock alPinStateLock(&m_csPinStateLock);

    for (int i=0; i<m_iPins; ++i) 
    {
        if ((IPin *)(m_paStreams[i]) == iPin) 
        {
            return i;
        }
    }

    return -1;
}


//
// GetPinCount
//
// Returns the number of pins this filter has
int CDynamicSource::GetPinCount(void) 
{

    // This function holds the pin state lock because it
    // uses m_iPins.
    CAutoLock alPinStateLock(&m_csPinStateLock);
    return m_iPins;
}


//
// GetPin
//
// Return a non-addref'd pointer to pin n
// needed by CBaseFilter
CBasePin *CDynamicSource::GetPin(int n)
{
    // This function holds the pin state lock because it
    // uses m_iPins and m_paStreams.
    CAutoLock alPinStateLock(&m_csPinStateLock);

    // n must be in the range 0..m_iPins-1
    // if m_iPins>n  && n>=0 it follows that m_iPins>0
    // which is what used to be checked (i.e. checking that we have a pin)
    if((n >= 0) && (n < m_iPins))
    {
        ASSERT(m_paStreams[n]);
        return m_paStreams[n];
    }

    return NULL;
}


STDMETHODIMP CDynamicSource::Stop(void)
{
    m_evFilterStoppingEvent.Set();

    HRESULT hr = CBaseFilter::Stop();

    // The following code ensures that a pins thread will be destroyed even 
    // if the pin is disconnected when CBaseFilter::Stop() is called.
    int nCurrentPin;
    CDynamicSourceStream* pOutputPin;
    {
        // This code holds the pin state lock because it
        // does not want the number of pins to change 
        // while it executes.
        CAutoLock alPinStateLock(&m_csPinStateLock);

        for(nCurrentPin = 0; nCurrentPin < GetPinCount(); nCurrentPin++)
        {
            pOutputPin = (CDynamicSourceStream*)GetPin(nCurrentPin);
            if(pOutputPin->ThreadExists())
            {
                pOutputPin->DestroySourceThread();
            }
        }
    }

    if(FAILED(hr))
    {
        return hr;
    }

    return NOERROR;
}


STDMETHODIMP CDynamicSource::Pause(void)
{
    m_evFilterStoppingEvent.Reset();

    return CBaseFilter::Pause();
}


STDMETHODIMP CDynamicSource::JoinFilterGraph(IFilterGraph* pGraph, LPCWSTR pName)
{
    CAutoLock lock(&m_cStateLock);

    HRESULT hr;
    int nCurrentPin;
    CDynamicSourceStream* pOutputPin;

    // The filter is joining the filter graph.
    if(NULL != pGraph)
    {
        IGraphConfig* pGraphConfig = NULL;

        hr = pGraph->QueryInterface(IID_IGraphConfig, (void**)&pGraphConfig);
        if(FAILED(hr))
        {
            return hr;
        }

        hr = CBaseFilter::JoinFilterGraph(pGraph, pName);
        if(FAILED(hr))
        {
            pGraphConfig->Release();
            return hr;
        }

        for(nCurrentPin = 0; nCurrentPin < GetPinCount(); nCurrentPin++)
        {
            pOutputPin = (CDynamicSourceStream*) GetPin(nCurrentPin);
            pOutputPin->SetConfigInfo(pGraphConfig, m_evFilterStoppingEvent);
        }

        pGraphConfig->Release();
    }
    else
    {
        hr = CBaseFilter::JoinFilterGraph(pGraph, pName);
        if(FAILED(hr))
        {
            return hr;
        }

        for(nCurrentPin = 0; nCurrentPin < GetPinCount(); nCurrentPin++)
        {
            pOutputPin = (CDynamicSourceStream*)GetPin(nCurrentPin);
            pOutputPin->SetConfigInfo(NULL, NULL);
        }
    }

    return S_OK;
}


// *
// * --- CDynamicSourceStream ----
// *

//
// Set Id to point to a CoTaskMemAlloc'd
STDMETHODIMP CDynamicSourceStream::QueryId(LPWSTR *pId)
{
    CheckPointer(pId,E_POINTER);
    ValidateReadWritePtr(pId,sizeof(LPWSTR));

    // We give the pins id's which are 1,2,...
    // FindPinNumber returns -1 for an invalid pin
    int i = 1+ m_pFilter->FindPinNumber(this);

    if(i<1) 
        return VFW_E_NOT_FOUND;

    *pId = (LPWSTR)CoTaskMemAlloc(4*sizeof(WCHAR));
    if(*pId==NULL)
    {
        return E_OUTOFMEMORY;
    }

    IntToWstr(i, *pId);
    return NOERROR;
}


//
// CDynamicSourceStream::Constructor
//
// increments the number of pins present on the filter
CDynamicSourceStream::CDynamicSourceStream(
    TCHAR *pObjectName,
    HRESULT *phr,
    CDynamicSource*ps,
    LPCWSTR pPinName)
    : CDynamicOutputPin(pObjectName, ps, ps->pStateLock(), phr, pPinName),
      m_pFilter(ps),
      m_fReconnectOutputPin(false)
{
    ASSERT(phr);   
    *phr = m_pFilter->AddPin(this);
}


#ifdef UNICODE

CDynamicSourceStream::CDynamicSourceStream(
    char *pObjectName,
    HRESULT *phr,
    CDynamicSource*ps,
    LPCWSTR pPinName)
    : CDynamicOutputPin(pObjectName, ps, ps->pStateLock(), phr, pPinName),
      m_pFilter(ps),
      m_fReconnectOutputPin(false)
{
    ASSERT(phr);
    *phr = m_pFilter->AddPin(this);
}

#endif


//
// CDynamicSourceStream::Destructor
//
// Decrements the number of pins on this filter
CDynamicSourceStream::~CDynamicSourceStream(void)
{
    m_pFilter->RemovePin(this);
}


//
// CheckMediaType
//
// Do we support this type? Provides the default support for 1 type.
HRESULT CDynamicSourceStream::CheckMediaType(const CMediaType *pMediaType)
{
    CheckPointer(pMediaType,E_POINTER);
    CAutoLock lock(m_pFilter->pStateLock());

    CMediaType mt;
    GetMediaType(&mt);

    if(mt == *pMediaType)
    {
        return NOERROR;
    }

    return E_FAIL;
}


//
// GetMediaType
//
// By default we support only one type
// iPosition indexes are 0-n
HRESULT CDynamicSourceStream::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    CAutoLock lock(m_pFilter->pStateLock());

    if(iPosition<0)
    {
        return E_INVALIDARG;
    }
    if(iPosition>0)
    {
        return VFW_S_NO_MORE_ITEMS;
    }

    return GetMediaType(pMediaType);
}


//
// Active
//
// The pin is active - start up the worker thread
HRESULT CDynamicSourceStream::Active(void)
{
    CAutoLock lock(m_pFilter->pStateLock());

    HRESULT hr;

    if(m_pFilter->IsActive())
    {
        return S_FALSE; // succeeded, but did not allocate resources (they already exist...)
    }

    // do nothing if not connected - its ok not to connect to
    // all pins of a source filter
    if(!IsConnected())
    {
        return NOERROR;
    }

    hr = CDynamicOutputPin::Active();
    if(FAILED(hr))
    {
        return hr;
    }

    ASSERT(!ThreadExists());

    // start the thread
    if(!Create())
    {
        return E_FAIL;
    }

    // Tell thread to initialize. If OnThreadCreate Fails, so does this.
    hr = Init();
    if(FAILED(hr))
        return hr;

    return Pause();
}


HRESULT CDynamicSourceStream::BreakConnect(void)
{
    HRESULT hr = CDynamicOutputPin::BreakConnect();
    if(FAILED(hr))
    {
        return hr;
    }

    m_fReconnectOutputPin = false;

    return S_OK;
}


HRESULT CDynamicSourceStream::DestroySourceThread(void)
{
    // The pin's thread cannot be destroyed if the thread does not exist.
    ASSERT(ThreadExists());

    HRESULT hr = Stop();
    if(FAILED(hr))
    {
        return hr;
    }

    hr = Exit();
    if(FAILED(hr))
    {
        return hr;
    }

    Close();    // Wait for the thread to exit, then tidy up.

    return NOERROR;
}


//
// ThreadProc
//
// When this returns the thread exits
// Return codes > 0 indicate an error occured
DWORD CDynamicSourceStream::ThreadProc(void)
{
    HRESULT hr;  // the return code from calls
    Command com;

    do
    {
        com = GetRequest();
        if(com != CMD_INIT)
        {
            DbgLog((LOG_ERROR, 1, TEXT("Thread expected init command")));
            Reply((DWORD) E_UNEXPECTED);
        }

    } while(com != CMD_INIT);

    DbgLog((LOG_TRACE, 1, TEXT("CDynamicSourceStream worker thread initializing")));

    hr = OnThreadCreate(); // perform set up tasks
    if(FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CDynamicSourceStream::OnThreadCreate failed. Aborting thread.")));

        OnThreadDestroy();
        Reply(hr);  // send failed return code from OnThreadCreate
        return 1;
    }

    // Initialisation suceeded
    Reply(NOERROR);

    Command cmd;
    do
    {
        cmd = GetRequest();

        switch(cmd)
        {
            case CMD_EXIT:
                Reply(NOERROR);
                break;

            case CMD_RUN:
                DbgLog((LOG_ERROR, 1, TEXT("CMD_RUN received before a CMD_PAUSE???")));
                // !!! fall through

            case CMD_PAUSE:
                Reply(NOERROR);
                DoBufferProcessingLoop();
                break;

            case CMD_STOP:
                Reply(NOERROR);
                break;

            default:
                DbgLog((LOG_ERROR, 1, TEXT("Unknown command %d received!"), cmd));
                Reply((DWORD) E_NOTIMPL);
                break;
        }

    } while(cmd != CMD_EXIT);

    hr = OnThreadDestroy(); // tidy up.
    if(FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CDynamicSourceStream::OnThreadDestroy failed. Exiting thread.")));
        return 1;
    }

    DbgLog((LOG_TRACE, 1, TEXT("CDynamicSourceStream worker thread exiting")));
    return 0;
}


//
// DoBufferProcessingLoop
//
// Grabs a buffer and calls the users processing function.
// Overridable, so that different delivery styles can be catered for.
HRESULT CDynamicSourceStream::DoBufferProcessingLoop(void)
{
    Command com;
    bool fOutputFormatChanged = false;

    OnThreadStartPlay();

    do
    {
        while(!CheckRequest(&com))
        {
            // CAutoUsingOutputPin::CAutoUsingOutputPin() only changes the value of hr
            // if an error occurs.
            HRESULT hr = S_OK;

            CAutoUsingOutputPin auopUsingOutputPin(this, &hr);
            if(FAILED(hr))
            {
                FatalError(hr);
                return hr;
            }

            if(m_fReconnectOutputPin)
            {
                hr = DynamicReconnect(NULL);

                m_fReconnectOutputPin = false;

                if(FAILED(hr))
                {
                    FatalError(hr);
                    return hr;
                }

                fOutputFormatChanged = true;
            }

            IMediaSample *pSample;

            hr = GetDeliveryBuffer(&pSample,NULL,NULL,0);
            if(FAILED(hr))
            {
                Sleep(1);
                continue;   // go round again. Perhaps the error will go away
                // or the allocator is decommited & we will be asked to
                // exit soon.
            }

            if(fOutputFormatChanged)
            {
                pSample->SetDiscontinuity(TRUE);
                fOutputFormatChanged = false;
            }

            // Virtual function user will override.
            hr = FillBuffer(pSample);

            if(hr == S_OK)
            {
                hr = Deliver(pSample);
                pSample->Release();

                // downstream filter returns S_FALSE if it wants us to
                // stop or an error if it's reporting an error.
                if(hr != S_OK)
                {
                    DbgLog((LOG_TRACE, 2, TEXT("Deliver() returned %08x; stopping"), hr));
                    return S_OK;
                }

            }
            else if(hr == S_FALSE)
            {
                // derived class wants us to stop pushing data
                pSample->Release();
                DeliverEndOfStream();
                return S_OK;
            }
            else
            {
                // derived class encountered an error
                pSample->Release();
                DbgLog((LOG_ERROR, 1, TEXT("Error %08lX from FillBuffer!!!"), hr));

                FatalError(hr);
                return hr;
            }
            // all paths release the sample
        }

        // For all commands sent to us there must be a Reply call!
        if(com == CMD_RUN || com == CMD_PAUSE)
        {
            Reply(NOERROR);
        }
        else if(com != CMD_STOP)
        {
            Reply((DWORD) E_UNEXPECTED);
            DbgLog((LOG_ERROR, 1, TEXT("Unexpected command!!!")));
        }
    } while(com != CMD_STOP);

    return S_FALSE;
}


void CDynamicSourceStream::FatalError(HRESULT hr)
{
    // Make sure the user is reporting a failure.
    ASSERT(FAILED(hr));

    m_bRunTimeError = TRUE;
    DeliverEndOfStream();
    m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
}


void CDynamicSourceStream::OutputPinNeedsToBeReconnected(void)
{
    m_fReconnectOutputPin = true;
}

