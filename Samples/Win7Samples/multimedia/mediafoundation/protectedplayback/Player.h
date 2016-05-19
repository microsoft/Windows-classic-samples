//////////////////////////////////////////////////////////////////////////
//
// player.h : Playback helper class.
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

const UINT WM_APP_PLAYER_EVENT = WM_APP + 1;    // wparam = IMFMediaEvent*

enum PlayerState
{
    Closed = 0,     // No session.
    Ready,          // Session was created, ready to open a file. 
    OpenPending,    // Session is opening a file.
    Started,        // Session is playing a file.
    Paused,         // Session is paused.
    Stopped,        // Session is stopped (ready to play). 
    Closing         // Application has closed the session, but is waiting for MESessionClosed.
};

class CPlayer : public IMFAsyncCallback
{
public:
    static HRESULT CreateInstance(HWND hVideo, HWND hEvent, CPlayer **ppPlayer);

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFAsyncCallback methods
    STDMETHODIMP GetParameters(DWORD*, DWORD*)
    {
        // Implementation of this method is optional.
        return E_NOTIMPL;
    }

    STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult);

    // Playback
    HRESULT     OpenURL(const WCHAR *sURL);
    HRESULT     Play();
    HRESULT     Pause();
    HRESULT     Shutdown();
    HRESULT     HandleEvent(UINT_PTR pUnkPtr);
    PlayerState GetState() const { return m_state; }

    // Video functionality
    HRESULT     Repaint();
    HRESULT     ResizeVideo(WORD width, WORD height);
    BOOL        HasVideo() const { return (m_pVideoDisplay != NULL); }

    // Content protection manager
    HRESULT     GetContentProtectionManager(ContentProtectionManager **ppManager);


protected:
    
    // Constructor is private. Use static CreateInstance method to instantiate.
    CPlayer(HWND hVideo, HWND hEvent);

    // Destructor is private. Caller should call Release.
    virtual ~CPlayer();

    HRESULT Initialize();
    HRESULT CreateSession();
    HRESULT CloseSession();
    HRESULT StartPlayback();
    HRESULT CreateMediaSource(const WCHAR *sURL);
    HRESULT CreateTopologyFromSource(IMFTopology **ppTopology);

    HRESULT AddBranchToPartialTopology(
        IMFTopology *pTopology, 
        IMFPresentationDescriptor *pSourcePD, 
        DWORD iStream
        );


    // Media event handlers
    HRESULT OnTopologyReady(IMFMediaEvent *pEvent);
    HRESULT OnSessionStarted(IMFMediaEvent *pEvent);
    HRESULT OnSessionPaused(IMFMediaEvent *pEvent);
    HRESULT OnSessionClosed(IMFMediaEvent *pEvent);
    HRESULT OnPresentationEnded(IMFMediaEvent *pEvent);

    long                    m_nRefCount;        // Reference count.

    IMFMediaSession         *m_pSession;
    IMFMediaSource          *m_pSource;
    IMFVideoDisplayControl  *m_pVideoDisplay;

    HWND                    m_hwndVideo;        // Video window.
    HWND                    m_hwndEvent;        // App window to receive events.
    PlayerState             m_state;            // Current state of the media session.
    HANDLE                  m_hCloseEvent;      // Event to wait on while closing

    ContentProtectionManager    *m_pContentProtectionManager;

};