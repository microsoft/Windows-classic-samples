//////////////////////////////////////////////////////////////////////////
//
// MPEG1ByteStreamHandler.cpp
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


#include "MPEG1Source.h"
#include "MPEG1ByteStreamHandler.h"


//-------------------------------------------------------------------
// MPEG1ByteStreamHandler  class
//-------------------------------------------------------------------


//-------------------------------------------------------------------
// CreateInstance
// Static method to create an instance of the oject.
//
// This method is used by the class factory.
//
// pUnkOuter: Aggregating IUnknown.
// 
//-------------------------------------------------------------------

HRESULT MPEG1ByteStreamHandler::CreateInstance(IUnknown *pUnkOuter, REFIID iid, void **ppv)
{
    if (ppv == NULL)
    {
        return E_POINTER;
    }

    // This object does not support aggregation.
    if (pUnkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    HRESULT hr = S_OK;

    MPEG1ByteStreamHandler *pHandler = new MPEG1ByteStreamHandler(hr);
    if (pHandler == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = pHandler->QueryInterface(iid, ppv);
    }

    SAFE_RELEASE(pHandler);
    return hr;
}


//-------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------

MPEG1ByteStreamHandler::MPEG1ByteStreamHandler(HRESULT& hr) 
    : m_pSource(NULL), m_pResult(NULL)
{

}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------

MPEG1ByteStreamHandler::~MPEG1ByteStreamHandler()
{
    SAFE_RELEASE(m_pSource);
    SAFE_RELEASE(m_pResult);
}


//-------------------------------------------------------------------
// IUnknown methods
//-------------------------------------------------------------------

ULONG MPEG1ByteStreamHandler::AddRef()
{
    return RefCountedObject::AddRef();
}

ULONG MPEG1ByteStreamHandler::Release()
{
    return RefCountedObject::Release();
}

HRESULT MPEG1ByteStreamHandler::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(MPEG1ByteStreamHandler, IMFByteStreamHandler),
        QITABENT(MPEG1ByteStreamHandler, IMFAsyncCallback),
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}


//-------------------------------------------------------------------
// IMFByteStreamHandler methods
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// BeginCreateObject
// Starts creating the media source.
//-------------------------------------------------------------------

HRESULT MPEG1ByteStreamHandler::BeginCreateObject( 
        /* [in] */ IMFByteStream *pByteStream,
        /* [in] */ LPCWSTR pwszURL,
        /* [in] */ DWORD dwFlags,
        /* [in] */ IPropertyStore *pProps,
        /* [out] */ IUnknown **ppIUnknownCancelCookie,  // Can be NULL
        /* [in] */ IMFAsyncCallback *pCallback,
        /* [in] */ IUnknown *punkState                  // Can be NULL
        )
{
    if (pByteStream == NULL)
    {
        return E_POINTER;
    }

    if (pCallback == NULL)
    {
        return E_POINTER;
    }

    if ((dwFlags & MF_RESOLUTION_MEDIASOURCE) == 0)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    IMFAsyncResult *pResult = NULL;
    MPEG1Source    *pSource = NULL;

    // Create an instance of the media source.
    CHECK_HR(hr = MPEG1Source::CreateInstance(&pSource));

    // Create a result object for the caller's async callback.
    CHECK_HR(hr = MFCreateAsyncResult(NULL, pCallback, punkState, &pResult));

    // Start opening the source. This is an async operation. 
    // When it completes, the source will invoke our callback
    // and then we will invoke the caller's callback.
    CHECK_HR(hr = pSource->BeginOpen(pByteStream, this, NULL));

    if (ppIUnknownCancelCookie)
    {
        *ppIUnknownCancelCookie = NULL;
    }

    m_pSource = pSource;
    m_pSource->AddRef();

    m_pResult = pResult;
    m_pResult->AddRef();


done:
    SAFE_RELEASE(pSource);
    SAFE_RELEASE(pResult);
    return hr;
}
    
//-------------------------------------------------------------------
// EndCreateObject
// Completes the BeginCreateObject operation.
//-------------------------------------------------------------------

HRESULT MPEG1ByteStreamHandler::EndCreateObject( 
        /* [in] */ IMFAsyncResult *pResult,
        /* [out] */ MF_OBJECT_TYPE *pObjectType,
        /* [out] */ IUnknown **ppObject)
{
    if (pResult == NULL || pObjectType == NULL || ppObject == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    *pObjectType = MF_OBJECT_INVALID;
    *ppObject = NULL;

    CHECK_HR(hr = pResult->GetStatus());

    *pObjectType = MF_OBJECT_MEDIASOURCE;

    assert(m_pSource != NULL);

    hr = m_pSource->QueryInterface(IID_PPV_ARGS(ppObject));

done:
    SAFE_RELEASE(m_pSource);
    SAFE_RELEASE(m_pResult);

    return hr;
}

    
HRESULT MPEG1ByteStreamHandler::CancelObjectCreation(IUnknown *pIUnknownCancelCookie)
{
    return E_NOTIMPL;
}

HRESULT MPEG1ByteStreamHandler::GetMaxNumberOfBytesRequiredForResolution(QWORD* pqwBytes)
{
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// Invoke
// Callback for the media source's BeginOpen method.
//-------------------------------------------------------------------

HRESULT MPEG1ByteStreamHandler::Invoke(IMFAsyncResult* pResult)
{
    HRESULT hr = S_OK;

    if (m_pSource)
    {
        hr = m_pSource->EndOpen(pResult);
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    m_pResult->SetStatus(hr);

    hr = MFInvokeCallback(m_pResult);

    return hr;
}

