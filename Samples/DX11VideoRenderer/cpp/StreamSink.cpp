#include "StreamSink.h"

GUID const* const DX11VideoRenderer::CStreamSink::s_pVideoFormats[] =
{
    &MFVideoFormat_NV12,
    &MFVideoFormat_IYUV,
    &MFVideoFormat_YUY2,
    &MFVideoFormat_YV12,
    &MFVideoFormat_RGB32,
    &MFVideoFormat_RGB32,
    &MFVideoFormat_RGB24,
    &MFVideoFormat_RGB555,
    &MFVideoFormat_RGB565,
    &MFVideoFormat_RGB8,
    &MFVideoFormat_AYUV,
    &MFVideoFormat_UYVY,
    &MFVideoFormat_YVYU,
    &MFVideoFormat_YVU9,
    &MEDIASUBTYPE_V216,
    &MFVideoFormat_v410,
    &MFVideoFormat_I420,
    &MFVideoFormat_NV11,
    &MFVideoFormat_420O
};

const DWORD DX11VideoRenderer::CStreamSink::s_dwNumVideoFormats = sizeof(DX11VideoRenderer::CStreamSink::s_pVideoFormats) / sizeof(DX11VideoRenderer::CStreamSink::s_pVideoFormats[0]);

const MFRatio DX11VideoRenderer::CStreamSink::s_DefaultFrameRate = { 30, 1 };

const DX11VideoRenderer::CStreamSink::FormatEntry DX11VideoRenderer::CStreamSink::s_DXGIFormatMapping[] =
{
    { MFVideoFormat_RGB32,      DXGI_FORMAT_B8G8R8X8_UNORM },
    { MFVideoFormat_ARGB32,     DXGI_FORMAT_R8G8B8A8_UNORM },
    { MFVideoFormat_AYUV,      DXGI_FORMAT_AYUV            },
    { MFVideoFormat_YUY2,      DXGI_FORMAT_YUY2            },
    { MFVideoFormat_NV12,      DXGI_FORMAT_NV12            },
    { MFVideoFormat_NV11,      DXGI_FORMAT_NV11            },
    { MFVideoFormat_AI44,      DXGI_FORMAT_AI44            },
    { MFVideoFormat_P010,      DXGI_FORMAT_P010            },
    { MFVideoFormat_P016,      DXGI_FORMAT_P016            },
    { MFVideoFormat_Y210,      DXGI_FORMAT_Y210            },
    { MFVideoFormat_Y216,      DXGI_FORMAT_Y216            },
    { MFVideoFormat_Y410,      DXGI_FORMAT_Y410            },
    { MFVideoFormat_Y416,      DXGI_FORMAT_Y416            },
    { MFVideoFormat_420O,      DXGI_FORMAT_420_OPAQUE      }
};

// Control how we batch work from the decoder.
// On receiving a sample we request another one if the number on the queue is
// less than the hi water threshold.
// When displaying samples (removing them from the sample queue) we request
// another one if the number of falls below the lo water threshold
//
#define SAMPLE_QUEUE_HIWATER_THRESHOLD 3
// maximum # of past reference frames required for deinterlacing
#define MAX_PAST_FRAMES         3

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CStreamSink class. - Implements the stream sink.
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
//      To avoid deadlocks, do not hold the CStreamSink lock followed by the CMediaSink lock.
//      The other order is OK (CMediaSink, then CStreamSink).
//
/////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------
// Name: ValidStateMatrix
// Description: Class-static matrix of operations vs states.
//
// If an entry is TRUE, the operation is valid from that state.
//-------------------------------------------------------------------

BOOL DX11VideoRenderer::CStreamSink::ValidStateMatrix[DX11VideoRenderer::CStreamSink::State_Count][DX11VideoRenderer::CStreamSink::Op_Count] =
{
    // States:    Operations:
    //            SetType   Start     Restart   Pause     Stop      Sample    Marker
    /* NotSet */  TRUE,     FALSE,    FALSE,    FALSE,    FALSE,    FALSE,    FALSE,

    /* Ready */   TRUE,     TRUE,     TRUE,     TRUE,     TRUE,     FALSE,    TRUE,

    /* Start */   TRUE,     TRUE,     FALSE,    TRUE,     TRUE,     TRUE,     TRUE,

    /* Pause */   TRUE,     TRUE,     TRUE,     TRUE,     TRUE,     TRUE,     TRUE,

    /* Stop */    TRUE,     TRUE,     FALSE,    FALSE,    TRUE,     FALSE,    TRUE

    // Note about states:
    // 1. OnClockRestart should only be called from paused state.
    // 2. While paused, the sink accepts samples but does not process them.

};

//-------------------------------------------------------------------
// CStreamSink constructor
//-------------------------------------------------------------------

#pragma warning( push )
#pragma warning( disable : 4355 )  // 'this' used in base member initializer list

DX11VideoRenderer::CStreamSink::CStreamSink(DWORD dwStreamId, CCritSec& critSec, CScheduler* pScheduler) :
    STREAM_ID(dwStreamId),
    m_nRefCount(1),
    m_critSec(critSec),
    m_state(State_TypeNotSet),
    m_IsShutdown(FALSE),
    m_WorkQueueId(0),
    m_WorkQueueCB(this, &CStreamSink::OnDispatchWorkItem),
    m_ConsumeData(ProcessFrames),
    m_StartTime(0),
    m_cbDataWritten(0),
    m_cOutstandingSampleRequests(0),
    m_pSink(NULL),
    m_pEventQueue(NULL),
    m_pByteStream(NULL),
    m_pPresenter(NULL),
    m_pScheduler(pScheduler),
    m_pCurrentType(NULL),
    m_fPrerolling(FALSE),
    m_fWaitingForOnClockStart(FALSE),
    m_SamplesToProcess(), // default ctor
    m_unInterlaceMode(MFVideoInterlace_Progressive),
    m_imageBytesPP(), // default ctor
    m_dxgiFormat(DXGI_FORMAT_UNKNOWN)
{
    m_imageBytesPP.Numerator = 1;
    m_imageBytesPP.Denominator = 1;
}

#pragma warning( pop )

//-------------------------------------------------------------------
// CStreamSink destructor
//-------------------------------------------------------------------

DX11VideoRenderer::CStreamSink::~CStreamSink(void)
{
}

// IUnknown methods

ULONG DX11VideoRenderer::CStreamSink::AddRef(void)
{
    return InterlockedIncrement(&m_nRefCount);
}

HRESULT DX11VideoRenderer::CStreamSink::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv)
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
        *ppv = static_cast<IMFStreamSink*>(this);
    }
    else if (iid == __uuidof(IMFMediaEventGenerator))
    {
        *ppv = static_cast<IMFMediaEventGenerator*>(this);
    }
    else if (iid == __uuidof(IMFMediaTypeHandler))
    {
        *ppv = static_cast<IMFMediaTypeHandler*>(this);
    }
    else if (iid == __uuidof(IMFGetService))
    {
        *ppv = static_cast<IMFGetService*>(this);
    }
    else if (iid == __uuidof(IMFAttributes))
    {
        *ppv = static_cast<IMFAttributes*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG  DX11VideoRenderer::CStreamSink::Release(void)
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

/// IMFStreamSink methods

//-------------------------------------------------------------------
// Name: Flush
// Description: Discards all samples that were not processed yet.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::Flush(void)
{
    CAutoLock lock(&m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        m_ConsumeData = DropFrames;
        // Note: Even though we are flushing data, we still need to send
        // any marker events that were queued.
        hr = ProcessSamplesFromQueue(m_ConsumeData);
    }

    if (SUCCEEDED(hr))
    {
        // This call blocks until the scheduler threads discards all scheduled samples.
        m_pScheduler->Flush();

        hr = m_pPresenter->Flush();
    }

    m_ConsumeData = ProcessFrames;

    return hr;
}

//-------------------------------------------------------------------
// Name: GetIdentifier
// Description: Returns the stream identifier.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::GetIdentifier(__RPC__out DWORD* pdwIdentifier)
{
    CAutoLock lock(&m_critSec);

    if (pdwIdentifier == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *pdwIdentifier = STREAM_ID;
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: GetMediaSink
// Description: Returns the parent media sink.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::GetMediaSink(__RPC__deref_out_opt IMFMediaSink** ppMediaSink)
{
    CAutoLock lock(&m_critSec);

    if (ppMediaSink == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *ppMediaSink = m_pSink;
        (*ppMediaSink)->AddRef();
    }

    return hr;

}

//-------------------------------------------------------------------
// Name: GetMediaTypeHandler
// Description: Returns a media type handler for this stream.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::GetMediaTypeHandler(__RPC__deref_out_opt IMFMediaTypeHandler** ppHandler)
{
    CAutoLock lock(&m_critSec);

    if (ppHandler == NULL)
    {
        return E_POINTER;
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
// Name: PlaceMarker
// Description: Receives a marker. [Asynchronous]
//
// Note: The client can call PlaceMarker at any time. In response,
//       we need to queue an MEStreamSinkMarker event, but not until
//       *after* we have processed all samples that we have received
//       up to this point.
//
//       Also, in general you might need to handle specific marker
//       types, although this sink does not.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::PlaceMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType, __RPC__in const PROPVARIANT* pvarMarkerValue, __RPC__in const PROPVARIANT* pvarContextValue)
{

    CAutoLock lock(&m_critSec);

    HRESULT hr = S_OK;

    IMarker* pMarker = NULL;

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
        hr = m_SamplesToProcess.Queue(pMarker);
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
    SafeRelease(pMarker);

    return hr;
}

//-------------------------------------------------------------------
// Name: ProcessSample
// Description: Receives an input sample. [Asynchronous]
//
// Note: The client should only give us a sample after we send an
//       MEStreamSinkRequestSample event.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::ProcessSample(__RPC__in_opt IMFSample* pSample)
{
    CAutoLock lock(&m_critSec);

    if (pSample == NULL)
    {
        return E_POINTER;
    }

    if (0 == m_cOutstandingSampleRequests)
    {
        return MF_E_INVALIDREQUEST;
    }

    HRESULT hr = S_OK;

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        m_cOutstandingSampleRequests--;

        if (!m_fPrerolling && !m_fWaitingForOnClockStart)
        {
            // Validate the operation.
            hr = ValidateOperation(OpProcessSample);
            if (FAILED(hr))
            {
                break;
            }
        }

        // Add the sample to the sample queue.
        hr = m_SamplesToProcess.Queue(pSample);
        if (FAILED(hr))
        {
            break;
        }

        if (m_fPrerolling)
        {
            m_fPrerolling = FALSE;
            return QueueEvent(MEStreamSinkPrerolled, GUID_NULL, S_OK, NULL);
        }

        // Unless we are paused/stopped, start an async operation to dispatch the next sample.
        if (m_state != State_Paused && m_state != State_Stopped)
        {
            // Queue the operation.
            hr = QueueAsyncOperation(OpProcessSample);
        }
    }
    while (FALSE);

    return hr;
}

// IMFMediaEventGenerator methods.
// Note: These methods call through to the event queue helper object.

HRESULT DX11VideoRenderer::CStreamSink::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_critSec);
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->BeginGetEvent(pCallback, punkState);
    }

    return hr;
}

HRESULT DX11VideoRenderer::CStreamSink::EndGetEvent(IMFAsyncResult* pResult, _Out_  IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_critSec);
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->EndGetEvent(pResult, ppEvent);
    }

    return hr;
}

HRESULT DX11VideoRenderer::CStreamSink::GetEvent(DWORD dwFlags, __RPC__deref_out_opt IMFMediaEvent** ppEvent)
{
    // NOTE:
    // GetEvent can block indefinitely, so we don't hold the lock.
    // This requires some juggling with the event queue pointer.

    HRESULT hr = S_OK;

    IMFMediaEventQueue* pQueue = NULL;

    { // scope for lock

        CAutoLock lock(&m_critSec);

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

    SafeRelease(pQueue);

    return hr;
}

HRESULT DX11VideoRenderer::CStreamSink::QueueEvent(MediaEventType met, __RPC__in REFGUID guidExtendedType, HRESULT hrStatus, __RPC__in_opt const PROPVARIANT* pvValue)
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_critSec);
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
    }

    return hr;
}

/// IMFMediaTypeHandler methods

//-------------------------------------------------------------------
// Name: GetCurrentMediaType
// Description: Return the current media type, if any.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::GetCurrentMediaType(_Outptr_ IMFMediaType** ppMediaType)
{
    CAutoLock lock(&m_critSec);

    if (ppMediaType == NULL)
    {
        return E_POINTER;
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

HRESULT DX11VideoRenderer::CStreamSink::GetMajorType(__RPC__out GUID* pguidMajorType)
{
    if (pguidMajorType == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = CheckShutdown();
    if (FAILED(hr))
    {
        return hr;
    }

    if (m_pCurrentType == NULL)
    {
        return MF_E_NOT_INITIALIZED;
    }

    return m_pCurrentType->GetGUID(MF_MT_MAJOR_TYPE, pguidMajorType);
}

//-------------------------------------------------------------------
// Name: GetMediaTypeByIndex
// Description: Return a preferred media type by index.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::GetMediaTypeByIndex(DWORD dwIndex, _Outptr_ IMFMediaType** ppType)
{
    HRESULT hr = S_OK;

    do
    {
        if (ppType == NULL)
        {
            hr = E_POINTER;
            break;
        }

        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        if (dwIndex >= s_dwNumVideoFormats)
        {
            hr = MF_E_NO_MORE_TYPES;
            break;
        }

        IMFMediaType* pVideoMediaType = NULL;

        do
        {
            hr = MFCreateMediaType(&pVideoMediaType);
            if (FAILED(hr))
            {
                break;
            }

            hr = pVideoMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            if (FAILED(hr))
            {
                break;
            }

            hr = pVideoMediaType->SetGUID(MF_MT_SUBTYPE, *(s_pVideoFormats[dwIndex]));
            if (FAILED(hr))
            {
                break;
            }

            pVideoMediaType->AddRef();
            *ppType = pVideoMediaType;
        }
        while (FALSE);

        SafeRelease(pVideoMediaType);

        if (FAILED(hr))
        {
            break;
        }
    }
    while (FALSE);

    return hr;
}

//-------------------------------------------------------------------
// Name: GetMediaTypeCount
// Description: Return the number of preferred media types.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::GetMediaTypeCount(__RPC__out DWORD* pdwTypeCount)
{
    HRESULT hr = S_OK;

    do
    {
        if (pdwTypeCount == NULL)
        {
            hr = E_POINTER;
            break;
        }

        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        *pdwTypeCount = s_dwNumVideoFormats;
    }
    while (FALSE);

    return hr;
}

//-------------------------------------------------------------------
// Name: IsMediaTypeSupported
// Description: Check if a media type is supported.
//
// pMediaType: The media type to check.
// ppMediaType: Optionally, receives a "close match" media type.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::IsMediaTypeSupported(IMFMediaType* pMediaType, _Outptr_opt_result_maybenull_ IMFMediaType** ppMediaType)
{
    HRESULT hr = S_OK;
    GUID subType = GUID_NULL;

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        if (pMediaType == NULL)
        {
            hr = E_POINTER;
            break;
        }

        hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subType);
        if (FAILED(hr))
        {
            break;
        }

        hr = MF_E_INVALIDMEDIATYPE; // This will be set to OK if we find the subtype is accepted

        for(DWORD i = 0; i < s_dwNumVideoFormats; i++)
        {
            if (subType == (*s_pVideoFormats[i]))
            {
                hr = S_OK;
                break;
            }
        }

        if (FAILED(hr))
        {
            break;
        }

        for( DWORD i = 0; i < ARRAYSIZE( s_DXGIFormatMapping ); i++ )
        {
            const FormatEntry& e = s_DXGIFormatMapping[i];
            if ( e.Subtype == subType )
            {
                m_dxgiFormat = e.DXGIFormat;
                break;
            }
        }

        hr = m_pPresenter->IsMediaTypeSupported(pMediaType, m_dxgiFormat);
        if (FAILED(hr))
        {
            break;
        }
    }
    while (FALSE);

    // We don't return any "close match" types.
    if (ppMediaType)
    {
        *ppMediaType = NULL;
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: SetCurrentMediaType
// Description: Set the current media type.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::SetCurrentMediaType(IMFMediaType* pMediaType)
{
    if (pMediaType == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    MFRatio fps = { 0, 0 };
    GUID guidSubtype = GUID_NULL;

    CAutoLock lock(&m_critSec);

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        hr = ValidateOperation(OpSetMediaType);
        if (FAILED(hr))
        {
            break;
        }

        hr = IsMediaTypeSupported(pMediaType, NULL);
        if (FAILED(hr))
        {
            break;
        }

        SafeRelease(m_pCurrentType);
        m_pCurrentType = pMediaType;
        m_pCurrentType->AddRef();

        pMediaType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);

        if ((guidSubtype == MFVideoFormat_NV12) ||
            (guidSubtype == MFVideoFormat_YV12) ||
            (guidSubtype == MFVideoFormat_IYUV) ||
            (guidSubtype == MFVideoFormat_YVU9) ||
            (guidSubtype == MFVideoFormat_I420))
        {
            m_imageBytesPP.Numerator = 3;
            m_imageBytesPP.Denominator = 2;
        }
        else if ((guidSubtype == MFVideoFormat_YUY2)||
            (guidSubtype == MFVideoFormat_RGB555)   ||
            (guidSubtype == MFVideoFormat_RGB565)   ||
            (guidSubtype == MFVideoFormat_UYVY)     ||
            (guidSubtype == MFVideoFormat_YVYU)     ||
            (guidSubtype == MEDIASUBTYPE_V216))
        {
            m_imageBytesPP.Numerator = 2;
            m_imageBytesPP.Denominator = 1;
        }
        else if (guidSubtype == MFVideoFormat_RGB24)
        {
            m_imageBytesPP.Numerator = 3;
            m_imageBytesPP.Denominator = 1;
        }
        else if (guidSubtype == MFVideoFormat_RGB32)
        {
            m_imageBytesPP.Numerator = 4;
            m_imageBytesPP.Denominator = 1;
        }
        else if (guidSubtype == MFVideoFormat_v410)
        {
            m_imageBytesPP.Numerator = 5;
            m_imageBytesPP.Denominator = 4;
        }
        else // includes:
            // MFVideoFormat_RGB8
            // MFVideoFormat_AYUV
            // MFVideoFormat_NV11
        {
            // This is just a fail-safe
            m_imageBytesPP.Numerator = 1;
            m_imageBytesPP.Denominator = 1;
        }

        pMediaType->GetUINT32(MF_MT_INTERLACE_MODE, &m_unInterlaceMode);

        // Set the frame rate on the scheduler.
        if (SUCCEEDED(GetFrameRate(pMediaType, &fps)) && (fps.Numerator != 0) && (fps.Denominator != 0))
        {
            if (MFVideoInterlace_FieldInterleavedUpperFirst == m_unInterlaceMode ||
                MFVideoInterlace_FieldInterleavedLowerFirst == m_unInterlaceMode ||
                MFVideoInterlace_FieldSingleUpper == m_unInterlaceMode ||
                MFVideoInterlace_FieldSingleLower == m_unInterlaceMode ||
                MFVideoInterlace_MixedInterlaceOrProgressive == m_unInterlaceMode)
            {
                fps.Numerator*=2;
            }

            m_pScheduler->SetFrameRate(fps);
        }
        else
        {
            // NOTE: The mixer's proposed type might not have a frame rate, in which case
            // we'll use an arbitary default. (Although it's unlikely the video source
            // does not have a frame rate.)
            m_pScheduler->SetFrameRate(s_DefaultFrameRate);
        }

        // Update the required sample count based on the media type (progressive vs. interlaced)
        if (m_unInterlaceMode == MFVideoInterlace_Progressive)
        {
            // XVP will hold on to 1 sample but that's the same sample we will internally hold on to
            hr = SetUINT32(MF_SA_REQUIRED_SAMPLE_COUNT, SAMPLE_QUEUE_HIWATER_THRESHOLD);
        }
        else
        {
            // Assume we will need a maximum of 3 backward reference frames for deinterlacing
            // However, one of the frames is "shared" with SVR
            hr = SetUINT32(MF_SA_REQUIRED_SAMPLE_COUNT, SAMPLE_QUEUE_HIWATER_THRESHOLD + MAX_PAST_FRAMES - 1);
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pPresenter->SetCurrentMediaType(pMediaType);
            if (FAILED(hr))
            {
                break;
            }
        }

        if ( State_Started != m_state && State_Paused != m_state )
        {
            m_state = State_Ready;
        }
        else
        {
            //Flush all current samples in the Queue as this is a format change
            hr = Flush();
        }
    }
    while (FALSE);

    return hr;
}

//-------------------------------------------------------------------------
// Name: GetService
// Description: IMFGetService
//-------------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::GetService(__RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID* ppvObject)
{
    IMFGetService* pGetService = NULL;
    HRESULT hr = m_pSink->QueryInterface(IID_PPV_ARGS(&pGetService));
    if (SUCCEEDED(hr))
    {
        hr = pGetService->GetService(guidService, riid, ppvObject);
    }
    SafeRelease(pGetService);
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     PresentFrame
//
//  Synopsis:   Present the current outstanding frame in the DX queue
//
//--------------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::PresentFrame(void)
{
    HRESULT hr = S_OK;

    if (DropFrames == m_ConsumeData)
    {
        return hr;
    }

    CAutoLock lock(&m_critSec);

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        hr = m_pPresenter->PresentFrame();
        if (FAILED(hr))
        {
            break;
        }
    }
    while (FALSE);

    if (SUCCEEDED(hr))
    {
        // Unless we are paused/stopped, start an async operation to dispatch the next sample.
        if (m_state != State_Paused && m_state != State_Stopped)
        {
            // Queue the operation.
            hr = QueueAsyncOperation(OpProcessSample);
        }
    }
    else
    {
        // We are in the middle of an asynchronous operation, so if something failed, send an error.
        hr = QueueEvent(MEError, GUID_NULL, hr, NULL);
    }

    return hr;
}

HRESULT DX11VideoRenderer::CStreamSink::GetMaxRate(BOOL fThin, float* pflRate)
{
    HRESULT hr = S_OK;
    DWORD dwMonitorRefreshRate = 0;
    UINT32 unNumerator = 0;
    UINT32 unDenominator = 0;

    do
    {
        hr = m_pPresenter->GetMonitorRefreshRate(&dwMonitorRefreshRate);
        if (FAILED(hr))
        {
            break;
        }

        if (m_pCurrentType == NULL)
        {
            hr = MF_E_INVALIDREQUEST;
            break;
        }

        if (fThin == TRUE)
        {
            *pflRate = FLT_MAX;
            break;
        }

        MFGetAttributeRatio(m_pCurrentType, MF_MT_FRAME_RATE, &unNumerator, &unDenominator);

        if (unNumerator == 0 || unDenominator == 0)
        {
            // We support anything.
            *pflRate = FLT_MAX;
        }
        else
        {
            if (MFVideoInterlace_FieldInterleavedUpperFirst == m_unInterlaceMode ||
                MFVideoInterlace_FieldInterleavedLowerFirst == m_unInterlaceMode ||
                MFVideoInterlace_FieldSingleUpper == m_unInterlaceMode           ||
                MFVideoInterlace_FieldSingleLower == m_unInterlaceMode           ||
                MFVideoInterlace_MixedInterlaceOrProgressive == m_unInterlaceMode)
            {
                unNumerator*=2;
            }

            //
            // Only support rates up to the refresh rate of the monitor.
            //
            *pflRate = (float)MulDiv(dwMonitorRefreshRate, unDenominator, unNumerator);
        }
    }
    while (FALSE);

    return hr;
}

//-------------------------------------------------------------------
// Name: Initialize
// Description: Initializes the stream sink.
//
// Note: This method is called once when the media sink is first
//       initialized.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::Initialize(IMFMediaSink* pParent, CPresenter* pPresenter)
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = CMFAttributesImpl::Initialize();
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
        m_pPresenter = pPresenter;
        m_pPresenter->AddRef();
    }

    if (SUCCEEDED(hr))
    {
        m_pSink = pParent;
        m_pSink->AddRef();
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: Pause
// Description: Called when the presentation clock pauses.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::Pause(void)
{
    CAutoLock lock(&m_critSec);

    HRESULT hr = ValidateOperation(OpPause);

    if (SUCCEEDED(hr))
    {
        m_state = State_Paused;
        hr = QueueAsyncOperation(OpPause);
    }

    return hr;
}

HRESULT DX11VideoRenderer::CStreamSink::Preroll(void)
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_critSec);

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        m_fPrerolling = TRUE;
        m_fWaitingForOnClockStart = TRUE;

        // Kick things off by requesting a sample...
        m_cOutstandingSampleRequests++;

        hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL);
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: Restart
// Description: Called when the presentation clock restarts.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::Restart(void)
{
    CAutoLock lock(&m_critSec);

    HRESULT hr = ValidateOperation(OpRestart);

    if (SUCCEEDED(hr))
    {
        m_state = State_Started;
        hr = QueueAsyncOperation(OpRestart);
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: Shutdown
// Description: Shuts down the stream sink.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::Shutdown(void)
{
    CAutoLock lock(&m_critSec);

    m_IsShutdown = TRUE;

    if (m_pEventQueue)
    {
        m_pEventQueue->Shutdown();
    }

    MFUnlockWorkQueue(m_WorkQueueId);

    m_SamplesToProcess.Clear();

    SafeRelease(m_pSink);
    SafeRelease(m_pEventQueue);
    SafeRelease(m_pByteStream);
    SafeRelease(m_pPresenter);
    SafeRelease(m_pCurrentType);

    return MF_E_SHUTDOWN;
}

//-------------------------------------------------------------------
// Name: Start
// Description: Called when the presentation clock starts.
// Note: Start time can be PRESENTATION_CURRENT_POSITION meaning
//       resume from the last current position.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::Start(MFTIME start)
{
    CAutoLock lock(&m_critSec);

    HRESULT hr = S_OK;

    do
    {
        hr = ValidateOperation(OpStart);
        if (FAILED(hr))
        {
            break;
        }

        if (start != PRESENTATION_CURRENT_POSITION)
        {
            // We're starting from a "new" position
            m_StartTime = start;        // Cache the start time.
        }

        m_state = State_Started;
        hr = QueueAsyncOperation(OpStart);
    }
    while (FALSE);

    m_fWaitingForOnClockStart = FALSE;

    return hr;
}

//-------------------------------------------------------------------
// Name: Stop
// Description: Called when the presentation clock stops.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::Stop(void)
{
    CAutoLock lock(&m_critSec);

    HRESULT hr = ValidateOperation(OpStop);

    if (SUCCEEDED(hr))
    {
        m_state = State_Stopped;
        hr = QueueAsyncOperation(OpStop);
    }

    return hr;
}

// private methods

//-------------------------------------------------------------------
// Name: DispatchProcessSample
// Description: Complete a ProcessSample or PlaceMarker request.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::DispatchProcessSample(CAsyncOperation* pOp)
{
    HRESULT hr = S_OK;
    assert(pOp != NULL);

    hr = CheckShutdown();
    if (FAILED(hr))
    {
        return hr;
    }

    if (m_pPresenter->CanProcessNextSample())
    {
        hr = ProcessSamplesFromQueue(ProcessFrames);

        // Ask for another sample
        if (SUCCEEDED(hr))
        {
            if (pOp->m_op == OpProcessSample)
            {
                hr = RequestSamples();
            }
        }

        // We are in the middle of an asynchronous operation, so if something failed, send an error.
        if (FAILED(hr))
        {
            hr = QueueEvent(MEError, GUID_NULL, hr, NULL);
        }
    }

    return hr;
}

HRESULT DX11VideoRenderer::CStreamSink::CheckShutdown(void) const
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

inline HRESULT DX11VideoRenderer::CStreamSink::GetFrameRate(IMFMediaType* pType, MFRatio* pRatio)
{
    return MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, (UINT32*)&pRatio->Numerator, (UINT32*)&pRatio->Denominator);
}

//+-------------------------------------------------------------------------
//
//  Member:     NeedMoreSamples
//
//  Synopsis:   Returns true if the number of samples in flight
//              (queued + requested) is less than the max allowed
//
//--------------------------------------------------------------------------
BOOL DX11VideoRenderer::CStreamSink::NeedMoreSamples(void)
{
    const DWORD cSamplesInFlight = m_SamplesToProcess.GetCount() + m_cOutstandingSampleRequests;

    return cSamplesInFlight < SAMPLE_QUEUE_HIWATER_THRESHOLD;
}

//-------------------------------------------------------------------
// Name: OnDispatchWorkItem
// Description: Callback for MFPutWorkItem.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::OnDispatchWorkItem(IMFAsyncResult* pAsyncResult)
{
    // Called by work queue thread. Need to hold the critical section.
    CAutoLock lock(&m_critSec);

    HRESULT hr = CheckShutdown();
    if (FAILED(hr))
    {
        return hr;
    }

    IUnknown* pState = NULL;

    hr = pAsyncResult->GetState(&pState);

    if (SUCCEEDED(hr))
    {
        // The state object is a CAsncOperation object.
        CAsyncOperation* pOp = (CAsyncOperation*)pState;

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
                m_cOutstandingSampleRequests++;
                hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL);
            }

            // There might be samples queue from earlier (ie, while paused).
            if (SUCCEEDED(hr))
            {
                hr = ProcessSamplesFromQueue(m_ConsumeData);
            }

            break;

        case OpStop:

            m_pPresenter->SetFullscreen(FALSE);

            // Drop samples from queue.
            Flush();

            m_cOutstandingSampleRequests = 0;

            // Send the event even if the previous call failed.
            hr = QueueEvent(MEStreamSinkStopped, GUID_NULL, hr, NULL);

            break;

        case OpPause:
            hr = QueueEvent(MEStreamSinkPaused, GUID_NULL, hr, NULL);
            break;

        case OpProcessSample:
        case OpPlaceMarker:
            if (!(m_fWaitingForOnClockStart))
            {
                hr = DispatchProcessSample(pOp);
            }
            break;
        }
    }

    SafeRelease(pState);

    return hr;
}

//-------------------------------------------------------------------
// Name: ProcessSamplesFromQueue
// Description:
//
// Removes all of the samples and markers that are currently in the
// queue and processes them.
//
// If bConsumeData = DropFrames
//     For each marker, send an MEStreamSinkMarker event, with hr = E_ABORT.
//     For each sample, drop the sample.
//
// If bConsumeData = ProcessFrames
//     For each marker, send an MEStreamSinkMarker event, with hr = S_OK.
//     For each sample, write the sample to the file.
//
// This method is called when we flush, stop, restart, receive a new
// sample, or receive a marker.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::ProcessSamplesFromQueue(ConsumeState bConsumeData)
{
    HRESULT hr = S_OK;
    IUnknown* pUnk = NULL;
    BOOL bProcessMoreSamples = TRUE;
    BOOL bDeviceChanged = FALSE;
    BOOL bProcessAgain = FALSE;

    // Enumerate all of the samples/markers in the queue.

    // Note: Dequeue returns S_FALSE when the queue is empty.
    while (m_SamplesToProcess.Dequeue(&pUnk) == S_OK)
    {
        bProcessMoreSamples = TRUE;
        IMarker* pMarker = NULL;
        IMFSample* pSample = NULL;
        IMFSample* pOutSample = NULL;

        // Figure out if this is a marker or a sample.

        hr = pUnk->QueryInterface(__uuidof(IMarker), (void**)&pMarker);
        if (hr == E_NOINTERFACE)
        {
            // If this is a sample, write it to the file.
            hr = pUnk->QueryInterface(IID_IMFSample, (void**)&pSample);
        }

        // Now handle the sample/marker appropriately.
        if (SUCCEEDED(hr))
        {
            if (pMarker)
            {
                hr = SendMarkerEvent(pMarker, bConsumeData);
            }
            else
            {
                assert(pSample != NULL);    // Not a marker, must be a sample
                if (bConsumeData == ProcessFrames)
                {
                    hr = m_pPresenter->ProcessFrame(m_pCurrentType, pSample, &m_unInterlaceMode, &bDeviceChanged, &bProcessAgain, &pOutSample);

                    if (SUCCEEDED(hr))
                    {
                        if (bDeviceChanged)
                        {
                            QueueEvent(MEStreamSinkDeviceChanged, GUID_NULL, S_OK, NULL);
                        }

                        if (bProcessAgain)
                        {
                            // The input sample is not used. Return it to the queue.
                            hr = m_SamplesToProcess.PutBack(pSample);
                        }
                    }

                    // If we are not actively playing, OR we are scrubbing (rate = 0) OR this is a
                    // repaint request, then we need to present the sample immediately. Otherwise,
                    // schedule it normally.
                    if (SUCCEEDED(hr))
                    {
                        if (pOutSample)
                        {
                            hr = m_pScheduler->ScheduleSample(pOutSample, (State_Started != m_state));
                            bProcessMoreSamples = FALSE;
                        }
                    }
                }
            }
        }

        SafeRelease(pUnk);
        SafeRelease(pMarker);
        SafeRelease(pSample);
        SafeRelease(pOutSample);

        if (!bProcessMoreSamples)
        {
            break;
        }
    }       // while loop

    SafeRelease(pUnk);

    return hr;
}

//-------------------------------------------------------------------
// Name: QueueAsyncOperation
// Description: Puts an async operation on the work queue.
//-------------------------------------------------------------------
HRESULT DX11VideoRenderer::CStreamSink::QueueAsyncOperation(StreamOperation op)
{
    HRESULT hr = S_OK;
    CAsyncOperation* pOp = new CAsyncOperation(op); // Created with ref count = 1
    if (pOp == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = MFPutWorkItem(m_WorkQueueId, &m_WorkQueueCB, pOp);
    }

    SafeRelease(pOp);  // Releases ref count

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     RequestSamples
//
//  Synopsis:   Issue more sample requests if necessary.
//
//--------------------------------------------------------------------------
HRESULT DX11VideoRenderer::CStreamSink::RequestSamples(void)
{
    HRESULT hr = S_OK;

    while (NeedMoreSamples())
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        m_cOutstandingSampleRequests++;

        hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, S_OK, NULL);
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

HRESULT DX11VideoRenderer::CStreamSink::SendMarkerEvent(IMarker* pMarker, ConsumeState ConsumeState)
{
    HRESULT hr = S_OK;
    HRESULT hrStatus = S_OK;  // Status code for marker event.

    PROPVARIANT var;
    PropVariantInit(&var);

    do
    {
        if (ConsumeState == DropFrames)
        {
            hrStatus = E_ABORT;
        }

        // Get the context data.
        hr = pMarker->GetContext(&var);

        if (SUCCEEDED(hr))
        {
            hr = QueueEvent(MEStreamSinkMarker, GUID_NULL, hrStatus, &var);
        }
    }
    while (FALSE);

    PropVariantClear(&var);

    return hr;
}

//-------------------------------------------------------------------
// Name: ValidateOperation
// Description: Checks if an operation is valid in the current state.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CStreamSink::ValidateOperation(StreamOperation op)
{
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

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CAsyncOperation class. - Private class used by CStreamSink class.
//
/////////////////////////////////////////////////////////////////////////////////////////////

DX11VideoRenderer::CStreamSink::CAsyncOperation::CAsyncOperation(StreamOperation op) :
    m_nRefCount(1),
    m_op(op)
{
}

ULONG DX11VideoRenderer::CStreamSink::CAsyncOperation::AddRef(void)
{
    return InterlockedIncrement(&m_nRefCount);
}

HRESULT DX11VideoRenderer::CStreamSink::CAsyncOperation::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv)
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

ULONG DX11VideoRenderer::CStreamSink::CAsyncOperation::Release(void)
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

DX11VideoRenderer::CStreamSink::CAsyncOperation::~CAsyncOperation(void)
{
}