// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
#include "StdAfx.h"
#include "MediaPlayer.h"
#include <mmdeviceapi.h>


//
//  Constuctor for the "Media Player" application.
//
//  Initialize a bunch of stuff.
//
CMediaPlayer::CMediaPlayer(HWND AppWindow) :
        _refCount(1),
        _FileName(NULL),
        _GraphBuilder(NULL),
        _MediaSeeking(NULL),
        _MediaPlayerTime(0),
        _MediaEvent(NULL),
        _AppWindow(AppWindow),
        _DuckingRegistered(FALSE),
        _SimpleVolume(NULL),
        _SessionNotificationRegistered(FALSE),
        _SessionControl2(NULL),
        _SessionManager2(NULL)
{
    UuidCreate(&_MediaPlayerEventContext);
}

//
//  Initialize the media player.  Instantiates DShow, retrieves the session control for 
//  the current audio session and registers for notifications on that session control.
//
HRESULT CMediaPlayer::Initialize()
{
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_GraphBuilder));
    if (SUCCEEDED(hr))
    {
        hr = _GraphBuilder->QueryInterface(IID_PPV_ARGS(&_MediaEvent));
        if (SUCCEEDED(hr))
        {
            hr = _MediaEvent->SetNotifyWindow(reinterpret_cast<OAHWND>(_AppWindow), WM_APP_GRAPHNOTIFY, 0);
        }
        if (SUCCEEDED(hr))
        {
            hr = _GraphBuilder->QueryInterface(IID_PPV_ARGS(&_MediaSeeking));
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = GetSessionControl2();
        if (SUCCEEDED(hr))
        {
            hr = _SessionControl2->RegisterAudioSessionNotification(this);
            if (SUCCEEDED(hr))
            {
                _SessionNotificationRegistered = true;
            }
        }
    }
    return hr;
}

//
//  Shut down the media player - releases all the resources associated with the media player.
//
void CMediaPlayer::Shutdown()
{
    SafeRelease(&_MediaEvent);
    SafeRelease(&_MediaSeeking);
    SafeRelease(&_GraphBuilder);

    if (_SessionManager2)
    {
        if (_DuckingRegistered)
        {
            HRESULT hr = _SessionManager2->UnregisterDuckNotification(this);
            if (FAILED(hr))
            {
				// Failures here are highly unlikely and could indicate an application defect
                MessageBox(_AppWindow, L"Unable to unregister for ducking notifications", L"Stop Player Error", MB_OK);
            }
            _DuckingRegistered = FALSE;
        }
        _SessionManager2->Release();
        _SessionManager2 = NULL;
    }

    SafeRelease(&_SimpleVolume);

    if (_SessionControl2)
    {
        if (_SessionNotificationRegistered)
        {
            HRESULT hr = _SessionControl2->UnregisterAudioSessionNotification(this);
            if (FAILED(hr))
            {
				// Failures here are highly unlikely and could indicate an application defect
                MessageBox(_AppWindow, L"Unable to unregister for session notifications", L"Stop PlayerError", MB_OK);
            }
            _SessionNotificationRegistered = FALSE;
        }
        _SessionControl2->Release();
        _SessionControl2 = NULL;
    }

    free(_FileName);
    _FileName = NULL;
}

//
//  Destructor for the "Media Player". 
//
//  Does nothing because all the cleanup happened in the Shutdown() method.
//
CMediaPlayer::~CMediaPlayer()
{
}

//
// IUnknown implmentation
//
IFACEMETHODIMP CMediaPlayer::QueryInterface(REFIID iid, void **pvObject)
{
    if (pvObject == NULL)
    {
        return E_POINTER;
    }
    *pvObject = NULL;
    if (iid == IID_IUnknown)
    {
        *pvObject = static_cast<IUnknown *>(static_cast<IAudioVolumeDuckNotification *>(this));
        AddRef();
    }
    else if (iid == __uuidof(IAudioVolumeDuckNotification))
    {
        *pvObject = static_cast<IAudioVolumeDuckNotification *>(this);
        AddRef();
    }
    else if (iid == __uuidof(IAudioSessionEvents))
    {
        *pvObject = static_cast<IAudioSessionEvents *>(this);
        AddRef();
    }
    else
    {
        return E_NOINTERFACE;
    }
    return S_OK;
}

IFACEMETHODIMP_(ULONG) CMediaPlayer::AddRef()
{
    return InterlockedIncrement(&_refCount);
}

IFACEMETHODIMP_(ULONG) CMediaPlayer::Release()
{
    if (InterlockedDecrement(&_refCount) == 0)
    {
        delete this;
    }
    return _refCount;
}

//
//  Removes any filters in the audio graph - called before rebuilding the audio graph.
//
void CMediaPlayer::RemoveAllFilters()
{
    IEnumFilters *enumFilters;
    HRESULT hr = _GraphBuilder->EnumFilters(&enumFilters);

    if (SUCCEEDED(hr))
    {
        IBaseFilter *filter = NULL;
        hr = enumFilters->Next(1, &filter, NULL);
        while (hr == S_OK)
        {
            //
            //  Remove the filter from the graph.
            //
            _GraphBuilder->RemoveFilter(filter);
            filter->Release();

            //
            //  Reset the enumeration since we removed the filter (which invalidates the enumeration).
            //
            enumFilters->Reset();

            hr = enumFilters->Next(1, &filter, NULL);
        }
        enumFilters->Release();
    }
}

//
//  Sets the file we're going to play.
//
//  Returns true if the file can be played, false otherwise.
//
bool CMediaPlayer::SetFileName(LPCWSTR FileName)
{
    RemoveAllFilters();
    if (_FileName != NULL)
    {
        free(_FileName);
    }

    _FileName = _wcsdup(FileName);
    if (_FileName == NULL)
    {
        return false;
    }

    //
    //  Ask DirectShow to build a render graph for this file.
    //
    HRESULT hr = _GraphBuilder->RenderFile(_FileName, NULL);
    if (FAILED(hr))
    {
        MessageBox(_AppWindow, L"Unable to build graph for media file", L"Set Filename Error", MB_OK);
        return false;
    }


    //
    //  If we can figure out the length of this track retrieve it.
    //
    DWORD caps = AM_SEEKING_CanGetDuration;
    bool canSeek = (S_OK == _MediaSeeking->CheckCapabilities(&caps));
    if (canSeek)
    {
        _MediaSeeking->GetDuration(&this->_MediaPlayerTime);
    }
    return true;
}

//
//  Starts the media player.
//
void CMediaPlayer::Play()
{
    IMediaControl *mediaControl;
    _GraphBuilder->QueryInterface(IID_PPV_ARGS(&mediaControl));

    if (mediaControl != NULL)
    {
        REFERENCE_TIME timeBegin = 0;
        _MediaSeeking->SetPositions(&timeBegin, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);

        mediaControl->Run();
        mediaControl->Release();
    }
}
//
//  Pauses media playback if it is currently running.
//
bool CMediaPlayer::Pause()
{
    bool isPaused = false;
    IMediaControl *mediaControl;

    HRESULT hr = _GraphBuilder->QueryInterface(IID_PPV_ARGS(&mediaControl));
    if (SUCCEEDED(hr))
    {
        mediaControl->Pause();
        mediaControl->Release();
        isPaused = true;
    }
    return isPaused;
}

//
//  Continues media playback if it is currently paused.
//
bool CMediaPlayer::Continue()
{
    bool isContinued = false;
    IMediaControl *mediaControl;

    HRESULT hr = _GraphBuilder->QueryInterface(IID_PPV_ARGS(&mediaControl));
    if (SUCCEEDED(hr))
    {
        mediaControl->Run();
        mediaControl->Release();
        isContinued = true;
    }
    return isContinued;
}

//
//  Toggle the pause state for the media player.  Returns true if the media player pauses, false if it runs.
//
bool CMediaPlayer::TogglePauseState()
{
    bool isPaused = false;
    IMediaControl *mediaControl;

    HRESULT hr = _GraphBuilder->QueryInterface(IID_PPV_ARGS(&mediaControl));
    if (SUCCEEDED(hr))
    {
        OAFilterState filterState;

        hr = mediaControl->GetState(INFINITE, &filterState);
        if (SUCCEEDED(hr))
        {
            if (filterState == State_Running)
            {
                mediaControl->Pause();
                isPaused = true;
            }
            else if (filterState == State_Paused)
            {
                mediaControl->Run();
                isPaused = false;
            }
        }
        mediaControl->Release();
    }
    return isPaused;
}

//
//  Stops media playback.
//
void CMediaPlayer::Stop()
{
    IMediaControl *mediaControl;

    HRESULT hr = _GraphBuilder->QueryInterface(IID_PPV_ARGS(&mediaControl));
    if (SUCCEEDED(hr))
    {
        mediaControl->Stop();
        mediaControl->Release();
    }
}

//
//  Handles DirectShow graph events.
//
//  Returns true if the player should be stopped.
//
bool CMediaPlayer::HandleGraphEvent()
{
    bool stopped = false;
    // Disregard if we don't have an IMediaEventEx pointer.
    if (_MediaEvent== NULL)
    {
        return stopped;
    }

    // Get all the events
    long evCode;
    LONG_PTR param1, param2;
    while (SUCCEEDED(_MediaEvent->GetEvent(&evCode, &param1, &param2, 0)))
    {
        _MediaEvent->FreeEventParams(evCode, param1, param2);
        switch (evCode)
        {
        case EC_COMPLETE:
        {
            // Stop playback, we're done.
            {
                IMediaControl *mediaControl;
                _GraphBuilder->QueryInterface(IID_PPV_ARGS(&mediaControl));

                mediaControl->Stop();
                mediaControl->Release();
            }

            REFERENCE_TIME timeBegin = 0;
            _MediaSeeking->SetPositions(&timeBegin, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
            stopped = true;
        }
        break;
        case EC_USERABORT: // Fall through.
        case EC_ERRORABORT:
            stopped = false;
        }
    }
    return stopped;
}

//
//  Returns the position in the song being played in units 0..1000
//
long CMediaPlayer::GetPosition()
{
    if (_MediaSeeking && _MediaPlayerTime != 0)
    {
        REFERENCE_TIME position;

        if (SUCCEEDED(_MediaSeeking->GetCurrentPosition(&position)))
        {
            long sliderTick = (long)((position * 1000) / _MediaPlayerTime);
            return sliderTick;
        }
    }
    return 0;
}

//
//  Sync's the "Pause On Duck" state for the media player.
//
//  Either registers or unregisters for ducking notification.
//
void CMediaPlayer::SyncPauseOnDuck(bool PauseOnDuckChecked)
{
    LPWSTR sessionId = NULL;

    HRESULT hr = GetSessionManager2();

    //
    //  Retrieve the current session ID.  We'll use that to request that the ducking manager
    //  filter our notifications (so we only see ducking notifications for our session).
    //
    if (SUCCEEDED(hr))
    {
        hr = GetCurrentSessionId(&sessionId);

        //
        //  And either register or unregister for ducking notifications based on whether or not the Pause On Duck state is checked.
        //
        if (SUCCEEDED(hr))
        {
            if (PauseOnDuckChecked)
            {
                if (!_DuckingRegistered)
                {
                    hr = _SessionManager2->RegisterDuckNotification(sessionId, this);
                    if (SUCCEEDED(hr))
                    {
                        _DuckingRegistered = TRUE;
                    }
                }
            }
            else
            {
                if (_DuckingRegistered)
                {
                    hr = _SessionManager2->UnregisterDuckNotification(this);
                    if (SUCCEEDED(hr))
                    {
                        _DuckingRegistered = FALSE;
                    }
                }
            }

            if (FAILED(hr))
            {
                MessageBox(_AppWindow, L"Unable to register or unregister for ducking notifications", L"Sync Ducking Pause Error", MB_OK);
            }

            CoTaskMemFree(sessionId);
        }
    }
}

//
//  When we receive a duck notification, post a "Session Ducked" message to the application window.
//
STDMETHODIMP CMediaPlayer::OnVolumeDuckNotification(LPCWSTR SessionID, UINT32 CountCommunicationsSessions)
{
    PostMessage(_AppWindow, WM_APP_SESSION_DUCKED, 0, 0);
    return 0;
}

//
//  When we receive an unduck notification, post a "Session Unducked" message to the application window.
//
STDMETHODIMP CMediaPlayer::OnVolumeUnduckNotification(LPCWSTR SessionID)
{
    PostMessage(_AppWindow, WM_APP_SESSION_UNDUCKED, 0, 0);
    return 0;
}

//
//  Sync the "Ducking Opt Out" state with the UI - either enable or disable ducking for this session.
//
void CMediaPlayer::SyncDuckingOptOut(bool DuckingOptOutChecked)
{
    HRESULT hr = GetSessionControl2();

    //
    //  Sync our ducking state to the UI.
    //
    if (SUCCEEDED(hr))
    {
        if (DuckingOptOutChecked)
        {
            hr = _SessionControl2->SetDuckingPreference(TRUE);
        }
        else
        {
            hr = _SessionControl2->SetDuckingPreference(FALSE);
        }
        if (FAILED(hr))
        {
            MessageBox(_AppWindow, L"Unable to update the ducking preference", L"Sync Ducking State Error", MB_OK);
        }
    }
}


//
//  Get the volume on the current audio session.
//
float CMediaPlayer::GetVolume()
{
    HRESULT hr = GetSimpleVolume();
    if (SUCCEEDED(hr))
    {
        float volume;
        hr = _SimpleVolume->GetMasterVolume(&volume);
        if (SUCCEEDED(hr))
        {
            return volume;
        }
        else
        {
            MessageBox(_AppWindow, L"Unable to retrieve volume for current session", L"Get Volume Error", MB_OK);
        }
    }
    else
    {
        MessageBox(_AppWindow, L"Unable to retrieve simple volume control for current session", L"Get Volume Error", MB_OK);
    }
    return 0.0f;
}

//
//  Set the volume on the current audio session.
//
//  We set a specific event context on the SetMasterVolume call - when we receive the simple volume changed
//  notification we can use this event context to determine if the volume change call came from our application or another 
//  application.
//
void CMediaPlayer::SetVolume(float Volume)
{
    HRESULT hr = GetSimpleVolume();
    if (SUCCEEDED(hr))
    {
        hr = _SimpleVolume->SetMasterVolume(Volume, &_MediaPlayerEventContext); 
        if (FAILED(hr))
        {
            MessageBox(_AppWindow, L"Unable to retrieve volume for current session", L"Set Volume Error", MB_OK);
        }
    }
    else
    {
        MessageBox(_AppWindow, L"Unable to retrieve simple volume control for current session", L"Set Volume Error", MB_OK);
    }
}
//
//  Get the mute state for the current audio session.
//
bool CMediaPlayer::GetMute()
{
    HRESULT hr = GetSimpleVolume();
    if (SUCCEEDED(hr))
    {
        BOOL mute;
        hr = _SimpleVolume->GetMute(&mute);
        if (SUCCEEDED(hr))
        {
            return (mute != FALSE);
        }
        else
        {
            MessageBox(_AppWindow, L"Unable to retrieve mute for current session", L"Get Mute Error", MB_OK);
        }
    }
    else
    {
        MessageBox(_AppWindow, L"Unable to retrieve simple volume control for current session", L"Get Mute Error", MB_OK);
    }
    return false;
}

//
//  Set the mute state on the current audio session.
//
//  We set a specific event context on the SetMasterVolume call - when we receive the simple volume changed
//  notification we can use this event context to determine if the volume change call came from our application or another 
//  application.
//
void CMediaPlayer::SetMute(bool Mute)
{
    HRESULT hr = GetSimpleVolume();
    if (SUCCEEDED(hr))
    {
        hr = _SimpleVolume->SetMute(Mute, &_MediaPlayerEventContext); 
        if (FAILED(hr))
        {
            MessageBox(_AppWindow, L"Unable to set mute for current session", L"Set Mute Error", MB_OK);
        }
    }
    else
    {
        MessageBox(_AppWindow, L"Unable to retrieve simple volume control for current session", L"Set Mute Error", MB_OK);
    }
}

//
//  Someone changed the volume on the media player session. 
//
//  If the person making the change wasn't the current player, let our UI know that the volume changed (and what the new volume is)
//  so it can update the UI to reflect the new state.
//
STDMETHODIMP CMediaPlayer::OnSimpleVolumeChanged(float NewSimpleVolume, BOOL NewMute, LPCGUID EventContext)
{
    if (EventContext && *EventContext != _MediaPlayerEventContext)
    {
        PostMessage(_AppWindow, WM_APP_SESSION_VOLUME_CHANGED, NewMute, FLOAT2LPARAM(NewSimpleVolume));
    }
    return S_OK;
}

//
//  Utility function to retrieve the session manager for the default audio endpoint.
//
HRESULT CMediaPlayer::GetSessionManager2()
{
    HRESULT hr = S_OK;
    if (_SessionManager2 == NULL)
    {
        IMMDeviceEnumerator *deviceEnumerator;
        IMMDevice *endpoint = NULL;

        //
        //  Start with the default endpoint.
        //
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
        if (SUCCEEDED(hr))
        {
            hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &endpoint);

            deviceEnumerator->Release();
            deviceEnumerator = NULL;
        }
        else
        {
            MessageBox(_AppWindow, L"Unable to instantiate MMDeviceEnumerator", L"Get SessionManager Error", MB_OK);
        }

        if (SUCCEEDED(hr))
        {
            hr = endpoint->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER,
                    NULL, reinterpret_cast<void **>(&_SessionManager2));
            endpoint->Release();
            if (FAILED(hr))
            {
                MessageBox(_AppWindow, L"Unable to Activate session manager", L"Get SessionManager Error", MB_OK);
            }
        }
        else
        {
            MessageBox(_AppWindow, L"Unable to get default endpoint", L"Get SessionManager Error", MB_OK);
        }
    }
    return hr;
}
//
//  Utility function to retrieve the session control interface for the current audio session.
//
//  We assume that DirectShow uses the NULL session GUID and doesn't specify any session specific flags.
//
HRESULT CMediaPlayer::GetSessionControl2()
{
    HRESULT hr = S_OK;
    if (_SessionControl2 == NULL)
    {
        hr = GetSessionManager2();
        if (SUCCEEDED(hr))
        {
            IAudioSessionControl * sessionControl;
            hr = _SessionManager2->GetAudioSessionControl(NULL, 0, &sessionControl);
            if (SUCCEEDED(hr))
            {
                hr = sessionControl->QueryInterface(IID_PPV_ARGS(&_SessionControl2));
                if (FAILED(hr))
                {
                    MessageBox(_AppWindow, L"Unable to QI for SessionControl2", L"Get SessionControl Error", MB_OK);
                }

                sessionControl->Release();
                sessionControl = NULL;
            }
            else
            {
                MessageBox(_AppWindow, L"Unable to get Session Control", L"Get SessionControl Error", MB_OK);
            }
        }
    }
    return hr;
}

//
//  Utility function to retrieve the simple volume control interface for the current audio session.
//
//  We assume that DirectShow uses the NULL session GUID and doesn't specify any session specific flags.
//
HRESULT CMediaPlayer::GetSimpleVolume()
{
    HRESULT hr = S_OK;
    if (_SimpleVolume == NULL)
    {
        hr = GetSessionManager2();
        if (SUCCEEDED(hr))
        {
            hr = _SessionManager2->GetSimpleAudioVolume(NULL, 0, &_SimpleVolume);
            if (FAILED(hr))
            {
                MessageBox(_AppWindow, L"Unable to get Simple Volume", L"Get Simple Volume Error", MB_OK);
            }
        }
    }
    return hr;
}

//
//  Utility function to retrieve the Session ID for the current audio session.  
//
HRESULT CMediaPlayer::GetCurrentSessionId(LPWSTR *SessionId)
{
    HRESULT hr = GetSessionControl2();
    if (SUCCEEDED(hr))
    {
        hr = _SessionControl2->GetSessionInstanceIdentifier(SessionId);
        if (FAILED(hr))
        {
            MessageBox(_AppWindow, L"Unable to get the session instance ID", L"Get session instance ID Error", MB_OK);
        }
    }
    return hr;
}

