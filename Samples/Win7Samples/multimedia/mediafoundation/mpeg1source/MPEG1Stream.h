//////////////////////////////////////////////////////////////////////////
//
// MPEG1Stream.h
// Implements the stream object (IMFMediaStream) for the MPEG-1 source.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

class MPEG1Source;

// The media stream object.
class MPEG1Stream : BaseObject, RefCountedObject, public IMFMediaStream 
{
public:

    MPEG1Stream(MPEG1Source *pSource, IMFStreamDescriptor *pSD, HRESULT& hr);
    ~MPEG1Stream();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFMediaEventGenerator
    STDMETHODIMP BeginGetEvent(IMFAsyncCallback* pCallback,IUnknown* punkState);
    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent);
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent);
    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue);

    // IMFMediaStream
    STDMETHODIMP GetMediaSource(IMFMediaSource** ppMediaSource);
    STDMETHODIMP GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor);
    STDMETHODIMP RequestSample(IUnknown* pToken);

    // Other methods (called by source)
    HRESULT     Activate(BOOL bActive);
    HRESULT     Start(const PROPVARIANT& varStart);
    HRESULT     Pause();
    HRESULT     Stop();
    HRESULT     EndOfStream();
    HRESULT     Shutdown();

    BOOL        IsActive() const { return m_bActive; }
    BOOL        NeedsData();

    HRESULT     DeliverPayload(IMFSample *pSample);

    // Callbacks
    HRESULT     OnDispatchSamples(IMFAsyncResult *pResult);

private:

    // SourceLock class:
    // Small helper class to lock and unlock the source. 
    // It works like the AutoLock class in Common\critsec.h.
    class SourceLock
    {
    private:
        MPEG1Source *m_pSource;
    public:
        SourceLock(MPEG1Source *pSource);
        ~SourceLock();
    };

private:

    HRESULT CheckShutdown() const 
    {
        return ( m_state == STATE_SHUTDOWN ? MF_E_SHUTDOWN : S_OK );
    }
    HRESULT DispatchSamples();


private:
    MPEG1Source         *m_pSource;             // Parent media source
    IMFStreamDescriptor *m_pStreamDescriptor;
    IMFMediaEventQueue  *m_pEventQueue;         // Event generator helper

    SourceState         m_state;                // Current state (running, stopped, paused)
    BOOL                m_bActive;              // Is the stream active?
    BOOL                m_bEOS;                 // Did the source reach the end of the stream?

    SampleList          m_Samples;              // Samples waiting to be delivered.
    TokenList           m_Requests;             // Sample requests, waiting to be dispatched.
};


