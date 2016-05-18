//////////////////////////////////////////////////////////////////////////
//
// MediaController.h : CMediaController class declaration.
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


#include "MF_ASFParser.h"

class CMediaController : public IUnknown
{
public:
    static HRESULT CMediaController::CreateInstance(CMediaController **ppMediaController);
    // callback for waveOut functions
    static DWORD WINAPI WaveOutThreadProc( LPVOID lpParameter );

    CMediaController(HRESULT* hr);
    ~CMediaController();
    HRESULT CreateBitmapForKeyFrame(BYTE* pPixelData, IMFMediaType* pMediaType);
    HRESULT DrawKeyFrame(HWND hWnd);
    HRESULT GetBitmapDimensions(UINT32 *pWidth, UINT32 *pHeight);

    HRESULT AddToAudioTestSample ( IMFSample *pBuffer);
    HRESULT Reset();
    HRESULT OpenAudioDevice(IMFMediaType* pMediaType);
    HRESULT CloseAudioDevice();

    HRESULT PlayAudio();

    BOOL HasTestMedia(){return m_fHasTestMedia;};


    // IUnknown methods

    STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
    {
        if (!ppv)
        {
            return E_POINTER;
        }
        if (iid == IID_IUnknown)
        {
            *ppv = static_cast<IUnknown*>(this);
        }
        else
        {            
            *ppv = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_nRefCount);
    }

    STDMETHODIMP_(ULONG) Release()
    {
        ULONG uCount = InterlockedDecrement(&m_nRefCount);
        
        if (uCount == 0)
        {
            delete this;
        }
        // For thread safety, return a temporary variable.
        return uCount;
    }


private:
    long        m_nRefCount;
    ULONG_PTR   m_gdiplusToken;

    Bitmap*     m_pBitmap;
    IMFSample*  m_pAudioTestSample;

    UINT32      m_Width;
    UINT32      m_Height;

    HWND        m_hWnd;
    HWAVEOUT    m_hWaveOut;         // handle to waveout device
    HANDLE      m_hThread;          // handle to the thread.

    BOOL        m_fHasTestMedia;

    BOOL        m_fAudioDeviceBusy;

    WAVEHDR     m_WaveHeader;
    
    void    DoWaveOutThread();

};