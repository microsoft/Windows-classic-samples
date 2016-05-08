//////////////////////////////////////////////////////////////////////////
//
// WavByteStreamHandler.cpp: Bytestream handler for .wav files.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "WavSource.h"
#include "WavByteStreamHandler.h"

HRESULT WavByteStreamHandler_CreateInstance(REFIID riid, void **ppv)
{
    return CWavByteStreamHandler::CreateInstance(riid, ppv);
}


//-------------------------------------------------------------------
// Name: CreateInstance
// Description: Static method to create an instance of the object.
//
// iid:         IID of the requested interface on the object.
// ppSource:    Receives a ref-counted pointer to the object.
//-------------------------------------------------------------------

HRESULT CWavByteStreamHandler::CreateInstance(REFIID iid, void **ppMEG)
{
    if (ppMEG == NULL)
    {
        return E_POINTER;
    }

    // Create the object.

    HRESULT hr = S_OK;
    CWavByteStreamHandler *pHandler = new (std::nothrow) CWavByteStreamHandler(hr);
    if (pHandler == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Query for the requested interface.
    if (SUCCEEDED(hr))
    {
        hr = pHandler->QueryInterface(iid, ppMEG);
    }

    if (FAILED(hr))
    {
        delete pHandler;
    }
    return hr;
}


//-------------------------------------------------------------------
// CWavByteStreamHandler constructor
//-------------------------------------------------------------------

CWavByteStreamHandler::CWavByteStreamHandler(HRESULT &hr) : m_nRefCount(0)
{
    DllAddRef();

    hr = S_OK;
}
   

//-------------------------------------------------------------------
// CWavByteStreamHandler destructor
//-------------------------------------------------------------------

CWavByteStreamHandler::~CWavByteStreamHandler()
{
    assert(m_nRefCount == 0);

    DllRelease();
}




// IUnknown methods

ULONG CWavByteStreamHandler::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG  CWavByteStreamHandler::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT CWavByteStreamHandler::QueryInterface(REFIID iid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CWavByteStreamHandler, IMFByteStreamHandler),
        { 0 }
    };
    return QISearch(this, qit, iid, ppv);
}


// IMFByteStreamHandler methods

HRESULT CWavByteStreamHandler::BeginCreateObject( 
        /* [in] */ IMFByteStream *pByteStream,
        /* [in] */ LPCWSTR pwszURL,
        /* [in] */ DWORD /*dwFlags*/,
        /* [in] */ IPropertyStore * /*pProps*/,              // Can be NULL.
        /* [out] */ IUnknown ** ppIUnknownCancelCookie,  // Can be NULL.
        /* [in] */ IMFAsyncCallback *pCallback,
        /* [in] */ IUnknown *punkState                  // Can be NULL
        )
{
    if ((pByteStream == NULL) || (pwszURL == NULL) || (pCallback == NULL))
    {
        return E_INVALIDARG;
    }

    if (ppIUnknownCancelCookie)
    {
        *ppIUnknownCancelCookie = NULL; // We don't return a cancellation cookie.
    }

    IMFMediaSource *pSource = NULL;
    IMFAsyncResult *pResult = NULL;

    HRESULT hr = S_OK;

    hr = WavSource::CreateInstance(IID_IMFMediaSource, (void**)&pSource);

    if (SUCCEEDED(hr))
    {
        hr = ((WavSource*)pSource)->Open(pByteStream);
    }

    if (SUCCEEDED(hr))
    {
        hr = MFCreateAsyncResult(pSource, pCallback, punkState, &pResult);
    }
    if (SUCCEEDED(hr))
    {
        MFInvokeCallback(pResult);
    }

    SafeRelease(&pResult);
    SafeRelease(&pSource);

    return hr;
}
    
HRESULT CWavByteStreamHandler::EndCreateObject( 
        /* [in] */ IMFAsyncResult *pResult,
        /* [out] */ MF_OBJECT_TYPE *pObjectType,
        /* [out] */ IUnknown **ppObject)
{
    HRESULT hr = S_OK;

    if ((pResult == NULL) || (ppObject == NULL) || (ppObject == NULL))
    {
        return E_INVALIDARG;
    }

    IMFMediaSource *pSource = NULL;
    IUnknown *pUnk = NULL;

    hr = pResult->GetObject(&pUnk);

    if (SUCCEEDED(hr))
    {
        // Minimal sanity check - is it really a media source?
        hr = pUnk->QueryInterface(IID_PPV_ARGS(&pSource));
    }

    if (SUCCEEDED(hr))
    {
        *ppObject = pUnk;
        (*ppObject)->AddRef();
        *pObjectType = MF_OBJECT_MEDIASOURCE;
    }

    SafeRelease(&pSource);
    SafeRelease(&pUnk);

    return hr;

}

HRESULT CWavByteStreamHandler::CancelObjectCreation(IUnknown * /*pIUnknownCancelCookie*/)
{
    return E_NOTIMPL;
}


HRESULT CWavByteStreamHandler::GetMaxNumberOfBytesRequiredForResolution(QWORD* pqwBytes)
{
    if (pqwBytes == NULL)
    {
        return E_INVALIDARG;
    }

    // In a canonical PCM .wav file, the start of the 'data' chunk is at byte offset 44.
    *pqwBytes = 44;
    return S_OK;
}
