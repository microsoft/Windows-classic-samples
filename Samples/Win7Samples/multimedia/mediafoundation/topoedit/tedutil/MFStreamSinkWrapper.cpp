// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "MFStreamSinkWrapper.h"

#include <assert.h>

#include "atlconv.h"

CMFStreamSinkWrapper::CMFStreamSinkWrapper(IMFStreamSink* pWrappedStreamSink, CLogger* pLogger)
    : m_spWrappedStreamSink(pWrappedStreamSink)
    , m_pLogger(pLogger)
    , m_cRef(0)
{
    assert(m_spWrappedStreamSink.p != NULL);
    assert(m_pLogger != NULL);

    m_pLogger->AddRef();
}

CMFStreamSinkWrapper::~CMFStreamSinkWrapper()
{
    m_pLogger->Release();
}

HRESULT CMFStreamSinkWrapper::GetMediaSink(/* [out] */ __RPC__deref_out_opt IMFMediaSink **ppMediaSink)
{
    HRESULT hr = m_spWrappedStreamSink->GetMediaSink(ppMediaSink);
    m_pLogger->LogFormat("GetMediaSink(%p) returns %x\r\n", ppMediaSink, hr);

    return hr;
}

HRESULT CMFStreamSinkWrapper::GetIdentifier( 
        /* [out] */ __RPC__out DWORD *pdwIdentifier)
{
    HRESULT hr = m_spWrappedStreamSink->GetIdentifier(pdwIdentifier);
    m_pLogger->LogFormat("GetIdentifier(%p) returns %x\r\n", pdwIdentifier, hr);
    if(SUCCEEDED(hr)) m_pLogger->LogFormat("--> Arg(1, out) identifier: %d", *pdwIdentifier);

    return hr;
}

HRESULT CMFStreamSinkWrapper::GetMediaTypeHandler( 
        /* [out] */ __RPC__deref_out_opt IMFMediaTypeHandler **ppHandler)
{
    HRESULT hr = m_spWrappedStreamSink->GetMediaTypeHandler(ppHandler);
    m_pLogger->LogFormat("GetMediaTypeHandler(%p) returns %x\r\n", ppHandler, hr);

    return hr;
}

HRESULT CMFStreamSinkWrapper::ProcessSample( 
        /* [in] */ __RPC__in_opt IMFSample *pSample)
{
    HRESULT hr = m_spWrappedStreamSink->ProcessSample(pSample);
    m_pLogger->LogFormat("ProcessSample(%p) returns %x\r\n", pSample, hr);

    return hr;
}

HRESULT CMFStreamSinkWrapper::PlaceMarker( 
        /* [in] */ MFSTREAMSINK_MARKER_TYPE eMarkerType,
        /* [in] */ __RPC__in const PROPVARIANT *pvarMarkerValue,
        /* [in] */ __RPC__in const PROPVARIANT *pvarContextValue)
{
    HRESULT hr = m_spWrappedStreamSink->PlaceMarker(eMarkerType, pvarMarkerValue, pvarContextValue);
    m_pLogger->LogFormat("PlaceMarker(%d %p %p) returns %x\r\n", eMarkerType, pvarMarkerValue, pvarContextValue, hr);

    return hr;
}

HRESULT CMFStreamSinkWrapper::Flush()
{
    HRESULT hr = m_spWrappedStreamSink->Flush();
    m_pLogger->LogFormat("Flush() returns %x\r\n", hr);

    return hr;
}

HRESULT CMFStreamSinkWrapper::GetEvent( 
        /* [in] */ DWORD dwFlags,
        /* [out] */ __RPC__deref_out_opt IMFMediaEvent **ppEvent)
{
    HRESULT hr = m_spWrappedStreamSink->GetEvent(dwFlags, ppEvent);
    m_pLogger->LogFormat("GetEvent(%d %p) returns %x\r\n", dwFlags, ppEvent, hr);

    return hr;    
}
HRESULT CMFStreamSinkWrapper::BeginGetEvent( 
        /* [in] */ IMFAsyncCallback *pCallback,
        /* [in] */ IUnknown *punkState)
{
    HRESULT hr = m_spWrappedStreamSink->BeginGetEvent(pCallback, punkState);
    m_pLogger->LogFormat("BeginGetEvent(%p %p) returns %x\r\n", pCallback, punkState, hr);

    return hr;
}

HRESULT CMFStreamSinkWrapper::EndGetEvent( 
        /* [in] */ IMFAsyncResult *pResult,
        /* [out] */ 
        __out  IMFMediaEvent **ppEvent)
{
    HRESULT hr = m_spWrappedStreamSink->EndGetEvent(pResult, ppEvent);
    m_pLogger->LogFormat("EndGetEvent(%p %p) returns %x\r\n", pResult, ppEvent, hr);

    return hr;
}

HRESULT CMFStreamSinkWrapper::QueueEvent( 
        /* [in] */ MediaEventType met,
        /* [in] */ __RPC__in REFGUID guidExtendedType,
        /* [in] */ HRESULT hrStatus,
        /* [unique][in] */ __RPC__in_opt const PROPVARIANT *pvValue)
{
    HRESULT hr = m_spWrappedStreamSink->QueueEvent(met, guidExtendedType, hrStatus, pvValue);

    LPOLESTR guidStr = NULL;
    StringFromCLSID(guidExtendedType, &guidStr);
    CAtlStringA strGuidExtendedType( (CW2A(guidStr)) );
    CoTaskMemFree(guidStr);
    
    m_pLogger->LogFormat("GetMediaSink(%d %s %x %p) returns %x\r\n", met, strGuidExtendedType, hrStatus, pvValue, hr);

    return hr;
}
                                        
