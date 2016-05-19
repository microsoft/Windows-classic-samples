//////////////////////////////////////////////////////////////////////////
//
// MPEG1Stream.cpp
// Implements the stream object (IMFMediaStream) for the MPEG-1 source.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "MPEG1Source.h"

#pragma warning( push )
#pragma warning( disable : 4355 )  // 'this' used in base member initializer list


/* MPEG1Stream::SourceLock class methods */

//-------------------------------------------------------------------
// MPEG1Stream::SourceLock constructor - locks the source
//-------------------------------------------------------------------

MPEG1Stream::SourceLock::SourceLock(MPEG1Source *pSource)
    : m_pSource(NULL)
{
    if (pSource)
    {
        m_pSource = pSource;
        m_pSource->AddRef();
        m_pSource->Lock();
    }
}

//-------------------------------------------------------------------
// MPEG1Stream::SourceLock destructor - unlocks the source
//-------------------------------------------------------------------

MPEG1Stream::SourceLock::~SourceLock()
{
    if (m_pSource)
    {
        m_pSource->Unlock();
        m_pSource->Release();
    }
}



/* Public class methods */

//-------------------------------------------------------------------
// IUnknown methods
//-------------------------------------------------------------------

ULONG MPEG1Stream::AddRef() { return RefCountedObject::AddRef(); }

ULONG MPEG1Stream::Release() { return RefCountedObject::Release(); }

HRESULT MPEG1Stream::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(MPEG1Stream, IMFMediaEventGenerator),
        QITABENT(MPEG1Stream, IMFMediaStream),
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}


//-------------------------------------------------------------------
// IMFMediaEventGenerator methods
//
// For remarks, see MPEG1Source.cpp
//-------------------------------------------------------------------

HRESULT MPEG1Stream::BeginGetEvent(IMFAsyncCallback* pCallback,IUnknown* punkState)
{
    HRESULT hr = S_OK;

    SourceLock lock(m_pSource);

    CHECK_HR(hr = CheckShutdown());
    CHECK_HR(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

done:
    return hr;
}

HRESULT MPEG1Stream::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;

    SourceLock lock(m_pSource);

    CHECK_HR(hr = CheckShutdown());
    CHECK_HR(hr = m_pEventQueue->EndGetEvent(pResult, ppEvent));

done:
    return hr;
}

HRESULT MPEG1Stream::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;

    IMFMediaEventQueue *pQueue = NULL;

    { // scope for lock
      
        SourceLock lock(m_pSource);

        // Check shutdown
        CHECK_HR(hr = CheckShutdown());

        // Cache a local pointer to the queue.
        pQueue = m_pEventQueue;
        pQueue->AddRef();

    }   // release lock

    // Use the local pointer to call GetEvent.
    CHECK_HR(hr = pQueue->GetEvent(dwFlags, ppEvent));

done:
    SAFE_RELEASE(pQueue);
    return hr;
}

HRESULT MPEG1Stream::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue)
{
    HRESULT hr = S_OK;

    SourceLock lock(m_pSource);

    CHECK_HR(hr = CheckShutdown());

    CHECK_HR(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

done:
    return hr;
}

//-------------------------------------------------------------------
// IMFMediaStream methods
//-------------------------------------------------------------------


//-------------------------------------------------------------------
// GetMediaSource:
// Returns a pointer to the media source.
//-------------------------------------------------------------------

HRESULT MPEG1Stream::GetMediaSource(IMFMediaSource** ppMediaSource)
{
    SourceLock lock(m_pSource);

    if (ppMediaSource == NULL)
    {
        return E_POINTER;
    }

    if (m_pSource == NULL)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    
    CHECK_HR(hr = CheckShutdown());

    // QI the source for IMFMediaSource.
    // (Does not hold the source's critical section.)
    CHECK_HR(hr = m_pSource->QueryInterface(IID_PPV_ARGS(ppMediaSource)));

done:
    return hr;
}


//-------------------------------------------------------------------
// GetStreamDescriptor:
// Returns a pointer to the stream descriptor for this stream.
//-------------------------------------------------------------------

HRESULT MPEG1Stream::GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor)
{
    SourceLock lock(m_pSource);

    if (ppStreamDescriptor == NULL)
    {
        return E_POINTER;
    }

    if (m_pStreamDescriptor == NULL)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *ppStreamDescriptor = m_pStreamDescriptor;
        (*ppStreamDescriptor)->AddRef();

    };
    return hr;
}


//-------------------------------------------------------------------
// RequestSample:
// Requests data from the stream.
//
// pToken: Token used to track the request. Can be NULL.
//-------------------------------------------------------------------

HRESULT MPEG1Stream::RequestSample(IUnknown* pToken)
{
    HRESULT hr = S_OK;
    IMFMediaSource *pSource = NULL;

    SourceLock lock(m_pSource);

    CHECK_HR(hr = CheckShutdown());

    if (m_state == STATE_STOPPED)
    {
        CHECK_HR(hr = MF_E_INVALIDREQUEST);
    }

    if (!m_bActive)
    {
        // If the stream is not active, it should not get sample requests. 
        CHECK_HR(hr = MF_E_INVALIDREQUEST);
    }

    // Fail if we reached the end of the stream AND the sample queue is empty,
    if (m_bEOS && m_Samples.IsEmpty())
    {
        CHECK_HR(hr = MF_E_END_OF_STREAM);
    }

    CHECK_HR(hr = m_Requests.InsertBack(pToken));

    // Dispatch the request.
    CHECK_HR(hr = DispatchSamples());

done:

    // If there was an error, queue MEError from the source (except after shutdown).
    if (FAILED(hr) && (m_state != STATE_SHUTDOWN))
    {
        hr = m_pSource->QueueEvent(MEError, GUID_NULL, hr, NULL);
    }

    return hr;
}



//-------------------------------------------------------------------
// Public non-interface methods
//-------------------------------------------------------------------

    
MPEG1Stream::MPEG1Stream(MPEG1Source *pSource, IMFStreamDescriptor *pSD, HRESULT& hr) :
    m_pEventQueue(NULL),
    m_state(STATE_STOPPED),
    m_bActive(FALSE),
    m_bEOS(FALSE)
{
    assert(pSource != NULL);
    assert(pSD != NULL);

    m_pSource = pSource;
    m_pSource->AddRef();

    m_pStreamDescriptor = pSD;
    m_pStreamDescriptor->AddRef();

    // Create the media event queue.
    hr = MFCreateEventQueue(&m_pEventQueue);
}

MPEG1Stream::~MPEG1Stream()
{
    assert(m_state == STATE_SHUTDOWN);
    SAFE_RELEASE(m_pSource);
}



//-------------------------------------------------------------------
// Activate
// Activates or deactivates the stream. Called by the media source.
//-------------------------------------------------------------------

HRESULT MPEG1Stream::Activate(BOOL bActive)
{
    SourceLock lock(m_pSource);

    if (bActive == m_bActive)
    {
        return S_OK; // No op
    }

    m_bActive = bActive;

    if (!bActive)
    {
        m_Samples.Clear();
        m_Requests.Clear();
    }
    return S_OK;
}


//-------------------------------------------------------------------
// Start
// Starts the stream. Called by the media source.
//
// varStart: Starting position.
//-------------------------------------------------------------------

HRESULT MPEG1Stream::Start(const PROPVARIANT& varStart)
{
    SourceLock lock(m_pSource);

    HRESULT hr = S_OK;

    CHECK_HR(hr = CheckShutdown());

    // Queue the stream-started event.
    CHECK_HR(hr = QueueEvent(
        MEStreamStarted,
        GUID_NULL,
        S_OK,
        &varStart
        ));

    m_state = STATE_STARTED;

    // If we are restarting from paused, there may be 
    // queue sample requests. Dispatch them now.
    CHECK_HR(hr = DispatchSamples());

done:
    return hr;
}


//-------------------------------------------------------------------
// Pause
// Pauses the stream. Called by the media source.
//-------------------------------------------------------------------

HRESULT MPEG1Stream::Pause()
{
    SourceLock lock(m_pSource);

    HRESULT hr = S_OK;

    CHECK_HR(hr = CheckShutdown());

    m_state = STATE_PAUSED;

    CHECK_HR(hr = QueueEvent(MEStreamPaused, GUID_NULL, S_OK, NULL));

done:
    return hr;
}


//-------------------------------------------------------------------
// Stop
// Stops the stream. Called by the media source.
//-------------------------------------------------------------------

HRESULT MPEG1Stream::Stop()
{
    SourceLock lock(m_pSource);

    HRESULT hr = S_OK;

    CHECK_HR(hr = CheckShutdown());

    m_Requests.Clear();
    m_Samples.Clear();

    m_state = STATE_STOPPED;

    CHECK_HR(hr = QueueEvent(MEStreamStopped, GUID_NULL, S_OK, NULL));

done:
    return hr;
}


//-------------------------------------------------------------------
// EndOfStream
// Notifies the stream that the source reached the end of the MPEG-1
// stream. For more information, see MPEG1Source::EndOfMPEGStream().
//-------------------------------------------------------------------

HRESULT MPEG1Stream::EndOfStream()
{
    SourceLock lock(m_pSource);

    m_bEOS = TRUE;    

    return DispatchSamples();
}


//-------------------------------------------------------------------
// Shutdown
// Shuts down the stream and releases all resources.
//-------------------------------------------------------------------

HRESULT MPEG1Stream::Shutdown()
{
    SourceLock lock(m_pSource);

    HRESULT hr = S_OK;

    CHECK_HR(hr = CheckShutdown());

    m_state = STATE_SHUTDOWN;

    // Shut down the event queue.
    if (m_pEventQueue)
    {
        m_pEventQueue->Shutdown();
    }

    // Release objects.
    m_Samples.Clear();
    m_Requests.Clear();

    SAFE_RELEASE(m_pStreamDescriptor);
    SAFE_RELEASE(m_pEventQueue);

    // NOTE:
    // Do NOT release the source pointer here, because the stream uses
    // it to hold the critical section. In particular, the stream must
    // hold the critical section when checking the shutdown status,
    // which obviously can occur after the stream is shut down.

    // It is OK to hold a ref count on the source after shutdown, 
    // because the source releases its ref count(s) on the streams,
    // which breaks the circular ref count.

done:
    return hr;
}


//-------------------------------------------------------------------
// NeedsData
// Returns TRUE if the stream needs more data.
//-------------------------------------------------------------------

BOOL MPEG1Stream::NeedsData()
{
    SourceLock lock(m_pSource);

    // Note: The stream tries to keep a minimum number of samples
    // queued ahead.

    return (m_bActive && !m_bEOS && (m_Samples.GetCount() < SAMPLE_QUEUE));
}


//-------------------------------------------------------------------
// DeliverPayload
// Delivers a sample to the stream.
//-------------------------------------------------------------------

HRESULT MPEG1Stream::DeliverPayload(IMFSample *pSample)
{
    SourceLock lock(m_pSource);

    HRESULT hr = S_OK;

    // Queue the sample.
    CHECK_HR(hr = m_Samples.InsertBack(pSample));

    // Deliver the sample if there is an outstanding request.
    CHECK_HR(hr = DispatchSamples());
        
done:
    return hr;
}

/* Private methods */

//-------------------------------------------------------------------
// DispatchSamples
// Dispatches as many pending sample requests as possible.
//-------------------------------------------------------------------

HRESULT MPEG1Stream::DispatchSamples()
{
    HRESULT hr = S_OK;
    BOOL bNeedData = FALSE;
    BOOL bEOS = FALSE;

    IMFSample *pSample = NULL;
    IUnknown  *pToken = NULL;

    SourceLock lock(m_pSource);

    // It's possible that an I/O request completed after the source
    // paused, stopped, or shut down. We should not deliver any samples
    // unless the source is running.
    if (m_state != STATE_STARTED)
    {
        hr = S_OK;
        goto done;
    }

    // Deliver as many samples as we can.
    while (!m_Samples.IsEmpty() && !m_Requests.IsEmpty())
    {
        // Pull the next sample from the queue.
        CHECK_HR(hr = m_Samples.RemoveFront(&pSample));

        // Pull the next request token from the queue. Tokens can be NULL.
        CHECK_HR(hr = m_Requests.RemoveFront(&pToken));

        if (pToken)
        {
            CHECK_HR(hr = pSample->SetUnknown(MFSampleExtension_Token, pToken));
        }

        CHECK_HR(hr = m_pEventQueue->QueueEventParamUnk(MEMediaSample, GUID_NULL, S_OK, pSample)); 

        SAFE_RELEASE(pSample);
        SAFE_RELEASE(pToken);
    }

    if (m_Samples.IsEmpty() && m_bEOS)
    {
        // The sample queue is empty AND we have reached the end of the source stream.
        // Notify the pipeline by sending the end-of-stream event.
        CHECK_HR(hr = m_pEventQueue->QueueEventParamVar(MEEndOfStream, GUID_NULL, S_OK, NULL));

        // Also notify the source, so that it can send the end-of-presentation event.
        CHECK_HR(hr = m_pSource->QueueAsyncOperation(SourceOp::OP_END_OF_STREAM));
    }
    else if (NeedsData())
    {
        // The sample queue is empty and the request queue is not empty (and we did not
        // reach the end of the stream). Ask the source for more data.
        CHECK_HR(hr = m_pSource->QueueAsyncOperation(SourceOp::OP_REQUEST_DATA));
    }

done:

    // If there was an error, queue MEError from the source (except after shutdown).
    if (FAILED(hr) && (m_state != STATE_SHUTDOWN))
    {
        m_pSource->QueueEvent(MEError, GUID_NULL, hr, NULL);
    }

    SAFE_RELEASE(pSample);
    SAFE_RELEASE(pToken);
    return S_OK;
}

#pragma warning( pop )