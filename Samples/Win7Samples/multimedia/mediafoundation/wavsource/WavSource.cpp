//////////////////////////////////////////////////////////////////////
//
// WavSource.cpp : Sample media source for Media Foundation
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////


// Misc implementation notes.
//
// Locking: 
//     The source and stream objects both have critical sections. If you 
//     hold both locks, the source lock must be held FIRST, to avoid 
//     deadlocks. 
//
// Shutdown: 
//     Most methods start by calling CheckShutdown(). This method
//     fails if the source was shut down.
//

#include "WavSource.h"

#include <assert.h>


template <class T>
T AlignUp(T num, T mult)
{
    assert(num >= 0);
    T tmp = num + mult - 1;
    return tmp - (tmp % mult);
}



// Helper Functions

HRESULT QueueEventWithIUnknown(
    IMFMediaEventGenerator *pMEG,
    MediaEventType meType,
    HRESULT hrStatus,
    IUnknown *pUnk);

HRESULT ValidateWaveFormat(const WAVEFORMATEX *pWav, DWORD cbSize);

LONGLONG BufferSizeFromAudioDuration(const WAVEFORMATEX *pWav, LONGLONG duration);


//-------------------------------------------------------------------
// Name: CreateInstance
// Description: Static method to create an instance of the source.
//
// iid:         IID of the requested interface on the source.
// ppSource:    Receives a ref-counted pointer to the source.
//-------------------------------------------------------------------

HRESULT WavSource::CreateInstance(REFIID iid, void **ppSource)
{
    if (ppSource == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    WavSource *pSource = new (std::nothrow) WavSource(hr);
    if (pSource == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = pSource->QueryInterface(iid, ppSource);
    }

    SafeRelease(&pSource);
    return hr;
}


//-------------------------------------------------------------------
// WavSource constructor.
//
// hr: If the constructor fails, this value is set to a failure code.
//-------------------------------------------------------------------

WavSource::WavSource(HRESULT& hr) 
  : m_nRefCount(1),
    m_pEventQueue(NULL),
    m_pPresentationDescriptor(NULL),
    m_IsShutdown(FALSE),
    m_state(STATE_STOPPED),
    m_pStream(NULL),
    m_pRiff(NULL)
{
    DllAddRef();

    // Create the media event queue.
    hr = MFCreateEventQueue(&m_pEventQueue);

    InitializeCriticalSection(&m_critSec);
}


//-------------------------------------------------------------------
// WavSource destructor.
//-------------------------------------------------------------------


WavSource::~WavSource()
{
    DllRelease();

    assert(m_IsShutdown);
    assert(m_nRefCount == 0);

    DeleteCriticalSection(&m_critSec);
}


// IUnknown methods

ULONG WavSource::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG  WavSource::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT WavSource::QueryInterface(REFIID iid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(WavSource, IMFMediaEventGenerator),
        QITABENT(WavSource, IMFMediaSource),
        { 0 }
    };
    return QISearch(this, qit, iid, ppv);
}


// IMFMediaEventGenerator methods
//
// All of the IMFMediaEventGenerator methods do the following:
// 1. Check for shutdown status.
// 2. Call the event generator helper object.

HRESULT WavSource::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {   
        hr = m_pEventQueue->BeginGetEvent(pCallback, punkState);
    }

    LeaveCriticalSection(&m_critSec);

    return hr;
}

HRESULT WavSource::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {   
        hr = m_pEventQueue->EndGetEvent(pResult, ppEvent);
    }

    LeaveCriticalSection(&m_critSec);

    return hr;
}

HRESULT WavSource::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    // NOTE: GetEvent can block indefinitely, so we don't hold the
    //       WavSource lock. This requires some juggling with the
    //       event queue pointer.

    HRESULT hr = S_OK;

    IMFMediaEventQueue *pQueue = NULL;

    EnterCriticalSection(&m_critSec);

    // Check shutdown
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        pQueue = m_pEventQueue;
        pQueue->AddRef();
    }

    LeaveCriticalSection(&m_critSec);

    if (SUCCEEDED(hr))
    {   
        hr = pQueue->GetEvent(dwFlags, ppEvent);
    }

    SafeRelease(&pQueue);
    return hr;
}

HRESULT WavSource::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {   
        hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
    }

    LeaveCriticalSection(&m_critSec);

    return hr;
}


// IMFMediaSource methods


//-------------------------------------------------------------------
// Name: CreatePresentationDescriptor
// Description: Returns a copy of the default presentation descriptor.
//-------------------------------------------------------------------

HRESULT WavSource::CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor)
{
    if (ppPresentationDescriptor == NULL)
    {
        return E_POINTER;
    }

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {   
        if (m_pPresentationDescriptor == NULL)
        {
            hr = CreatePresentationDescriptor();
        }
    }

    // Clone our default presentation descriptor.
    if (SUCCEEDED(hr))
    {   
        hr = m_pPresentationDescriptor->Clone(ppPresentationDescriptor);
    }

    LeaveCriticalSection(&m_critSec);

    return hr;
}


//-------------------------------------------------------------------
// Name: GetCharacteristics
// Description: Returns flags the describe the source.
//-------------------------------------------------------------------

HRESULT WavSource::GetCharacteristics(DWORD* pdwCharacteristics)
{
    if (pdwCharacteristics == NULL)
    {
        return E_POINTER;
    }

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;
    
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {   
        *pdwCharacteristics =  MFMEDIASOURCE_CAN_PAUSE | MFMEDIASOURCE_CAN_SEEK;
    }

    LeaveCriticalSection(&m_critSec);

    return hr;
}

//-------------------------------------------------------------------
// Name: Start
// Description: Switches to running state.
//-------------------------------------------------------------------

HRESULT WavSource::Start(
    IMFPresentationDescriptor* pPresentationDescriptor,
    const GUID* pguidTimeFormat,
    const PROPVARIANT* pvarStartPosition
    )
{
    HRESULT hr = S_OK;
    LONGLONG llStartOffset = 0;
    BOOL bIsSeek = FALSE;    
    BOOL bIsRestartFromCurrentPosition = FALSE;     
    BOOL bQueuedStartEvent = FALSE;

    IMFMediaEvent *pEvent = NULL;

    PROPVARIANT var;
    PropVariantInit(&var);

    // Check parameters. 
    // Start position and presentation descriptor cannot be NULL.
    if (pvarStartPosition == NULL || pPresentationDescriptor == NULL)
    {
        return E_INVALIDARG;
    }

    // Check the time format. Must be "reference time" units.
    if ((pguidTimeFormat != NULL) && (*pguidTimeFormat != GUID_NULL))
    {
        // Unrecognized time format GUID.
        return MF_E_UNSUPPORTED_TIME_FORMAT;
    }

    EnterCriticalSection(&m_critSec);

    // Fail if the source is shut down.
    hr = CheckShutdown();

    if (FAILED(hr)) { goto done; }

    // Check the start position.
    if (pvarStartPosition->vt == VT_I8)
    {
        // Start position is given in pvarStartPosition in 100-ns units.
        llStartOffset = pvarStartPosition->hVal.QuadPart;

        if (m_state != STATE_STOPPED)
        {
            // Source is running or paused, so this is a seek.
            bIsSeek = TRUE;
        }
    }
    else if (pvarStartPosition->vt == VT_EMPTY)
    {
        // Start position is "current position". 
        // For stopped, that means 0. Otherwise, use the current position.
        if (m_state == STATE_STOPPED)
        {
            llStartOffset = 0;
        }
        else
        {
            llStartOffset = GetCurrentPosition();
            bIsRestartFromCurrentPosition = TRUE;
        }
    }
    else
    {
        // We don't support this time format.
        hr =  MF_E_UNSUPPORTED_TIME_FORMAT;
        goto done;
    }

    // Validate the caller's presentation descriptor.
    hr = ValidatePresentationDescriptor(pPresentationDescriptor);

    if (FAILED(hr)) { goto done; }

    // Sends the MENewStream or MEUpdatedStream event.
    hr = QueueNewStreamEvent(pPresentationDescriptor);

    if (FAILED(hr)) { goto done; }

    // Notify the stream of the new start time.
    hr = m_pStream->SetPosition(llStartOffset);

    if (FAILED(hr)) { goto done; }

    // Send Started or Seeked events. 

    var.vt = VT_I8;
    var.hVal.QuadPart = llStartOffset;

    // Send the source event.
    if (bIsSeek)
    {
        hr = QueueEvent(MESourceSeeked, GUID_NULL, hr, &var);

        if (FAILED(hr)) { goto done; }
    }
    else
    {
        // For starting, if we are RESTARTING from the current position and our 
        // previous state was running/paused, then we need to add the 
        // MF_EVENT_SOURCE_ACTUAL_START attribute to the event. This requires
        // creating the event object first.

        // Create the event.
        hr = MFCreateMediaEvent(MESourceStarted, GUID_NULL, hr, &var, &pEvent);

        if (FAILED(hr)) { goto done; }

        // For restarts, set the actual start time as an attribute.
        if (bIsRestartFromCurrentPosition)
        {
            hr = pEvent->SetUINT64(MF_EVENT_SOURCE_ACTUAL_START, llStartOffset);
            if (FAILED(hr)) { goto done; }
        }
        
        // Now  queue the event.
        hr = m_pEventQueue->QueueEvent(pEvent);

        if (FAILED(hr)) { goto done; }
    }

    bQueuedStartEvent = TRUE;

    // Send the stream event.
    if (m_pStream)
    {
        if (bIsSeek)
        {
            hr = m_pStream->QueueEvent(MEStreamSeeked, GUID_NULL, hr, &var);
        }
        else
        {
            hr = m_pStream->QueueEvent(MEStreamStarted, GUID_NULL, hr, &var);
        }
    
        if (FAILED(hr)) { goto done; }
    }

    if (bIsSeek)
    {
        // For seek requests, flush any queued samples.
        hr = m_pStream->Flush();
    }
    else
    {
        // Otherwise, deliver any queued samples.
        hr = m_pStream->DeliverQueuedSamples();
    }
    
    if (FAILED(hr)) { goto done; }

    m_state = STATE_STARTED;

done:

    // If a failure occurred and we have not sent the 
    // MESourceStarted/MESourceSeeked event yet, then it is 
    // OK just to return an error code from Start().

    // If a failure occurred and we have already sent the 
    // event (with a success code), then we need to raise an
    // MEError event.

    if (FAILED(hr) && bQueuedStartEvent)
    {
        hr = QueueEvent(MEError, GUID_NULL, hr, &var);
    }

    PropVariantClear(&var);
    SafeRelease(&pEvent);

    LeaveCriticalSection(&m_critSec);

    return hr;
}


//-------------------------------------------------------------------
// Name: Pause
// Description: Switches to paused state.
//-------------------------------------------------------------------

HRESULT WavSource::Pause()
{
    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;
    
    hr = CheckShutdown();

    // Pause is only allowed from started state.
    if (SUCCEEDED(hr))
    {   
        if (m_state != STATE_STARTED)
        {
            hr = MF_E_INVALID_STATE_TRANSITION;
        }
    }

    // Send the appropriate events.
    if (SUCCEEDED(hr))
    {   
        if (m_pStream)
        {
            hr = m_pStream->QueueEvent(MEStreamPaused, GUID_NULL, S_OK, NULL);
        }
    }

    if (SUCCEEDED(hr))
    {   
        hr = QueueEvent(MESourcePaused, GUID_NULL, S_OK, NULL);
    }

    // Update our state. 
    if (SUCCEEDED(hr))
    {   
        m_state = STATE_PAUSED;
    }

    LeaveCriticalSection(&m_critSec);

    return hr;
}


//-------------------------------------------------------------------
// Name: Stop
// Description: Switches to stopped state.
//-------------------------------------------------------------------

HRESULT WavSource::Stop()
{
    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;
    
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {   
        // Update our state. 
        m_state = STATE_STOPPED;

        // Flush all queued samples.
        hr = m_pStream->Flush();
    }

    //
    // Queue events.
    //

    if (SUCCEEDED(hr))
    {   
        if (m_pStream)
        {
            hr = m_pStream->QueueEvent(MEStreamStopped, GUID_NULL, S_OK, NULL);
        }
    }
    if (SUCCEEDED(hr))
    {   
        hr = QueueEvent(MESourceStopped, GUID_NULL, S_OK, NULL);
    }

    LeaveCriticalSection(&m_critSec);

    return hr;
}


//-------------------------------------------------------------------
// Name: Shutdown
// Description: Releases resources.
//
// The source and stream objects hold reference counts on each other.
// To avoid memory leaks caused by circular ref. counts, the Shutdown
// method releases the pointer to the stream.
//-------------------------------------------------------------------

HRESULT WavSource::Shutdown()
{
    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;
    
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {   
        // Shut down the stream object.
        if (m_pStream)
        {
            (void)m_pStream->Shutdown();
        }

        // Shut down the event queue.
        if (m_pEventQueue)
        {
            (void)m_pEventQueue->Shutdown();
        }

        // Release objects. 
        SafeRelease(&m_pStream);
        SafeRelease(&m_pEventQueue);
        SafeRelease(&m_pPresentationDescriptor);

        delete m_pRiff;
        m_pRiff = NULL;

        // Set our shutdown flag.
        m_IsShutdown = TRUE;
    }

    LeaveCriticalSection(&m_critSec);

    return hr;
}


/// Other public methods


//-------------------------------------------------------------------
// Name: Open
// Description: Opens the source from a bytestream.
//
// The bytestream handler calls this method after it creates the 
// source. 
//
// Note: This method is not a public API. It is a custom method on 
// for our bytestream class to use.
//-------------------------------------------------------------------

HRESULT WavSource::Open(IMFByteStream *pStream)
{
    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    if (m_pRiff != NULL)
    {
        // The media source has already been opened.
        hr = MF_E_INVALIDREQUEST;
    }

    // Create a new WAVE RIFF parser object to parse the stream.
    if (SUCCEEDED(hr))
    {   
        hr = CWavRiffParser::Create(pStream, &m_pRiff);
    }

    // Parse the WAVE header. This fails if the header is not
    // well-formed.
    if (SUCCEEDED(hr))
    {   
        hr = m_pRiff->ParseWAVEHeader();
    }

    // Validate the WAVEFORMATEX structure from the file header.
    if (SUCCEEDED(hr))
    {   
        hr = ValidateWaveFormat(m_pRiff->Format(), m_pRiff->FormatSize());
    }

    if (FAILED(hr))
    {
        Shutdown();
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}



/////////////// Private WavSource methods

// NOTE: These private methods do not hold the source's critical
// section. The caller must ensure the critical section is held.
// Also, these methods do not check for shut-down.


//-------------------------------------------------------------------
// Name: WaveFormat
// Description: 
// Returns a pointer to the WAVEFORMATEX structure that describes the
// audio format. Returns NULL if no format is set.
//-------------------------------------------------------------------

const WAVEFORMATEX* WavSource::WaveFormat() const 
{ 
    if (m_pRiff)
    {
        return m_pRiff->Format();
    }
    else
    {
        return NULL;
    }
}

//-------------------------------------------------------------------
// Name: WaveFormatSize
// Description: 
// Returns the size of the WAVEFORMATEX structure.
//-------------------------------------------------------------------

DWORD WavSource::WaveFormatSize() const 
{  
    if (m_pRiff)
    {
        return m_pRiff->FormatSize();
    }
    else
    {
        return 0;
    }
}



//-------------------------------------------------------------------
// Name: CreatePresentationDescriptor
// Description: Creates the default presentation descriptor.
//-------------------------------------------------------------------

HRESULT WavSource::CreatePresentationDescriptor()
{
    HRESULT hr = S_OK;
    MFTIME duration = 0;

    IMFMediaType *pMediaType = NULL;
    IMFStreamDescriptor *pStreamDescriptor = NULL;
    IMFMediaTypeHandler *pHandler = NULL;

    assert(WaveFormat() != NULL);

    // Create an empty media type.
    hr = MFCreateMediaType(&pMediaType);

    // Initialize the media type from the WAVEFORMATEX structure.
    if (SUCCEEDED(hr))
    {   
        hr = MFInitMediaTypeFromWaveFormatEx(pMediaType, WaveFormat(), WaveFormatSize());
    }

    // Create the stream descriptor.
    if (SUCCEEDED(hr))
    {   
        hr = MFCreateStreamDescriptor(
            0,          // stream identifier
            1,          // Number of media types.
            &pMediaType, // Array of media types
            &pStreamDescriptor
            );
    }

    // Set the default media type on the media type handler.
    if (SUCCEEDED(hr))
    {   
        hr = pStreamDescriptor->GetMediaTypeHandler(&pHandler);
    }

    if (SUCCEEDED(hr))
    {   
        hr = pHandler->SetCurrentMediaType(pMediaType);
    }

    // Create the presentation descriptor.
    if (SUCCEEDED(hr))
    {   
        hr = MFCreatePresentationDescriptor(
            1,                      // Number of stream descriptors
            &pStreamDescriptor,     // Array of stream descriptors
            &m_pPresentationDescriptor
            );
    }

    // Select the first stream
    if (SUCCEEDED(hr))
    {   
        hr = m_pPresentationDescriptor->SelectStream(0);
    }

    // Set the file duration as an attribute on the presentation descriptor.
    if (SUCCEEDED(hr))
    {   
        duration = m_pRiff->FileDuration();
        hr = m_pPresentationDescriptor->SetUINT64(MF_PD_DURATION, (UINT64)duration);
    }

    SafeRelease(&pMediaType);
    SafeRelease(&pStreamDescriptor);
    SafeRelease(&pHandler);
    return hr;
}



//-------------------------------------------------------------------
// Name: ValidatePresentationDescriptor
// Description: Validates the caller's presentation descriptor.
//
// This method is called when Start() is called with a non-NULL
// presentation descriptor. The caller is supposed to give us back
// the same PD that we gave out in CreatePresentationDescriptor().
// This method performs a sanity check on the caller's PD to make 
// sure it matches ours.
//
// Note: Because this media source has one stream with single, fixed 
//       media type, there is not much for the caller to decide. In
//       a more complicated source, the caller might select different
//       streams, or select from a list of media types.
//-------------------------------------------------------------------

HRESULT WavSource::ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD)
{
    HRESULT hr;

    assert(pPD != NULL);

    IMFStreamDescriptor *pStreamDescriptor = NULL;
    IMFMediaTypeHandler *pHandler = NULL;
    IMFMediaType        *pMediaType = NULL;
    WAVEFORMATEX        *pFormat = NULL;

    DWORD   cStreamDescriptors = 0;
    BOOL    fSelected = FALSE;
    UINT32  cbWaveFormat = 0;

    // Make sure there is only one stream.
    hr = pPD->GetStreamDescriptorCount(&cStreamDescriptors);

    if (SUCCEEDED(hr))
    {   
        if (cStreamDescriptors != 1)
        {
            hr = MF_E_UNSUPPORTED_REPRESENTATION;
        }
    }

    // Get the stream descriptor.
    if (SUCCEEDED(hr))
    {   
        hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pStreamDescriptor);
    }

    // Make sure it's selected. (This media source has only one stream, so it
    // is not useful to deselect the only stream.)
    if (SUCCEEDED(hr))
    {   
        if (!fSelected)
        {
            hr = MF_E_UNSUPPORTED_REPRESENTATION;
        }
    }

    // Get the media type handler, so that we can get the media type.
    if (SUCCEEDED(hr))
    {   
        hr = pStreamDescriptor->GetMediaTypeHandler(&pHandler);
    }

    if (SUCCEEDED(hr))
    {   
        hr = pHandler->GetCurrentMediaType(&pMediaType);
    }

    if (SUCCEEDED(hr))
    {   
        hr = MFCreateWaveFormatExFromMFMediaType(
            pMediaType, 
            &pFormat,
            &cbWaveFormat);
    }

    if (SUCCEEDED(hr))
    {   
        assert(this->WaveFormat() != NULL);

        if (cbWaveFormat < this->WaveFormatSize())
        {
            hr = MF_E_INVALIDMEDIATYPE;
        }
    }

    if (SUCCEEDED(hr))
    {   
        if (memcmp(pFormat, WaveFormat(), WaveFormatSize()) != 0)
        {
            hr = MF_E_INVALIDMEDIATYPE;
        }
    }


    SafeRelease(&pStreamDescriptor);
    SafeRelease(&pHandler);
    SafeRelease(&pMediaType);
    CoTaskMemFree(pFormat);

    return hr;
}


//-------------------------------------------------------------------
// Name: QueueNewStreamEvent
// Description: 
// Queues an MENewStream or MEUpdatedStream event during Start.
//
// pPD: The presentation descriptor.
//
// Precondition: The presentation descriptor is assumed to be valid. 
// Call ValidatePresentationDescriptor before calling this method.
//-------------------------------------------------------------------

HRESULT WavSource::QueueNewStreamEvent(IMFPresentationDescriptor *pPD)
{
    assert(pPD != NULL);

    HRESULT hr = S_OK;
    IMFStreamDescriptor *pSD = NULL;

    BOOL fSelected = FALSE;
    
    hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);

    if (SUCCEEDED(hr))
    {   
        // The stream must be selected, because we don't allow the app
        // to de-select the stream. See ValidatePresentationDescriptor.
        assert(fSelected);

        if (m_pStream)
        {
            // The stream already exists, and is still selected.
            // Send the MEUpdatedStream event.
            hr = QueueEventWithIUnknown(this, MEUpdatedStream, S_OK, m_pStream); 
        }
        else
        {
            // The stream does not exist, and is now selected.
            // Create a new stream.

            hr = CreateWavStream(pSD);

            if (SUCCEEDED(hr))
            {   
                // CreateWavStream creates the stream, so m_pStream is no longer NULL.
                assert(m_pStream != NULL);

                // Send the MENewStream event.
                hr = QueueEventWithIUnknown(this, MENewStream, S_OK, m_pStream);
            }
        }
    }

    SafeRelease(&pSD);
    return hr;
}

//-------------------------------------------------------------------
// Name: CreateWavStream
// Description: Creates the source's media stream object.
//-------------------------------------------------------------------

HRESULT WavSource::CreateWavStream(IMFStreamDescriptor *pSD)
{
    HRESULT hr = S_OK;
    m_pStream = new (std::nothrow) WavStream(this, m_pRiff, pSD, hr);

    if (m_pStream == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
        SafeRelease(&m_pStream);
    }

    return hr;
}



//-------------------------------------------------------------------
// Name: GetCurrentPosition
// Description: Returns the current playback position.
//-------------------------------------------------------------------

LONGLONG WavSource::GetCurrentPosition() const
{
    if (m_pStream)
    {
        return m_pStream->GetCurrentPosition();
    }
    else
    {
        // If no stream is selected, we are at time 0 by definition.
        return 0;
    }
}



////////// AUDIO STREAM

//-------------------------------------------------------------------
// WavStream constructor.
//
// pSource: Parent media source.
// pSD: Stream descriptor that describes this stream.
// hr: If the constructor fails, this value is set to a failure code.
//-------------------------------------------------------------------


WavStream::WavStream(WavSource *pSource,  CWavRiffParser *pRiff, IMFStreamDescriptor *pSD, HRESULT& hr) :
    m_nRefCount(1),
    m_pEventQueue(NULL),
    m_IsShutdown(FALSE),
    m_rtCurrentPosition(0),
    m_discontinuity(FALSE),
    m_EOS(FALSE)
{
    m_pSource = pSource;
    m_pSource->AddRef();

    m_pStreamDescriptor = pSD;
    m_pStreamDescriptor->AddRef();

    m_pRiff = pRiff;

    // Create the media event queue.
    hr = MFCreateEventQueue(&m_pEventQueue);

    InitializeCriticalSection(&m_critSec);
}


//-------------------------------------------------------------------
// WavStream destructor.
//-------------------------------------------------------------------

WavStream::~WavStream()
{
    assert(m_IsShutdown);
    assert(m_nRefCount == 0);

    DeleteCriticalSection(&m_critSec);
}


// IUnknown methods

ULONG WavStream::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG  WavStream::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT WavStream::QueryInterface(REFIID iid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(WavStream, IMFMediaEventGenerator),
        QITABENT(WavStream, IMFMediaStream),
        { 0 }
    };
    return QISearch(this, qit, iid, ppv);
}


// IMFMediaEventGenerator methods
// [See note for WavSource class]

HRESULT WavStream::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    hr = CheckShutdown();
    if (SUCCEEDED(hr))
    {   
        hr = m_pEventQueue->BeginGetEvent(pCallback, punkState);
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}

HRESULT WavStream::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    hr = CheckShutdown();
    if (SUCCEEDED(hr))
    {   
        hr = m_pEventQueue->EndGetEvent(pResult, ppEvent);
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}

HRESULT WavStream::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;

    IMFMediaEventQueue *pQueue = NULL;
    
    EnterCriticalSection(&m_critSec);

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {   
        pQueue = m_pEventQueue;
        pQueue->AddRef();
    }

    LeaveCriticalSection(&m_critSec);

    if (SUCCEEDED(hr))
    {   
        hr = pQueue->GetEvent(dwFlags, ppEvent);
    }

    SafeRelease(&pQueue);
    return hr;
}

HRESULT WavStream::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    hr = CheckShutdown();
    if (SUCCEEDED(hr))
    {   
        hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}


// IMFMediaStream methods.


//-------------------------------------------------------------------
// Name: GetMediaSource
// Description: Returns a pointer to the media source.
//-------------------------------------------------------------------

HRESULT WavStream::GetMediaSource(IMFMediaSource** ppMediaSource)
{
    if (ppMediaSource == NULL)
    {
        return E_POINTER;
    }

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;
    
    // If called after shutdown, them m_pSource is NULL.
    // Otherwise, m_pSource should not be NULL.

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {   
        if (m_pSource == NULL)
        {
            hr = E_UNEXPECTED;
        }
    }

    if (SUCCEEDED(hr))
    {   
        hr = m_pSource->QueryInterface(IID_PPV_ARGS(ppMediaSource));
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}


//-------------------------------------------------------------------
// Name: GetStreamDescriptor
// Description: Returns the stream descriptor for this stream.
//-------------------------------------------------------------------

HRESULT WavStream::GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor)
{
    if (ppStreamDescriptor == NULL)
    {
        return E_POINTER;
    }

    if (m_pStreamDescriptor == NULL)
    {
        return E_UNEXPECTED;
    }

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;
    
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {   
        *ppStreamDescriptor = m_pStreamDescriptor;
        (*ppStreamDescriptor)->AddRef();
    }

    LeaveCriticalSection(&m_critSec);

    return hr;
}



//-------------------------------------------------------------------
// Name: RequestSample
// Description: Requests a new sample.
// 
// pToken: Token object. Can be NULL.
//-------------------------------------------------------------------

HRESULT WavStream::RequestSample(IUnknown* pToken)
{
    if (m_pSource == NULL)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;

    IMFMediaSource *pSource = NULL;
    IMFSample *pSample = NULL;  // Sample to deliver.

    EnterCriticalSection(&m_critSec);

    // Check if we are shut down.
    hr = CheckShutdown();

    // Check if we already reached the end of the stream.
    if (SUCCEEDED(hr))
    {   
        if (m_EOS)
        {
            hr = MF_E_END_OF_STREAM;
        }
    }

    // Check the source is stopped.
    // GetState does not hold the source's critical section. Safe to call.
    if (SUCCEEDED(hr))
    {   
        if (m_pSource->GetState() == WavSource::STATE_STOPPED)
        {
            hr = MF_E_INVALIDREQUEST;
        }
    }

    if (SUCCEEDED(hr))
    {   
        // Create a new audio sample.
        hr = CreateAudioSample(&pSample);
    }

    if (SUCCEEDED(hr))
    {   
        // If the caller provided a token, attach it to the sample as
        // an attribute. 

        // NOTE: If we processed sample requests asynchronously, we would
        // need to call AddRef on the token and put the token onto a FIFO
        // queue. See documenation for IMFMediaStream::RequestSample.
        if (pToken)
        {
            hr = pSample->SetUnknown(MFSampleExtension_Token, pToken);
        }
    }

    // If paused, queue the sample for later delivery. Otherwise, deliver the sample now.
    if (SUCCEEDED(hr))
    {   
        if (m_pSource->GetState() == WavSource::STATE_PAUSED)
        {
            hr = m_sampleQueue.Queue(pSample);
        }
        else
        {
            hr = DeliverSample(pSample);
        }
    }

    // Cache a pointer to the source, prior to leaving the critical section.
    if (SUCCEEDED(hr))
    {   
        pSource = m_pSource;
        pSource->AddRef();
    }

    LeaveCriticalSection(&m_critSec);


    // We only have one stream, so the end of the stream is also the end of the
    // presentation. Therefore, when we reach the end of the stream, we need to 
    // queue the end-of-presentation event from the source. Logically we would do 
    // this inside the CheckEndOfStream method. However, we cannot hold the
    // source's critical section while holding the stream's critical section, at
    // risk of deadlock. 

    if (SUCCEEDED(hr))
    {   
        if (m_EOS)
        {
            hr = pSource->QueueEvent(MEEndOfPresentation, GUID_NULL, S_OK, NULL);
        }
    }

    SafeRelease(&pSample);
    SafeRelease(&pSource);
    return hr;
}


///// Private WavStream methods

// NOTE: Some of these methods hold the stream's critical section
// because they are called by the media source object.

//-------------------------------------------------------------------
// Name: CreateAudioSample
// Description: Creates a new audio sample.
//-------------------------------------------------------------------

HRESULT WavStream::CreateAudioSample(IMFSample **ppSample)
{
    HRESULT hr = S_OK;

    IMFMediaBuffer *pBuffer = NULL;
    IMFSample *pSample = NULL;

    DWORD       cbBuffer = 0;
    BYTE        *pData = NULL;
    LONGLONG    duration = 0;

    // Start with one second of data, rounded up to the nearest block.
    cbBuffer = AlignUp<DWORD>(m_pRiff->Format()->nAvgBytesPerSec, m_pRiff->Format()->nBlockAlign);

    // Don't request any more than what's left.
    cbBuffer = min(cbBuffer, m_pRiff->BytesRemainingInChunk());

    // Create the buffer.
    hr = MFCreateMemoryBuffer(cbBuffer, &pBuffer);

    // Get a pointer to the buffer memory.
    if (SUCCEEDED(hr))
    {   
        hr = pBuffer->Lock(&pData, NULL, NULL);
    }

    // Fill the buffer
    if (SUCCEEDED(hr))
    {   
        hr = m_pRiff->ReadDataFromChunk(pData, cbBuffer);
    }

    // Unlock the buffer.
    if (SUCCEEDED(hr))
    {   
        hr = pBuffer->Unlock();
        pData = NULL;
    }

    // Set the size of the valid data in the buffer.
    if (SUCCEEDED(hr))
    {   
        hr = pBuffer->SetCurrentLength(cbBuffer);
    }

    // Create a new sample and add the buffer to it.
    if (SUCCEEDED(hr))
    {   
        hr = MFCreateSample(&pSample);
    }

    if (SUCCEEDED(hr))
    {   
        hr = pSample->AddBuffer(pBuffer);
    }

    // Set the time stamps, duration, and sample flags.
    if (SUCCEEDED(hr))
    {   
        hr = pSample->SetSampleTime(m_rtCurrentPosition);
    }

    if (SUCCEEDED(hr))
    {   
        duration = AudioDurationFromBufferSize(m_pRiff->Format(), cbBuffer);
        hr = pSample->SetSampleDuration(duration);
    }

    // Set the discontinuity flag.
    if (SUCCEEDED(hr))
    {   
        if (m_discontinuity)
        {
            hr = pSample->SetUINT32(MFSampleExtension_Discontinuity, TRUE);
        }
    }

    if (SUCCEEDED(hr))
    {   
        // Update our current position.
        m_rtCurrentPosition += duration;

        // Give the pointer to the caller.
        *ppSample = pSample;
        (*ppSample)->AddRef();
    }

    if (pData && pBuffer)
    {
        hr = pBuffer->Unlock();
    }

    SafeRelease(&pBuffer);
    SafeRelease(&pSample);
    return hr;
}

//-------------------------------------------------------------------
// Name: DeliverSample
// Description: Delivers a sample by sending an MEMediaSample event.
//-------------------------------------------------------------------
HRESULT WavStream::DeliverSample(IMFSample *pSample)
{
    HRESULT hr = S_OK;

    // Send the MEMediaSample event with the new sample.
    hr = QueueEventWithIUnknown(this, MEMediaSample, hr, pSample); 

    // See if we reached the end of the stream.
    if (SUCCEEDED(hr))
    {   
        hr = CheckEndOfStream();    // This method sends MEEndOfStream if needed.
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: DeliverQueuedSamples
// Description: Delivers any samples waiting in the queue.
//
// Note: If the client requests a sample while the source is paused, 
// the sample is queued and delivered on the next non-seeking call
// to Start(). The queue is flushed if the source is seeked or 
// stopped.
//-------------------------------------------------------------------

HRESULT WavStream::DeliverQueuedSamples()
{
    HRESULT hr = S_OK;
    IMFSample *pSample = NULL;

    EnterCriticalSection(&m_critSec);

    // If we already reached the end of the stream, send the MEEndStream 
    // event again.
    if (m_EOS)
    {
        hr = QueueEvent(MEEndOfStream, GUID_NULL, S_OK, NULL);
    }

    if (SUCCEEDED(hr))
    {   
        // Deliver any queued samples. 
        while (!m_sampleQueue.IsEmpty())
        {
            hr = m_sampleQueue.Dequeue(&pSample);
            if (FAILED(hr))
            {
                break;
            }

            hr = DeliverSample(pSample);
            if (FAILED(hr))
            {
                break;
            }

            SafeRelease(&pSample);
        }
    }

    LeaveCriticalSection(&m_critSec);

    // If we reached the end of the stream, send the end-of-presentation event from
    // the media source.
    if (SUCCEEDED(hr))
    {   
        if (m_EOS)
        {
            hr = m_pSource->QueueEvent(MEEndOfPresentation, GUID_NULL, S_OK, NULL);
        }
    }

    SafeRelease(&pSample);
    return hr;
}


//-------------------------------------------------------------------
// Name: Flush
// Description: Flushes the sample queue.
//-------------------------------------------------------------------

HRESULT WavStream::Flush()
{
    EnterCriticalSection(&m_critSec);

    m_sampleQueue.Clear();

    LeaveCriticalSection(&m_critSec);
    return S_OK;
}


//-------------------------------------------------------------------
// Name: Shutdown
// Description: Notifies the stream that the source was shut down.
//-------------------------------------------------------------------

HRESULT WavStream::Shutdown()
{
    EnterCriticalSection(&m_critSec);

    // Flush queued samples.
    Flush();

    // Shut down the event queue.
    if (m_pEventQueue)
    {
        m_pEventQueue->Shutdown();
    }

    SafeRelease(&m_pEventQueue);
    SafeRelease(&m_pSource);
    SafeRelease(&m_pStreamDescriptor);
   
    m_pRiff = NULL;

    m_IsShutdown = TRUE;

    LeaveCriticalSection(&m_critSec);
    return S_OK;
}

//-------------------------------------------------------------------
// Name: SetPosition
// Description: Updates the new stream position.
//-------------------------------------------------------------------

HRESULT WavStream::SetPosition(LONGLONG rtNewPosition)
{
    EnterCriticalSection(&m_critSec);

    // Check if the requested position is beyond the end of the stream.
    LONGLONG duration = AudioDurationFromBufferSize(m_pRiff->Format(), m_pRiff->Chunk().DataSize());

    if (rtNewPosition > duration)
    {
        LeaveCriticalSection(&m_critSec);

        return MF_E_INVALIDREQUEST; // Start position is past the end of the presentation.
    }

    HRESULT hr = S_OK;

    if (m_rtCurrentPosition != rtNewPosition)
    {
        LONGLONG offset = BufferSizeFromAudioDuration(m_pRiff->Format(), rtNewPosition);

        // The chunk size is a DWORD. So if our calculations are correct, there is no
        // way that the maximum valid seek position can be larger than a DWORD. 
        assert(offset <= MAXDWORD);

        hr = m_pRiff->MoveToChunkOffset((DWORD)offset);

        if (SUCCEEDED(hr))
        {   
            m_rtCurrentPosition = rtNewPosition;
            m_discontinuity = TRUE;
            m_EOS = FALSE;
        }
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}

HRESULT WavStream::CheckEndOfStream()
{
    HRESULT hr = S_OK;

    if (m_pRiff->BytesRemainingInChunk() < m_pRiff->Format()->nBlockAlign)
    {
        // The remaining data is smaller than the audio block size. (In theory there shouldn't be
        // partial bits of data at the end, so we should reach an even zero bytes, but the file
        // might not be authored correctly.)
        m_EOS = TRUE;

        // Send the end-of-stream event,
        hr = QueueEvent(MEEndOfStream, GUID_NULL, S_OK, NULL);
    }
    return hr; 
}




//-------------------------------------------------------------------
// Name: QueueEventWithIUnknown
// Description: Helper function to queue an event with an IUnknown
//              pointer value.
//
// pMEG:        Media event generator that will queue the event.
// meType:      Media event type.
// hrStatus:    Status code for the event.
// pUnk:        IUnknown pointer value.
//
//-------------------------------------------------------------------


HRESULT QueueEventWithIUnknown(
    IMFMediaEventGenerator *pMEG,
    MediaEventType meType,
    HRESULT hrStatus,
    IUnknown *pUnk)
{

    // Create the PROPVARIANT to hold the IUnknown value.
    PROPVARIANT var;
    var.vt = VT_UNKNOWN;
    var.punkVal = pUnk;
    pUnk->AddRef();

    // Queue the event.
    HRESULT hr = pMEG->QueueEvent(meType, GUID_NULL, hrStatus, &var);

    // Clear the PROPVARIANT.
    PropVariantClear(&var);

    return hr;
}



//-------------------------------------------------------------------
// Name: ValidateWaveFormat
// Description: Validates a WAVEFORMATEX structure. 
//
// This method is called when the byte stream handler opens the
// source. The WAVEFORMATEX structure is copied directly from the 
// .wav file. Therefore the source should not trust any of the
// values in the format header.
//
// Just to keep the sample as simple as possible, we only accept 
// uncompressed PCM formats in this media source.
//-------------------------------------------------------------------


HRESULT ValidateWaveFormat(const WAVEFORMATEX *pWav, DWORD cbSize)
{
    if (cbSize < sizeof(WAVEFORMATEX))
    {
        return MF_E_INVALIDMEDIATYPE;
    }

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

LONGLONG AudioDurationFromBufferSize(const WAVEFORMATEX *pWav, DWORD cbAudioDataSize)
{
    assert(pWav != NULL);

    if (pWav->nAvgBytesPerSec == 0)
    {
        return 0;
    }
    return (LONGLONG)cbAudioDataSize * 10000000 / pWav->nAvgBytesPerSec;
}

LONGLONG BufferSizeFromAudioDuration(const WAVEFORMATEX *pWav, LONGLONG duration)
{
    LONGLONG cbSize = duration * pWav->nAvgBytesPerSec / 10000000;

    ULONG ulRemainder = (ULONG)(cbSize % pWav->nBlockAlign);

    // Round up to the next block. 
    if(ulRemainder) 
    {
        cbSize += pWav->nBlockAlign - ulRemainder;
    }

    return cbSize;
}


