//////////////////////////////////////////////////////////////////////////
//
// preview.cpp : Preview helper class.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include <new>
#include <windows.h>
#include <mfplay.h>
#include <Dbt.h>
#include <shlwapi.h>

#include "Preview.h"

void ShowErrorMessage(HWND hwnd, PCWSTR format, HRESULT hr);

//-------------------------------------------------------------------
//  CreateInstance
//  Static class method to create the CPreview object.
//  
//  hVideo:   Handle to the video window.
//  ppPlayer: Receives an AddRef's pointer to the CPreview object. 
//            The caller must release the pointer.
//-------------------------------------------------------------------

HRESULT CPreview::CreateInstance(HWND hVideo, CPreview **ppPlayer)
{
    if (ppPlayer == NULL)
    {
        return E_POINTER;
    }

    CPreview *pPlayer = new (std::nothrow) CPreview(hVideo);

    if (pPlayer == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // The CPreview constructor sets the ref count to 1.
    *ppPlayer = pPlayer;

    return S_OK;
}


//-------------------------------------------------------------------
//  CPreview constructor
//-------------------------------------------------------------------

CPreview::CPreview(HWND hVideo) : 
    m_pPlayer(NULL),
    m_pSource(NULL),
    m_nRefCount(1),
    m_hwnd(hVideo),
    m_bHasVideo(FALSE),
    m_pwszSymbolicLink(NULL),
    m_cchSymbolicLink(0)
{

}

//-------------------------------------------------------------------
//  CPreview destructor
//-------------------------------------------------------------------

CPreview::~CPreview()
{
    CloseDevice();
}


//// IUnknown methods


//-------------------------------------------------------------------
//  AddRef
//-------------------------------------------------------------------

ULONG CPreview::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}


//-------------------------------------------------------------------
//  Release
//-------------------------------------------------------------------

ULONG CPreview::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}



//-------------------------------------------------------------------
//  QueryInterface
//-------------------------------------------------------------------

HRESULT CPreview::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CPreview, IMFPMediaPlayerCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

//// IMFPMediaPlayerCallback methods


//-------------------------------------------------------------------
//  OnMediaPlayerEvent
//
//  This method is called by the MFPlay object to send events to
//  the application. For live preview, there are not many events to
//  worry about. (For example, no end-of-stream, paused, or stopped
//  events.)
//-------------------------------------------------------------------

void STDMETHODCALLTYPE CPreview::OnMediaPlayerEvent(MFP_EVENT_HEADER * pEventHeader)
{

    if (FAILED(pEventHeader->hrEvent))
    {
        ShowErrorMessage(NULL, L"Preview error.", pEventHeader->hrEvent);
        return;
    }
    
    switch (pEventHeader->eEventType)
    {
    case MFP_EVENT_TYPE_MEDIAITEM_CREATED:
        OnMediaItemCreated(MFP_GET_MEDIAITEM_CREATED_EVENT(pEventHeader));
        break;

    case MFP_EVENT_TYPE_MEDIAITEM_SET:
        OnMediaItemSet(MFP_GET_MEDIAITEM_SET_EVENT(pEventHeader));
        break;
    }
}


//// Class methods


//-------------------------------------------------------------------
//  SetDevice
// 
//  Sets the capture device source on the player.
//
//  pActivate: Pointer to the activation object for the device 
//             source.
//-------------------------------------------------------------------

HRESULT CPreview::SetDevice(IMFActivate *pActivate)
{
    HRESULT hr = S_OK;

    IMFMediaSource *pSource = NULL;

    // Release the current instance of the player (if any).
    CloseDevice();

    // Create a new instance of the player.
    hr = MFPCreateMediaPlayer(
        NULL,   // URL
        FALSE,
        0,      // Options
        this,   // Callback
        m_hwnd,
        &m_pPlayer
        );

    // Create the media source for the device.
    if (SUCCEEDED(hr))
    {
        hr = pActivate->ActivateObject(
            __uuidof(IMFMediaSource), 
            (void**)&pSource
            );
    }

    // Get the symbolic link. This is needed to handle device-
    // loss notifications. (See CheckDeviceLost.)

    if (SUCCEEDED(hr))
    {
        hr = pActivate->GetAllocatedString(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
            &m_pwszSymbolicLink,
            &m_cchSymbolicLink
            );

    }

    // Create a new media item for this media source.
    if (SUCCEEDED(hr))
    {
        hr = m_pPlayer->CreateMediaItemFromObject(
            pSource,
            FALSE,  // FALSE = asynchronous call
            0,
            NULL
            );
    }

    // When the method completes, MFPlay will call OnMediaPlayerEvent
    // with the MFP_EVENT_TYPE_MEDIAITEM_CREATED event.

    if (SUCCEEDED(hr))
    {
        m_pSource = pSource;
        m_pSource->AddRef();
    }

    if (FAILED(hr))
    {
        CloseDevice();
    }

    if (pSource)
    {
        pSource->Release();
    }
    return hr;
}


//-------------------------------------------------------------------
//  OnMediaItemCreated
//
//  Called when the IMFPMediaPlayer::CreateMediaItemFromObject method 
//  completes.
//-------------------------------------------------------------------

void CPreview::OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT *pEvent)
{
    HRESULT hr = S_OK;

    if (m_pPlayer)
    {
        // Check if there is video.

        BOOL bHasVideo = FALSE, bIsSelected = FALSE;

        hr = pEvent->pMediaItem->HasVideo(&bHasVideo, &bIsSelected);

        if (SUCCEEDED(hr))
        {
            m_bHasVideo = bHasVideo && bIsSelected;

            // Set this media item on the player.
            hr = m_pPlayer->SetMediaItem(pEvent->pMediaItem);
        }
    }

    if (FAILED(hr))
    {
        ShowErrorMessage(NULL, L"Preview error.", hr);
    }
}

//-------------------------------------------------------------------
//  OnMediaItemCreated
//
//  Called when the IMFPMediaPlayer::SetMediaItem method completes.
//-------------------------------------------------------------------

void CPreview::OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT * /*pEvent*/)
{
    HRESULT hr = S_OK;

    SIZE szVideo = { 0 };
    RECT rc = { 0 };

    // Adjust the preview window to match the native size
    // of the captured video.

    hr = m_pPlayer->GetNativeVideoSize(&szVideo, NULL);

    if (SUCCEEDED(hr))
    {
        SetRect(&rc, 0, 0, szVideo.cx, szVideo.cy);

        AdjustWindowRect(
            &rc, 
            GetWindowLong(m_hwnd, GWL_STYLE),
            TRUE
            );

        SetWindowPos(m_hwnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top, 
            SWP_NOZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER);

        hr = m_pPlayer->Play();
    }

    if (FAILED(hr))
    {
        ShowErrorMessage(NULL, L"Preview error.", hr);
    }
}



//-------------------------------------------------------------------
//  UpdateVideo
//
//  Repaints and resizes the video image. The application calls this 
//  method when it receives a WM_PAINT or WM_SIZE message.
//-------------------------------------------------------------------

HRESULT CPreview::UpdateVideo()
{
    HRESULT hr = S_OK;

    if (m_pPlayer)
    {
        hr = m_pPlayer->UpdateVideo();
    }
    return hr;
}


//-------------------------------------------------------------------
//  CloseDevice
//  Releases the video capture device.
//-------------------------------------------------------------------

HRESULT CPreview::CloseDevice()
{
    HRESULT hr = S_OK;

    if (m_pPlayer)
    {
        m_pPlayer->Shutdown();
        m_pPlayer->Release();
        m_pPlayer = NULL;
    }

    if (m_pSource)
    {
        m_pSource->Shutdown();
        m_pSource->Release();
        m_pSource = NULL;
    }

    m_bHasVideo = FALSE;

    CoTaskMemFree(m_pwszSymbolicLink);
    m_pwszSymbolicLink = NULL;

    m_cchSymbolicLink = 0;

    return hr;
}


//-------------------------------------------------------------------
//  CheckDeviceLost
//  Checks whether the video capture device was removed.
//
//  The application calls this method when is receives a 
//  WM_DEVICECHANGE message.
//-------------------------------------------------------------------

HRESULT CPreview::CheckDeviceLost(DEV_BROADCAST_HDR *pHdr, BOOL *pbDeviceLost)
{
    DEV_BROADCAST_DEVICEINTERFACE *pDi = NULL;

    if (pbDeviceLost == NULL)
    {
        return E_POINTER;
    }

    *pbDeviceLost = FALSE;
    
    if (m_pSource == NULL)
    {
        return S_OK;
    }
    if (pHdr == NULL)
    {
        return S_OK;
    }
    if (pHdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
    {
        return S_OK;
    }

    // Compare the device name with the symbolic link.

    pDi = (DEV_BROADCAST_DEVICEINTERFACE*)pHdr;

    if (m_pwszSymbolicLink)
    {
        if (_wcsicmp(m_pwszSymbolicLink, pDi->dbcc_name) == 0)
        {
            *pbDeviceLost = TRUE;
        }
    }

    return S_OK;
}

