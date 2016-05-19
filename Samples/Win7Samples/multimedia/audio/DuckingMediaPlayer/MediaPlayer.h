#pragma once
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <dshow.h>
#include <audiopolicy.h>

class CMediaPlayer : public IAudioVolumeDuckNotification, public IAudioSessionEvents
{
    LONG _refCount;
    LPWSTR _FileName;
    IGraphBuilder *_GraphBuilder;
    IMediaSeeking *_MediaSeeking;
    IMediaEventEx *_MediaEvent;
    bool _DuckingRegistered;
    bool _SessionNotificationRegistered;
    HWND _AppWindow;

    //
    //  Event context used for media player volume changes.  This allows us to determine if a volume change request was initiated by
    //  this media player application or some other application.
    //
    GUID _MediaPlayerEventContext;
    REFERENCE_TIME _MediaPlayerTime;
    IAudioSessionManager2 *_SessionManager2;
    ISimpleAudioVolume *_SimpleVolume;
    IAudioSessionControl2 *_SessionControl2;

    ~CMediaPlayer();

	// IAudioVolumeDuckNotification
    STDMETHOD(OnVolumeDuckNotification) (LPCWSTR sessionID, UINT32 countCommunicationSessions);
    STDMETHOD(OnVolumeUnduckNotification) (LPCWSTR sessionID);

	// IAudioSessionEvents
    STDMETHOD(OnDisplayNameChanged) (LPCWSTR NewDisplayName, LPCGUID EventContext) { return S_OK; };
    STDMETHOD(OnIconPathChanged) (LPCWSTR NewIconPath, LPCGUID EventContext) { return S_OK; };
    STDMETHOD(OnSimpleVolumeChanged) (float NewSimpleVolume, BOOL NewMute, LPCGUID EventContext);
    STDMETHOD(OnChannelVolumeChanged) (DWORD ChannelCount, float NewChannelVolumesVolume[], DWORD ChangedChannel, LPCGUID EventContext) { return S_OK; };
    STDMETHOD(OnGroupingParamChanged) (LPCGUID NewGroupingParam, LPCGUID EventContext) { return S_OK; };
    STDMETHOD(OnStateChanged) (AudioSessionState NewState) { return S_OK; };
    STDMETHOD(OnSessionDisconnected) (AudioSessionDisconnectReason DisconnectReason) { return S_OK; };

	// Other
    HRESULT GetSessionManager2();
    HRESULT GetSessionControl2();
    HRESULT GetSimpleVolume();
    HRESULT GetCurrentSessionId(LPWSTR *SessionId);

public:
    CMediaPlayer(HWND hWnd);
    HRESULT Initialize();   // Initialize the media player.
    void Shutdown();        // Shuts down the media player if it's currently active.

    void SyncPauseOnDuck(bool PauseOnDuckChecked);
    void SyncDuckingOptOut(bool PauseOnDuckChecked);
    void Play();
    void Stop();
    bool Pause();
    void SetVolume(float Volume);
    float GetVolume();
    void SetMute(bool Mute);
    bool GetMute();
    bool Continue();
    bool TogglePauseState(); // Toggles the "Pause" state for the player.
    bool HandleGraphEvent();
    void RemoveAllFilters();
    bool SetFileName(LPCWSTR FileName);
    long GetPosition(); // Returns the current position in the file.
    void OnSessionDucked();
    void OnSessionUnducked();

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
};
