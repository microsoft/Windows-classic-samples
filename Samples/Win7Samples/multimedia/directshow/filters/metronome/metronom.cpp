//------------------------------------------------------------------------------
// File: Metronom.cpp
//
// Desc: DirectShow sample code - Metronome clock filter
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

/******************************************************************************
// This clock filter will listen to the sound card's input port, and listen
// for ticks.  The faster the ticks, the faster the clock will run.  So you can
// clap your hands into a microphone and control how fast a movie plays.
//
// NOTE: THIS FILTER DOES NOT WORK FOR AUDIO!!!
//      The default DirectSound renderer only allows ~2% variance in clocks.
//      This sample filter is intended to serve as a clock for a VIDEO clip.
//
// To use the filter:
//      - Build a filter graph with a video component (or audio and video)
//      - Delete the "Default DirectSound Renderer" from the graph (which
//          removes the reference clock from the graph)
//      - Select Graph->Insert Filters in GraphEdit
//      - Expand the DirectShow Filters node and insert the "Metronome" filter.
//      - Play the graph
//      - The video will begin playing at normal speed.  To modify the speed,
//          clap your hands or use a metronome to set a new speed.  For best
//          results, start clapping at the same rate that you were clapping
//          when you set the clock rate.
//
//  Please refer to this sample's README.TXT file for more instructions.
//
******************************************************************************
//
//  Release Notes
//  =============
//
//  0.  This filter opens the waveIn device whenever it is around, so you can't
//  have any other app that is recording audio running at the same time. Sorry.
//
//  1.  This filter works for VIDEO ONLY.  Our audio renderer is not capable
//  of slaving  to radically different clock rates.   Make a graph in graphedit
//  of some video file, and then Insert the "metronome" filter into the graph.
//  If the clip contains an audio component, then remove the "Default DirectSound
//  Renderer" from the filter graph.  Play the graph.
//
//  2.  Clap into your microphone.  If you clap 92 times per minute (once every
//  ~652 ms) the movie will play at the normal rate.  You can change this default
//  near the top of the .cpp file.  Clap faster or slower, and the playback rate
//  of the video will change accordingly.
//
//  3.  Or, hook up an electronic metronome by a cable to your sound card, and
//  turn the dial and watch the video playback speed change.
//
//  4.  If you don't clap for any length of time, it keeps on at the old rate,
//  so you can give your hands a rest.  But when you start clapping again, you must
//  start clapping at about the same speed you left off at, and then slow down
//  or speed up, or it will ignore you.  To be precise:  Don't suddenly start
//  clapping more than TWICE as slow as you were before you stopped, or you'll
//  be ignored.
//
//  5.  DirectShow can't play infinitely fast.  The limit depends on your CPU speed.
//  If you try and go much faster than is possible for any length of time, it will
//  get confused, will stop working, and you won't be able to slow it down.
//  Press STOP and PLAY again.
//
//
//  The only important rule for writing a clock is this:
//      Your clock can NEVER GO BACKWARDS!
//
//   When you are asked what time it is, never give a time before the last
//   reported time.
//
******************************************************************************/


#include <streams.h>
#include <initguid.h>
#include <limits.h>
#include <olectl.h>

#include "metronom.h"

#pragma warning(disable:4355)

//
//  Beats per minute - this value will be considered to be "standard" speed.
//  (MIDI applications often use 120 BPM to be the standard.)
//
#define BPM 92      // normal clock speed - when the metronome ticks at
                    // this rate, we'll provide normal wall clock time
                    // (NOTE: You can set this to any value, depending on what kind
                    // of metronome you're using.)

//
//  Sound threshold that must be reached in order to achieve a valid "beat".
//  If your sound card volume is set too high, then all sounds (including
//  background noise) may be registered as being above this threshold.
//
#define THRESHOLD 150   // 8 bit samples above this value are HIGH (remember,
                        // 128 is silence), else they are LOW
                        // (leaving 128-255 values as valid)

#define BUFSIZE 3072    // size per wave buffer

//
// Setup data
//
AMOVIESETUP_FILTER sudMetronomeFilter =
{
    &CLSID_MetronomeFilter, // Filter CLSID
    L"Metronome Filter",    // String name
    MERIT_DO_NOT_USE,       // Filter merit
    0,                      // Number pins
    NULL                    // Pin details
};

//
//  Filter registration
//
CFactoryTemplate g_Templates[1] =
{
    {L"Metronome Filter",
    &CLSID_MetronomeFilter,
    (LPFNNewCOMObject) CMetronomeFilter::CreateInstance,
    NULL,
    &sudMetronomeFilter }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);



CMetronomeFilter::CMetronomeFilter(LPUNKNOWN pUnk, HRESULT *phr)
                : CBaseFilter(NAME("Metronome Filter"), pUnk, &m_Lock, CLSID_NULL)
                , m_Clock(static_cast<IBaseFilter*>(this), phr)
{
    // Set the protected variable in CBaseFilter so that
    // CBaseFilter::GetClassID reports the correct ClassID!
    m_clsid = CLSID_MetronomeFilter;
}

//
// CreateInstance
//
CUnknown *CMetronomeFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    ASSERT(phr);

    CMetronomeFilter *pNewObject = new CMetronomeFilter(punk, phr);
    if(pNewObject == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}


STDMETHODIMP CMetronomeFilter::NonDelegatingQueryInterface( REFIID riid, void ** ppv )
{
    CheckPointer(ppv, E_POINTER);

    if(riid == IID_IReferenceClock)
    {
        return GetInterface(static_cast<IReferenceClock*>(&m_Clock), ppv);
    }
    else
    {
        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}


//  Pin enumeration
int CMetronomeFilter::GetPinCount()
{
    return 0;
}


CBasePin *CMetronomeFilter::GetPin(int i)
{
    UNREFERENCED_PARAMETER(i);
    return NULL;
}


STDMETHODIMP CMetronomeFilter::SetSyncSource(IReferenceClock * pClock)
{
    m_Clock.SetSyncSource(pClock);
    return CBaseFilter::SetSyncSource(pClock);
}


//
// GetSetupData
//
LPAMOVIESETUP_FILTER CMetronomeFilter::GetSetupData()
{
    return &sudMetronomeFilter;
}


//
// CMetronome methods
//
CMetronome::CMetronome(LPUNKNOWN pUnk, HRESULT *phr)
            : CBaseReferenceClock(NAME("Metronome Filter Clock"), pUnk, phr)
            , m_pCurrentRefClock(0), m_pPrevRefClock(0)
{
    EXECUTE_ASSERT(SUCCEEDED(OpenWaveDevice()));

    // last time we reported
    m_dwLastMet = 0;

    // similar to m_LastMet, but used to help switch between clocks somehow
    m_dwPrevSystemTime = timeGetTime();

    // what timeGetTime said last time we heard a tick
    m_LastTickTGT = m_dwPrevSystemTime;

    // the number we reported last time we heard a tick
    m_LastTickTime = m_LastTickTGT;

    // the last time we reported (in 100ns units)
    m_rtPrivateTime = (UNITS / MILLISECONDS) * m_dwPrevSystemTime;

    // what timeGetTime said the last time we were asked what time it was
    m_dwLastTGT = m_dwPrevSystemTime;

    // We start off assuming the clock is running at normal speed
    m_msPerTick = 60000 / BPM;

    DbgLog((LOG_TRACE,1,TEXT("Creating clock at ref tgt=%d"), m_LastTickTime));
}


CMetronome::~CMetronome(void)
{
    CloseWaveDevice();
}


STDMETHODIMP CMetronomeFilter::Pause()
{
    if(m_State == State_Stopped)
    {
        m_Clock.StartWaveDevice();
    }

    return CBaseFilter::Pause();
}


STDMETHODIMP CMetronomeFilter::Stop()
{
    m_Clock.StopWaveDevice();
    return CBaseFilter::Stop();
}


REFERENCE_TIME CMetronome::GetPrivateTime()
{
    CAutoLock cObjectLock(this);

   /* If the clock has wrapped then the current time will be less than
    * the last time we were notified so add on the extra milliseconds
    *
    * The time period is long enough so that the likelihood of
    * successive calls spanning the clock cycle is not considered.
    */

    // This returns the current time in ms according to our special clock.  If
    // we used timeGetTime() here, our clock would run normally.
    DWORD dwTime = MetGetTime();
    {
        REFERENCE_TIME delta = REFERENCE_TIME(dwTime) - REFERENCE_TIME(m_dwPrevSystemTime);
        if(dwTime < m_dwPrevSystemTime)
            delta +=  REFERENCE_TIME(UINT_MAX) + 1;

        m_dwPrevSystemTime = dwTime;

        delta *= (UNITS / MILLISECONDS);
        m_rtPrivateTime += delta;
    }

    return m_rtPrivateTime;
}


void CMetronome::SetSyncSource(IReferenceClock * pClock)
{
    m_pPrevRefClock = m_pCurrentRefClock;

    if(pClock)
    {
        m_dwPrevSystemTime = timeGetTime();

        if(IsEqualObject(pClock, pUnk()))
        {
            // Sync this clock up to the old one - just to be nice - for now
            m_LastTickTGT = m_dwPrevSystemTime;
            m_LastTickTime = m_LastTickTGT;
            m_rtPrivateTime = (UNITS / MILLISECONDS) * m_dwPrevSystemTime;

            if(m_pPrevRefClock)
            {
                if(SUCCEEDED(m_pPrevRefClock->GetTime(&m_rtPrivateTime)))
                {
                    m_dwPrevSystemTime += timeGetTime();
                    m_dwPrevSystemTime /= 2;
                }
                else
                    ASSERT(FALSE);
            }

            DbgLog((LOG_TRACE,1,TEXT("*** USING OUR CLOCK : reference is %d at tgt %d"),
                   (DWORD)(MILLISECONDS * m_rtPrivateTime / UNITS), m_LastTickTime));

        }
        else
        {
            // Sync our clock up to the new one
            m_LastTickTGT = m_dwPrevSystemTime;
            m_LastTickTime = m_LastTickTGT;
            EXECUTE_ASSERT(SUCCEEDED(pClock->GetTime(&m_rtPrivateTime)));

            m_dwPrevSystemTime += timeGetTime();
            m_dwPrevSystemTime /= 2;

            DbgLog((LOG_TRACE,1,TEXT("*** USING SOMEONE ELSE'S CLOCK : reference is %d at tgt %d"),
                   (DWORD)(MILLISECONDS * m_rtPrivateTime / UNITS), m_LastTickTime));
        }
    }

    m_pCurrentRefClock = pClock;
}


// The last time the metronome ticked we reported m_LastTickTime
// and timeGetTime() reported m_LastTimeTGT.
// It is currently ticking at a rate of m_msPerTick milliseconds per tick.
// When it ticks at 60000/BPM ms per tick, that's normal clock time.
// The last time somebody called us, we returned m_dwLastMet
//
DWORD CMetronome::MetGetTime(void)
{
    // Don't let anybody change our time variables on us while we're using them
    m_csClock.Lock();
    LONGLONG lfudge;

    // how many ms have elapsed since last time we were asked?
    DWORD tgt = timeGetTime();
    LONGLONG lms = tgt - m_LastTickTGT;

    // How many ms do we want to pretend elapsed?
    if(m_msPerTick)
        lfudge = lms * (60000 / BPM) / (LONGLONG)m_msPerTick;
    else
        lfudge = 0; // !!!

    // that's the new time to report
    DWORD dw = m_LastTickTime + (DWORD)lfudge;
    m_csClock.Unlock();

    // Under no circumstances do we let the clock run backwards.  Just stall it.
    if(dw < m_dwLastMet)
    {
        dw = m_dwLastMet;
        DbgLog((LOG_TRACE,1,TEXT("*** ACK! Tried to go backwards!")));
    }

    DbgLog((LOG_TRACE,3,TEXT("MetTGT: %dms elapsed. Adjusted to %dms"),
        (int)lms, (int)lfudge));
    DbgLog((LOG_TRACE,3,TEXT("        returning %d TGT=%d"), (int)dw,
        (int)timeGetTime()));

    m_dwLastMet = dw;
    m_dwLastTGT = tgt;
    return dw;
}


STDMETHODIMP CMetronome::OpenWaveDevice(void)
{
    WAVEFORMATEX wfx;

    // do what's quickest - 8 bit MONO 11kHz (ignore quality)
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nSamplesPerSec  = 11025;
    wfx.nChannels       = 1;
    wfx.wBitsPerSample  = 8;
    wfx.nBlockAlign     = (WORD) (wfx.nChannels * ((wfx.wBitsPerSample + 7) / 8));
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize          = 0;

    DbgLog((LOG_TRACE,1,TEXT("*** Opening wave device....")));

    UINT err = waveInOpen(&m_hwi,
        WAVE_MAPPER,
        &wfx,
        (DWORD_PTR) (&CMetronome::Callback),
        (DWORD_PTR) this,
        CALLBACK_FUNCTION);

    if(err != 0)
    {
        DbgLog((LOG_ERROR,1,TEXT("Error opening wave device: %u"), err));
        return E_FAIL;
    }
    waveInStop(m_hwi);

    // 4 buffers should be good enough
    DbgLog((LOG_TRACE,1,TEXT("allocating wave buffers")));
    m_pwh1 = (LPWAVEHDR)QzTaskMemAlloc(sizeof(WAVEHDR) + BUFSIZE);
    m_pwh2 = (LPWAVEHDR)QzTaskMemAlloc(sizeof(WAVEHDR) + BUFSIZE);
    m_pwh3 = (LPWAVEHDR)QzTaskMemAlloc(sizeof(WAVEHDR) + BUFSIZE);
    m_pwh4 = (LPWAVEHDR)QzTaskMemAlloc(sizeof(WAVEHDR) + BUFSIZE);

    if(!m_pwh1 || !m_pwh2 || !m_pwh3 || !m_pwh4)
    {
        if(m_pwh1) QzTaskMemFree(m_pwh1);
        if(m_pwh2) QzTaskMemFree(m_pwh2);
        if(m_pwh3) QzTaskMemFree(m_pwh3);
        if(m_pwh4) QzTaskMemFree(m_pwh4);

        DbgLog((LOG_ERROR,1,TEXT("ERROR allocating wave buffers")));
        waveInClose(m_hwi);
        return E_FAIL;
    }

    // Zero the wave headers
    _fmemset(m_pwh1, 0, sizeof(WAVEHDR));
    _fmemset(m_pwh2, 0, sizeof(WAVEHDR));
    _fmemset(m_pwh3, 0, sizeof(WAVEHDR));
    _fmemset(m_pwh4, 0, sizeof(WAVEHDR));

    m_pwh1->lpData = (LPSTR)m_pwh1 + sizeof(WAVEHDR);
    m_pwh1->dwBufferLength = BUFSIZE;
    m_pwh1->dwBytesRecorded = 0;

    m_pwh2->lpData = (LPSTR)m_pwh2 + sizeof(WAVEHDR);
    m_pwh2->dwBufferLength = BUFSIZE;
    m_pwh2->dwBytesRecorded = 0;

    m_pwh3->lpData = (LPSTR)m_pwh3 + sizeof(WAVEHDR);
    m_pwh3->dwBufferLength = BUFSIZE;
    m_pwh3->dwBytesRecorded = 0;

    m_pwh4->lpData = (LPSTR)m_pwh4 + sizeof(WAVEHDR);
    m_pwh4->dwBufferLength = BUFSIZE;
    m_pwh4->dwBytesRecorded = 0;

    DbgLog((LOG_TRACE,1,TEXT("preparing headers")));

    // Prepare the wave headers
    DWORD dw1 = waveInPrepareHeader(m_hwi, m_pwh1, sizeof(WAVEHDR));
    DWORD dw2 = waveInPrepareHeader(m_hwi, m_pwh2, sizeof(WAVEHDR));
    DWORD dw3 = waveInPrepareHeader(m_hwi, m_pwh3, sizeof(WAVEHDR));
    DWORD dw4 = waveInPrepareHeader(m_hwi, m_pwh4, sizeof(WAVEHDR));

    if(dw1 || dw2 || dw3 || dw4)
    {
        DbgLog((LOG_ERROR,1,TEXT("error %d preparing headers"), dw1));
        return E_FAIL;
    }

    return S_OK;
}


STDMETHODIMP CMetronome::StartWaveDevice(void)
{
    DbgLog((LOG_TRACE,1,TEXT("*** Starting the wave device and adding buffers")));

    DWORD dw1 = waveInAddBuffer(m_hwi, m_pwh1, sizeof(WAVEHDR));
    DWORD dw2 = waveInAddBuffer(m_hwi, m_pwh2, sizeof(WAVEHDR));
    DWORD dw3 = waveInAddBuffer(m_hwi, m_pwh3, sizeof(WAVEHDR));
    DWORD dw4 = waveInAddBuffer(m_hwi, m_pwh4, sizeof(WAVEHDR));

    if(dw1 || dw2 || dw3 || dw4)
    {
        DbgLog((LOG_ERROR,1,TEXT("error %d adding the buffers"), dw1));
        return E_FAIL;
    }

    m_fWaveRunning      = TRUE;
    m_SamplesSinceTick  = 0;
    m_SamplesSinceSpike = 0;
    m_fSpikeAtStart     = FALSE;

    dw1 = waveInStart(m_hwi);
    if(dw1)
    {
        DbgLog((LOG_ERROR,1,TEXT("error %d starting the wave device"), dw1));
        return E_FAIL;
    }

    return S_OK;
}


STDMETHODIMP CMetronome::StopWaveDevice(void)
{
    DbgLog((LOG_TRACE,1,TEXT("*** Stopping the wave device")));
    m_fWaveRunning = FALSE;

    if(m_hwi)
    {
        waveInStop(m_hwi);
        waveInReset(m_hwi);
    }

    return S_OK;
}


STDMETHODIMP CMetronome::CloseWaveDevice(void)
{
    DbgLog((LOG_TRACE,1,TEXT("*** Closing down the wave device")));

    if(m_hwi)
    {
        waveInUnprepareHeader(m_hwi, m_pwh1, sizeof(WAVEHDR));
        waveInUnprepareHeader(m_hwi, m_pwh2, sizeof(WAVEHDR));
        waveInUnprepareHeader(m_hwi, m_pwh3, sizeof(WAVEHDR));
        waveInUnprepareHeader(m_hwi, m_pwh4, sizeof(WAVEHDR));
        waveInClose(m_hwi);
    }

    return S_OK;
}


// This function is given a bunch of input wave data and finds a tick or spike
// in the sound.  It returns how many bytes into the data the spike begins.
// Once it finds a tick, it sets m_fSpikeAtStart, which means there is a spike
// at the beginning of the data that is left that we need to skip past before
// we start looking for another spike
//
// Returning -1 means there is no spike
int CMetronome::FindSpike(LPBYTE lp, int len)
{
    int lenOrig = len;

TryAgain:
    // too small to deal with
    if(len < 2)
        return -1;

    // we are supposed to skip any spike at the beginning of this buffer
    // we wait until we see 2 samples in a row under the threshold.
    if(m_fSpikeAtStart)
    {
        while(len >=2 && (*lp > THRESHOLD || *(lp+1) > THRESHOLD))
        {
            lp++;
            len--;
            m_SamplesSinceSpike++;
        }
        if(len < 2)
            return -1;
    }
    m_fSpikeAtStart = FALSE;

    // look for spike - at least 2 consecutive samples above the threshold
    while(len >=2 && (*lp <= THRESHOLD || *(lp+1) <= THRESHOLD))
    {
        lp++;
        len--;
        m_SamplesSinceSpike ++;
    }
    if(len < 2)
        return -1;

    m_fSpikeAtStart = TRUE;

    // too close together, don't count it - so random music or frequncies above
    // 100 Hz or so will be seen as silence with no ticks.  This way if you are
    // using a digital metronome and set it to generate A440 it won't make the
    // clock go wild.
    if(m_SamplesSinceSpike < 1000)
    {
        m_SamplesSinceSpike = 0;
        goto TryAgain;
    }

    m_SamplesSinceSpike = 0;
    return lenOrig - len;
}


void CALLBACK CMetronome::Callback(HDRVR hdrvr, UINT uMsg, DWORD_PTR dwUser,
                                   DWORD_PTR dw1, DWORD_PTR dw2)
{
    CMetronome *pFilter = (CMetronome *) dwUser;

    switch(uMsg)
    {
        case WIM_DATA:
            {
                // really need a second worker thread here, because
                // one shouldn't do this much in a wave callback.

                LPWAVEHDR lpwh = (LPWAVEHDR) dw1;

                ASSERT(lpwh);
                ASSERT(pFilter);

                DbgLog((LOG_TRACE,4,TEXT("WAVEIN Callback: %d bytes recorded"),
                        lpwh->dwBytesRecorded));

                LPBYTE lp = (LPBYTE)(lpwh->lpData);
                int len = lpwh->dwBytesRecorded;
                int spike;

                // look for a spike in this buffer we just recorded
                while((spike = pFilter->FindSpike(lp, len)) != -1)
                {
                    // don't let anybody else mess with our timing variables
                    pFilter->m_csClock.Lock();
                    lp += spike;
                    len -= spike;

                    // How long has it been since we saw a tick?
                    pFilter->m_SamplesSinceTick += spike;
                    DWORD msPerTickPrev = pFilter->m_msPerTick;

                    // Even though we just got the callback now, this stuff was
                    // recorded who knows how long ago, so what we're doing is not
                    // entirely correct... we're assuming that since we just noticed
                    // the tick means that it happened right now.  As long as our
                    // buffers are really small, and the system is very responsive,
                    // this won't be too bad.
                    DWORD dwTGT = timeGetTime();

                    // deal with clock stopping altogether for a while - pretend
                    // it kept ticking at its old rate or else we will think we're
                    // way ahead and the clock will freeze for the length of time
                    // the clock was stopped
                    // So if it's been a while since the last tick, don't use that
                    // long interval as a new tempo.  This way you can stop clapping
                    // and the movie will keep the current tempo until you start
                    // clapping a new tempo.
                    // (If it's been > 1.5s since the last tick, this is probably
                    //  the start of a new tempo).
                    if(pFilter->m_SamplesSinceTick * 1000 / 11025 > 1500)
                    {
                        DbgLog((LOG_TRACE,2,TEXT("Ignoring 1st TICK after long gap")));
                    }
                    else
                    {
                        // running our clock at the old rate, we'd be here right now
                        pFilter->m_LastTickTime = pFilter->m_dwLastMet +
                            (dwTGT - pFilter->m_dwLastTGT) *
                            (60000 / BPM) / pFilter->m_msPerTick;

                        pFilter->m_msPerTick = (DWORD)((LONGLONG)
                            pFilter->m_SamplesSinceTick * 1000 / 11025);

                        pFilter->m_LastTickTGT = dwTGT;

                        DbgLog((LOG_TRACE,2,TEXT("TICK! after %dms, reporting %d tgt=%d"), pFilter->m_msPerTick, pFilter->m_LastTickTime, pFilter->m_LastTickTGT));
                    }

                    pFilter->m_SamplesSinceTick = 0;
                    pFilter->m_csClock.Unlock();
                }

                // we went the whole buffer without seeing a tick.
                pFilter->m_SamplesSinceTick += len;

                if(pFilter->m_fWaveRunning)
                {
                    DbgLog((LOG_TRACE,4,TEXT("Sending the buffer back")));
                    waveInAddBuffer(pFilter->m_hwi, lpwh, sizeof(WAVEHDR));
                }

            }
            break;

        case WIM_OPEN:
        case WIM_CLOSE:
            break;

        default:
            DbgLog((LOG_TRACE,2,TEXT("Unexpected wave callback message %d"), uMsg));
            break;
    }
}


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  dwReason,
                      LPVOID lpReserved)
{
    return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

