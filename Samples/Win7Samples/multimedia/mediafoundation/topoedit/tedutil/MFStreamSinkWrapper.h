// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "stdafx.h"
#include "Logger.h"

// {D378DA82-7D48-4a00-B021-2EA1AFBA757F}
DEFINE_GUID(CLSID_CMFStreamSinkWrapper, 
0xd378da82, 0x7d48, 0x4a00, 0xb0, 0x21, 0x2e, 0xa1, 0xaf, 0xba, 0x75, 0x7f);

class CMFStreamSinkWrapper
    : public CComCoClass<CMFStreamSinkWrapper, &CLSID_CMFStreamSinkWrapper>
    , public IMFStreamSink
{
public:
    CMFStreamSinkWrapper(IMFStreamSink* pWrappedStreamSink, CLogger* pLogger);
    virtual ~CMFStreamSinkWrapper();
    
    virtual HRESULT STDMETHODCALLTYPE GetMediaSink( 
        /* [out] */ __RPC__deref_out_opt IMFMediaSink **ppMediaSink);
    
    virtual HRESULT STDMETHODCALLTYPE GetIdentifier( 
        /* [out] */ __RPC__out DWORD *pdwIdentifier);
    
    virtual HRESULT STDMETHODCALLTYPE GetMediaTypeHandler( 
        /* [out] */ __RPC__deref_out_opt IMFMediaTypeHandler **ppHandler);
    
    virtual HRESULT STDMETHODCALLTYPE ProcessSample( 
        /* [in] */ __RPC__in_opt IMFSample *pSample);
    
    virtual HRESULT STDMETHODCALLTYPE PlaceMarker( 
        /* [in] */ MFSTREAMSINK_MARKER_TYPE eMarkerType,
        /* [in] */ __RPC__in const PROPVARIANT *pvarMarkerValue,
        /* [in] */ __RPC__in const PROPVARIANT *pvarContextValue);
    
    virtual HRESULT STDMETHODCALLTYPE Flush( void);

    virtual HRESULT STDMETHODCALLTYPE GetEvent( 
        /* [in] */ DWORD dwFlags,
        /* [out] */ __RPC__deref_out_opt IMFMediaEvent **ppEvent);
    
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE BeginGetEvent( 
        /* [in] */ IMFAsyncCallback *pCallback,
        /* [in] */ IUnknown *punkState);
    
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE EndGetEvent( 
        /* [in] */ IMFAsyncResult *pResult,
        /* [out] */ 
        __out  IMFMediaEvent **ppEvent);
    
    virtual HRESULT STDMETHODCALLTYPE QueueEvent( 
        /* [in] */ MediaEventType met,
        /* [in] */ __RPC__in REFGUID guidExtendedType,
        /* [in] */ HRESULT hrStatus,
        /* [unique][in] */ __RPC__in_opt const PROPVARIANT *pvValue);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** pp)
    { 
        if(pp == NULL)
        {
            return E_POINTER;
        }
        
        if(riid == IID_IUnknown || riid == IID_IMFStreamSink)
        {
            *pp = this;
            AddRef();
            return S_OK;
        }
        else if( SUCCEEDED(m_spWrappedStreamSink->QueryInterface(riid, pp)) )
        {
             return S_OK;
        }

        return E_NOINTERFACE;
    }
    
    virtual ULONG STDMETHODCALLTYPE AddRef()
    {
        LONG cRef = InterlockedIncrement(&m_cRef);

        return cRef;
    }
    
    virtual ULONG STDMETHODCALLTYPE Release() 
    {
        LONG cRef = InterlockedDecrement( &m_cRef );

        if( cRef == 0 )
        {
            delete this;
        }
        return( cRef );
    }
    
    
private:
    CComPtr<IMFStreamSink> m_spWrappedStreamSink;
    CLogger* m_pLogger;
    LONG m_cRef;
};

