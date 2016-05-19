//////////////////////////////////////////////////////////////////////////
//
// Scheduler.h: Schedules when video frames are presented.
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


struct SchedulerCallback;

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

class Scheduler
{
public:
    Scheduler();
    virtual ~Scheduler();

    void SetCallback(SchedulerCallback *pCB)
    {
        m_pCB = pCB;
    }

    void SetFrameRate(const MFRatio& fps);
    void SetClockRate(float fRate) { m_fRate = fRate; }

    const LONGLONG& LastSampleTime() const { return m_LastSampleTime; }
    const LONGLONG& FrameDuration() const { return m_PerFrameInterval; }

    HRESULT StartScheduler(IMFClock *pClock);
    HRESULT StopScheduler();

    HRESULT ScheduleSample(IMFSample *pSample, BOOL bPresentNow);
    HRESULT ProcessSamplesInQueue(LONG *plNextSleep);
    HRESULT ProcessSample(IMFSample *pSample, LONG *plNextSleep);
    HRESULT Flush();

    // ThreadProc for the scheduler thread.
    static DWORD WINAPI SchedulerThreadProc(LPVOID lpParameter);

private: 
    // non-static version of SchedulerThreadProc.
    DWORD SchedulerThreadProcPrivate();


private:
    ThreadSafeQueue<IMFSample>  m_ScheduledSamples;     // Samples waiting to be presented.

    IMFClock            *m_pClock;  // Presentation clock. Can be NULL.
    SchedulerCallback   *m_pCB;     // Weak reference; do not delete.

    DWORD               m_dwThreadID;
    HANDLE              m_hSchedulerThread;
    HANDLE              m_hThreadReadyEvent;
    HANDLE              m_hFlushEvent;

    float               m_fRate;                // Playback rate.
    MFTIME              m_PerFrameInterval;     // Duration of each frame.
    LONGLONG            m_PerFrame_1_4th;       // 1/4th of the frame duration.
    MFTIME              m_LastSampleTime;       // Most recent sample time.
};


//-----------------------------------------------------------------------------
// SchedulerCallback
//
// Defines the callback method to present samples. 
//-----------------------------------------------------------------------------

struct SchedulerCallback
{
    virtual HRESULT PresentSample(IMFSample *pSample, LONGLONG llTarget) = 0;
};
