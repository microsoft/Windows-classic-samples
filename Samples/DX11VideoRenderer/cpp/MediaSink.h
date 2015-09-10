#pragma once

#include "Common.h"
#include "Presenter.h"
#include "Scheduler.h"
#include "StreamSink.h"

namespace DX11VideoRenderer
{
    class CMediaSink :
        public IMFMediaSink,
        public IMFClockStateSink,
        public IMFGetService,
        public IMFRateSupport,
        public IMFMediaSinkPreroll,
        private CBase
    {
    public:

        // Static method to create the object.
        static HRESULT CreateInstance(_In_ REFIID iid, _COM_Outptr_ void** ppSink);

        // IUnknown
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv);
        STDMETHODIMP_(ULONG) Release(void);

        // IMFMediaSink methods
        STDMETHODIMP AddStreamSink(DWORD dwStreamSinkIdentifier, __RPC__in_opt IMFMediaType* pMediaType, __RPC__deref_out_opt IMFStreamSink** ppStreamSink);
        STDMETHODIMP GetCharacteristics(__RPC__out DWORD* pdwCharacteristics);
        STDMETHODIMP GetPresentationClock(__RPC__deref_out_opt IMFPresentationClock** ppPresentationClock);
        STDMETHODIMP GetStreamSinkById(DWORD dwIdentifier, __RPC__deref_out_opt IMFStreamSink** ppStreamSink);
        STDMETHODIMP GetStreamSinkByIndex(DWORD dwIndex, __RPC__deref_out_opt IMFStreamSink** ppStreamSink);
        STDMETHODIMP GetStreamSinkCount(__RPC__out DWORD* pcStreamSinkCount);
        STDMETHODIMP RemoveStreamSink(DWORD dwStreamSinkIdentifier);
        STDMETHODIMP SetPresentationClock(__RPC__in_opt IMFPresentationClock* pPresentationClock);
        STDMETHODIMP Shutdown(void);

        // IMFClockStateSink methods
        STDMETHODIMP OnClockPause(MFTIME hnsSystemTime);
        STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime);
        STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate);
        STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
        STDMETHODIMP OnClockStop(MFTIME hnsSystemTime);

        // IMFGetService
        STDMETHODIMP GetService(__RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID* ppvObject);

        // IMFRateSupport
        STDMETHODIMP GetFastestRate(MFRATE_DIRECTION eDirection, BOOL fThin, _Out_ float* pflRate);
        STDMETHODIMP GetSlowestRate(MFRATE_DIRECTION eDirection, BOOL fThin, _Out_ float* pflRate);
        STDMETHODIMP IsRateSupported(BOOL fThin, float flRate, __RPC__inout_opt float* pflNearestSupportedRate);

        // IMFMediaSinkPreroll
        STDMETHODIMP NotifyPreroll(MFTIME hnsUpcomingStartTime);

    private:

        static CCritSec s_csStreamSinkAndScheduler; // critical section for thread safety, used for CStreamSink and CScheduler

        CMediaSink(void);
        virtual ~CMediaSink(void);

        HRESULT CheckShutdown(void) const;
        HRESULT Initialize(void);

        const DWORD             STREAM_ID;      // The stream ID of the one stream on the sink.
        long                    m_nRefCount;    // reference count
        CCritSec                m_csMediaSink;  // critical section for thread safety, used for CMediaSink
        BOOL                    m_IsShutdown;   // Flag to indicate if Shutdown() method was called.
        CStreamSink*            m_pStream;      // Byte stream
        IMFPresentationClock*   m_pClock;       // Presentation clock.
        CScheduler*             m_pScheduler;    // Manages scheduling of samples.
        CPresenter*             m_pPresenter;
    };
}
