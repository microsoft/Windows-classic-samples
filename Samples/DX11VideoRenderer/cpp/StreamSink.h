#pragma once

#include "Common.h"
#include "display.h"
#include "MFAttributesImpl.h"
#include "Marker.h"
#include "Presenter.h"
#include "Scheduler.h"

namespace DX11VideoRenderer
{
    class CStreamSink :
        public IMFStreamSink,
        public IMFMediaTypeHandler,
        public IMFGetService,
        public SchedulerCallback,
        public CMFAttributesImpl<IMFAttributes>,
        private CBase
    {
    public:

        CStreamSink(DWORD dwStreamId, CCritSec& critSec, CScheduler* pScheduler);
        virtual ~CStreamSink(void);

        // IUnknown
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv);
        STDMETHODIMP_(ULONG) Release(void);

        // IMFStreamSink
        STDMETHODIMP Flush(void);
        STDMETHODIMP GetIdentifier(__RPC__out DWORD* pdwIdentifier);
        STDMETHODIMP GetMediaSink(__RPC__deref_out_opt IMFMediaSink** ppMediaSink);
        STDMETHODIMP GetMediaTypeHandler(__RPC__deref_out_opt IMFMediaTypeHandler** ppHandler);
        STDMETHODIMP PlaceMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType, __RPC__in const PROPVARIANT* pvarMarkerValue, __RPC__in const PROPVARIANT* pvarContextValue);
        STDMETHODIMP ProcessSample(__RPC__in_opt IMFSample* pSample);

        // IMFMediaEventGenerator (from IMFStreamSink)
        STDMETHODIMP BeginGetEvent(IMFAsyncCallback* pCallback,IUnknown* punkState);
        STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, _Out_ IMFMediaEvent** ppEvent);
        STDMETHODIMP GetEvent(DWORD dwFlags, __RPC__deref_out_opt IMFMediaEvent** ppEvent);
        STDMETHODIMP QueueEvent(MediaEventType met, __RPC__in REFGUID guidExtendedType, HRESULT hrStatus, __RPC__in_opt const PROPVARIANT* pvValue);

        // IMFMediaTypeHandler
        STDMETHODIMP GetCurrentMediaType(_Outptr_ IMFMediaType** ppMediaType);
        STDMETHODIMP GetMajorType(__RPC__out GUID* pguidMajorType);
        STDMETHODIMP GetMediaTypeByIndex(DWORD dwIndex, _Outptr_ IMFMediaType** ppType);
        STDMETHODIMP GetMediaTypeCount(__RPC__out DWORD* pdwTypeCount);
        STDMETHODIMP IsMediaTypeSupported(IMFMediaType* pMediaType, _Outptr_opt_result_maybenull_ IMFMediaType** ppMediaType);
        STDMETHODIMP SetCurrentMediaType(IMFMediaType* pMediaType);

        // IMFGetService
        STDMETHODIMP GetService(__RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID* ppvObject);

        // SchedulerCallback
        HRESULT PresentFrame(void);

        HRESULT GetMaxRate(BOOL fThin, float* pflRate);
        HRESULT Initialize(IMFMediaSink* pParent, CPresenter* pPresenter);
        inline BOOL IsActive(void) const // IsActive: The "active" state is started or paused.
        {
            return ((m_state == State_Started) || (m_state == State_Paused));
        }
        HRESULT Pause(void);
        HRESULT Preroll(void);
        HRESULT Restart(void);
        HRESULT Shutdown(void);
        HRESULT Start(MFTIME start);
        HRESULT Stop(void);

    private:

        // State enum: Defines the current state of the stream.
        enum State
        {
            State_TypeNotSet = 0,   // No media type is set
            State_Ready,            // Media type is set, Start has never been called.
            State_Started,
            State_Paused,
            State_Stopped,

            State_Count             // Number of states
        };

        // StreamOperation: Defines various operations that can be performed on the stream.
        enum StreamOperation
        {
            OpSetMediaType = 0,
            OpStart,
            OpRestart,
            OpPause,
            OpStop,
            OpProcessSample,
            OpPlaceMarker,

            Op_Count                // Number of operations
        };

        enum ConsumeState
        {
            DropFrames = 0,
            ProcessFrames
        };

        // CAsyncOperation:
        // Used to queue asynchronous operations. When we call MFPutWorkItem, we use this
        // object for the callback state (pState). Then, when the callback is invoked,
        // we can use the object to determine which asynchronous operation to perform.

        // Optional data can include a sample (IMFSample) or a marker.
        // When ProcessSample is called, we use it to queue a sample. When ProcessMarker is
        // called, we use it to queue a marker. This way, samples and markers can live in
        // the same queue. We need this because the sink has to serialize marker events
        // with sample processing.
        class CAsyncOperation : public IUnknown
        {
        public:

            CAsyncOperation(StreamOperation op);

            // IUnknown methods.
            STDMETHODIMP_(ULONG) AddRef(void);
            STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv);
            STDMETHODIMP_(ULONG) Release(void);

            // Structure data.
            StreamOperation m_op;   // The operation to perform.
            PROPVARIANT m_varDataWM;   // Optional data, only used for water mark

        private:

            virtual ~CAsyncOperation(void);

            long m_nRefCount;
        };

        static GUID const* const s_pVideoFormats[];
        static const DWORD s_dwNumVideoFormats;
        static const MFRatio s_DefaultFrameRate;
        static const struct FormatEntry
        {
            GUID            Subtype;
            DXGI_FORMAT     DXGIFormat;
        } s_DXGIFormatMapping[];
        static BOOL ValidStateMatrix[State_Count][Op_Count]; // Defines a look-up table that says which operations are valid from which states.

        HRESULT DispatchProcessSample(CAsyncOperation* pOp);
        HRESULT CheckShutdown(void) const;
        HRESULT GetFrameRate(IMFMediaType* pType, MFRatio* pRatio);
        BOOL    NeedMoreSamples(void);
        HRESULT OnDispatchWorkItem(IMFAsyncResult* pAsyncResult);
        HRESULT ProcessSamplesFromQueue(ConsumeState bConsumeData);
        HRESULT QueueAsyncOperation(StreamOperation op);
        HRESULT RequestSamples(void);
        HRESULT SendMarkerEvent(IMarker* pMarker, ConsumeState bConsumeData);
        HRESULT ValidateOperation(StreamOperation op);

        const DWORD                 STREAM_ID;
        long                        m_nRefCount;                    // reference count
        CCritSec&                   m_critSec;                      // critical section for thread safety
        State                       m_state;
        BOOL                        m_IsShutdown;                   // Flag to indicate if Shutdown() method was called.
        DWORD                       m_WorkQueueId;                  // ID of the work queue for asynchronous operations.
        CAsyncCallback<CStreamSink> m_WorkQueueCB;                  // Callback for the work queue.
        ConsumeState                m_ConsumeData;                  // Flag to indicate process or drop frames
        MFTIME                      m_StartTime;                    // Presentation time when the clock started.
        DWORD                       m_cbDataWritten;                // How many bytes we have written so far.
        DWORD                       m_cOutstandingSampleRequests;   // Outstanding reuqests for samples.
        IMFMediaSink*               m_pSink;                        // Parent media sink
        IMFMediaEventQueue*         m_pEventQueue;                  // Event queue
        IMFByteStream*              m_pByteStream;                  // Bytestream where we write the data.
        CPresenter*                 m_pPresenter;
        CScheduler*                 m_pScheduler;
        IMFMediaType*               m_pCurrentType;
        BOOL                        m_fPrerolling;
        BOOL                        m_fWaitingForOnClockStart;
        ThreadSafeQueue<IUnknown>   m_SamplesToProcess;             // Queue to hold samples and markers. Applies to: ProcessSample, PlaceMarker
        UINT32                      m_unInterlaceMode;
        struct sFraction
        {
            DWORD Numerator;
            DWORD Denominator;
        }                           m_imageBytesPP;
        DXGI_FORMAT                 m_dxgiFormat;
    };
}
