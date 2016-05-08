//////////////////////////////////////////////////////////////////////////
//
// MPEG1Source.h
// Implements the MPEG-1 media source object.
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

//-------------------------------------------------------------------
//
// Notes:
// This sample contains an MPEG-1 source. 
//
// - The source parses MPEG-1 systems-layer streams and generates 
//   samples that contain MPEG-1 payloads.
// - The source does not support files that contain a raw MPEG-1 
//   video or audio stream.
// - The source does not support seeking.
//
//-------------------------------------------------------------------

#pragma warning( push )
#pragma warning( disable : 4355 )  // 'this' used in base member initializer list


HRESULT CreateVideoMediaType(const MPEG1VideoSeqHeader& videoSeqHdr, IMFMediaType **ppType);
HRESULT CreateAudioMediaType(const MPEG1AudioFrameHeader& audioHeader, IMFMediaType **ppType);
BOOL    SampleRequestMatch(SourceOp *pOp1, SourceOp *pOp2);


/* Public class methods */

//-------------------------------------------------------------------
// Name: CreateInstance
// Static method to create an instance of the source.
//
// ppSource:    Receives a ref-counted pointer to the source.
//-------------------------------------------------------------------

HRESULT MPEG1Source::CreateInstance(MPEG1Source **ppSource)
{
    CheckPointer(ppSource, E_POINTER);

    HRESULT hr = S_OK;
    MPEG1Source *pSource = new MPEG1Source(hr);
    if (pSource == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        *ppSource = pSource;
        (*ppSource)->AddRef();
    }

    SAFE_RELEASE(pSource);
    return hr;
}


//-------------------------------------------------------------------
// IUnknown methods
//-------------------------------------------------------------------

HRESULT MPEG1Source::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(MPEG1Source, IMFMediaEventGenerator),
        QITABENT(MPEG1Source, IMFMediaSource),
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

//-------------------------------------------------------------------
// IMFMediaEventGenerator methods
//
// All of the IMFMediaEventGenerator methods do the following:
// 1. Check for shutdown status.
// 2. Call the event queue helper object.
//-------------------------------------------------------------------

HRESULT MPEG1Source::BeginGetEvent(IMFAsyncCallback* pCallback,IUnknown* punkState)
{
    HRESULT hr = S_OK;

    AutoLock lock(m_critSec);

    CHECK_HR(hr = CheckShutdown());
    CHECK_HR(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

done:
    return hr;
}

HRESULT MPEG1Source::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;

    AutoLock lock(m_critSec);

    CHECK_HR(hr = CheckShutdown());
    CHECK_HR(hr = m_pEventQueue->EndGetEvent(pResult, ppEvent));

done:
    return hr;
}

HRESULT MPEG1Source::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    // NOTE: 
    // GetEvent can block indefinitely, so we don't hold the critical 
    // section. Therefore we need to use a local copy of the event queue 
    // pointer, to make sure the pointer remains valid.

    HRESULT hr = S_OK;

    IMFMediaEventQueue *pQueue = NULL;

    { // scope for lock
      
        AutoLock lock(m_critSec);

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

HRESULT MPEG1Source::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue)
{
    HRESULT hr = S_OK;

    AutoLock lock(m_critSec);
    CHECK_HR(hr = CheckShutdown());

    CHECK_HR(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

done:
    return hr;
}

//-------------------------------------------------------------------
// IMFMediaSource methods
//-------------------------------------------------------------------


//-------------------------------------------------------------------
// CreatePresentationDescriptor
// Returns a shallow copy of the source's presentation descriptor.
//-------------------------------------------------------------------

HRESULT MPEG1Source::CreatePresentationDescriptor(
    IMFPresentationDescriptor** ppPresentationDescriptor
    )
{
    AutoLock lock(m_critSec);

    if (ppPresentationDescriptor == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    
    // Fail if the source is shut down.
    CHECK_HR(hr = CheckShutdown());

    // Fail if the source was not initialized yet.
    CHECK_HR(hr = IsInitialized());

    // Do we have a valid presentation descriptor?
    if (m_pPresentationDescriptor == NULL)
    {
        CHECK_HR(hr = MF_E_NOT_INITIALIZED);
    }

    // Clone our presentation descriptor.
    CHECK_HR(hr = m_pPresentationDescriptor->Clone(ppPresentationDescriptor));

done:
    return hr;
}


//-------------------------------------------------------------------
// GetCharacteristics
// Returns capabilities flags.
//-------------------------------------------------------------------

HRESULT MPEG1Source::GetCharacteristics(DWORD* pdwCharacteristics)
{
    AutoLock lock(m_critSec);

    if (pdwCharacteristics == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    
    CHECK_HR(hr = CheckShutdown());

    *pdwCharacteristics =  MFMEDIASOURCE_CAN_PAUSE;

    // NOTE: This sample does not implement seeking, so we do not 
    // include the MFMEDIASOURCE_CAN_SEEK flag.

done:
    return hr;
}


//-------------------------------------------------------------------
// Pause
// Pauses the source.
//-------------------------------------------------------------------

HRESULT MPEG1Source::Pause()
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    // Fail if the source is shut down.
    CHECK_HR(hr = CheckShutdown());

    // Queue the operation.
    CHECK_HR(hr = QueueAsyncOperation(SourceOp::OP_PAUSE));

done:
    return hr;
}

//-------------------------------------------------------------------
// Shutdown
// Shuts down the source and releases all resources.
//-------------------------------------------------------------------

HRESULT MPEG1Source::Shutdown()
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    MPEG1Stream *pStream = NULL;

    CHECK_HR(hr = CheckShutdown());

    // Shut down the stream objects.
    for (DWORD i = 0; i < m_streams.GetCount(); i++)
    {
        (void)m_streams[i]->Shutdown();
    }

    // Shut down the event queue.
    if (m_pEventQueue)
    {
        (void)m_pEventQueue->Shutdown();
    }

    // Release objects. 
    for (DWORD i = 0; i < m_streams.GetCount(); i++)
    {
        SAFE_RELEASE(m_streams[i]);
    }
    m_streams.SetSize(0);
    m_streamMap.Clear();

    SAFE_RELEASE(m_pEventQueue);
    SAFE_RELEASE(m_pPresentationDescriptor);
    SAFE_RELEASE(m_pBeginOpenResult);
    SAFE_RELEASE(m_pByteStream);
    SAFE_RELEASE(m_pCurrentOp);

    CoTaskMemFree(m_pHeader);
    m_pHeader = NULL;

    SAFE_DELETE(m_pParser);

    // Set the state.
    m_state = STATE_SHUTDOWN;

done:
    return hr;
}


//-------------------------------------------------------------------
// Start
// Starts or seeks the media source.
//-------------------------------------------------------------------

HRESULT MPEG1Source::Start(
        IMFPresentationDescriptor* pPresentationDescriptor,
        const GUID* pguidTimeFormat,
        const PROPVARIANT* pvarStartPosition
    )
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;
    SourceOp *pAsyncOp = NULL;

    // Check parameters. 
    // Start position and presentation descriptor cannot be NULL.
    if (pvarStartPosition == NULL || pPresentationDescriptor == NULL)
    {
        return E_INVALIDARG;
    }

    // Check the time format. We support only reference time, which is
    // indicated by a NULL parameter or by time format = GUID_NULL.
    if ((pguidTimeFormat != NULL) && (*pguidTimeFormat != GUID_NULL))
    {
        // Unrecognized time format GUID.
        return MF_E_UNSUPPORTED_TIME_FORMAT;
    }

    // Check the data type of the start position.
    if ((pvarStartPosition->vt != VT_I8) && (pvarStartPosition->vt != VT_EMPTY))
    {
        return MF_E_UNSUPPORTED_TIME_FORMAT;
    }

    // Check if this is a seek request. 
    // Currently, this sample does not support seeking.

    if (pvarStartPosition->vt == VT_I8)
    {
        // If the current state is STOPPED, then position 0 is valid.

        // If the current state is anything else, then the 
        // start position must be VT_EMPTY (current position).

        if ((m_state != STATE_STOPPED) || (pvarStartPosition->hVal.QuadPart != 0))
        {
            return MF_E_INVALIDREQUEST;
        }
    }

    // Fail if the source is shut down.
    CHECK_HR(hr = CheckShutdown());

    // Fail if the source was not initialized yet.
    CHECK_HR(hr = IsInitialized());

    // Perform a sanity check on the caller's presentation descriptor.
    CHECK_HR(hr = ValidatePresentationDescriptor(pPresentationDescriptor));


    // The operation looks OK. Complete the operation asynchronously.

    // Create the state object for the async operation. 
    CHECK_HR(hr = SourceOp::CreateStartOp(pPresentationDescriptor, &pAsyncOp));

    CHECK_HR(hr = pAsyncOp->SetData(*pvarStartPosition));

    // Queue the operation.
    CHECK_HR(hr = QueueOperation(pAsyncOp));

done:
    SAFE_RELEASE(pAsyncOp);
    return hr;
}


//-------------------------------------------------------------------
// Stop
// Stops the media source.
//-------------------------------------------------------------------

HRESULT MPEG1Source::Stop()
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    // Fail if the source is shut down.
    CHECK_HR(hr = CheckShutdown());

    // Fail if the source was not initialized yet.
    CHECK_HR(hr = IsInitialized());

    // Queue the operation.
    CHECK_HR(hr = QueueAsyncOperation(SourceOp::OP_STOP));

done:
    return hr;
}


//-------------------------------------------------------------------
// Public non-interface methods
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// BeginOpen
// Begins reading the byte stream to initialize the source.
// Called by the byte-stream handler when it creates the source.
//
// This method is asynchronous. When the operation completes,
// the callback is invoked and the byte-stream handler calls
// EndOpen. 
//
// pStream: Pointer to the byte stream for the MPEG-1 stream.
// pCB: Pointer to the byte-stream handler's callback.
// pState: State object for the async callback. (Can be NULL.)
//
// Note: The source reads enough data to find one packet header
// for each audio or video stream. This enables the source to 
// create a presentation descriptor that describes the format of
// each stream. The source queues the packets that it reads during
// BeginOpen. 
//-------------------------------------------------------------------

HRESULT MPEG1Source::BeginOpen(IMFByteStream *pStream, IMFAsyncCallback *pCB, IUnknown *pState)
{
    AutoLock lock(m_critSec);

    if (pStream == NULL || pCB == NULL)
    {
        return E_POINTER;
    }

    if (m_state != STATE_INVALID)
    {
        return MF_E_INVALIDREQUEST;
    }

    HRESULT hr = S_OK;
    DWORD dwCaps = 0;

    // Cache the byte-stream pointer.
    m_pByteStream = pStream;
    m_pByteStream->AddRef();

    // Validate the capabilities of the byte stream. 
    // The byte stream must be readable and seekable.
    CHECK_HR(hr = pStream->GetCapabilities(&dwCaps));

    if ((dwCaps & MFBYTESTREAM_IS_SEEKABLE) == 0)
    {
        CHECK_HR(hr = MF_E_BYTESTREAM_NOT_SEEKABLE);
    }
    if ((dwCaps & MFBYTESTREAM_IS_READABLE) == 0)
    {
        CHECK_HR(hr = E_FAIL);
    }

    // Reserve space in the read buffer.
    CHECK_HR(hr = m_ReadBuffer.Initalize(INITIAL_BUFFER_SIZE));

    // Create the MPEG-1 parser.
    m_pParser = new Parser();
    if (m_pParser == NULL)
    {
        CHECK_HR(hr = E_OUTOFMEMORY);
    }

    // Create an async result object. We'll use it later to invoke the callback.
    CHECK_HR(hr = MFCreateAsyncResult(NULL, pCB, pState, &m_pBeginOpenResult));

    // Start reading data from the stream.
    CHECK_HR(hr = RequestData(READ_SIZE));

    // At this point, we now guarantee to invoke the callback.
    m_state = STATE_OPENING;

done:
    return hr;
}


//-------------------------------------------------------------------
// EndOpen
// Completes the BeginOpen operation. 
// Called by the byte-stream handler when it creates the source.
//-------------------------------------------------------------------

HRESULT MPEG1Source::EndOpen(IMFAsyncResult *pResult)
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;
    
    hr = pResult->GetStatus();

    if (FAILED(hr))
    {
        // The source is not designed to recover after failing to open.
        // Switch to shut-down state.
        Shutdown();
    }
    return hr;
}



//-------------------------------------------------------------------
// OnByteStreamRead
// Called when an asynchronous read completes. 
// 
// Read requests are issued in the RequestData() method.
//-------------------------------------------------------------------

HRESULT MPEG1Source::OnByteStreamRead(IMFAsyncResult *pResult)
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;
    DWORD cbRead = 0;

    IUnknown *pState = NULL;

    if (m_state == STATE_SHUTDOWN)
    {
        // If we are shut down, then we've already released the 
        // byte stream. Nothing to do.
        return S_OK;
    }

    // Get the state object. This is either NULL or the most
    // recent OP_REQUEST_DATA operation. 
    (void)pResult->GetState(&pState);

    // Complete the read opertation.
    CHECK_HR(hr = m_pByteStream->EndRead(pResult, &cbRead));

    // If the source stops and restarts in rapid succession, there is
    // a chance this is a "stale" read request, initiated before the
    // stop/restart. 
    
    // To ensure that we don't deliver stale data, we store the
    // OP_REQUEST_DATA operation as a state object in pResult, and compare
    // this against the current value of m_cRestartCounter.

    // If they don't match, we discard the data.

    // NOTE: During BeginOpen, pState is NULL

    if ((pState == NULL) || ( ((SourceOp*)pState)->Data().ulVal == m_cRestartCounter) )    
    {
        // This data is OK to parse.

        if (cbRead == 0)
        {
            // There is no more data in the stream. Signal end-of-stream.
            CHECK_HR(hr = EndOfMPEGStream());
        }
        else
        {
            // Update the end-position of the read buffer.
            CHECK_HR(hr = m_ReadBuffer.MoveEnd(cbRead));

            // Parse the new data.
            CHECK_HR(hr = ParseData());  
        }
    }

done:
    if (FAILED(hr))
    {
        StreamingError(hr);
    }
    SAFE_RELEASE(pState);
    return hr;
}



/* Private methods */

MPEG1Source::MPEG1Source(HRESULT& hr) : 
    OpQueue(m_critSec),
    m_pEventQueue(NULL), 
    m_pPresentationDescriptor(NULL), 
    m_pBeginOpenResult(NULL),
    m_pParser(NULL),
    m_pByteStream(NULL),
    m_pHeader(NULL),
    m_state(STATE_INVALID),
    m_pCurrentOp(NULL),
    m_pSampleRequest(NULL),
    m_cRestartCounter(0),
    m_OnByteStreamRead(this, &MPEG1Source::OnByteStreamRead)
{
    // Create the media event queue.
    hr = MFCreateEventQueue(&m_pEventQueue);
}

MPEG1Source::~MPEG1Source()
{
    if (m_state != STATE_SHUTDOWN)
    {
        Shutdown();
    }
}


//-------------------------------------------------------------------
// CompleteOpen
// 
// Completes the asynchronous BeginOpen operation.
//
// hrStatus: Status of the BeginOpen operation.
//-------------------------------------------------------------------

HRESULT MPEG1Source::CompleteOpen(HRESULT hrStatus)
{
    HRESULT hr = S_OK;

    if (m_pBeginOpenResult)
    {
        CHECK_HR(hr = m_pBeginOpenResult->SetStatus(hrStatus));
        CHECK_HR(hr = MFInvokeCallback(m_pBeginOpenResult));
    }

done:
    SAFE_RELEASE(m_pBeginOpenResult);
    return hr;
}


//-------------------------------------------------------------------
// IsInitialized:
// Returns S_OK if the source is correctly initialized with an 
// MPEG-1 byte stream. Otherwise, returns MF_E_NOT_INITIALIZED.
//-------------------------------------------------------------------

HRESULT MPEG1Source::IsInitialized() const
{
    if (m_state == STATE_OPENING || m_state == STATE_INVALID)
    {
        return MF_E_NOT_INITIALIZED;
    }
    else
    {
        return S_OK;
    }
}


//-------------------------------------------------------------------
// IsStreamTypeSupported:
// Returns TRUE if the source supports the specified MPEG-1 stream
// type.
//-------------------------------------------------------------------

BOOL MPEG1Source::IsStreamTypeSupported(StreamType type) const
{
    // We support audio and video streams. 
    return (type == StreamType_Video || type == StreamType_Audio); 
}

//-------------------------------------------------------------------
// IsStreamActive:
// Returns TRUE if the source should deliver a payload, whose type
// is indicated by the specified packet header.
//
// Note: This method does not test the started/paused/stopped state
//       of the source. 
//-------------------------------------------------------------------

BOOL MPEG1Source::IsStreamActive(const MPEG1PacketHeader& packetHdr)
{
    if (m_state == STATE_OPENING)
    {
        // The source is still opening. 
        // Deliver payloads for every supported stream type. 
        return IsStreamTypeSupported(packetHdr.type); 
    }
    else
    {
        // The source is already opened. Check if the stream is active.
        MPEG1Stream *pStream = GetStream(packetHdr.stream_id);

        if (pStream == NULL)
        {
            return FALSE;
        }
        else
        {
            return pStream->IsActive();
        }
    }
}


//-------------------------------------------------------------------
// InitPresentationDescriptor
// Create the source's presentation descriptor, if possible.
//
// During the BeginOpen operation, the source reads packets looking
// for headers for each stream. This enables the source to create the 
// presentation descriptor, which describes the stream formats. 
//
// This method tests whether the source has seen enough packets
// to create the PD. If so, it invokes the callback to complete
// the BeginOpen operation. 
//-------------------------------------------------------------------

HRESULT MPEG1Source::InitPresentationDescriptor()
{
    HRESULT hr = S_OK;
    DWORD cStreams = 0;

    assert(m_pPresentationDescriptor == NULL);
    assert(m_state == STATE_OPENING);

    if (m_pHeader == NULL)
    {
        return E_FAIL;
    }

    // Calculate how many streams we should have.
    for (DWORD i = 0; i < m_pHeader->cStreams; i++)
    {
        if (IsStreamTypeSupported(m_pHeader->streams[i].type))
        {
            cStreams++;
        }
    }

    // How many streams do we actually have? 
    if (cStreams > m_streams.GetCount())
    {
        // Not enough streams. Keep reading data until we have seen a packet for each stream.
        return S_OK;
    }

    assert(cStreams == m_streams.GetCount()); // We should never create a stream we don't support.

    // We're ready to create the presentation descriptor.

    // Create an array of IMFStreamDescriptor pointers.
    IMFStreamDescriptor **ppSD = new IMFStreamDescriptor*[cStreams];
    if (ppSD == NULL)
    {
        CHECK_HR(hr = E_OUTOFMEMORY);
    }
    ZeroMemory(ppSD, cStreams * sizeof(IMFStreamDescriptor*));

    // Fill the array by getting the stream descriptors from the MPEG1Stream objects. 
    // (We have already initialized these.)
    for (DWORD i = 0; i < cStreams; i++)
    {
        CHECK_HR(hr = m_streams[i]->GetStreamDescriptor(&ppSD[i]));
    }

    // Create the presentation descriptor.
    CHECK_HR(hr = MFCreatePresentationDescriptor(cStreams, ppSD, &m_pPresentationDescriptor));

    // Select the first video stream (if any).
    // NOTE: We don't select audio, because this sample does not include an audio decoder.
    for (DWORD i = 0; i < cStreams; i++)
    {
        GUID majorType = GUID_NULL;
        CHECK_HR(hr = GetStreamMajorType(ppSD[i], &majorType));

        if (majorType == MFMediaType_Video)
        {
            CHECK_HR(hr = m_pPresentationDescriptor->SelectStream(i));
            break;
        }
    }

    // Switch state from "opening" to "stopped."
    m_state = STATE_STOPPED;

    // Invoke the async callback to complete the BeginOpen operation.
    CHECK_HR(hr = CompleteOpen(S_OK));

done:
    if (ppSD)
    {
        for (DWORD i = 0; i < cStreams; i++)
        {
            SAFE_RELEASE(ppSD[i]);
        }
        delete [] ppSD;
    }
    return hr;
}


//-------------------------------------------------------------------
// QueueAsyncOperation
// Queue an asynchronous operation.
//
// OpType: Type of operation to queue. 
//
// Note: If the SourceOp object requires additional information, call
// OpQueue<SourceOp>::QueueOperation, which takes a SourceOp pointer.
//-------------------------------------------------------------------

HRESULT MPEG1Source::QueueAsyncOperation(SourceOp::Operation OpType)
{
    HRESULT hr = S_OK;
    SourceOp *pOp = NULL;

    CHECK_HR(hr = SourceOp::CreateOp(OpType, &pOp));
    CHECK_HR(hr = QueueOperation(pOp));

done:
    SAFE_RELEASE(pOp);
    return hr;
}

//-------------------------------------------------------------------
// BeginAsyncOp
//
// Starts an asynchronous operation. Called by the source at the 
// begining of any asynchronous operation.
//-------------------------------------------------------------------

HRESULT MPEG1Source::BeginAsyncOp(SourceOp *pOp)
{
    // At this point, the current operation should be NULL (the
    // previous operation is NULL) and the new operation (pOp)
    // must not be NULL.

    if (pOp == NULL || m_pCurrentOp != NULL)
    {
        assert(FALSE);
        return E_FAIL;
    }

    // Store the new operation as the current operation.

    m_pCurrentOp = pOp;
    m_pCurrentOp->AddRef();

    return S_OK;
}

//-------------------------------------------------------------------
// CompleteAsyncOp
//
// Completes an asynchronous operation. Called by the source at the 
// end of any asynchronous operation.
//-------------------------------------------------------------------

HRESULT MPEG1Source::CompleteAsyncOp(SourceOp *pOp)
{
    HRESULT hr = S_OK;

    // At this point, the current operation (m_pCurrentOp)
    // must match the operation that is ending (pOp). 

    if (pOp == NULL || m_pCurrentOp == NULL)
    {
        assert(FALSE);
        return E_FAIL;
    }

    if (m_pCurrentOp != pOp)
    {
        assert(FALSE);
        return E_FAIL;
    }

    // Release the current operation.
    SAFE_RELEASE(m_pCurrentOp);

    // Process the next operation on the queue.
    CHECK_HR(hr = ProcessQueue());

done:
    return hr;
}


//-------------------------------------------------------------------
// DispatchOperation
//
// Performs the asynchronous operation indicated by pOp.
//
// NOTE: 
// This method implements the pure-virtual OpQueue::DispatchOperation
// method. It is always called from a work-queue thread.
//-------------------------------------------------------------------

HRESULT MPEG1Source::DispatchOperation(SourceOp *pOp)
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    if (m_state == STATE_SHUTDOWN)
    {
        return S_OK; // Already shut down, ignore the request.
    }

    switch (pOp->Op())
    {

    // IMFMediaSource methods:

    case SourceOp::OP_START:
        hr = DoStart((StartOp*)pOp);
        break;

    case SourceOp::OP_STOP:
        hr = DoStop(pOp);
        break;

    case SourceOp::OP_PAUSE:
        hr = DoPause(pOp);
        break;

    // Operations requested by the streams:

    case SourceOp::OP_REQUEST_DATA:
        hr = OnStreamRequestSample(pOp);
        break;

    case SourceOp::OP_END_OF_STREAM:
        hr = OnEndOfStream(pOp);
        break;

    default:
        hr = E_UNEXPECTED;
    }

    if (FAILED(hr))
    {
        StreamingError(hr);
    }
    return hr;
}


//-------------------------------------------------------------------
// ValidateOperation
//
// Checks whether the source can perform the operation indicated
// by pOp at this time.
//
// If the source cannot perform the operation now, the method
// returns MF_E_NOTACCEPTING.
//
// NOTE: 
// Implements the pure-virtual OpQueue::ValidateOperation method. 
//-------------------------------------------------------------------

HRESULT MPEG1Source::ValidateOperation(SourceOp *pOp)
{
    if (m_pCurrentOp != NULL)
    {
        return MF_E_NOTACCEPTING;
    }
    return S_OK;
}



//-------------------------------------------------------------------
// DoStart
// Perform an async start operation (IMFMediaSource::Start)
//
// pOp: Contains the start parameters. 
//
// Note: This sample currently does not implement seeking, and the 
// Start() method fails if the caller requests a seek. 
//-------------------------------------------------------------------

HRESULT MPEG1Source::DoStart(StartOp *pOp)
{
    TRACE((L"DoStart\n"));

    assert(pOp->Op() == SourceOp::OP_START);

    IMFPresentationDescriptor *pPD = NULL;
    IMFMediaEvent  *pEvent = NULL;

    HRESULT     hr = S_OK;
    LONGLONG    llStartOffset = 0;
    BOOL        bRestartFromCurrentPosition = FALSE;
    BOOL        bSentEvents = FALSE;

    CHECK_HR(hr = BeginAsyncOp(pOp));

    // Get the presentation descriptor from the SourceOp object.
    // This is the PD that the caller passed into the Start() method.
    // The PD has already been validated.
    CHECK_HR(hr = pOp->GetPresentationDescriptor(&pPD));

    // Because this sample does not support seeking, the start
    // position must be 0 (from stopped) or "current position."

    // If the sample supported seeking, we would need to get the
    // start position from the PROPVARIANT data contained in pOp.

    // Select/deselect streams, based on what the caller set in the PD.
    CHECK_HR(hr = SelectStreams(pPD, pOp->Data()));

    m_state = STATE_STARTED;

    // Queue the "started" event. The event data is the start position.
    CHECK_HR(hr = m_pEventQueue->QueueEventParamVar(MESourceStarted, GUID_NULL, S_OK, &pOp->Data()));

done:
    if (FAILED(hr))
    {
        // Failure. Send the MESourceStarted or MESourceSeeked event with the error code. 

        // Note: It's possible that QueueEvent itself failed, in which case it is likely
        // to fail again. But there is no good way to recover in that case.

        (void)m_pEventQueue->QueueEventParamVar(MESourceStarted, GUID_NULL, hr, NULL);
    }

    CompleteAsyncOp(pOp);

    SAFE_RELEASE(pEvent);
    SAFE_RELEASE(pPD);
    return hr;
}


//-------------------------------------------------------------------
// DoStop 
// Perform an async stop operation (IMFMediaSource::Stop)
//-------------------------------------------------------------------

HRESULT MPEG1Source::DoStop(SourceOp *pOp)
{
    HRESULT hr = S_OK;
    QWORD qwCurrentPosition = 0;

    CHECK_HR(hr = BeginAsyncOp(pOp));

    // Stop the active streams.
    for (DWORD i = 0; i < m_streams.GetCount(); i++)
    {
        if (m_streams[i]->IsActive())
        {
            CHECK_HR(hr = m_streams[i]->Stop());
        }
    }

    // Seek to the start of the file. If we restart after stopping,
    // we will start from the beginning of the file again.
    CHECK_HR(hr = m_pByteStream->Seek(
        msoBegin, 
        0, 
        MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, 
        &qwCurrentPosition
        ));

    // Increment the counter that tracks "stale" read requests.
    ++m_cRestartCounter; // This counter is allowed to overflow.

    SAFE_RELEASE(m_pSampleRequest);

    m_state = STATE_STOPPED;

done:

    // Send the "stopped" event. This might include a failure code.
    (void)m_pEventQueue->QueueEventParamVar(MESourceStopped, GUID_NULL, hr, NULL);

    CompleteAsyncOp(pOp);

    return hr;
}


//-------------------------------------------------------------------
// DoPause 
// Perform an async pause operation (IMFMediaSource::Pause)
//-------------------------------------------------------------------

HRESULT MPEG1Source::DoPause(SourceOp *pOp)
{
    TRACE((L"DoPause\n"));

    HRESULT hr = S_OK;

    CHECK_HR(hr = BeginAsyncOp(pOp));

    // Pause is only allowed while running.
    if (m_state != STATE_STARTED)
    {
        CHECK_HR(hr = MF_E_INVALID_STATE_TRANSITION);
    }

    // Pause the active streams.
    for (DWORD i = 0; i < m_streams.GetCount(); i++)
    {
        if (m_streams[i]->IsActive())
        {
            CHECK_HR(hr = m_streams[i]->Pause());
        }
    }

    m_state = STATE_PAUSED;

done:

    // Send the "paused" event. This might include a failure code.
    (void)m_pEventQueue->QueueEventParamVar(MESourcePaused, GUID_NULL, hr, NULL);

    CompleteAsyncOp(pOp);

    return hr;
}


//-------------------------------------------------------------------
// StreamRequestSample 
// Called by streams when they need more data.
//
// Note: This is an async operation. The stream requests more data
// by queueing an OP_REQUEST_DATA operation.
//-------------------------------------------------------------------

HRESULT MPEG1Source::OnStreamRequestSample(SourceOp *pOp)
{
    HRESULT hr = S_OK;

    CHECK_HR(hr = BeginAsyncOp(pOp));

    // Ignore this request if we are already handling an earlier request.
    // (In that case m_pSampleRequest will be non-NULL.)

    if (m_pSampleRequest == NULL)
    {
        // Add the request counter as data to the operation.
        // This counter tracks whether a read request becomes "stale."

        PROPVARIANT var;
        var.vt = VT_UI4;
        var.ulVal = m_cRestartCounter;

        CHECK_HR(hr = pOp->SetData(var));

        // Store this while the request is pending.
        m_pSampleRequest = pOp;
        m_pSampleRequest->AddRef();

        // Try to parse data - this will invoke a read request if needed.
        ParseData();
    }

    CompleteAsyncOp(pOp);

done:
    return hr;
}


//-------------------------------------------------------------------
// OnEndOfStream 
// Called by each stream when it sends the last sample in the stream.
//
// Note: When the media source reaches the end of the MPEG-1 stream,
// it calls EndOfStream on each stream object. The streams might have
// data still in their queues. As each stream empties its queue, it
// notifies the source through an async OP_END_OF_STREAM operation.
//
// When every stream notifies the source, the source can send the 
// "end-of-presentation" event.
//-------------------------------------------------------------------

HRESULT MPEG1Source::OnEndOfStream(SourceOp *pOp)
{
    HRESULT hr = S_OK;

    CHECK_HR(hr = BeginAsyncOp(pOp));

    // Decrement the count of end-of-stream notifications. 
    --m_cPendingEOS;
    if (m_cPendingEOS == 0)
    {
        // No more streams. Send the end-of-presentation event.
        hr = m_pEventQueue->QueueEventParamVar(MEEndOfPresentation, GUID_NULL, S_OK, NULL);
    }

    CompleteAsyncOp(pOp);

done:
    return hr;
}



//-------------------------------------------------------------------
// SelectStreams
// Called during START operations to select and deselect streams.
//-------------------------------------------------------------------

HRESULT MPEG1Source::SelectStreams(
    IMFPresentationDescriptor *pPD,   // Presentation descriptor.
    const PROPVARIANT varStart        // New start position.
    )
{
    HRESULT hr = S_OK;
    BOOL    fSelected = FALSE;
    BOOL    fWasSelected = FALSE;
    DWORD   stream_id = 0;

    IMFStreamDescriptor *pSD = NULL;

    MPEG1Stream *pStream = NULL; // Not add-ref'd

    // Reset the pending EOS count.  
    m_cPendingEOS = 0;

    // Loop throught the stream descriptors to find which streams are active.
    for (DWORD i = 0; i < m_streams.GetCount(); i++)
    {
        CHECK_HR(hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD));

        CHECK_HR(hr = pSD->GetStreamIdentifier(&stream_id));

        pStream = GetStream((BYTE)stream_id);
        if (pStream == NULL)
        {
            CHECK_HR(hr = E_INVALIDARG);
        }

        // Was the stream active already?
        fWasSelected = pStream->IsActive();

        // Activate or deactivate the stream.
        CHECK_HR(hr = pStream->Activate(fSelected));

        if (fSelected)
        {
            m_cPendingEOS++;

            if (fWasSelected)
            {
                // This stream was previously selected. Queue the "updated stream" event.
                CHECK_HR(hr = m_pEventQueue->QueueEventParamUnk(MEUpdatedStream, GUID_NULL, hr, pStream));
            }
            else
            {
                // This stream was not previously selected. Queue the "new stream" event.
                CHECK_HR(hr = m_pEventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, hr, pStream));
            }

            // Start the stream. The stream will send the appropriate stream event.
            CHECK_HR(hr = pStream->Start(varStart));
        }

        SAFE_RELEASE(pSD);
    }

done:
    SAFE_RELEASE(pSD);
    return hr;
}


//-------------------------------------------------------------------
// RequestData
// Request the next batch of data.
// 
// cbRequest: Amount of data to read, in bytes.
//-------------------------------------------------------------------

HRESULT MPEG1Source::RequestData(DWORD cbRequest)
{
    HRESULT hr = S_OK;

    // Reserve a sufficient read buffer.
    CHECK_HR(hr = m_ReadBuffer.Reserve(cbRequest));

    // Submit the async read request.
    // When it completes, our OnByteStreamRead method will be invoked.

    CHECK_HR(hr = m_pByteStream->BeginRead(
        m_ReadBuffer.DataPtr() + m_ReadBuffer.DataSize(), 
        cbRequest,
        &m_OnByteStreamRead, 
        m_pSampleRequest
        ));

done:
    return hr;
}


//-------------------------------------------------------------------
// ParseData
// Parses the next batch of data.
//-------------------------------------------------------------------

HRESULT MPEG1Source::ParseData()
{
    HRESULT hr = S_OK;

    DWORD cbNextRequest = 0; 
    BOOL  bNeedMoreData = FALSE;

    // Keep processing data until
    // (a) All streams have enough samples, or 
    // (b) The parser needs more data in the buffer.

    while ( StreamsNeedData() )
    {
        DWORD cbAte = 0;    // How much data we consumed from the read buffer.

        // Check if we got the first system header.
        if (m_pHeader == NULL && m_pParser->HasSystemHeader())
        {
            CHECK_HR(hr = m_pParser->GetSystemHeader(&m_pHeader));

            // Allocate room for the streams.
            CHECK_HR(hr = m_streams.Allocate(m_pHeader->cStreams));
        }

        if (m_pParser->IsEndOfStream())
        {
            // The parser reached the end of the MPEG-1 stream. Notify the streams.
            CHECK_HR(hr = EndOfMPEGStream());
        }
        else if (m_pParser->HasPacket())
        {   
            // The parser reached the start of a new packet.
            CHECK_HR(hr = ReadPayload(&cbAte, &cbNextRequest));
        }
        else
        {
            // Parse more data.
            CHECK_HR(hr = m_pParser->ParseBytes(m_ReadBuffer.DataPtr(), m_ReadBuffer.DataSize(), &cbAte));

            // Parser::ParseBytes() can return S_FALSE, meaning "Need more data"
            if (hr == S_FALSE)
            {
                bNeedMoreData = TRUE;
            }
        }

        // Advance the start of the read buffer by the amount consumed.
        CHECK_HR(hr = m_ReadBuffer.MoveStart(cbAte));

        // If we need more data, start an async read operation.
        if (bNeedMoreData)
        {
            CHECK_HR(hr = RequestData( max(READ_SIZE, cbNextRequest) ));

            // Break from the loop because we need to wait for the async read to complete.
            break; 
        }
    }

    // Flag our state. If a stream requests more data while we are waiting for an async 
    // read to complete, we can ignore the stream's request, because the request will be 
    // dispatched as soon as we get more data.
    if (!bNeedMoreData)
    {
        SAFE_RELEASE(m_pSampleRequest);
    }

done:
    return hr;
}

//-------------------------------------------------------------------
// ReadPayload
// Read the next MPEG-1 payload.
//
// When this method is called:
// - The read position has reached the beginning of a payload.
// - We have the packet header, but not necessarily the entire payload.
//-------------------------------------------------------------------

HRESULT MPEG1Source::ReadPayload(DWORD *pcbAte, DWORD *pcbNextRequest)
{
    assert(m_pParser != NULL);
    assert(m_pParser->HasPacket());

    HRESULT hr = S_OK;

    DWORD cbPayloadRead = 0;
    DWORD cbPayloadUnread = 0;

    // At this point, the read buffer might be larger or smaller than the payload.
    // Calculate which portion of the payload has been read.
    if (m_pParser->PayloadSize() > m_ReadBuffer.DataSize())
    {
        cbPayloadUnread = m_pParser->PayloadSize() - m_ReadBuffer.DataSize();
    }

    cbPayloadRead = m_pParser->PayloadSize() - cbPayloadUnread;

    // Do we need to deliver this payload?
    if ( !IsStreamActive(m_pParser->PacketHeader()) )
    {
        QWORD qwCurrentPosition = 0;

        // Skip this payload. Seek past the unread portion of the payload.
        CHECK_HR(hr = m_pByteStream->Seek(
            msoCurrent, 
            cbPayloadUnread, 
            MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, 
            &qwCurrentPosition
            ));

        // Advance the data buffer to the end of payload, or the portion
        // that has been read.

        *pcbAte = cbPayloadRead;

        // Tell the parser that we are done with this packet.
        m_pParser->ClearPacket();

    }
    else if (cbPayloadUnread > 0)
    {
        // Some portion of this payload has not been read. Schedule a read.
        *pcbNextRequest = cbPayloadUnread;

        *pcbAte = 0;

        hr = S_FALSE; // Need more data.
    }
    else
    {
        // The entire payload is in the data buffer. Deliver the packet.
        CHECK_HR(hr = DeliverPayload());

        *pcbAte = cbPayloadRead;

        // Tell the parser that we are done with this packet.
        m_pParser->ClearPacket();
    }

done:
    return hr;
}

//-------------------------------------------------------------------
// EndOfMPEGStream:
// Called when the parser reaches the end of the MPEG1 stream.
//-------------------------------------------------------------------

HRESULT MPEG1Source::EndOfMPEGStream()
{
    // Notify the streams. The streams might have pending samples.
    // When each stream delivers the last sample, it will send the
    // end-of-stream event to the pipeline and then notify the 
    // source.

    // When every stream is done, the source sends the end-of-
    // presentation event.

    HRESULT hr = S_OK;

    for (DWORD i = 0; i < m_streams.GetCount(); i++)
    {
        if (m_streams[i]->IsActive())
        {
            CHECK_HR(hr = m_streams[i]->EndOfStream());
        }
    }

done:
    return hr;
}



//-------------------------------------------------------------------
// StreamsNeedData:
// Returns TRUE if any streams need more data.
//-------------------------------------------------------------------

BOOL MPEG1Source::StreamsNeedData() const
{
    BOOL bNeedData = FALSE;

    switch (m_state)
    {
    case STATE_OPENING:
        // While opening, we always need data (until we get enough
        // to complete the open operation).
        return TRUE;

    case STATE_SHUTDOWN:
        // While shut down, we never need data.
        return FALSE;

    default:
        // If none of the above, ask the streams.
        for (DWORD i = 0; i < m_streams.GetCount(); i++)
        {
            if (m_streams[i]->NeedsData())
            {
                bNeedData = TRUE;
                break;
            }
        }
        return bNeedData;
    }
}


//-------------------------------------------------------------------
// DeliverPayload:
// Delivers an MPEG-1 payload.
//-------------------------------------------------------------------

HRESULT MPEG1Source::DeliverPayload()
{
    // When this method is called, the read buffer contains a complete
    // payload, and the payload belongs to a stream whose type we support.

    assert(m_pParser->HasPacket());

    HRESULT             hr = S_OK;
    MPEG1PacketHeader   packetHdr;
    MPEG1Stream         *pStream = NULL;    // not AddRef'd

    IMFMediaBuffer      *pBuffer = NULL;
    IMFSample           *pSample = NULL;
    BYTE                *pData = NULL;      // Pointer to the IMFMediaBuffer data.

    packetHdr = m_pParser->PacketHeader();

    if (packetHdr.cbPayload > m_ReadBuffer.DataSize())
    {
        assert(FALSE);
        CHECK_HR(hr = E_UNEXPECTED);
    }

    // If we are still opening the file, then we might need to create this stream.
    if (m_state == STATE_OPENING)
    {
        CHECK_HR(hr = CreateStream(packetHdr));
    }

    pStream = GetStream(packetHdr.stream_id);
    assert(pStream != NULL);


    // Create a media buffer for the payload.
    CHECK_HR(hr = MFCreateMemoryBuffer(packetHdr.cbPayload, &pBuffer));

    CHECK_HR(hr = pBuffer->Lock(&pData, NULL, NULL));

    CopyMemory(pData, m_ReadBuffer.DataPtr(), packetHdr.cbPayload);

    CHECK_HR(hr = pBuffer->Unlock());

    CHECK_HR(hr = pBuffer->SetCurrentLength(packetHdr.cbPayload));

    // Create a sample to hold the buffer.
    CHECK_HR(hr = MFCreateSample(&pSample));
    CHECK_HR(hr = pSample->AddBuffer(pBuffer));

    // Time stamp the sample.
    if (packetHdr.bHasPTS)
    {
        LONGLONG hnsStart = packetHdr.PTS * 10000 / 90;

        CHECK_HR(hr = pSample->SetSampleTime(hnsStart));
    }


    // Deliver the payload to the stream.
    CHECK_HR(hr = pStream->DeliverPayload(pSample));

    // If the open operation is still pending, check if we're done.
    if (m_state == STATE_OPENING)
    {
        CHECK_HR(hr = InitPresentationDescriptor());
    }

done:
    SAFE_RELEASE(pBuffer);
    SAFE_RELEASE(pSample);
    return hr;
}

    

//-------------------------------------------------------------------
// CreateStream:
// Creates a media stream, based on a packet header.
//-------------------------------------------------------------------

HRESULT MPEG1Source::CreateStream(const MPEG1PacketHeader& packetHdr)
{
    // We validate the stream type before calling this method.
    assert(IsStreamTypeSupported(packetHdr.type));


    // First see if the stream already exists.
    if ( GetStream(packetHdr.stream_id) != NULL )
    {
        // The stream already exists. Nothing to do.
        return S_OK; 
    }

    HRESULT hr = S_OK;
    DWORD cbAte = 0;
    DWORD cbHeader = 0;
    BYTE *pPayload = NULL;
    DWORD cStreams = m_streams.GetCount();

    IMFMediaType *pType = NULL;
    IMFStreamDescriptor *pSD = NULL;
    MPEG1Stream *pStream = NULL;
    IMFMediaTypeHandler *pHandler = NULL;

    MPEG1VideoSeqHeader videoSeqHdr;
    MPEG1AudioFrameHeader audioFrameHeader;

    // Get the header size and a pointer to the start of the payload.
    cbHeader = packetHdr.cbPacketSize - packetHdr.cbPayload;
    pPayload = m_ReadBuffer.DataPtr();

    // Create a media type, based on the packet type (audio/video)
    switch (packetHdr.type)
    {
    case StreamType_Video:
        // Video: Read the sequence header and use it to create a media type.
        CHECK_HR(hr = ReadVideoSequenceHeader(pPayload, packetHdr.cbPayload, videoSeqHdr, &cbAte));
        CHECK_HR(hr = CreateVideoMediaType(videoSeqHdr, &pType));
        break;

    case StreamType_Audio:
        // Audio: Read the frame header and use it to create a media type.
        CHECK_HR(hr = ReadAudioFrameHeader(pPayload, packetHdr.cbPayload, audioFrameHeader, &cbAte));   
        CHECK_HR(hr = CreateAudioMediaType(audioFrameHeader, &pType));
        break;

    default:
        assert(false); // If this case occurs, then IsStreamTypeSupported() is wrong.
        CHECK_HR(hr = E_UNEXPECTED);
    }

    assert(pType != NULL);

    // Create the stream descriptor from the media type.
    CHECK_HR(hr = MFCreateStreamDescriptor(packetHdr.stream_id, 1, &pType, &pSD));

    // Set the default media type on the stream handler.
    CHECK_HR(hr = pSD->GetMediaTypeHandler(&pHandler));
    CHECK_HR(hr = pHandler->SetCurrentMediaType(pType));

    // Create the new stream.
    pStream = new MPEG1Stream(this, pSD, hr);
    if (pStream == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    CHECK_HR(hr);

    // Resize the stream array.
    CHECK_HR(hr = m_streams.SetSize( cStreams + 1 ));

    // Add the stream to the array.
    m_streams[cStreams] = pStream;
    m_streams[cStreams]->AddRef();

    // Add an entry to the map (id/index).
    // This enables us to look up a stream by ID.
    CHECK_HR(hr = m_streamMap.Insert(packetHdr.stream_id, cStreams));

done:
    SAFE_RELEASE(pSD);
    SAFE_RELEASE(pStream);
    return hr;
}


//-------------------------------------------------------------------
// ValidatePresentationDescriptor:
// Validates the presentation descriptor that the caller specifies
// in IMFMediaSource::Start().
//
// Note: This method performs a basic sanity check on the PD. It is 
// not intended to be a thorough validation. 
//-------------------------------------------------------------------

HRESULT MPEG1Source::ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD)
{
    HRESULT hr = S_OK;
    BOOL fSelected = FALSE;
    DWORD cStreams = 0;

    IMFStreamDescriptor *pSD = NULL;

    if (m_pHeader == NULL)
    {
        return E_UNEXPECTED;
    }

    // The caller's PD must have the same number of streams as ours.
    CHECK_HR(hr = pPD->GetStreamDescriptorCount(&cStreams));

    if (cStreams != m_pHeader->cStreams)
    {
        CHECK_HR(hr = E_INVALIDARG);
    }

    // The caller must select at least one stream.
    for (DWORD i = 0; i < cStreams; i++)
    {
        CHECK_HR(hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD));
        if (fSelected)
        {
            break;
        }
        SAFE_RELEASE(pSD);
    }

    if (!fSelected)
    {
        CHECK_HR(hr = E_INVALIDARG);
    }

done:
    SAFE_RELEASE(pSD);
    return hr;
}


//-------------------------------------------------------------------
// StreamingError:
// Handles an error that occurs duing an asynchronous operation.
//
// hr: Error code of the operation that failed.
//-------------------------------------------------------------------

void MPEG1Source::StreamingError(HRESULT hr)
{
    if (m_state == STATE_OPENING)
    {
        // An error happened during BeginOpen. 
        // Invoke the callback with the status code.

        CompleteOpen(hr);
    }
    else if (m_state != STATE_SHUTDOWN)
    {
        // An error occurred during streaming. Send the MEError event
        // to notify the pipeline.

        QueueEvent(MEError, GUID_NULL, hr, NULL);
    }
}

//-------------------------------------------------------------------
// MPEG1Source:
// Returns a stream by ID.
//
// This method can return NULL if the source did not create a
// stream for this ID. In particular, this can happen if:
// 
// 1) The stream type is not supported. See IsStreamTypeSupported().
// 2) The source is still opening.
//
// Note: This method does not AddRef the stream object. The source
// uses this method to access the streams. If the source hands out
// a stream pointer (e.g. in the MENewStream event), the source
// must AddRef the stream object.
//-------------------------------------------------------------------

MPEG1Stream* MPEG1Source::GetStream(BYTE stream_id)
{
    MPEG1Stream *pStream = NULL;

    DWORD index = 0;
    HRESULT hr = m_streamMap.Find(stream_id, &index);
    if (SUCCEEDED(hr))
    {
        assert (m_streams.GetCount() > index);
        pStream = m_streams[index];
    }
    return pStream;
}


/* SourceOp class */


//-------------------------------------------------------------------
// CreateOp
// Static method to create a SourceOp instance.
//
// op: Specifies the async operation.
// ppOp: Receives a pointer to the SourceOp object.
//-------------------------------------------------------------------

HRESULT SourceOp::CreateOp(SourceOp::Operation op, SourceOp **ppOp)
{
    if (ppOp == NULL)
    {
        return E_POINTER;
    }

    SourceOp *pOp = new SourceOp(op);
    if (pOp  == NULL)
    {
        return E_OUTOFMEMORY;
    }
    *ppOp = pOp;

    return S_OK;
}

//-------------------------------------------------------------------
// CreateStartOp:
// Static method to create a SourceOp instance for the Start() 
// operation.
//
// pPD: Presentation descriptor from the caller.
// ppOp: Receives a pointer to the SourceOp object.
//-------------------------------------------------------------------

HRESULT SourceOp::CreateStartOp(IMFPresentationDescriptor *pPD, SourceOp **ppOp)
{
    if (ppOp == NULL)
    {
        return E_POINTER;
    }

    SourceOp *pOp = new StartOp(pPD);
    if (pOp == NULL)
    {
        return E_OUTOFMEMORY;
    }

    *ppOp = pOp;
    return S_OK;
}


HRESULT SourceOp::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(SourceOp, IUnknown),
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

SourceOp::SourceOp(Operation op) : m_op(op)
{
    PropVariantInit(&m_data);
}

SourceOp::~SourceOp() 
{
    PropVariantClear(&m_data);
} 

HRESULT SourceOp::SetData(const PROPVARIANT& var)
{
    return PropVariantCopy(&m_data, &var);
}


StartOp::StartOp(IMFPresentationDescriptor *pPD) : SourceOp(SourceOp::OP_START), m_pPD(pPD)
{
    if (m_pPD)
    {
        m_pPD->AddRef();
    }
}

StartOp::~StartOp()
{
    SAFE_RELEASE(m_pPD);
}


HRESULT StartOp::GetPresentationDescriptor(IMFPresentationDescriptor **ppPD)
{
    if (ppPD == NULL)
    {
        return E_POINTER;
    }
    if (m_pPD == NULL)
    {
        return MF_E_INVALIDREQUEST;
    }
    *ppPD = m_pPD;
    (*ppPD)->AddRef();
    return S_OK;
}


/*  Static functions */


//-------------------------------------------------------------------
// CreateVideoMediaType:
// Create a media type from an MPEG-1 video sequence header.
//-------------------------------------------------------------------

HRESULT CreateVideoMediaType(const MPEG1VideoSeqHeader& videoSeqHdr, IMFMediaType **ppType)
{
    HRESULT hr = S_OK;

    // Create the helper object for generating video media types.
    MPEGVideoType video_type;
    
    CHECK_HR(hr = video_type.CreateEmptyType());

    // Subtype = MPEG-1 payload
    CHECK_HR(hr = video_type.SetSubType(MEDIASUBTYPE_MPEG1Payload));

    // Format details.
    CHECK_HR(hr = video_type.SetFrameDimensions(videoSeqHdr.width, videoSeqHdr.height));
    CHECK_HR(hr = video_type.SetFrameRate(videoSeqHdr.frameRate));
    CHECK_HR(hr = video_type.SetPixelAspectRatio(videoSeqHdr.pixelAspectRatio));
    CHECK_HR(hr = video_type.SetAvgerageBitRate(videoSeqHdr.bitRate));
    CHECK_HR(hr = video_type.SetInterlaceMode(MFVideoInterlace_Progressive));

    // Copy the sequence header.
    CHECK_HR(hr = video_type.SetMpegSeqHeader(videoSeqHdr.header, videoSeqHdr.cbHeader));

    // Get the media type from the helper.
    *ppType =video_type.Detach();

done:
    return hr;
}


//-------------------------------------------------------------------
// CreateAudioMediaType:
// Create a media type from an MPEG-1 audio frame header.
//
// Note: This function fills in an MPEG1WAVEFORMAT structure and then
// converts the structure to a Media Foundation media type 
// (IMFMediaType). This is somewhat roundabout but it guarantees
// that the type can be converted back to an MPEG1WAVEFORMAT by the
// decoder if need be. 
//
// The WAVEFORMATEX portion of the MPEG1WAVEFORMAT structure is
// converted into attributes on the IMFMediaType object. The rest of
// the struct is stored in the MF_MT_USER_DATA attribute. 
//-------------------------------------------------------------------

HRESULT CreateAudioMediaType(const MPEG1AudioFrameHeader& audioHeader, IMFMediaType **ppType)
{
    HRESULT hr = S_OK;
    IMFMediaType *pType = NULL;

    MPEG1WAVEFORMAT format;
    ZeroMemory(&format, sizeof(format));

    format.wfx.wFormatTag = WAVE_FORMAT_MPEG;
    format.wfx.nChannels = audioHeader.nChannels;
    format.wfx.nSamplesPerSec = audioHeader.dwSamplesPerSec;
    if (audioHeader.dwBitRate > 0)
    {
        format.wfx.nAvgBytesPerSec = (audioHeader.dwBitRate * 1000) / 8;
    }
    format.wfx.nBlockAlign = audioHeader.nBlockAlign;
    format.wfx.wBitsPerSample = 0; // Not used.
    format.wfx.cbSize = sizeof(MPEG1WAVEFORMAT) - sizeof(WAVEFORMATEX);

    // MPEG-1 audio layer.
    switch (audioHeader.layer)
    {
    case MPEG1_Audio_Layer1:
        format.fwHeadLayer = ACM_MPEG_LAYER1;
        break;

    case MPEG1_Audio_Layer2:
        format.fwHeadLayer = ACM_MPEG_LAYER2;
        break;

    case MPEG1_Audio_Layer3:
        format.fwHeadLayer = ACM_MPEG_LAYER3;
        break;
    };

    format.dwHeadBitrate = audioHeader.dwBitRate * 1000;

    // Mode
    switch (audioHeader.mode)
    {
    case MPEG1_Audio_Stereo:
        format.fwHeadMode = ACM_MPEG_STEREO;
        break;

    case MPEG1_Audio_JointStereo:
        format.fwHeadMode = ACM_MPEG_JOINTSTEREO;
        break;

    case MPEG1_Audio_DualChannel:
        format.fwHeadMode = ACM_MPEG_DUALCHANNEL;
        break;

    case MPEG1_Audio_SingleChannel:
        format.fwHeadMode = ACM_MPEG_SINGLECHANNEL;
        break;
    };

    if (audioHeader.mode == ACM_MPEG_JOINTSTEREO)
    {
        // Convert the 'mode_extension' field to the correct MPEG1WAVEFORMAT value.
        if (audioHeader.modeExtension <= 0x03)
        {
            format.fwHeadModeExt = 0x01 << audioHeader.modeExtension;
        }
    }

    // Convert the 'emphasis' field to the correct MPEG1WAVEFORMAT value.
    if (audioHeader.emphasis <= 0x03)
    {
        format.wHeadEmphasis = audioHeader.emphasis + 1;
    }

    // The flags translate directly.
    format.fwHeadFlags = audioHeader.wFlags;
    // Add the "MPEG-1" flag, although it's somewhat redundant.
    format.fwHeadFlags |= ACM_MPEG_ID_MPEG1;

    // Use the structure to initialize the Media Foundation media type.
    CHECK_HR(hr = MFCreateMediaType(&pType));
    CHECK_HR(hr = MFInitMediaTypeFromWaveFormatEx(pType, (const WAVEFORMATEX*)&format, sizeof(format)));

    *ppType = pType;
    (*ppType)->AddRef();

done:
    SAFE_RELEASE(pType);
    return hr;
}

BOOL SampleRequestMatch(SourceOp *pOp1, SourceOp *pOp2)
{
    if ((pOp1 == NULL) && (pOp2 == NULL))
    {
        return TRUE;
    }
    else if ((pOp1 == NULL) || (pOp2 == NULL))
    {
        return FALSE;
    }
    else 
    {
        return (pOp1->Data().ulVal == pOp2->Data().ulVal);
    }
}


#pragma warning( pop )