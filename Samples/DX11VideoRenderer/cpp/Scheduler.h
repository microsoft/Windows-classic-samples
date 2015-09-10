#pragma once

#include "Common.h"

namespace DX11VideoRenderer
{
    //-----------------------------------------------------------------------------
    // SchedulerCallback
    //
    // Defines the callback method to present samples.
    //-----------------------------------------------------------------------------

    struct SchedulerCallback
    {
        virtual HRESULT PresentFrame(void) = 0;
    };

    //-----------------------------------------------------------------------------
    // Scheduler class
    //
    // Schedules when a sample should be displayed.
    //
    // Note: Presentation of each sample is performed by another object which
    // must implement SchedulerCallback::PresentSample.
    //
    // General design:
    // The scheduler generally receives samples before their presentation time. It
    // puts the samples on a queue and presents them in FIFO order on a worker
    // thread. The scheduler communicates with the worker thread by posting thread
    // messages.
    //
    // The caller has the option of presenting samples immediately (for example,
    // for repaints).
    //-----------------------------------------------------------------------------

    class CScheduler:
        public IUnknown,
        private CBase
    {
    public:

        CScheduler(CCritSec& critSec);
        virtual ~CScheduler(void);

        // IUnknown
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv);

        void SetCallback(SchedulerCallback* pCB)
        {
            m_pCB = pCB;
        }

        void SetFrameRate(const MFRatio& fps);
        void SetClockRate(float fRate) { m_fRate = fRate; }

        const LONGLONG& LastSampleTime(void) const { return m_LastSampleTime; }
        const LONGLONG& FrameDuration(void) const { return m_PerFrameInterval; }

        HRESULT StartScheduler(IMFClock* pClock);
        HRESULT StopScheduler(void);

        HRESULT ScheduleSample(IMFSample* pSample, BOOL bPresentNow);
        HRESULT ProcessSamplesInQueue(LONG* plNextSleep);
        HRESULT ProcessSample(IMFSample* pSample, LONG* plNextSleep);
        HRESULT Flush(void);

        DWORD GetCount(void){ return m_ScheduledSamples.GetCount(); }

    private:

        HRESULT StartProcessSample();
        HRESULT OnTimer(__RPC__in_opt IMFAsyncResult* pResult);
        METHODASYNCCALLBACKEX(OnTimer, CScheduler, 0, MFASYNC_CALLBACK_QUEUE_MULTITHREADED);

        long                        m_nRefCount;
        CCritSec&                   m_critSec;          // critical section for thread safety
        SchedulerCallback*          m_pCB;              // Weak reference; do not delete.
        ThreadSafeQueue<IMFSample>  m_ScheduledSamples; // Samples waiting to be presented.
        IMFClock*                   m_pClock;           // Presentation clock. Can be NULL.
        float                       m_fRate;            // Playback rate.
        HANDLE                      m_hWaitTimer;       // Wait Timer after which frame is presented.
        MFTIME                      m_LastSampleTime;   // Most recent sample time.
        MFTIME                      m_PerFrameInterval; // Duration of each frame.
        LONGLONG                    m_PerFrame_1_4th;   // 1/4th of the frame duration.
        MFWORKITEM_KEY              m_keyTimer;
    };
}
