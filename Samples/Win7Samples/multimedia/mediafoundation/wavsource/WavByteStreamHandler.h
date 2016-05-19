//////////////////////////////////////////////////////////////////////////
//
// WavByteStreamHandler.h: Bytestream handler for .wav files.
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


class CWavByteStreamHandler : public IMFByteStreamHandler
{

public:
    static HRESULT CreateInstance(REFIID iid, void **ppMEG);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFByteStreamHandler

    STDMETHODIMP BeginCreateObject( 
            /* [in] */ IMFByteStream *pByteStream,
            /* [in] */ LPCWSTR pwszURL,
            /* [in] */ DWORD dwFlags,
            /* [in] */ IPropertyStore *pProps,
            /* [out] */ IUnknown **ppIUnknownCancelCookie,
            /* [in] */ IMFAsyncCallback *pCallback,
            /* [in] */ IUnknown *punkState);
        
    STDMETHODIMP EndCreateObject( 
            /* [in] */ IMFAsyncResult *pResult,
            /* [out] */ MF_OBJECT_TYPE *pObjectType,
            /* [out] */ IUnknown **ppObject);
        
    STDMETHODIMP CancelObjectCreation(IUnknown *pIUnknownCancelCookie);
    STDMETHODIMP GetMaxNumberOfBytesRequiredForResolution(QWORD* pqwBytes);

private:

    CWavByteStreamHandler(HRESULT& hr);
    ~CWavByteStreamHandler();


    long            m_nRefCount;                // reference count
};