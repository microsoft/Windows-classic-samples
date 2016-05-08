// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "stdafx.h"
#include "Logger.h"

// {B41E404B-5502-444c-B21E-3D1F3D4CAC3D}
DEFINE_GUID( CLSID_CMFTransformWrapper,
0xb41e404b, 0x5502, 0x444c, 0xb2, 0x1e, 0x3d, 0x1f, 0x3d, 0x4c, 0xac, 0x3d);

class CMFTransformWrapper
    : public CComCoClass<CMFTransformWrapper, &CLSID_CMFTransformWrapper>
    , public IMFTransform
{
public:
    CMFTransformWrapper(IMFTransform* pWrappedTransform, CLogger* pLogger);
    virtual ~CMFTransformWrapper();

    virtual HRESULT STDMETHODCALLTYPE GetStreamLimits( 
            /* [out] */ __RPC__out DWORD *pdwInputMinimum,
            /* [out] */ __RPC__out DWORD *pdwInputMaximum,
            /* [out] */ __RPC__out DWORD *pdwOutputMinimum,
            /* [out] */ __RPC__out DWORD *pdwOutputMaximum);
        
    virtual HRESULT STDMETHODCALLTYPE GetStreamCount( 
        /* [out] */ __RPC__out DWORD *pcInputStreams,
        /* [out] */ __RPC__out DWORD *pcOutputStreams);
    
    virtual HRESULT STDMETHODCALLTYPE GetStreamIDs( 
        DWORD dwInputIDArraySize,
        /* [size_is][out] */ __RPC__out_ecount_full(dwInputIDArraySize) DWORD *pdwInputIDs,
        DWORD dwOutputIDArraySize,
        /* [size_is][out] */ __RPC__out_ecount_full(dwOutputIDArraySize) DWORD *pdwOutputIDs);
    
    virtual HRESULT STDMETHODCALLTYPE GetInputStreamInfo( 
        DWORD dwInputStreamID,
        /* [out] */ __RPC__out MFT_INPUT_STREAM_INFO *pStreamInfo);
    
    virtual HRESULT STDMETHODCALLTYPE GetOutputStreamInfo( 
        DWORD dwOutputStreamID,
        /* [out] */ __RPC__out MFT_OUTPUT_STREAM_INFO *pStreamInfo);
    
    virtual HRESULT STDMETHODCALLTYPE GetAttributes( 
        /* [out] */ __RPC__deref_out_opt IMFAttributes **pAttributes);
    
    virtual HRESULT STDMETHODCALLTYPE GetInputStreamAttributes( 
        DWORD dwInputStreamID,
        /* [out] */ __RPC__deref_out_opt IMFAttributes **pAttributes);
    
    virtual HRESULT STDMETHODCALLTYPE GetOutputStreamAttributes( 
        DWORD dwOutputStreamID,
        /* [out] */ __RPC__deref_out_opt IMFAttributes **pAttributes);
    
    virtual HRESULT STDMETHODCALLTYPE DeleteInputStream( 
        DWORD dwStreamID);
    
    virtual HRESULT STDMETHODCALLTYPE AddInputStreams( 
        DWORD cStreams,
        /* [in] */ __RPC__in DWORD *adwStreamIDs);
    
    virtual HRESULT STDMETHODCALLTYPE GetInputAvailableType( 
        DWORD dwInputStreamID,
        DWORD dwTypeIndex,
        /* [out] */ __RPC__deref_out_opt IMFMediaType **ppType);
    
    virtual HRESULT STDMETHODCALLTYPE GetOutputAvailableType( 
        DWORD dwOutputStreamID,
        DWORD dwTypeIndex,
        /* [out] */ __RPC__deref_out_opt IMFMediaType **ppType);
    
    virtual HRESULT STDMETHODCALLTYPE SetInputType( 
        DWORD dwInputStreamID,
        /* [in] */ __RPC__in_opt IMFMediaType *pType,
        DWORD dwFlags);
    
    virtual HRESULT STDMETHODCALLTYPE SetOutputType( 
        DWORD dwOutputStreamID,
        /* [in] */ __RPC__in_opt IMFMediaType *pType,
        DWORD dwFlags);
    
    virtual HRESULT STDMETHODCALLTYPE GetInputCurrentType( 
        DWORD dwInputStreamID,
        /* [out] */ __RPC__deref_out_opt IMFMediaType **ppType);
    
    virtual HRESULT STDMETHODCALLTYPE GetOutputCurrentType( 
        DWORD dwOutputStreamID,
        /* [out] */ __RPC__deref_out_opt IMFMediaType **ppType);
    
    virtual HRESULT STDMETHODCALLTYPE GetInputStatus( 
        DWORD dwInputStreamID,
        /* [out] */ __RPC__out DWORD *pdwFlags);
    
    virtual HRESULT STDMETHODCALLTYPE GetOutputStatus( 
        /* [out] */ __RPC__out DWORD *pdwFlags);
    
    virtual HRESULT STDMETHODCALLTYPE SetOutputBounds( 
        LONGLONG hnsLowerBound,
        LONGLONG hnsUpperBound);
    
    virtual HRESULT STDMETHODCALLTYPE ProcessEvent( 
        DWORD dwInputStreamID,
        /* [in] */ __RPC__in_opt IMFMediaEvent *pEvent);
    
    virtual HRESULT STDMETHODCALLTYPE ProcessMessage( 
        MFT_MESSAGE_TYPE eMessage,
        ULONG_PTR ulParam);
    
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE ProcessInput( 
        DWORD dwInputStreamID,
        IMFSample *pSample,
        DWORD dwFlags);
    
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE ProcessOutput( 
        DWORD dwFlags,
        DWORD cOutputBufferCount,
        /* [size_is][out][in] */ MFT_OUTPUT_DATA_BUFFER *pOutputSamples,
        /* [out] */ DWORD *pdwStatus);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** pp)
    { 
        if(pp == NULL)
        {
            return E_POINTER;
        }
        
        if(riid == IID_IUnknown || riid == IID_IMFTransform)
        {
            *pp = this;
            AddRef();
            return S_OK;
        }
        else if( SUCCEEDED(m_spWrappedTransform->QueryInterface(riid, pp)) )
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
    CComPtr<IMFTransform> m_spWrappedTransform;
    CLogger* m_pLogger;
    LONG m_cRef;
};

