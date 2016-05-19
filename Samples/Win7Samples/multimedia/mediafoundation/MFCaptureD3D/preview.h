//////////////////////////////////////////////////////////////////////////
//
// preview.h: Manages video preview.
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

const UINT WM_APP_PREVIEW_ERROR = WM_APP + 1;    // wparam = HRESULT

class CPreview : public IMFSourceReaderCallback
{
public:
    static HRESULT CreateInstance(
        HWND hVideo, 
        HWND hEvent, 
        CPreview **ppPlayer
    );

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFSourceReaderCallback methods
    STDMETHODIMP OnReadSample(
        HRESULT hrStatus,
        DWORD dwStreamIndex,
        DWORD dwStreamFlags,
        LONGLONG llTimestamp,
        IMFSample *pSample
    );

    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *)
    {
        return S_OK;
    }

    STDMETHODIMP OnFlush(DWORD)
    {
        return S_OK;
    }

    HRESULT       SetDevice(IMFActivate *pActivate);
    HRESULT       CloseDevice();
    HRESULT       ResizeVideo(WORD width, WORD height);
    HRESULT       CheckDeviceLost(DEV_BROADCAST_HDR *pHdr, BOOL *pbDeviceLost);

protected:
    
    // Constructor is private. Use static CreateInstance method to create.
    CPreview(HWND hVideo, HWND hEvent);

    // Destructor is private. Caller should call Release.
    virtual ~CPreview();

    HRESULT Initialize();
    void    NotifyError(HRESULT hr) { PostMessage(m_hwndEvent, WM_APP_PREVIEW_ERROR, (WPARAM)hr, 0L); }
    HRESULT TryMediaType(IMFMediaType *pType);

    long                    m_nRefCount;        // Reference count.
    CRITICAL_SECTION        m_critsec;

    HWND                    m_hwndVideo;        // Video window.
    HWND                    m_hwndEvent;        // Application window to receive events. 

    IMFSourceReader         *m_pReader;

    DrawDevice              m_draw;             // Manages the Direct3D device.

    WCHAR                   *m_pwszSymbolicLink;
    UINT32                  m_cchSymbolicLink;
};