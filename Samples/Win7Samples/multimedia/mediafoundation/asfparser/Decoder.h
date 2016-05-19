//////////////////////////////////////////////////////////////////////////
//
// CDecoder.h : CDecoder class declaration.
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
#include "MediaController.h"


class CDecoder : public IUnknown
{
public:
    static HRESULT CDecoder::CreateInstance(CDecoder **ppDecoder);

    CDecoder();
    ~CDecoder();
    HRESULT Initialize(CLSID clsid, 
                               IMFMediaType *pMediaType);


    HRESULT ProcessAudio(IMFSample *pSample);
    
    HRESULT ProcessVideo(IMFSample *pSample);

    HRESULT StartDecoding(void);

    HRESULT StopDecoding(void);

    DWORD GetDecoderStatus ()
    {
        return  m_DecoderState;
    }

    HRESULT GetMediaController (CMediaController** pMediaController)
    {
        if (!m_pMediaController)
        {
            return MF_E_NOT_INITIALIZED;
        }

        *pMediaController = m_pMediaController;

        return S_OK;
    }

    void Reset (void)
    {
        SAFE_RELEASE( m_pMFT);
        SAFE_RELEASE (m_pMediaController);
    }

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
    long    m_nRefCount;

    IMFTransform* m_pMFT; //Currently loaded decoder MFT

    DWORD m_DecoderState; //Current state of the decoder, Streaming, Not Streaming

    DWORD m_dwInputID; //Input stream ID for the decoder MFT.

    DWORD m_dwOutputID; //Output stream ID for the decoder MFT.

    CMediaController* m_pMediaController; //Pointer to the class for handling decoded media data

    HRESULT ConfigureDecoder( IMFMediaType *pMediaType); //Configures the decoder MFT to work with a particular stream type.

    HRESULT UnLoad(); //Resets the decoder MFT

};