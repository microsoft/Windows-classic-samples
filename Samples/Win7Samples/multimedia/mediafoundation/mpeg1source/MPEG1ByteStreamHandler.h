//////////////////////////////////////////////////////////////////////////
//
// MPEG1ByteStreamHandler.h
// Implements the byte-stream handler for the MPEG1 source.
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


//-------------------------------------------------------------------
// MPEG1ByteStreamHandler  class
//
// Byte-stream handler for MPEG-1 streams.
//-------------------------------------------------------------------

class MPEG1ByteStreamHandler 
    : BaseObject, 
      RefCountedObject, 
      public IMFByteStreamHandler, 
      public IMFAsyncCallback
{
public:
    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID iid, void **ppMEG);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFAsyncCallback
    STDMETHODIMP GetParameters(DWORD* pdwFlags, DWORD* pdwQueue)
    {
        // Implementation of this method is optional.
        return E_NOTIMPL;
    }

    STDMETHODIMP Invoke(IMFAsyncResult* pResult);

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

    MPEG1ByteStreamHandler(HRESULT& hr);
    ~MPEG1ByteStreamHandler();

    MPEG1Source     *m_pSource;
    IMFAsyncResult  *m_pResult;
};