//------------------------------------------------------------------------------
//
// Manages the audio session.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//------------------------------------------------------------------------------

#pragma once

#include <Audiopolicy.h>
#include <Mmdeviceapi.h>

class CAudioSessionVolume : public IAudioSessionEvents
{
public:
    // Static method to create an instance of the object.
    static HRESULT CreateInstance( 
        UINT uNotificationMessage, 
        HWND hwndNotification, 
        CAudioSessionVolume **ppAudioSessionVolume 
    );

    // IUnknown methods.
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IAudioSessionEvents methods.

    STDMETHODIMP OnSimpleVolumeChanged( 
        float NewVolume, 
        BOOL NewMute, 
        LPCGUID EventContext 
        );

    STDMETHODIMP OnDisplayNameChanged( 
        LPCWSTR /*NewDisplayName*/,
        LPCGUID /*EventContext*/
        )
    {
        return S_OK;
    }
        
    STDMETHODIMP OnIconPathChanged(
        LPCWSTR /*NewIconPath*/, 
        LPCGUID /*EventContext*/
        )
    {
        return S_OK;
    }
        
    STDMETHODIMP OnChannelVolumeChanged( 
        DWORD /*ChannelCount*/,
        float /*NewChannelVolumeArray*/[],
        DWORD /*ChangedChannel*/,
        LPCGUID /*EventContext*/
        )
    {
        return S_OK;
    }
        
    STDMETHODIMP OnGroupingParamChanged( 
        LPCGUID /*NewGroupingParam*/,
        LPCGUID /*EventContext*/
        )
    {
        return S_OK;
    }
        
    STDMETHODIMP OnStateChanged(
        AudioSessionState /*NewState*/
        )
    {
        return S_OK;
    }
        
    STDMETHODIMP OnSessionDisconnected( 
        AudioSessionDisconnectReason /*DisconnectReason*/
        )
    {
        return S_OK;
    }

    // Other methods
    HRESULT EnableNotifications(BOOL bEnable );
    HRESULT GetVolume(float *pflVolume);
    HRESULT SetVolume(float flVolume);
    HRESULT GetMute(BOOL *pbMute);
    HRESULT SetMute(BOOL bMute);
    HRESULT SetDisplayName(const WCHAR *wszName);

protected:
    CAudioSessionVolume(UINT uNotificationMessage, HWND hwndNotification);
    ~CAudioSessionVolume();

    HRESULT Initialize();

protected:
    LONG m_cRef;                        // Reference count.
    UINT m_uNotificationMessage;        // Window message to send when an audio event occurs.
    HWND m_hwndNotification;            // Window to receives messages.
    BOOL m_bNotificationsEnabled;       // Are audio notifications enabled?

    IAudioSessionControl    *m_pAudioSession;
    ISimpleAudioVolume      *m_pSimpleAudioVolume;
};