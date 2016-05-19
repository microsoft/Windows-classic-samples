// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once 

class CVolumeMonitor : IMMNotificationClient, IAudioEndpointVolumeCallback
{
private:
    BOOL                            m_bRegisteredForEndpointNotifications;
    BOOL                            m_bRegisteredForVolumeNotifications;
    CComPtr<IMMDeviceEnumerator>    m_spEnumerator;
    CComPtr<IMMDevice>              m_spAudioEndpoint;
    CComPtr<IAudioEndpointVolume>   m_spVolumeControl;
    CCriticalSection                m_csEndpoint;

    long                            m_cRef;

    ~CVolumeMonitor();       // refcounted object... make the destructor private
    HRESULT AttachToDefaultEndpoint();
    void    DetachFromEndpoint();


    // IMMNotificationClient (only need to really implement OnDefaultDeviceChanged)
    IFACEMETHODIMP OnDeviceStateChanged(LPCWSTR /*pwstrDeviceId*/, DWORD /*dwNewState*/)    {   return S_OK;    }
    IFACEMETHODIMP OnDeviceAdded(LPCWSTR /*pwstrDeviceId*/)   {   return S_OK;    }
    IFACEMETHODIMP OnDeviceRemoved(LPCWSTR /*pwstrDeviceId*/) {   return S_OK;    }
    IFACEMETHODIMP OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId);   // ****
    IFACEMETHODIMP OnPropertyValueChanged(LPCWSTR /*pwstrDeviceId*/, const PROPERTYKEY /*key*/)   {   return S_OK;    }
    IFACEMETHODIMP OnDeviceQueryRemove()   {   return S_OK;    }
    IFACEMETHODIMP OnDeviceQueryRemoveFailed() {   return S_OK;    }
    IFACEMETHODIMP OnDeviceRemovePending() {   return S_OK;    }

    // IAudioEndpointVolumeCallback
    IFACEMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);

    // IUnknown
    IFACEMETHODIMP QueryInterface(const IID& iid, void** ppUnk);

public:
    CVolumeMonitor();

    HRESULT Initialize();
    void    Dispose();
    HRESULT GetLevelInfo(VOLUME_INFO* pInfo);
    void    ChangeEndpoint();


    // IUnknown
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
};
