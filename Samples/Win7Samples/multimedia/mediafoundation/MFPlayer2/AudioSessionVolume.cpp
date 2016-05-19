//------------------------------------------------------------------------------
//
// Manages the audio session.
//
// The CAudioSessionVolume class performs two functions:
//
// - Enables the application to set the volume on the audio session that
//   MFPlay uses for audio playback.
//
// - Notifies the application when the session volume is changed externally
//   (eg through SndVol application)
//
// This class uses two WASAPI interfaces:
//
// - IAudioSessionControl
// - ISimpleAudioVolume
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//------------------------------------------------------------------------------

#include "MFPlayer.h"
#include "AudioSessionVolume.h"

// {2715279F-4139-4ba0-9CB1-B351F1B58A4A}
static const GUID AudioSessionVolumeCtx = 
{ 0x2715279f, 0x4139, 0x4ba0, { 0x9c, 0xb1, 0xb3, 0x51, 0xf1, 0xb5, 0x8a, 0x4a } };


//-------------------------------------------------------------------
//  Constructor
//-------------------------------------------------------------------

CAudioSessionVolume::CAudioSessionVolume(
    UINT uNotificationMessage, 
    HWND hwndNotification
    )
    : m_cRef(1),
      m_uNotificationMessage(uNotificationMessage),
      m_hwndNotification(hwndNotification),
      m_bNotificationsEnabled(FALSE),
      m_pAudioSession(NULL),
      m_pSimpleAudioVolume(NULL)
{
}

//-------------------------------------------------------------------
//  Destructor
//-------------------------------------------------------------------

CAudioSessionVolume::~CAudioSessionVolume()
{
    EnableNotifications(FALSE);

    SafeRelease(&m_pAudioSession);
    SafeRelease(&m_pSimpleAudioVolume);
};


//-------------------------------------------------------------------
//  CreateInstance
//
//  Creates an instance of the CAudioSessionVolume object.
//-------------------------------------------------------------------

/* static */ 
HRESULT CAudioSessionVolume::CreateInstance( 
    UINT uNotificationMessage, 
    HWND hwndNotification, 
    CAudioSessionVolume **ppAudioSessionVolume 
    )
{
    HRESULT hr = S_OK;
   
    CAudioSessionVolume *pAudioSessionVolume = NULL;

    pAudioSessionVolume = new (std::nothrow) CAudioSessionVolume(
        uNotificationMessage, hwndNotification);

    if (pAudioSessionVolume == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = pAudioSessionVolume->Initialize();
    if (FAILED(hr)) { goto done; }

    *ppAudioSessionVolume = pAudioSessionVolume;
    (*ppAudioSessionVolume)->AddRef();

done:
    SafeRelease(&pAudioSessionVolume);
    return hr;
}


//-------------------------------------------------------------------
//  Initialize
//
//  Initializes the CAudioSessionVolume object.
//-------------------------------------------------------------------

HRESULT CAudioSessionVolume::Initialize()
{
    HRESULT hr = S_OK;

    IMMDeviceEnumerator *pDeviceEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioSessionManager *pAudioSessionManager = NULL;

    // Get the enumerator for the audio endpoint devices.
    hr = CoCreateInstance( 
        __uuidof(MMDeviceEnumerator), 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_PPV_ARGS(&pDeviceEnumerator)
        );

    if (FAILED(hr)) { goto done; }

    // Get the default audio endpoint that the SAR will use.
    hr = pDeviceEnumerator->GetDefaultAudioEndpoint( 
        eRender, 
        eConsole,   // The SAR uses 'eConsole' by default.
        &pDevice
        );
    if (FAILED(hr)) { goto done; }

    // Get the session manager for this device.
    hr = pDevice->Activate( 
        __uuidof(IAudioSessionManager), 
        CLSCTX_INPROC_SERVER, 
        NULL, 
        (void**) &pAudioSessionManager 
        );
    if (FAILED(hr)) { goto done; }

    // Get the audio session. 
    hr = pAudioSessionManager->GetAudioSessionControl( 
        &GUID_NULL,     // Get the default audio session. 
        FALSE,          // The session is not cross-process.
        &m_pAudioSession 
        );
    if (FAILED(hr)) { goto done; }

    hr = pAudioSessionManager->GetSimpleAudioVolume( 
        &GUID_NULL, 0, &m_pSimpleAudioVolume 
        );

done:
    SafeRelease(&pDeviceEnumerator);
    SafeRelease(&pDevice);
    SafeRelease(&pAudioSessionManager);
    return hr;
}

/* IUnknown methods */

STDMETHODIMP CAudioSessionVolume::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CAudioSessionVolume, IAudioSessionEvents),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}
    
STDMETHODIMP_(ULONG) CAudioSessionVolume::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CAudioSessionVolume::Release()
{
    LONG c = InterlockedDecrement( &m_cRef );
    if (c == 0)
    {
        delete this;
    }
    return c;
}

//-------------------------------------------------------------------
//  EnableNotifications
//
//  Enables or disables notifications from the audio session. For
//  example, if the user mutes the audio through the system volume-
//  control program (Sndvol), the application will be notified.
// 
//-------------------------------------------------------------------

HRESULT CAudioSessionVolume::EnableNotifications(BOOL bEnable)
{
    HRESULT hr = S_OK;

    if (m_hwndNotification == NULL || m_pAudioSession == NULL)
    {
        return E_FAIL;
    }

    if (m_bNotificationsEnabled == bEnable)
    {
        // No change.
        return S_OK;
    }

    if (bEnable)
    {
        hr = m_pAudioSession->RegisterAudioSessionNotification(this);
    }
    else
    {
        hr = m_pAudioSession->UnregisterAudioSessionNotification(this);
    }

    if (SUCCEEDED(hr))
    {
        m_bNotificationsEnabled = bEnable;
    }

    return hr;
}


//-------------------------------------------------------------------
//  GetVolume
//
//  Gets the session volume level.
// 
//-------------------------------------------------------------------

HRESULT CAudioSessionVolume::GetVolume(float *pflVolume)
{
    HRESULT hr = S_OK;

    if ( m_pSimpleAudioVolume == NULL)
    {
        hr = E_FAIL;
    }
    else
    {
        hr = m_pSimpleAudioVolume->GetMasterVolume(pflVolume);
    }
    return hr;
}


//-------------------------------------------------------------------
//  SetVolume
//
//  Sets the session volume level.
// 
//  flVolume: Ranges from 0 (silent) to 1 (full volume)
//-------------------------------------------------------------------

HRESULT CAudioSessionVolume::SetVolume(float flVolume)
{
    HRESULT hr = S_OK;

    if (m_pSimpleAudioVolume == NULL)
    {
        hr = E_FAIL;
    }
    else
    {
        hr = m_pSimpleAudioVolume->SetMasterVolume( 
            flVolume, 
            &AudioSessionVolumeCtx  // Event context.
            );
    }
    return hr;
}


//-------------------------------------------------------------------
//  GetMute
//
//  Gets the muting state of the session.
//
//-------------------------------------------------------------------

HRESULT CAudioSessionVolume::GetMute(BOOL *pbMute)
{
    HRESULT hr = S_OK;

    if (m_pSimpleAudioVolume == NULL)
    {
        hr = E_FAIL;
    }
    else
    {
        hr = m_pSimpleAudioVolume->GetMute(pbMute);
    }
    return hr;
}

//-------------------------------------------------------------------
//  SetMute
//
//  Mutes or unmutes the session audio.
//
//-------------------------------------------------------------------

HRESULT CAudioSessionVolume::SetMute(BOOL bMute)
{
    HRESULT hr = S_OK;

    if (m_pSimpleAudioVolume == NULL)
    {
        hr = E_FAIL;
    }
    else
    {
        hr = m_pSimpleAudioVolume->SetMute( 
            bMute,
            &AudioSessionVolumeCtx  // Event context.
            );
    }
    return hr;
}

//-------------------------------------------------------------------
//  SetDisplayName
//
//  Sets the display name for the session audio.
//
//-------------------------------------------------------------------

HRESULT CAudioSessionVolume::SetDisplayName(const WCHAR *wszName)
{
    HRESULT hr = S_OK;

    if (m_pAudioSession == NULL)
    {
        hr = E_FAIL;
    }
    else
    {
        hr = m_pAudioSession->SetDisplayName(wszName, NULL);
    }
    return hr;
}


//-------------------------------------------------------------------
//  OnSimpleVolumeChanged
//
//  Callback when the session volume level or muting state changes.
//  (Implements IAudioSessionEvents::OnSimpleVolumeChanged.)
//
//-------------------------------------------------------------------

HRESULT CAudioSessionVolume::OnSimpleVolumeChanged( 
    float NewVolume, 
    BOOL NewMute, 
    LPCGUID EventContext 
    )
{
    // Check if we should post a message to the application.

    if ( m_bNotificationsEnabled &&  
        (*EventContext != AudioSessionVolumeCtx) &&  
        (m_hwndNotification != NULL)
        )
    {
        // Notifications are enabled, AND
        // We did not trigger the event ourselves, AND
        // We have a valid window handle.

        // Post the message.
        ::PostMessage( 
            m_hwndNotification, 
            m_uNotificationMessage, 
            *((WPARAM*)(&NewVolume)),  // Coerce the float.
            (LPARAM)NewMute
            );
    }
    return S_OK;
}

