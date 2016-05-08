// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "MFTransformWrapper.h"
#include "mediatypetrace.h"

#include <assert.h>

CMFTransformWrapper::CMFTransformWrapper(IMFTransform* pWrappedTransform, CLogger* pLogger)
    : m_spWrappedTransform(pWrappedTransform)
    , m_pLogger(pLogger)
    , m_cRef(0)
{
    assert(m_spWrappedTransform.p != NULL);
    assert (m_pLogger != NULL);

    m_pLogger->AddRef();
}

CMFTransformWrapper::~CMFTransformWrapper()
{
    m_pLogger->Release();
}

HRESULT CMFTransformWrapper::GetStreamLimits( 
            /* [out] */ __RPC__out DWORD *pdwInputMinimum,
            /* [out] */ __RPC__out DWORD *pdwInputMaximum,
            /* [out] */ __RPC__out DWORD *pdwOutputMinimum,
            /* [out] */ __RPC__out DWORD *pdwOutputMaximum)
{
    HRESULT hr = m_spWrappedTransform->GetStreamLimits(pdwInputMinimum, pdwInputMaximum, pdwOutputMinimum, pdwOutputMaximum);
    m_pLogger->LogFormat("GetStreamLimits(%p %p %p %p) returns %x\r\n", pdwInputMinimum, pdwInputMaximum, pdwOutputMinimum, pdwOutputMaximum, hr);

    return hr;
}

HRESULT CMFTransformWrapper::GetStreamCount( 
            /* [out] */ __RPC__out DWORD *pcInputStreams,
            /* [out] */ __RPC__out DWORD *pcOutputStreams)
{
    HRESULT hr = m_spWrappedTransform->GetStreamCount(pcInputStreams, pcOutputStreams);
    m_pLogger->LogFormat("GetStreamCount(%p %p) returns %x\r\n", pcInputStreams, pcOutputStreams, hr);

    return hr;
}

HRESULT CMFTransformWrapper::GetStreamIDs( 
            DWORD dwInputIDArraySize,
            /* [size_is][out] */ __RPC__out_ecount_full(dwInputIDArraySize) DWORD *pdwInputIDs,
            DWORD dwOutputIDArraySize,
            /* [size_is][out] */ __RPC__out_ecount_full(dwOutputIDArraySize) DWORD *pdwOutputIDs)
{
    HRESULT hr = m_spWrappedTransform->GetStreamIDs(dwInputIDArraySize, pdwInputIDs, dwOutputIDArraySize, pdwOutputIDs);
    m_pLogger->LogFormat("GetStreamIDs(%d %p %d %p) returns %x\r\n", dwInputIDArraySize, pdwInputIDs, dwOutputIDArraySize, pdwOutputIDs, hr);

    return hr;
}

HRESULT CMFTransformWrapper::GetInputStreamInfo( 
            DWORD dwInputStreamID,
            /* [out] */ __RPC__out MFT_INPUT_STREAM_INFO *pStreamInfo)
{
    HRESULT hr = m_spWrappedTransform->GetInputStreamInfo(dwInputStreamID, pStreamInfo);
    m_pLogger->LogFormat("GetInputStreamInfo(%d %p) returns %x\r\n", dwInputStreamID, pStreamInfo, hr);
    
    return hr;
}

HRESULT CMFTransformWrapper::GetOutputStreamInfo( 
            DWORD dwOutputStreamID,
            /* [out] */ __RPC__out MFT_OUTPUT_STREAM_INFO *pStreamInfo)
{
    HRESULT hr = m_spWrappedTransform->GetOutputStreamInfo(dwOutputStreamID, pStreamInfo);
    m_pLogger->LogFormat("GetOutputStreamInfo(%d %p) returns %x\r\n", dwOutputStreamID, pStreamInfo, hr);
    
    return hr;
}

HRESULT CMFTransformWrapper::GetAttributes( 
            /* [out] */ __RPC__deref_out_opt IMFAttributes **pAttributes)
{
    HRESULT hr = m_spWrappedTransform->GetAttributes(pAttributes);
    m_pLogger->LogFormat("GetAttributes(%p) returns %x\r\n", pAttributes, hr);
    
    return hr;
}

HRESULT CMFTransformWrapper::GetInputStreamAttributes( 
            DWORD dwInputStreamID,
            /* [out] */ __RPC__deref_out_opt IMFAttributes **pAttributes)
{
    HRESULT hr = m_spWrappedTransform->GetInputStreamAttributes(dwInputStreamID, pAttributes);
    m_pLogger->LogFormat("GetInputStreamAttributes(%d, %p) returns %x\r\n", dwInputStreamID, pAttributes, hr);
    
    return hr;
}

HRESULT CMFTransformWrapper::GetOutputStreamAttributes( 
            DWORD dwOutputStreamID,
            /* [out] */ __RPC__deref_out_opt IMFAttributes **pAttributes)
{
    HRESULT hr = m_spWrappedTransform->GetInputStreamAttributes(dwOutputStreamID, pAttributes);
    m_pLogger->LogFormat("GetOutputStreamAttributes(%d, %p) returns %x\r\n", dwOutputStreamID, pAttributes, hr);
    
    return hr;
}

HRESULT CMFTransformWrapper::DeleteInputStream( 
            DWORD dwStreamID)
{
    HRESULT hr = m_spWrappedTransform->DeleteInputStream(dwStreamID);
    m_pLogger->LogFormat("DeleteInputStream(%d) returns %x\r\n", dwStreamID, hr);
    
    return hr;
}

HRESULT CMFTransformWrapper::AddInputStreams( 
            DWORD cStreams,
            /* [in] */ __RPC__in DWORD *adwStreamIDs)
{
    HRESULT hr = m_spWrappedTransform->AddInputStreams(cStreams, adwStreamIDs);
    m_pLogger->LogFormat("AddInputStreams(%d %p) returns %x\r\n", cStreams, adwStreamIDs, hr);
    
    return hr;    
}

HRESULT CMFTransformWrapper::GetInputAvailableType( 
            DWORD dwInputStreamID,
            DWORD dwTypeIndex,
            /* [out] */ __RPC__deref_out_opt IMFMediaType **ppType)
{
    HRESULT hr = m_spWrappedTransform->GetInputAvailableType(dwInputStreamID, dwTypeIndex, ppType);
    m_pLogger->LogFormat("GetInputAvailableType(%d %d %p) returns %x\r\n", dwInputStreamID, dwTypeIndex, ppType, hr);
    if(SUCCEEDED(hr)) m_pLogger->LogFormat("--> Arg(3, out) Media type: %s\r\n", CMediaTypeTrace(*ppType).GetString());
    
    return hr; 
}

HRESULT CMFTransformWrapper::GetOutputAvailableType( 
            DWORD dwOutputStreamID,
            DWORD dwTypeIndex,
            /* [out] */ __RPC__deref_out_opt IMFMediaType **ppType)
{
    HRESULT hr = m_spWrappedTransform->GetOutputAvailableType(dwOutputStreamID, dwTypeIndex, ppType);
    m_pLogger->LogFormat("GetOutputAvailableType(%d %d %p) returns %x\r\n", dwOutputStreamID, dwTypeIndex, ppType, hr);
    if(SUCCEEDED(hr)) m_pLogger->LogFormat("--> Arg(3, out) Media type: %s\r\n", CMediaTypeTrace(*ppType).GetString());

    return hr; 
}

HRESULT CMFTransformWrapper::SetInputType( 
            DWORD dwInputStreamID,
            /* [in] */ __RPC__in_opt IMFMediaType *pType,
            DWORD dwFlags)
{
    HRESULT hr = m_spWrappedTransform->SetInputType(dwInputStreamID, pType, dwFlags);
    m_pLogger->LogFormat("SetInputType(%d %p %d) returns %x\r\n", dwInputStreamID, pType, dwFlags, hr);
    m_pLogger->LogFormat("--> Arg(2, in) Media type: %s\r\n", CMediaTypeTrace(pType).GetString());
    
    return hr; 
}

HRESULT CMFTransformWrapper::SetOutputType( 
            DWORD dwOutputStreamID,
            /* [in] */ __RPC__in_opt IMFMediaType *pType,
            DWORD dwFlags)
{
    HRESULT hr = m_spWrappedTransform->SetOutputType(dwOutputStreamID, pType, dwFlags);
    m_pLogger->LogFormat("SetOutputType(%d %p %d) returns %x\r\n", dwOutputStreamID, pType, dwFlags, hr);
    m_pLogger->LogFormat("--> Arg(2, in) Media type: %s\r\n", CMediaTypeTrace(pType).GetString());
    
    return hr; 
}

HRESULT CMFTransformWrapper::GetInputCurrentType( 
            DWORD dwInputStreamID,
            /* [out] */ __RPC__deref_out_opt IMFMediaType **ppType)
{
    HRESULT hr = m_spWrappedTransform->GetInputCurrentType(dwInputStreamID, ppType);
    m_pLogger->LogFormat("GetInputCurrentType(%d %p) returns %x\r\n", dwInputStreamID, ppType, hr);
    if(SUCCEEDED(hr)) m_pLogger->LogFormat("--> Arg(2, out) Media type: %s\r\n", CMediaTypeTrace(*ppType).GetString());
    
    return hr; 
}

HRESULT CMFTransformWrapper::GetOutputCurrentType( 
            DWORD dwOutputStreamID,
            /* [out] */ __RPC__deref_out_opt IMFMediaType **ppType)
{
    HRESULT hr = m_spWrappedTransform->GetOutputCurrentType(dwOutputStreamID, ppType);
    m_pLogger->LogFormat("GetOutputCurrentType(%d %p) returns %x\r\n", dwOutputStreamID, ppType, hr);
    if(SUCCEEDED(hr)) m_pLogger->LogFormat("--> Arg(2, out) Media type: %s\r\n", CMediaTypeTrace(*ppType).GetString());
        
    return hr; 
}

HRESULT CMFTransformWrapper::GetInputStatus( 
            DWORD dwInputStreamID,
            /* [out] */ __RPC__out DWORD *pdwFlags)
{
    HRESULT hr = m_spWrappedTransform->GetInputStatus(dwInputStreamID, pdwFlags);
    m_pLogger->LogFormat("GetInputStatus(%d %p) returns %x\r\n", dwInputStreamID, pdwFlags, hr);
    if(SUCCEEDED(hr)) m_pLogger->LogFormat("--> Arg(2, out) Flags: %x\r\n", *pdwFlags);
    
    return hr;     
}

HRESULT CMFTransformWrapper::GetOutputStatus( 
            /* [out] */ __RPC__out DWORD *pdwFlags)
{
    HRESULT hr = m_spWrappedTransform->GetOutputStatus(pdwFlags);
    m_pLogger->LogFormat("GetOutputStatus(%p) returns %x\r\n", pdwFlags, hr);
    if(SUCCEEDED(hr)) m_pLogger->LogFormat("--> Arg(1, out) Flags: %x\r\n", *pdwFlags);
    
    return hr;     
}

HRESULT CMFTransformWrapper::SetOutputBounds( 
            LONGLONG hnsLowerBound,
            LONGLONG hnsUpperBound)
{
    HRESULT hr = m_spWrappedTransform->SetOutputBounds(hnsLowerBound, hnsUpperBound);
    m_pLogger->LogFormat("SetOutputBounds(%ld %ld) returns %x\r\n", hnsLowerBound, hnsUpperBound, hr);
    
    return hr; 
}

HRESULT CMFTransformWrapper::ProcessEvent( 
            DWORD dwInputStreamID,
            /* [in] */ __RPC__in_opt IMFMediaEvent *pEvent)
{
    HRESULT hr = m_spWrappedTransform->ProcessEvent(dwInputStreamID, pEvent);
    m_pLogger->LogFormat("ProcessEvent(%d %p) returns %x\r\n", dwInputStreamID, pEvent, hr);
    
    return hr; 
}

HRESULT CMFTransformWrapper::ProcessMessage( 
            MFT_MESSAGE_TYPE eMessage,
            ULONG_PTR ulParam)
{
    HRESULT hr = m_spWrappedTransform->ProcessMessage(eMessage, ulParam);
    m_pLogger->LogFormat("ProcessMessage(%d %p) returns %x\r\n", eMessage, ulParam, hr);
    
    return hr; 
}

HRESULT CMFTransformWrapper::ProcessInput( 
            DWORD dwInputStreamID,
            IMFSample *pSample,
            DWORD dwFlags)
{
    HRESULT hr = m_spWrappedTransform->ProcessInput(dwInputStreamID, pSample, dwFlags);
    m_pLogger->LogFormat("ProcessInput(%d %p %d) returns %x\r\n", dwInputStreamID, pSample, dwFlags, hr);
    
    return hr; 
}

HRESULT CMFTransformWrapper::ProcessOutput( 
    DWORD dwFlags,
    DWORD cOutputBufferCount,
    /* [size_is][out][in] */ MFT_OUTPUT_DATA_BUFFER *pOutputSamples,
    /* [out] */ DWORD *pdwStatus)
{
    HRESULT hr = m_spWrappedTransform->ProcessOutput(dwFlags, cOutputBufferCount, pOutputSamples, pdwStatus);
    m_pLogger->LogFormat("ProcessInput(%d %d %p %p) returns %x\r\n", dwFlags, cOutputBufferCount, pOutputSamples, pdwStatus, hr);
    
    return hr; 
}
