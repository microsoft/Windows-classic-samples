//////////////////////////////////////////////////////////////////////////
//
// Presenter.h : Defines the presenter object.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
//////////////////////////////////////////////////////////////////////////


#pragma once

// RENDER_STATE: Defines the state of the presenter. 
enum RENDER_STATE
{
    RENDER_STATE_STARTED = 1,
    RENDER_STATE_STOPPED,
    RENDER_STATE_PAUSED,
    RENDER_STATE_SHUTDOWN,  // Initial state. 

    // State transitions:

    // InitServicePointers                  -> STOPPED
    // ReleaseServicePointers               -> SHUTDOWN
    // IMFClockStateSink::OnClockStart      -> STARTED
    // IMFClockStateSink::OnClockRestart    -> STARTED
    // IMFClockStateSink::OnClockPause      -> PAUSED
    // IMFClockStateSink::OnClockStop       -> STOPPED
};

// FRAMESTEP_STATE: Defines the presenter's state with respect to frame-stepping.
enum FRAMESTEP_STATE
{
    FRAMESTEP_NONE,             // Not frame stepping.
    FRAMESTEP_WAITING_START,    // Frame stepping, but the clock is not started.
    FRAMESTEP_PENDING,          // Clock is started. Waiting for samples.
    FRAMESTEP_SCHEDULED,        // Submitted a sample for rendering.
    FRAMESTEP_COMPLETE          // Sample was rendered. 

    // State transitions:

    // MFVP_MESSAGE_STEP                -> WAITING_START
    // OnClockStart/OnClockRestart      -> PENDING
    // MFVP_MESSAGE_PROCESSINPUTNOTIFY  -> SUBMITTED
    // OnSampleFree                     -> COMPLETE
    // MFVP_MESSAGE_CANCEL              -> NONE
    // OnClockStop                      -> NONE
    // OnClockSetRate( non-zero )       -> NONE
};


//-----------------------------------------------------------------------------
//  EVRCustomPresenter class
//  Description: Implements the custom presenter.
//-----------------------------------------------------------------------------

class EVRCustomPresenter : 
    BaseObject,  
    RefCountedObject, 
    // COM interfaces:
    public IMFVideoDeviceID,
    public IMFVideoPresenter, // Inherits IMFClockStateSink
    public IMFRateSupport,
    public IMFGetService,
    public IMFTopologyServiceLookupClient,
    public IMFVideoDisplayControl
{

public:
    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID iid, void **ppv);

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IMFGetService methods
    STDMETHOD(GetService)(REFGUID guidService, REFIID riid, LPVOID *ppvObject);

    // IMFVideoPresenter methods
    STDMETHOD(ProcessMessage)(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam);
    STDMETHOD(GetCurrentMediaType)(IMFVideoMediaType** ppMediaType);

    // IMFClockStateSink methods
    STDMETHOD(OnClockStart)(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
    STDMETHOD(OnClockStop)(MFTIME hnsSystemTime);
    STDMETHOD(OnClockPause)(MFTIME hnsSystemTime);
    STDMETHOD(OnClockRestart)(MFTIME hnsSystemTime);
    STDMETHOD(OnClockSetRate)(MFTIME hnsSystemTime, float flRate);

    // IMFRateSupport methods
    STDMETHOD(GetSlowestRate)(MFRATE_DIRECTION eDirection, BOOL bThin, float *pflRate);
    STDMETHOD(GetFastestRate)(MFRATE_DIRECTION eDirection, BOOL bThin, float *pflRate);
    STDMETHOD(IsRateSupported)(BOOL bThin, float flRate, float *pflNearestSupportedRate);

    // IMFVideoDeviceID methods
    STDMETHOD(GetDeviceID)(IID* pDeviceID);

    // IMFTopologyServiceLookupClient methods
    STDMETHOD(InitServicePointers)(IMFTopologyServiceLookup *pLookup);
    STDMETHOD(ReleaseServicePointers)();

    // IMFVideoDisplayControl methods
    STDMETHOD(GetNativeVideoSize)(SIZE* pszVideo, SIZE* pszARVideo) { return E_NOTIMPL; }
    STDMETHOD(GetIdealVideoSize)(SIZE* pszMin, SIZE* pszMax) { return E_NOTIMPL; }
    STDMETHOD(SetVideoPosition)(const MFVideoNormalizedRect* pnrcSource, const LPRECT prcDest);
    STDMETHOD(GetVideoPosition)(MFVideoNormalizedRect* pnrcSource, LPRECT prcDest);
    STDMETHOD(SetAspectRatioMode)(DWORD dwAspectRatioMode) { return E_NOTIMPL; }
    STDMETHOD(GetAspectRatioMode)(DWORD* pdwAspectRatioMode) { return E_NOTIMPL; }
    STDMETHOD(SetVideoWindow)(HWND hwndVideo);
    STDMETHOD(GetVideoWindow)(HWND* phwndVideo);
    STDMETHOD(RepaintVideo)();
    STDMETHOD(GetCurrentImage)(BITMAPINFOHEADER* pBih, BYTE** pDib, DWORD* pcbDib, LONGLONG* pTimeStamp) { return E_NOTIMPL; }
    STDMETHOD(SetBorderColor)(COLORREF Clr) { return E_NOTIMPL; }
    STDMETHOD(GetBorderColor)(COLORREF* pClr) { return E_NOTIMPL; }
    STDMETHOD(SetRenderingPrefs)(DWORD dwRenderFlags) { return E_NOTIMPL; }
    STDMETHOD(GetRenderingPrefs)(DWORD* pdwRenderFlags) { return E_NOTIMPL; }
    STDMETHOD(SetFullscreen)(BOOL bFullscreen) { return E_NOTIMPL; }
    STDMETHOD(GetFullscreen)(BOOL* pbFullscreen) { return E_NOTIMPL; }

protected:
    EVRCustomPresenter(HRESULT& hr);
    virtual ~EVRCustomPresenter();

    // CheckShutdown: 
    //     Returns MF_E_SHUTDOWN if the presenter is shutdown.
    //     Call this at the start of any methods that should fail after shutdown.
    inline HRESULT CheckShutdown() const 
    {
        if (m_RenderState == RENDER_STATE_SHUTDOWN)
        {
            return MF_E_SHUTDOWN;
        }
        else
        {
            return S_OK;
        }
    }

    // IsActive: The "active" state is started or paused.
    inline BOOL IsActive() const
    {
        return ((m_RenderState == RENDER_STATE_STARTED) || (m_RenderState == RENDER_STATE_PAUSED));
    }

    // IsScrubbing: Scrubbing occurs when the frame rate is 0.
    inline BOOL IsScrubbing() const { return m_fRate == 0.0f; }

    // NotifyEvent: Send an event to the EVR through its IMediaEventSink interface.
    void NotifyEvent(long EventCode, LONG_PTR Param1, LONG_PTR Param2)
    {
        if (m_pMediaEventSink)
        {
            m_pMediaEventSink->Notify(EventCode, Param1, Param2);
        }
    }

    float GetMaxRate(BOOL bThin);

    // Mixer operations
    HRESULT ConfigureMixer(IMFTransform *pMixer);

    // Formats
    HRESULT CreateOptimalVideoType(IMFMediaType* pProposed, IMFMediaType **ppOptimal);
    HRESULT CalculateOutputRectangle(IMFMediaType *pProposed, RECT *prcOutput);
    HRESULT SetMediaType(IMFMediaType *pMediaType);
    HRESULT IsMediaTypeSupported(IMFMediaType *pMediaType);

    // Message handlers
    HRESULT Flush();
    HRESULT RenegotiateMediaType();
    HRESULT ProcessInputNotify();
    HRESULT BeginStreaming();
    HRESULT EndStreaming();
    HRESULT CheckEndOfStream();

    // Managing samples
    void    ProcessOutputLoop();   
    HRESULT ProcessOutput();
    HRESULT DeliverSample(IMFSample *pSample, BOOL bRepaint);
    HRESULT TrackSample(IMFSample *pSample);
    void    ReleaseResources();

    // Frame-stepping
    HRESULT PrepareFrameStep(DWORD cSteps);
    HRESULT StartFrameStep();
    HRESULT DeliverFrameStepSample(IMFSample *pSample);
    HRESULT CompleteFrameStep(IMFSample *pSample);
    HRESULT CancelFrameStep();

    // Callbacks

    // Callback when a video sample is released.
    HRESULT OnSampleFree(IMFAsyncResult *pResult);
    AsyncCallback<EVRCustomPresenter>   m_SampleFreeCB;

protected:

    // FrameStep: Holds information related to frame-stepping. 
    // Note: The purpose of this structure is simply to organize the data in one variable.
    struct FrameStep
    {
        FrameStep() : state(FRAMESTEP_NONE), steps(0), pSampleNoRef(NULL)
        {
        }

        FRAMESTEP_STATE     state;          // Current frame-step state.
        VideoSampleList     samples;        // List of pending samples for frame-stepping.
        DWORD               steps;          // Number of steps left.
        DWORD_PTR           pSampleNoRef;   // Identifies the frame-step sample.
    };


protected:

    RENDER_STATE                m_RenderState;          // Rendering state.
    FrameStep                   m_FrameStep;            // Frame-stepping information.

    CritSec                     m_ObjectLock;           // Serializes our public methods.  

    // Samples and scheduling
    Scheduler                   m_scheduler;            // Manages scheduling of samples.
    SamplePool                  m_SamplePool;           // Pool of allocated samples.
    DWORD                       m_TokenCounter;         // Counter. Incremented whenever we create new samples.

    // Rendering state
    BOOL                        m_bSampleNotify;        // Did the mixer signal it has an input sample?
    BOOL                        m_bRepaint;             // Do we need to repaint the last sample?
    BOOL                        m_bPrerolled;           // Have we presented at least one sample?
    BOOL                        m_bEndStreaming;        // Did we reach the end of the stream (EOS)?

    MFVideoNormalizedRect       m_nrcSource;            // Source rectangle.
    float                       m_fRate;                // Playback rate.

    // Deletable objects.
    D3DPresentEngine            *m_pD3DPresentEngine;    // Rendering engine. (Never null if the constructor succeeds.)

    // COM interfaces.
    IMFClock                    *m_pClock;               // The EVR's clock.
    IMFTransform                *m_pMixer;               // The mixer.
    IMediaEventSink             *m_pMediaEventSink;      // The EVR's event-sink interface.
    IMFMediaType                *m_pMediaType;           // Output media type
};

