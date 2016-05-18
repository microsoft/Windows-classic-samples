//////////////////////////////////////////////////////////////////////////
//
// preview.h : Preview helper class.
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

class CPreview : public IMFPMediaPlayerCallback
{
public:
    static HRESULT CreateInstance(
        HWND hVideo, 
        CPreview **ppPlayer
    );

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    void STDMETHODCALLTYPE OnMediaPlayerEvent(MFP_EVENT_HEADER * pEventHeader);

    HRESULT       SetDevice(IMFActivate *pActivate);
    HRESULT       CloseDevice();
    HRESULT       UpdateVideo();
    HRESULT       CheckDeviceLost(DEV_BROADCAST_HDR *pHdr, BOOL *pbDeviceLost);
    BOOL          HasVideo() const { return m_bHasVideo; }

protected:
    
    // Constructor is private. Use static CreateInstance method to instantiate.
    CPreview(HWND hVideo);

    // Destructor is private. Caller should call Release.
    virtual ~CPreview();

    // Event handlers
    void OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT *pEvent);
    void OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT *pEvent); 

protected:

    long                    m_nRefCount;        // Reference count.
    IMFPMediaPlayer         *m_pPlayer;
    IMFMediaSource          *m_pSource;
    HWND                    m_hwnd;
    BOOL                    m_bHasVideo;         

    WCHAR                   *m_pwszSymbolicLink;
    UINT32                  m_cchSymbolicLink;

};