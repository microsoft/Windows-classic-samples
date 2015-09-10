#include "MediaSink.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CMediaSink class. - Implements the media sink.
//
// Notes:
// - Most public methods calls CheckShutdown. This method fails if the sink was shut down.
//
/////////////////////////////////////////////////////////////////////////////////////////////

DX11VideoRenderer::CCritSec DX11VideoRenderer::CMediaSink::s_csStreamSinkAndScheduler;

//-------------------------------------------------------------------
// Name: CreateInstance
// Description: Creates an instance of the DX11 Video Renderer sink object.
//-------------------------------------------------------------------

/* static */ HRESULT DX11VideoRenderer::CMediaSink::CreateInstance(_In_ REFIID iid, _COM_Outptr_ void** ppSink)
{
    if (ppSink == NULL)
    {
        return E_POINTER;
    }

    *ppSink = NULL;

    HRESULT hr = S_OK;
    CMediaSink* pSink = new CMediaSink(); // Created with ref count = 1.

    if (pSink == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = pSink->Initialize();
    }

    if (SUCCEEDED(hr))
    {
        hr = pSink->QueryInterface(iid, ppSink);
    }

    SafeRelease(pSink);

    return hr;
}

// IUnknown methods

ULONG DX11VideoRenderer::CMediaSink::AddRef(void)
{
    return InterlockedIncrement(&m_nRefCount);
}

HRESULT DX11VideoRenderer::CMediaSink::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IMFMediaSink*>(this));
    }
    else if (iid == __uuidof(IMFMediaSink))
    {
        *ppv = static_cast<IMFMediaSink*>(this);
    }
    else if (iid == __uuidof(IMFClockStateSink))
    {
        *ppv = static_cast<IMFClockStateSink*>(this);
    }
    else if (iid == __uuidof(IMFGetService))
    {
        *ppv = static_cast<IMFGetService*>(this);
    }
    else if (iid == IID_IMFRateSupport)
    {
        *ppv = static_cast<IMFRateSupport*>(this);
    }
    else if (iid == IID_IMFMediaSinkPreroll)
    {
        *ppv = static_cast<IMFMediaSinkPreroll*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG  DX11VideoRenderer::CMediaSink::Release(void)
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

///  IMFMediaSink methods.

//-------------------------------------------------------------------
// Name: AddStreamSink
// Description: Adds a new stream to the sink.
//
// Note: This sink has a fixed number of streams, so this method
//       always returns MF_E_STREAMSINKS_FIXED.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::AddStreamSink(DWORD dwStreamSinkIdentifier, __RPC__in_opt IMFMediaType* pMediaType, __RPC__deref_out_opt IMFStreamSink** ppStreamSink)
{
    return MF_E_STREAMSINKS_FIXED;
}

//-------------------------------------------------------------------
// Name: GetCharacteristics
// Description: Returns the characteristics flags.
//
// Note: This sink has a fixed number of streams.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::GetCharacteristics(__RPC__out DWORD* pdwCharacteristics)
{
    CAutoLock lock(&m_csMediaSink);

    if (pdwCharacteristics == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *pdwCharacteristics = MEDIASINK_FIXED_STREAMS | MEDIASINK_CAN_PREROLL;
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: GetPresentationClock
// Description: Returns a pointer to the presentation clock.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::GetPresentationClock(__RPC__deref_out_opt IMFPresentationClock** ppPresentationClock)
{
    CAutoLock lock(&m_csMediaSink);

    if (ppPresentationClock == NULL)
    {
        return E_POINTER;
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
// Name: GetStreamSinkById
// Description: Retrieves a stream by ID.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::GetStreamSinkById(DWORD dwStreamSinkIdentifier, __RPC__deref_out_opt IMFStreamSink** ppStreamSink)
{
    CAutoLock lock(&m_csMediaSink);

    if (ppStreamSink == NULL)
    {
        return E_POINTER;
    }

    // Fixed stream ID.
    if (dwStreamSinkIdentifier != STREAM_ID)
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
// Name: GetStreamSinkByIndex
// Description: Retrieves a stream by index.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::GetStreamSinkByIndex(DWORD dwIndex, __RPC__deref_out_opt IMFStreamSink** ppStreamSink)
{
    CAutoLock lock(&m_csMediaSink);

    if (ppStreamSink == NULL)
    {
        return E_POINTER;
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
// Name: GetStreamSinkCount
// Description: Returns the number of streams.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::GetStreamSinkCount(__RPC__out DWORD* pcStreamSinkCount)
{
    CAutoLock lock(&m_csMediaSink);

    if (pcStreamSinkCount == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *pcStreamSinkCount = 1;  // Fixed number of streams.
    }

    return hr;

}

//-------------------------------------------------------------------
// Name: RemoveStreamSink
// Description: Removes a stream from the sink.
//
// Note: This sink has a fixed number of streams, so this method
//       always returns MF_E_STREAMSINKS_FIXED.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::RemoveStreamSink(DWORD dwStreamSinkIdentifier)
{
    return MF_E_STREAMSINKS_FIXED;
}

//-------------------------------------------------------------------
// Name: SetPresentationClock
// Description: Sets the presentation clock.
//
// pPresentationClock: Pointer to the clock. Can be NULL.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::SetPresentationClock(__RPC__in_opt IMFPresentationClock* pPresentationClock)
{
    CAutoLock lock(&m_csMediaSink);

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

        SafeRelease(m_pClock);
        m_pClock = pPresentationClock;
        if (m_pClock)
        {
            m_pClock->AddRef();
        }
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: Shutdown
// Description: Releases resources held by the media sink.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::Shutdown(void)
{
    CAutoLock lock(&m_csMediaSink);

    HRESULT hr = MF_E_SHUTDOWN;

    m_IsShutdown = TRUE;

    if (m_pStream != NULL)
    {
        m_pStream->Shutdown();
    }

    if (m_pPresenter != NULL)
    {
        m_pPresenter->Shutdown();
    }

    SafeRelease(m_pClock);
    SafeRelease(m_pStream);
    SafeRelease(m_pPresenter);

    if (m_pScheduler != NULL)
    {
        hr = m_pScheduler->StopScheduler();
    }

    SafeRelease(m_pScheduler);

    return hr;
}

//-------------------------------------------------------------------
// Name: OnClockPause
// Description: Called when the presentation clock paused.
//
// Note: For an archive sink, the paused state is equivalent to the
//       running (started) state. We still accept data and archive it.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::OnClockPause(
    /* [in] */ MFTIME hnsSystemTime)
{
    CAutoLock lock(&m_csMediaSink);

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

HRESULT DX11VideoRenderer::CMediaSink::OnClockRestart(
    /* [in] */ MFTIME hnsSystemTime)
{
    CAutoLock lock(&m_csMediaSink);

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

HRESULT DX11VideoRenderer::CMediaSink::OnClockSetRate(
    /* [in] */ MFTIME hnsSystemTime,
    /* [in] */ float flRate)
{
    if (m_pScheduler != NULL)
    {
        // Tell the scheduler about the new rate.
        m_pScheduler->SetClockRate(flRate);
    }

    return S_OK;
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

HRESULT DX11VideoRenderer::CMediaSink::OnClockStart(
    /* [in] */ MFTIME hnsSystemTime,
    /* [in] */ LONGLONG llClockStartOffset)
{
    CAutoLock lock(&m_csMediaSink);

    HRESULT hr = CheckShutdown();
    if (FAILED(hr))
    {
        return hr;
    }

    // Check if the clock is already active (not stopped).
    // And if the clock position changes while the clock is active, it
    // is a seek request. We need to flush all pending samples.
    if (m_pStream->IsActive() && llClockStartOffset != PRESENTATION_CURRENT_POSITION)
    {
        // This call blocks until the scheduler threads discards all scheduled samples.
        hr = m_pStream->Flush();
    }
    else
    {
        if (m_pScheduler != NULL)
        {
            // Start the scheduler thread.
            hr = m_pScheduler->StartScheduler(m_pClock);
        }
    }

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

HRESULT DX11VideoRenderer::CMediaSink::OnClockStop(
    /* [in] */ MFTIME hnsSystemTime)
{
    CAutoLock lock(&m_csMediaSink);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Stop();
    }

    if (SUCCEEDED(hr))
    {
        if (m_pScheduler != NULL)
        {
            // Stop the scheduler thread.
            hr = m_pScheduler->StopScheduler();
        }
    }

    return hr;
}

//-------------------------------------------------------------------------
// Name: GetService
// Description: IMFGetService
//-------------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::GetService(__RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID* ppvObject)
{
    HRESULT hr = S_OK;

    if (guidService == MF_RATE_CONTROL_SERVICE)
    {
        hr = QueryInterface(riid, ppvObject);
    }
    else if (guidService == MR_VIDEO_RENDER_SERVICE)
    {
        hr = m_pPresenter->QueryInterface(riid, ppvObject);
    }
    else if (guidService == MR_VIDEO_ACCELERATION_SERVICE)
    {
        hr = m_pPresenter->GetService(guidService, riid, ppvObject);
    }
    else
    {
        hr = MF_E_UNSUPPORTED_SERVICE;
    }

    return hr;
}

STDMETHODIMP DX11VideoRenderer::CMediaSink::GetFastestRate(
    MFRATE_DIRECTION eDirection,
    BOOL fThin,
    _Out_ float *pflRate
    )
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_csMediaSink);

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        if (NULL == pflRate)
        {
            hr = E_POINTER;
            break;
        }

        float rate;

        hr = m_pStream->GetMaxRate(fThin, &rate);
        if (FAILED(hr))
        {
            break;
        }

        if (MFRATE_FORWARD == eDirection)
        {
            *pflRate = rate;
        }
        else
        {
            *pflRate = -rate;
        }
    }
    while (FALSE);

    return hr;
}

//-------------------------------------------------------------------------
// Name: GetSlowestRate
// Description: IMFRateSupport
//-------------------------------------------------------------------------

STDMETHODIMP DX11VideoRenderer::CMediaSink::GetSlowestRate(
    MFRATE_DIRECTION eDirection,
    BOOL fThin,
    _Out_ float* pflRate
    )
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_csMediaSink);

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        if (NULL == pflRate)
        {
            hr = E_POINTER;
            break;
        }

        if (SUCCEEDED(hr))
        {
            //
            // We go as slow as you want!
            //
            *pflRate = 0;
        }
    }
    while (FALSE);

    return hr;
}

STDMETHODIMP DX11VideoRenderer::CMediaSink::IsRateSupported(BOOL fThin, float flRate, __RPC__inout_opt float* pflNearestSupportedRate)
{
    HRESULT hr = S_OK;
    float flNearestSupportedRate = flRate;

    CAutoLock lock(&m_csMediaSink);

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        //
        // Only support rates up to the refresh rate of the monitor.
        // This check makes sense only if we're going to be receiving
        // all frames
        //
        if ( !fThin )
        {
            float rate;

            hr = m_pStream->GetMaxRate(fThin, &rate);
            if (FAILED(hr))
            {
                break;
            }

            if ( (flRate > 0 && flRate > (float)rate) ||
                (flRate < 0 && flRate < -(float)rate) )
            {
                hr = MF_E_UNSUPPORTED_RATE;
                flNearestSupportedRate = ( flRate >= 0.0f ) ? rate : -rate;

                break;
            }
        }
    }
    while (FALSE);

    if ( NULL != pflNearestSupportedRate )
    {
        *pflNearestSupportedRate = flNearestSupportedRate;
    }

    return hr;
}

STDMETHODIMP DX11VideoRenderer::CMediaSink::NotifyPreroll(MFTIME hnsUpcomingStartTime)
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_csMediaSink);

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Preroll();
    }

    return hr;
}

/// Private methods

//-------------------------------------------------------------------
// CMediaSink constructor.
//-------------------------------------------------------------------

DX11VideoRenderer::CMediaSink::CMediaSink(void) :
    STREAM_ID(1),
    m_nRefCount(1),
    m_csMediaSink(), // default ctor
    m_IsShutdown(FALSE),
    m_pStream(NULL),
    m_pClock(NULL),
    m_pScheduler(NULL),
    m_pPresenter(NULL)
{
}

//-------------------------------------------------------------------
// CMediaSink destructor.
//-------------------------------------------------------------------

DX11VideoRenderer::CMediaSink::~CMediaSink(void)
{
}

HRESULT DX11VideoRenderer::CMediaSink::CheckShutdown(void) const
{
    if (m_IsShutdown)
    {
        return MF_E_SHUTDOWN;
    }
    else
    {
        return S_OK;
    }
}

//-------------------------------------------------------------------
// Name: Initialize
// Description: Initializes the media sink.
//
// Note: This method is called once when the media sink is first
//       initialized.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CMediaSink::Initialize(void)
{
    HRESULT hr = S_OK;
    IMFMediaSink* pSink = NULL;

    do
    {
        m_pScheduler = new CScheduler(s_csStreamSinkAndScheduler);
        if (m_pScheduler == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        m_pStream = new CStreamSink(STREAM_ID, s_csStreamSinkAndScheduler, m_pScheduler);
        if (m_pStream == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        m_pPresenter = new CPresenter(); // Created with ref count = 1.
        if (m_pPresenter == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = QueryInterface(IID_PPV_ARGS(&pSink));
        if (FAILED(hr))
        {
            break;
        }

        hr = m_pStream->Initialize(pSink, m_pPresenter);
        if (FAILED(hr))
        {
            break;
        }

        m_pScheduler->SetCallback(static_cast<SchedulerCallback*>(m_pStream));
    }
    while (FALSE);

    if (FAILED(hr))
    {
        Shutdown();
    }

    SafeRelease(pSink);

    return hr;
}
