//-----------------------------------------------------------------------------
// File: WaveSink.cpp
// Description: Archive sink for creating .wav files.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "WavSink.h"

#include <aviriff.h>

#pragma warning( push )
#pragma warning( disable : 4355 )  // 'this' used in base member initializer list


//-------------------------------------------------------------------
// Name: CreateWavSink 
// Description: Creates an instance of the WavSink object. 
//
// To use the WavSink, include this header file in an application
// and link to the library file created by this project.
//
// pStream: Pointer to a bytestream where the .wav file will be
//          written. The bystream must support writing and seeking.
//
// ppSink:  Receives a pointer to the IMFMediaSink interface. The
//          caller must release the interface.
//-------------------------------------------------------------------

HRESULT CreateWavSink(IMFByteStream *pStream, IMFMediaSink **ppSink)
{
    return CWavSink::CreateInstance(pStream, IID_IMFMediaSink, (void**)ppSink);
}


// The stream ID of the one stream on the sink.
const DWORD WAV_SINK_STREAM_ID = 1;


// WAV_FILE_HEADER
// This structure contains the first part of the .wav file, up to the
// data portion. (Wave files are so simple there is no reason to write
// a general-purpose RIFF authoring object.)
struct WAV_FILE_HEADER
{
    RIFFCHUNK       FileHeader;
    DWORD           fccWaveType;    // must be 'WAVE'
    RIFFCHUNK       WaveHeader;
    WAVEFORMATEX    WaveFormat;
    RIFFCHUNK       DataHeader;
};

// PCM_Audio_Format_Params
// Defines parameters for uncompressed PCM audio formats.
// The remaining fields can be derived from these.
struct PCM_Audio_Format_Params
{
    DWORD   nSamplesPerSec; // Samples per second.
    WORD    wBitsPerSample; // Bits per sample.
    WORD    nChannels;      // Number of channels.
};


// g_AudioFormats: Static list of our preferred formats.

// This is an ordered list that we use to hand out formats in the 
// stream's IMFMediaTypeHandler::GetMediaTypeByIndex method. The 
// stream will accept other bit rates not listed here.

PCM_Audio_Format_Params g_AudioFormats[] =
{
    { 48000, 16, 2 },
    { 48000, 8, 2 },
    { 44100, 16, 2 },
    { 44100, 8, 2 },
    { 22050, 16, 2 },
    { 22050, 8, 2 },

    { 48000, 16, 1 },
    { 48000, 8, 1 },
    { 44100, 16, 1 },
    { 44100, 8, 1 },
    { 22050, 16, 1 },
    { 22050, 8, 1 },
};

DWORD g_NumAudioFormats = ARRAY_SIZE(g_AudioFormats);

// Forward declares
HRESULT ValidateWaveFormat(const WAVEFORMATEX *pWav, DWORD cbSize);

HRESULT CreatePCMAudioType(
    UINT32 sampleRate,        // Samples per second
    UINT32 bitsPerSample,     // Bits per sample
    UINT32 cChannels,         // Number of channels
    IMFMediaType **ppType     // Receives a pointer to the media type.
    );

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CWavSink class. - Implements the media sink.
//
// Notes:
// - Most public methods calls CheckShutdown. This method fails if the sink was shut down.
//
/////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------
// Name: CreateInstance 
// Description: Creates an instance of the WavSink object. 
// [See CreateWavSink]
//-------------------------------------------------------------------

/* static */ HRESULT CWavSink::CreateInstance(IMFByteStream *pStream, REFIID iid, void **ppSink)
{
    if (pStream == NULL || ppSink == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    CWavSink *pSink = new CWavSink();   // Created with ref count = 1.

    if (pSink == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = pSink->Initialize(pStream);
    }

    if (SUCCEEDED(hr))
    {
        hr = pSink->QueryInterface(iid, ppSink);
    }

    SAFE_RELEASE(pSink);

    return hr;
}


//-------------------------------------------------------------------
// CWavSink constructor.
//-------------------------------------------------------------------

CWavSink::CWavSink() :
    m_nRefCount(1), m_IsShutdown(FALSE), m_pStream(NULL), m_pClock(NULL)
{

}

//-------------------------------------------------------------------
// CWavSink destructor.
//-------------------------------------------------------------------

CWavSink::~CWavSink()
{
    TRACE((L"~CWavSink\n"));
    assert(m_IsShutdown);
}

// IUnknown methods

ULONG CWavSink::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG  CWavSink::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT CWavSink::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IMFFinalizableMediaSink*>(this));
    }
    else if (iid == __uuidof(IMFMediaSink))
    {
        *ppv = static_cast<IMFMediaSink*>(this);
    }
    else if (iid == __uuidof(IMFFinalizableMediaSink))
    {
        *ppv = static_cast<IMFFinalizableMediaSink*>(this);
    }
    else if (iid == __uuidof(IMFClockStateSink))
    {
        *ppv = static_cast<IMFClockStateSink*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}




///  IMFMediaSink methods.


//-------------------------------------------------------------------
// Name: GetCharacteristics 
// Description: Returns the characteristics flags. 
//
// Note: This sink has a fixed number of streams and is rateless.
//-------------------------------------------------------------------

HRESULT CWavSink::GetCharacteristics(DWORD *pdwCharacteristics)
{
    AutoLock lock(m_critSec);

    if (pdwCharacteristics == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *pdwCharacteristics = MEDIASINK_FIXED_STREAMS | MEDIASINK_RATELESS;
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: AddStreamSink 
// Description: Adds a new stream to the sink. 
//
// Note: This sink has a fixed number of streams, so this method
//       always returns MF_E_STREAMSINKS_FIXED.
//-------------------------------------------------------------------

HRESULT CWavSink::AddStreamSink( 
    DWORD dwStreamSinkIdentifier,
    IMFMediaType *pMediaType,
    IMFStreamSink **ppStreamSink)
{
    return MF_E_STREAMSINKS_FIXED;
}



//-------------------------------------------------------------------
// Name: RemoveStreamSink 
// Description: Removes a stream from the sink. 
//
// Note: This sink has a fixed number of streams, so this method
//       always returns MF_E_STREAMSINKS_FIXED.
//-------------------------------------------------------------------

HRESULT CWavSink::RemoveStreamSink(DWORD dwStreamSinkIdentifier)
{
    return MF_E_STREAMSINKS_FIXED;
}


//-------------------------------------------------------------------
// Name: GetStreamSinkCount 
// Description: Returns the number of streams. 
//-------------------------------------------------------------------

HRESULT CWavSink::GetStreamSinkCount(DWORD *pcStreamSinkCount)
{
    AutoLock lock(m_critSec);

    if (pcStreamSinkCount == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *pcStreamSinkCount = 1;  // Fixed number of streams.
    }

    return hr;

}


//-------------------------------------------------------------------
// Name: GetStreamSinkByIndex 
// Description: Retrieves a stream by index. 
//-------------------------------------------------------------------

HRESULT CWavSink::GetStreamSinkByIndex( 
    DWORD dwIndex,
    IMFStreamSink **ppStreamSink)
{
    AutoLock lock(m_critSec);

    if (ppStreamSink == NULL)
    {
        return E_INVALIDARG;
    }

    // Fixed stream: Index 0. 
    if (dwIndex > 0)
    {
        return MF_E_INVALIDINDEX;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *ppStreamSink = m_pStream;
        (*ppStreamSink)->AddRef();
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: GetStreamSinkById 
// Description: Retrieves a stream by ID. 
//-------------------------------------------------------------------

HRESULT CWavSink::GetStreamSinkById( 
    DWORD dwStreamSinkIdentifier,
    IMFStreamSink **ppStreamSink)
{
    AutoLock lock(m_critSec);

    if (ppStreamSink == NULL)
    {
        return E_INVALIDARG;
    }

    // Fixed stream ID.
    if (dwStreamSinkIdentifier != WAV_SINK_STREAM_ID)
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *ppStreamSink = m_pStream;
        (*ppStreamSink)->AddRef();
    }

    return hr;

}


//-------------------------------------------------------------------
// Name: SetPresentationClock 
// Description: Sets the presentation clock. 
//
// pPresentationClock: Pointer to the clock. Can be NULL.
//-------------------------------------------------------------------

HRESULT CWavSink::SetPresentationClock(IMFPresentationClock *pPresentationClock)
{
    AutoLock lock(m_critSec);

    HRESULT hr = CheckShutdown();

    // If we already have a clock, remove ourselves from that clock's
    // state notifications.
    if (SUCCEEDED(hr))
    {
        if (m_pClock)
        {
            hr = m_pClock->RemoveClockStateSink(this);
        }
    }

    // Register ourselves to get state notifications from the new clock.
    if (SUCCEEDED(hr))
    {
        if (pPresentationClock)
        {
            hr = pPresentationClock->AddClockStateSink(this);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Release the pointer to the old clock.
        // Store the pointer to the new clock.

        SAFE_RELEASE(m_pClock);
        m_pClock = pPresentationClock;
        if (m_pClock)
        {
            m_pClock->AddRef();
        }
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: GetPresentationClock 
// Description: Returns a pointer to the presentation clock. 
//-------------------------------------------------------------------

HRESULT CWavSink::GetPresentationClock(IMFPresentationClock **ppPresentationClock)
{
    AutoLock lock(m_critSec);

    if (ppPresentationClock == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        if (m_pClock == NULL)
        {
            hr = MF_E_NO_CLOCK; // There is no presentation clock.
        }
        else
        {
            // Return the pointer to the caller.
            *ppPresentationClock = m_pClock;
            (*ppPresentationClock)->AddRef();
        }
    }


    return hr;
}


//-------------------------------------------------------------------
// Name: Shutdown 
// Description: Releases resources held by the media sink. 
//-------------------------------------------------------------------

HRESULT CWavSink::Shutdown()
{
    TRACE((L"CWavSink::Shutdown\n"));
    AutoLock lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Shutdown();

        SAFE_RELEASE(m_pClock);
        SAFE_RELEASE(m_pStream);
 
        m_IsShutdown = true;
    }

    return hr;
}

/// IMFFinalizableMediaSink methods


//-------------------------------------------------------------------
// Name: BeginFinalize 
// Description: Starts the asynchronous finalize operation.
//
// Note: We use the Finalize operation to write the RIFF headers.
//-------------------------------------------------------------------

HRESULT CWavSink::BeginFinalize( 
    IMFAsyncCallback *pCallback,
    IUnknown *punkState)
{
    TRACE((L"CWavSink::BeginFinalize\n"));

    AutoLock lock(m_critSec);

    HRESULT hr = CheckShutdown();

    // Tell the stream to finalize.
    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Finalize(pCallback, punkState);
    }
    return hr;
}


//-------------------------------------------------------------------
// Name: EndFinalize 
// Description: Completes the asynchronous finalize operation.
//-------------------------------------------------------------------

HRESULT CWavSink::EndFinalize(IMFAsyncResult *pResult)
{
    TRACE((L"CWavSink::EndFinalize\n"));

    HRESULT hr = S_OK;

    // Return the status code from the async result.
    if (pResult == NULL)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = pResult->GetStatus();
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: OnClockStart 
// Description: Called when the presentation clock starts.
//
// hnsSystemTime: System time when the clock started.
// llClockStartOffset: Starting presentatation time.
//
// Note: For an archive sink, we don't care about the system time.
//       But we need to cache the value of llClockStartOffset. This 
//       gives us the earliest time stamp that we archive. If any 
//       input samples have an earlier time stamp, we discard them.
//-------------------------------------------------------------------

HRESULT CWavSink::OnClockStart( 
        /* [in] */ MFTIME hnsSystemTime,
        /* [in] */ LONGLONG llClockStartOffset)
{
    AutoLock lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Start(llClockStartOffset);
    }

    return hr;
}    

//-------------------------------------------------------------------
// Name: OnClockStop 
// Description: Called when the presentation clock stops.
//
// Note: After this method is called, we stop accepting new data.
//-------------------------------------------------------------------

HRESULT CWavSink::OnClockStop( 
        /* [in] */ MFTIME hnsSystemTime)
{
    TRACE((L"CWavSink::OnClockStop\n"));
    AutoLock lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Stop();
    }

    return hr;
}    


//-------------------------------------------------------------------
// Name: OnClockPause 
// Description: Called when the presentation clock paused.
//
// Note: For an archive sink, the paused state is equivalent to the
//       running (started) state. We still accept data and archive it.
//-------------------------------------------------------------------

HRESULT CWavSink::OnClockPause( 
        /* [in] */ MFTIME hnsSystemTime)
{
    AutoLock lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Pause();
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: OnClockRestart 
// Description: Called when the presentation clock restarts.
//-------------------------------------------------------------------

HRESULT CWavSink::OnClockRestart( 
        /* [in] */ MFTIME hnsSystemTime)
{
    AutoLock lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Restart();
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: OnClockSetRate 
// Description: Called when the presentation clock's rate changes.
//
// Note: For a rateless sink, the clock rate is not important.
//-------------------------------------------------------------------

HRESULT CWavSink::OnClockSetRate( 
        /* [in] */ MFTIME hnsSystemTime,
        /* [in] */ float flRate)
{
    return S_OK;
}


/// Private methods


//-------------------------------------------------------------------
// Name: Initialize 
// Description: Initializes the media sink.
//
// Note: This method is called once when the media sink is first
//       initialized.
//-------------------------------------------------------------------

HRESULT CWavSink::Initialize(IMFByteStream *pByteStream)
{
    HRESULT hr = S_OK;

    m_pStream = new CWavStream();
    if (m_pStream == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    // Initialize the stream.
    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Initialize(this, pByteStream);
    }
    return hr;
}



/////////////////////////////////////////////////////////////////////////////////////////////
//
// CAsyncOperation class. - Private class used by CWavStream class.
//
/////////////////////////////////////////////////////////////////////////////////////////////

CWavStream::CAsyncOperation::CAsyncOperation(StreamOperation op) 
    : m_nRefCount(1), m_op(op)
{
}

CWavStream::CAsyncOperation::~CAsyncOperation()
{
    assert(m_nRefCount == 0);
}

ULONG CWavStream::CAsyncOperation::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG CWavStream::CAsyncOperation::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT CWavStream::CAsyncOperation::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
// CWavStream class. - Implements the stream sink.
//
// Notes: 
// - Most of the real work gets done in this class. 
// - The sink has one stream. If it had multiple streams, it would need to coordinate them.
// - Most operations are done asynchronously on a work queue.
// - Async methods are handled like this:
//      1. Call ValidateOperation to check if the operation is permitted at this time
//      2. Create an CAsyncOperation object for the operation.
//      3. Call QueueAsyncOperation. This puts the operation on the work queue.
//      4. The workqueue calls OnDispatchWorkItem.
// - Locking:
//      To avoid deadlocks, do not hold the CWavStream lock followed by the CWavSink lock.
//      The other order is OK (CWavSink, then CWavStream).
// 
/////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------
// CWavStream constructor
//-------------------------------------------------------------------

CWavStream::CWavStream() 
    : m_nRefCount(1), m_state(State_TypeNotSet), m_IsShutdown(FALSE), 
    m_pSink(NULL), m_pEventQueue(NULL), m_pByteStream(NULL), 
    m_pCurrentType(NULL), m_pFinalizeResult(NULL),
    m_StartTime(0), m_cbDataWritten(0), m_WorkQueueId(0), 
    m_WorkQueueCB(this, &CWavStream::OnDispatchWorkItem)
{

}


//-------------------------------------------------------------------
// CWavStream destructor
//-------------------------------------------------------------------

CWavStream::~CWavStream()
{
    TRACE((L"~CWavStream\n"));
    assert(m_IsShutdown);
}


// IUnknown methods

ULONG CWavStream::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG  CWavStream::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT CWavStream::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IMFStreamSink*>(this));
    }
    else if (iid == __uuidof(IMFStreamSink ))
    {
        *ppv = static_cast<IMFStreamSink *>(this);
    }
    else if (iid == __uuidof(IMFMediaEventGenerator))
    {
        *ppv = static_cast<IMFMediaEventGenerator*>(this);
    }
    else if (iid == __uuidof(IMFMediaTypeHandler))
    {
        *ppv = static_cast<IMFMediaTypeHandler*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}


// IMFMediaEventGenerator methods.
// Note: These methods call through to the event queue helper object.

HRESULT CWavStream::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    HRESULT hr = S_OK;

    AutoLock lock(m_critSec);
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->BeginGetEvent(pCallback, punkState);
    }

    return hr;
}

HRESULT CWavStream::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;

    AutoLock lock(m_critSec);
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->EndGetEvent(pResult, ppEvent);
    }

    return hr;
}

HRESULT CWavStream::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    // NOTE: 
    // GetEvent can block indefinitely, so we don't hold the lock.
    // This requires some juggling with the event queue pointer.

    HRESULT hr = S_OK;

    IMFMediaEventQueue *pQueue = NULL;

    { // scope for lock
      
        AutoLock lock(m_critSec);

        // Check shutdown
        hr = CheckShutdown();

        // Get the pointer to the event queue.
        if (SUCCEEDED(hr))
        {
            pQueue = m_pEventQueue;
            pQueue->AddRef();
        }

    }   // release lock

    // Now get the event.
    if (SUCCEEDED(hr))
    {
        hr = pQueue->GetEvent(dwFlags, ppEvent);
    }

    SAFE_RELEASE(pQueue);

    return hr;
}

HRESULT CWavStream::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue)
{

    HRESULT hr = S_OK;

    AutoLock lock(m_critSec);
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
    }

    return hr;
}




/// IMFStreamSink methods


//-------------------------------------------------------------------
// Name: GetMediaSink 
// Description: Returns the parent media sink.
//-------------------------------------------------------------------

HRESULT CWavStream::GetMediaSink(IMFMediaSink **ppMediaSink)
{
    AutoLock lock(m_critSec);

    if (ppMediaSink == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *ppMediaSink = (IMFMediaSink*)m_pSink;
        (*ppMediaSink)->AddRef();
    }

    return hr;

}


//-------------------------------------------------------------------
// Name: GetIdentifier 
// Description: Returns the stream identifier.
//-------------------------------------------------------------------

HRESULT CWavStream::GetIdentifier(DWORD *pdwIdentifier)
{
    AutoLock lock(m_critSec);

    if (pdwIdentifier == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *pdwIdentifier = WAV_SINK_STREAM_ID;
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: GetMediaTypeHandler 
// Description: Returns a media type handler for this stream.
//-------------------------------------------------------------------

HRESULT CWavStream::GetMediaTypeHandler(IMFMediaTypeHandler **ppHandler)
{
    AutoLock lock(m_critSec);

    if (ppHandler == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    // This stream object acts as its own type handler, so we QI ourselves.
    if (SUCCEEDED(hr))
    {
        hr = this->QueryInterface(IID_IMFMediaTypeHandler, (void**)ppHandler);
    }
    return hr;
}


//-------------------------------------------------------------------
// Name: ProcessSample 
// Description: Receives an input sample. [Asynchronous]
//
// Note: The client should only give us a sample after we send an
//       MEStreamSinkRequestSample event.
//-------------------------------------------------------------------

HRESULT CWavStream::ProcessSample(IMFSample *pSample)
{
    AutoLock lock(m_critSec);

    if (pSample == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    
    hr = CheckShutdown();

    // Validate the operation.
    if (SUCCEEDED(hr))
    {
        hr = ValidateOperation(OpProcessSample);
    }

    // Add the sample to the sample queue.
    if (SUCCEEDED(hr))
    {
        hr = m_SampleQueue.InsertBack(pSample);
    }

    // Unless we are paused, start an async operation to dispatch the next sample.
    if (SUCCEEDED(hr))
    {
        if (m_state != State_Paused)
        {
            // Queue the operation.
            hr = QueueAsyncOperation(OpProcessSample); 
        }
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: PlaceMarker 
// Description: Receives a marker. [Asynchronous]
//
// Note: The client can call PlaceMarker at any time. In response,
//       we need to queue an MEStreamSinkMarer event, but not until
//       *after* we have processed all samples that we have received
//       up to this point. 
//
//       Also, in general you might need to handle specific marker
//       types, although this sink does not.
//-------------------------------------------------------------------

HRESULT CWavStream::PlaceMarker( 
    MFSTREAMSINK_MARKER_TYPE eMarkerType,
    const PROPVARIANT *pvarMarkerValue,
    const PROPVARIANT *pvarContextValue)
{

    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;
    
    IMarker *pMarker = NULL;
    
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = ValidateOperation(OpPlaceMarker);
    }

    // Create a marker object and put it on the sample queue.
    if (SUCCEEDED(hr))
    {
        hr = CMarker::Create(
            eMarkerType,
            pvarMarkerValue,
            pvarContextValue,
            &pMarker);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_SampleQueue.InsertBack(pMarker);
    }

    // Unless we are paused, start an async operation to dispatch the next sample/marker.
    if (SUCCEEDED(hr))
    {
        if (m_state != State_Paused)
        {
            // Queue the operation.
            hr = QueueAsyncOperation(OpPlaceMarker); // Increments ref count on pOp.
        }
    }

    SAFE_RELEASE(pMarker);

    return hr;   
}


//-------------------------------------------------------------------
// Name: Flush 
// Description: Discards all samples that were not processed yet.
//-------------------------------------------------------------------

HRESULT CWavStream::Flush()
{
    AutoLock lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        // Note: Even though we are flushing data, we still need to send 
        // any marker events that were queued.
        hr = ProcessSamplesFromQueue(DropSamples);
    }

    return hr;
}


/// IMFMediaTypeHandler methods

//-------------------------------------------------------------------
// Name: IsMediaTypeSupported 
// Description: Check if a media type is supported.
//
// pMediaType: The media type to check.
// ppMediaType: Optionally, receives a "close match" media type.
//-------------------------------------------------------------------


HRESULT CWavStream::IsMediaTypeSupported( 
    /* [in] */ IMFMediaType *pMediaType,
    /* [out] */ IMFMediaType **ppMediaType)
{
    if (pMediaType == NULL)
    {
        return E_INVALIDARG;
    }

    AutoLock lock(m_critSec);

    GUID majorType = GUID_NULL;
    WAVEFORMATEX *pWav = NULL;
    UINT cbSize = 0;

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = pMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
    }

    // First make sure it's audio. 
    if (SUCCEEDED(hr))
    {
        if (majorType != MFMediaType_Audio)
        {
            hr = MF_E_INVALIDTYPE;
        }
    }

    // Get a WAVEFORMATEX structure to validate against.
    if (SUCCEEDED(hr))
    {
        hr = MFCreateWaveFormatExFromMFMediaType(pMediaType, &pWav, &cbSize);
    }

    // Validate the WAVEFORMATEX structure.
    if (SUCCEEDED(hr))
    {
        hr = ValidateWaveFormat(pWav, cbSize);
    }

    // We don't return any "close match" types.
    if (ppMediaType)
    {
        *ppMediaType = NULL;
    }


    CoTaskMemFree(pWav);

    return hr;
}


//-------------------------------------------------------------------
// Name: GetMediaTypeCount 
// Description: Return the number of preferred media types.
//-------------------------------------------------------------------

HRESULT CWavStream::GetMediaTypeCount(DWORD *pdwTypeCount)
{
    if (pdwTypeCount == NULL)
    {
        return E_INVALIDARG;
    }

    AutoLock lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *pdwTypeCount = g_NumAudioFormats;
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: GetMediaTypeByIndex 
// Description: Return a preferred media type by index.
//-------------------------------------------------------------------

HRESULT CWavStream::GetMediaTypeByIndex( 
    /* [in] */ DWORD dwIndex,
    /* [out] */ IMFMediaType **ppType)
{
    if (ppType == NULL)
    {
        return E_INVALIDARG;
    }

    AutoLock lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        if (dwIndex >= g_NumAudioFormats)
        {
            hr = MF_E_NO_MORE_TYPES;
        }
    }

    if (SUCCEEDED(hr))
    {
        const DWORD   nSamplesPerSec = g_AudioFormats[dwIndex].nSamplesPerSec;
        const WORD    wBitsPerSample = g_AudioFormats[dwIndex].wBitsPerSample;
        const WORD    nChannels = g_AudioFormats[dwIndex].nChannels;

        hr = CreatePCMAudioType(nSamplesPerSec, wBitsPerSample, nChannels, ppType);
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: SetCurrentMediaType 
// Description: Set the current media type.
//-------------------------------------------------------------------

HRESULT CWavStream::SetCurrentMediaType(IMFMediaType *pMediaType)
{
    if (pMediaType == NULL)
    {
        return E_INVALIDARG;
    }

    AutoLock lock(m_critSec);

    HRESULT hr = CheckShutdown();

    // We don't allow format changes after streaming starts,
    // because this would invalidate the .wav file.
    if (SUCCEEDED(hr))
    {
        hr = ValidateOperation(OpSetMediaType);
    }

    if (SUCCEEDED(hr))
    {
        hr = IsMediaTypeSupported(pMediaType, NULL);
    }

    if (SUCCEEDED(hr))
    {
        SAFE_RELEASE(m_pCurrentType);
        m_pCurrentType = pMediaType;
        m_pCurrentType->AddRef();

        m_state = State_Ready;
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: GetCurrentMediaType 
// Description: Return the current media type, if any.
//-------------------------------------------------------------------

HRESULT CWavStream::GetCurrentMediaType(IMFMediaType **ppMediaType)
{
    AutoLock lock(m_critSec);

    if (ppMediaType == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        if (m_pCurrentType == NULL)
        {
            hr = MF_E_NOT_INITIALIZED;
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppMediaType = m_pCurrentType;
        (*ppMediaType)->AddRef();
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: GetMajorType 
// Description: Return the major type GUID.
//-------------------------------------------------------------------

HRESULT CWavStream::GetMajorType(GUID *pguidMajorType)
{
    if (pguidMajorType == NULL)
    {
        return E_INVALIDARG;
    }

    *pguidMajorType = MFMediaType_Audio;

    return S_OK;
}


// private methods



//-------------------------------------------------------------------
// Name: Initialize 
// Description: Initializes the stream sink.
//
// Note: This method is called once when the media sink is first
//       initialized.
//-------------------------------------------------------------------

HRESULT CWavStream::Initialize(CWavSink *pParent, IMFByteStream *pByteStream)
{
    assert(pParent != NULL);
    assert(pByteStream != NULL);

    HRESULT hr = S_OK;
    
    DWORD dwCaps  = 0;
    const DWORD dwRequiredCaps = (MFBYTESTREAM_IS_WRITABLE | MFBYTESTREAM_IS_SEEKABLE);

    // Make sure the byte stream has the necessary caps bits.
    hr = pByteStream->GetCapabilities(&dwCaps);

    if (SUCCEEDED(hr))
    {
        if ((dwCaps & dwRequiredCaps) != dwRequiredCaps)
        {
            hr = E_FAIL;
        }
    }

    // Move the file pointer to leave room for the RIFF headers.
    if (SUCCEEDED(hr))
    {
        hr = pByteStream->SetCurrentPosition(sizeof(WAV_FILE_HEADER));
    }

    // Create the event queue helper.
    if (SUCCEEDED(hr))
    {
        hr = MFCreateEventQueue(&m_pEventQueue);
    }

    // Allocate a new work queue for async operations.
    if (SUCCEEDED(hr))
    {
        hr = MFAllocateWorkQueue(&m_WorkQueueId);
    }

    if (SUCCEEDED(hr))
    {
        m_pByteStream = pByteStream;
        m_pByteStream->AddRef();

        m_pSink = pParent;
        m_pSink->AddRef();
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: Start 
// Description: Called when the presentation clock starts.
//
// Note: Start time can be PRESENTATION_CURRENT_POSITION meaning
//       resume from the last current position.
//-------------------------------------------------------------------

HRESULT CWavStream::Start(MFTIME start)
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    hr = ValidateOperation(OpStart);

    if (SUCCEEDED(hr))
    {
        if (start != PRESENTATION_CURRENT_POSITION)
        {
            m_StartTime = start;        // Cache the start time.
        }
        m_state = State_Started;
        hr = QueueAsyncOperation(OpStart);
    }
    return hr;

}

//-------------------------------------------------------------------
// Name: Stop
// Description: Called when the presentation clock stops.
//-------------------------------------------------------------------

HRESULT CWavStream::Stop()
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    hr = ValidateOperation(OpStop);

    if (SUCCEEDED(hr))
    {
        m_state = State_Stopped;
        hr = QueueAsyncOperation(OpStop);
    }
    return hr;
}


//-------------------------------------------------------------------
// Name: Pause
// Description: Called when the presentation clock pauses.
//-------------------------------------------------------------------

HRESULT CWavStream::Pause()
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    hr = ValidateOperation(OpPause);

    if (SUCCEEDED(hr))
    {
        m_state = State_Paused;
        hr = QueueAsyncOperation(OpPause);
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: Restart
// Description: Called when the presentation clock restarts.
//-------------------------------------------------------------------

HRESULT CWavStream::Restart()
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    hr = ValidateOperation(OpRestart);

    if (SUCCEEDED(hr))
    {
        m_state = State_Started;
        hr = QueueAsyncOperation(OpRestart);
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: Finalize
// Description: Starts the async finalize operation.
//-------------------------------------------------------------------

HRESULT CWavStream::Finalize(IMFAsyncCallback *pCallback, IUnknown *punkState)
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    hr = ValidateOperation(OpFinalize);

    if (SUCCEEDED(hr))
    {
        if (m_pFinalizeResult != NULL)
        {
            hr = MF_E_INVALIDREQUEST;  // The operation is already pending.
        }
    }

    // Create and store the async result object.
    if (SUCCEEDED(hr))
    {
        hr = MFCreateAsyncResult(NULL, pCallback, punkState, &m_pFinalizeResult);
    }

    if (SUCCEEDED(hr))
    {
        m_state = State_Finalized;
        hr = QueueAsyncOperation(OpFinalize); 
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: ValidStateMatrix
// Description: Class-static matrix of operations vs states.
//
// If an entry is TRUE, the operation is valid from that state.
//-------------------------------------------------------------------

BOOL CWavStream::ValidStateMatrix[CWavStream::State_Count][CWavStream::Op_Count] = 
{
// States:    Operations:
//            SetType   Start     Restart   Pause     Stop      Sample    Marker    Finalize
/* NotSet */  TRUE,     FALSE,    FALSE,    FALSE,    FALSE,    FALSE,    FALSE,    FALSE,

/* Ready */   TRUE,     TRUE,     FALSE,    TRUE,     TRUE,     FALSE,    TRUE,     TRUE,

/* Start */   FALSE,    TRUE,     FALSE,    TRUE,     TRUE,     TRUE,     TRUE,     TRUE,

/* Pause */   FALSE,    TRUE,     TRUE,     TRUE,     TRUE,     TRUE,     TRUE,     TRUE,

/* Stop */    FALSE,    TRUE,     FALSE,    FALSE,    TRUE,     FALSE,    TRUE,     TRUE,

/* Final */   FALSE,    FALSE,    FALSE,    FALSE,    FALSE,    FALSE,    FALSE,    FALSE

// Note about states:
// 1. OnClockRestart should only be called from paused state.
// 2. While paused, the sink accepts samples but does not process them.

};


//-------------------------------------------------------------------
// Name: ValidateOperation
// Description: Checks if an operation is valid in the current state.
//-------------------------------------------------------------------

HRESULT CWavStream::ValidateOperation(StreamOperation op)
{
    assert(!m_IsShutdown);

    HRESULT hr = S_OK;
    
    BOOL bTransitionAllowed = ValidStateMatrix[m_state][op];

    if (bTransitionAllowed)
    {
        return S_OK;
    }
    else
    {
        return MF_E_INVALIDREQUEST;
    }
}

//-------------------------------------------------------------------
// Name: Shutdown
// Description: Shuts down the stream sink.
//-------------------------------------------------------------------

HRESULT CWavStream::Shutdown()
{
    assert(!m_IsShutdown);

    if (m_pEventQueue)
    {
        m_pEventQueue->Shutdown();
    }

    MFUnlockWorkQueue(m_WorkQueueId);

    m_SampleQueue.Clear();

    SAFE_RELEASE(m_pSink);
    SAFE_RELEASE(m_pEventQueue);
    SAFE_RELEASE(m_pByteStream);
    SAFE_RELEASE(m_pCurrentType);
    SAFE_RELEASE(m_pFinalizeResult);

    m_IsShutdown = TRUE;

    return S_OK;
}


//-------------------------------------------------------------------
// Name: QueueAsyncOperation
// Description: Puts an async operation on the work queue.
//-------------------------------------------------------------------

HRESULT CWavStream::QueueAsyncOperation(StreamOperation op)
{
    HRESULT hr = S_OK;
    CAsyncOperation *pOp = new CAsyncOperation(op); // Created with ref count = 1
    if (pOp == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = MFPutWorkItem(m_WorkQueueId, &m_WorkQueueCB, pOp); 
    }

    SAFE_RELEASE(pOp);

    return hr;
}



//-------------------------------------------------------------------
// Name: OnDispatchWorkItem
// Description: Callback for MFPutWorkItem.
//-------------------------------------------------------------------

HRESULT CWavStream::OnDispatchWorkItem(IMFAsyncResult* pAsyncResult)
{
    // Called by work queue thread. Need to hold the critical section.
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    IUnknown *pState = NULL;

    hr = pAsyncResult->GetState(&pState);

    if (SUCCEEDED(hr))
    {
        // The state object is a CAsncOperation object.
        CAsyncOperation *pOp = (CAsyncOperation*)pState;

        StreamOperation op = pOp->m_op;

        switch (op)
        {
        case OpStart:
        case OpRestart:
            // Send MEStreamSinkStarted.
            hr = QueueEvent(MEStreamSinkStarted, GUID_NULL, hr, NULL);
            
            // Kick things off by requesting two samples...
            if (SUCCEEDED(hr))
            {
                hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL);
            }
            if (SUCCEEDED(hr))
            {
                hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL);
            }

            // There might be samples queue from earlier (ie, while paused).
            if (SUCCEEDED(hr))
            {
                hr = ProcessSamplesFromQueue(WriteSamples);
            }
            break;

        case OpStop:
            // Drop samples from queue.
            hr = ProcessSamplesFromQueue(DropSamples);

            // Send the event even if the previous call failed.
            hr = QueueEvent(MEStreamSinkStopped, GUID_NULL, hr, NULL);
            break;

        case OpPause:
            hr = QueueEvent(MEStreamSinkPaused, GUID_NULL, hr, NULL);
            break;

        case OpProcessSample:
        case OpPlaceMarker:
            hr = DispatchProcessSample(pOp);
            break;

        case OpFinalize:
            hr = DispatchFinalize(pOp);
            break;
        }
    }

    SAFE_RELEASE(pState);

    return hr;
}


//-------------------------------------------------------------------
// Name: DispatchProcessSample
// Description: Complete a ProcessSample or PlaceMarker request.
//-------------------------------------------------------------------

HRESULT CWavStream::DispatchProcessSample(CAsyncOperation* pOp)
{
    HRESULT hr = S_OK;
    assert(pOp != NULL);

    hr = ProcessSamplesFromQueue(WriteSamples);

    // Ask for another sample
    if (SUCCEEDED(hr))
    {
        if (pOp->m_op == OpProcessSample)
        {
            hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, S_OK, NULL);
        }
    }

    // We are in the middle of an asynchronous operation, so if something failed, send an error.
    if (FAILED(hr))
    {
        hr = QueueEvent(MEError, GUID_NULL, hr, NULL);
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: ProcessSamplesFromQueue
// Description: 
//
// Removes all of the samples and markers that are currently in the
// queue and processes them.
//
// If bFlushData = DropSamples:
//     For each marker, send an MEStreamSinkMarker event, with hr = E_ABORT.
//     For each sample, drop the sample.
//
// If bFlushData = WriteSamples
//     For each marker, send an MEStreamSinkMarker event, with hr = S_OK.
//     For each sample, write the sample to the file.
//
// This method is called when we flush, stop, restart, receive a new
// sample, or receive a marker.
//-------------------------------------------------------------------

HRESULT CWavStream::ProcessSamplesFromQueue(FlushState bFlushData)
{
    HRESULT hr = S_OK;

    ComPtrList<IUnknown>::POSITION pos = m_SampleQueue.FrontPosition();

    // Enumerate all of the samples/markers in the queue.

    while (pos != m_SampleQueue.EndPosition())
    {
        IUnknown *pUnk = NULL;
        IMarker  *pMarker = NULL;
        IMFSample *pSample = NULL;

        hr = m_SampleQueue.GetItemPos(pos, &pUnk);

        assert(pUnk != NULL); // GetItemPos should not fail unless we reached the end of the list.

        // Figure out if this is a marker or a sample.
        if (SUCCEEDED(hr))
        {
            hr = pUnk->QueryInterface(__uuidof(IMarker), (void**)&pMarker);
            if (hr == E_NOINTERFACE)
            {
                // If this is a sample, write it to the file.
                hr = pUnk->QueryInterface(IID_IMFSample, (void**)&pSample);
            }
        }

        // Now handle the sample/marker appropriately.
        if (SUCCEEDED(hr))
        {
            if (pMarker)
            {
                hr = SendMarkerEvent(pMarker, bFlushData);
            }
            else 
            {
                assert(pSample != NULL);    // Not a marker, must be a sample
                if (bFlushData == WriteSamples)
                {
                    hr = WriteSampleToFile(pSample);
                }
            }
        }
        SAFE_RELEASE(pUnk);
        SAFE_RELEASE(pMarker);
        SAFE_RELEASE(pSample);

        if (FAILED(hr))
        {
            break;
        }

        pos = m_SampleQueue.Next(pos);

    }       // while loop

    // Now clear the list.
    m_SampleQueue.Clear();

    return hr;
}


//-------------------------------------------------------------------
// Name: WriteSampleToFile
// Description: Output one media sample to the file.
//-------------------------------------------------------------------

HRESULT CWavStream::WriteSampleToFile(IMFSample *pSample)
{
    HRESULT hr = S_OK;

    LONGLONG time = 0;
    DWORD cBufferCount = 0; // Number of buffers in the sample.
    BYTE *pData = NULL;
    DWORD cbData = 0;
    DWORD cbWritten = 0;

    // Get the time stamp
    hr = pSample->GetSampleTime(&time);

    if (SUCCEEDED(hr))
    {
        // If the time stamp is too early, just discard this sample.
        if (time < m_StartTime)
        {
            return S_OK;
        }
    }
    // Note: If there is no time stamp on the sample, proceed anyway.

    // Find how many buffers are in this sample.
    hr = pSample->GetBufferCount(&cBufferCount);
    if (SUCCEEDED(hr))
    {
        // Loop through all the buffers in the sample.
        for (DWORD iBuffer = 0; iBuffer < cBufferCount; iBuffer++)
        {
            IMFMediaBuffer *pBuffer = NULL;

            hr = pSample->GetBufferByIndex(iBuffer, &pBuffer);

            // Lock the buffer and write the data to the file.
            if (SUCCEEDED(hr))
            {
                hr = pBuffer->Lock(&pData, NULL, &cbData);
            }

            if (SUCCEEDED(hr))
            {
                hr = m_pByteStream->Write(pData, cbData, &cbWritten);
                pBuffer->Unlock();
            }

            // Update the running tally of bytes written.
            if (SUCCEEDED(hr))
            {
                m_cbDataWritten += cbData;
            }

            SAFE_RELEASE(pBuffer);

            if (FAILED(hr))
            {
                break;
            }
        }   // for loop
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: SendMarkerEvent
// Description: Saned a marker event.
// 
// pMarker: Pointer to our custom IMarker interface, which holds
//          the marker information.
//-------------------------------------------------------------------

HRESULT CWavStream::SendMarkerEvent(IMarker *pMarker, FlushState FlushState)
{
    HRESULT hr = S_OK;
    HRESULT hrStatus = S_OK;  // Status code for marker event.
    
    if (FlushState == DropSamples)
    {
        hrStatus = E_ABORT; 
    }

    PROPVARIANT var;
    PropVariantInit(&var);

    // Get the context data.
    hr = pMarker->GetContext(&var);

    if (SUCCEEDED(hr))
    {
        hr = QueueEvent(MEStreamSinkMarker, GUID_NULL, hrStatus, &var);
    }

    PropVariantClear(&var);
    return hr;
}


//-------------------------------------------------------------------
// Name: DispatchFinalize
// Description: Complete a BeginFinalize request.
//-------------------------------------------------------------------

HRESULT CWavStream::DispatchFinalize(CAsyncOperation* pOp)
{
    HRESULT hr = S_OK; 

    WAVEFORMATEX *pWav = NULL;
    UINT cbSize = 0;
    DWORD cbWritten = 0;

    WAV_FILE_HEADER header;
    ZeroMemory(&header, sizeof(header));

    DWORD cbFileSize = m_cbDataWritten + sizeof(WAV_FILE_HEADER) - sizeof(RIFFCHUNK);

    // Write any samples left in the queue...
    hr = ProcessSamplesFromQueue(WriteSamples);

    // Now we're done writing all of the audio data. 

    // Fill in the RIFF headers...
    if (SUCCEEDED(hr))
    {
        hr = MFCreateWaveFormatExFromMFMediaType(m_pCurrentType, &pWav, &cbSize);
    }

    if (SUCCEEDED(hr))
    {
        header.FileHeader.fcc = MAKEFOURCC('R', 'I', 'F', 'F');
        header.FileHeader.cb = cbFileSize;
        header.fccWaveType = MAKEFOURCC('W', 'A', 'V', 'E');
        header.WaveHeader.fcc = MAKEFOURCC('f', 'm', 't', ' ');
        header.WaveHeader.cb = RIFFROUND(sizeof(WAVEFORMATEX));
   
        CopyMemory(&header.WaveFormat, pWav, sizeof(WAVEFORMATEX));
        header.DataHeader.fcc = MAKEFOURCC('d', 'a', 't', 'a');
        header.DataHeader.cb = m_cbDataWritten;
    }
    
    // Move the file pointer back to the start of the file and write the
    // RIFF headers.
    if (SUCCEEDED(hr))
    {
        hr = m_pByteStream->SetCurrentPosition(0);
    }
    if (SUCCEEDED(hr))
    {
        hr = m_pByteStream->Write((BYTE*)&header, sizeof(WAV_FILE_HEADER), &cbWritten);
    }

    // Close the byte stream.
    if (SUCCEEDED(hr))
    {
        hr = m_pByteStream->Close();

    }

    // Set the async status and invoke the callback.
    m_pFinalizeResult->SetStatus(hr);
    hr = MFInvokeCallback(m_pFinalizeResult);

    CoTaskMemFree(pWav);

    return hr;
}




//////////////////////
// CMarker class
// Holds information from IMFStreamSink::PlaceMarker
// 

CMarker::CMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType) : m_nRefCount(1), m_eMarkerType(eMarkerType)
{
    PropVariantInit(&m_varMarkerValue);
    PropVariantInit(&m_varContextValue);
}

CMarker::~CMarker()
{
    assert(m_nRefCount == 0);

    PropVariantClear(&m_varMarkerValue);
    PropVariantClear(&m_varContextValue);
}

/* static */ 
HRESULT CMarker::Create(
    MFSTREAMSINK_MARKER_TYPE eMarkerType,
    const PROPVARIANT* pvarMarkerValue,     // Can be NULL.
    const PROPVARIANT* pvarContextValue,    // Can be NULL.
    IMarker **ppMarker
    )
{
    if (ppMarker == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    CMarker *pMarker = new CMarker(eMarkerType);

    if (pMarker == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    // Copy the marker data.
    if (SUCCEEDED(hr))
    {
        if (pvarMarkerValue)
        {
            hr = PropVariantCopy(&pMarker->m_varMarkerValue, pvarMarkerValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (pvarContextValue)
        {
            hr = PropVariantCopy(&pMarker->m_varContextValue, pvarContextValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppMarker = pMarker;
        (*ppMarker)->AddRef();
    }

    SAFE_RELEASE(pMarker);

    return hr;
}

// IUnknown methods.

ULONG CMarker::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG CMarker::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT CMarker::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == __uuidof(IMarker))
    {
        *ppv = static_cast<IMarker*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

// IMarker methods
HRESULT CMarker::GetMarkerType(MFSTREAMSINK_MARKER_TYPE *pType)
{
    if (pType == NULL)
    {
        return E_POINTER;
    }

    *pType = m_eMarkerType;
    return S_OK;
}

HRESULT CMarker::GetMarkerValue(PROPVARIANT *pvar)
{
    if (pvar == NULL)
    {
        return E_POINTER;
    }
    return PropVariantCopy(pvar, &m_varMarkerValue);

}
HRESULT CMarker::GetContext(PROPVARIANT *pvar)
{
    if (pvar == NULL)
    {
        return E_POINTER;
    }
    return PropVariantCopy(pvar, &m_varContextValue);
}


//-------------------------------------------------------------------
// CreatePCMAudioType
//
// Creates a media type that describes an uncompressed PCM audio
// format.
//-------------------------------------------------------------------

HRESULT CreatePCMAudioType(
    UINT32 sampleRate,        // Samples per second
    UINT32 bitsPerSample,     // Bits per sample
    UINT32 cChannels,         // Number of channels
    IMFMediaType **ppType     // Receives a pointer to the media type.
    )
{
    HRESULT hr = S_OK;

    IMFMediaType *pType = NULL;

    // Calculate derived values.
    UINT32 blockAlign = cChannels * (bitsPerSample / 8);
    UINT32 bytesPerSecond = blockAlign * sampleRate;

    // Create the empty media type.
    hr = MFCreateMediaType(&pType);

    // Set attributes on the type.
    if (SUCCEEDED(hr))
    {
        hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    }

    if (SUCCEEDED(hr))
    {
        hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    }

    if (SUCCEEDED(hr))
    {
        hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, cChannels);
    }

    if (SUCCEEDED(hr))
    {
        hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
    }

    if (SUCCEEDED(hr))
    {
        hr = pType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, blockAlign);
    }

    if (SUCCEEDED(hr))
    {
        hr = pType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bytesPerSecond);
    }

    if (SUCCEEDED(hr))
    {
        hr = pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
    }

    if (SUCCEEDED(hr))
    {
        hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Return the type to the caller.
        *ppType = pType;
        (*ppType)->AddRef();
    }

    SAFE_RELEASE(pType);
    return hr;
}




//-------------------------------------------------------------------
// Name: ValidateWaveFormat
// Description: Validates a WAVEFORMATEX structure. 
//
// Just to keep the sample as simple as possible, we only accept 
// uncompressed PCM formats.
//-------------------------------------------------------------------

HRESULT ValidateWaveFormat(const WAVEFORMATEX *pWav, DWORD cbSize)
{
    if (pWav->wFormatTag != WAVE_FORMAT_PCM)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    if (pWav->nChannels != 1 && pWav->nChannels != 2)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    if (pWav->wBitsPerSample != 8 && pWav->wBitsPerSample != 16)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    if (pWav->cbSize != 0)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    // Make sure block alignment was calculated correctly.
    if (pWav->nBlockAlign != pWav->nChannels * (pWav->wBitsPerSample / 8))
    {   
        return MF_E_INVALIDMEDIATYPE;
    }

    // Check possible overflow...
    if (pWav->nSamplesPerSec  > (DWORD)(MAXDWORD / pWav->nBlockAlign))        // Is (nSamplesPerSec * nBlockAlign > MAXDWORD) ?
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    // Make sure average bytes per second was calculated correctly.
    if (pWav->nAvgBytesPerSec != pWav->nSamplesPerSec * pWav->nBlockAlign)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    // Everything checked out.
    return S_OK;
}


#pragma warning( pop )



