/////////////////////////////////////////////////////////////////////////////
//
// [!output root].cpp : Implementation of C[!output Safe_root]
//
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
[!if DUALMODE]
#include <initguid.h>
[!endif]
#include "[!output root].h"
#include "[!output Root]PropPage.h"
#include <mediaerr.h>   // DirectX SDK media errors
#include <dmort.h>      // DirectX SDK DMO runtime support
#include <amvideo.h>
#include <dvdmedia.h>
#include <uuids.h>
[!if DUALMODE]
#include <mfapi.h>
#include <mferror.h>
[!endif]

/////////////////////////////////////////////////////////////////////////////
// Media types supported by C[!output Safe_root]
/////////////////////////////////////////////////////////////////////////////

const GUID* C[!output Safe_root]::k_guidValidSubtypes[] =
{
    &MEDIASUBTYPE_NV12,
    &MEDIASUBTYPE_YV12,
    &MEDIASUBTYPE_YUY2,
    &MEDIASUBTYPE_UYVY,
    &MEDIASUBTYPE_RGB32,
    &MEDIASUBTYPE_RGB24,
    &MEDIASUBTYPE_RGB565,
    &MEDIASUBTYPE_RGB555
};

const DWORD C[!output Safe_root]::k_dwValidSubtypesCount = sizeof( C[!output Safe_root]::k_guidValidSubtypes ) / sizeof( C[!output Safe_root]::k_guidValidSubtypes[0] );

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::C[!output Safe_root]
//
// Constructor
/////////////////////////////////////////////////////////////////////////////

C[!output Safe_root]::C[!output Safe_root]()
{
    m_fScaleFactor = 0.0;   // default to a scale factor of 0.0 (zero saturation)
    m_bValidTime = false;
    m_bValidLength = false;
    m_rtTimestamp = 0;
    m_rtTimelength = 0;
    m_bEnabled = TRUE;
    m_dwBufferSize = 0;

    ::ZeroMemory(&m_mtInput, sizeof(m_mtInput));
    ::ZeroMemory(&m_mtOutput, sizeof(m_mtOutput));
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::~C[!output Safe_root]
//
// Destructor
/////////////////////////////////////////////////////////////////////////////

C[!output Safe_root]::~C[!output Safe_root]()
{
    ::MoFreeMediaType(&m_mtInput);
    ::MoFreeMediaType(&m_mtOutput);
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::FinalConstruct
//
// Called when an plug-in is first loaded. Use this function to do one-time
// intializations that could fail instead of doing this in the constructor,
// which cannot return an error.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::FinalConstruct()
{
    CRegKey key;
    LONG    lResult;
    DWORD   dwValue = 0;

    // read DWORD scale factor from registry and convert to double
    lResult = key.Open(HKEY_CURRENT_USER, kwszPrefsRegKey, KEY_READ);
    if (ERROR_SUCCESS == lResult)
    {
[!if VSNET]
        DWORD dwType = 0;
        ULONG uLength = sizeof(dwValue);
        lResult = key.QueryValue(kwszPrefsScaleFactor, &dwType, &dwValue, &uLength);
[!else]
        lResult = key.QueryValue(dwValue, kwszPrefsScaleFactor );
[!endif]
        if (ERROR_SUCCESS == lResult)
        {
            m_fScaleFactor = dwValue / 65536.0;
        }
    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]:::FinalRelease
//
// Called when an plug-in is unloaded. Use this function to free any
// resources allocated.
/////////////////////////////////////////////////////////////////////////////

void C[!output Safe_root]::FinalRelease()
{
    FreeStreamingResources();  // In case client does not call this.
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetStreamCount
//
// Implementation of IMediaObject::GetStreamCount
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::GetStreamCount( 
               DWORD *pcInputStreams,
               DWORD *pcOutputStreams)
{
    HRESULT hr = S_OK;

    if ( NULL == pcInputStreams )
    {
        return E_POINTER;
    }

    if ( NULL == pcOutputStreams )
    {
        return E_POINTER;
    }

    // The plug-in uses one stream in each direction.
    *pcInputStreams = 1;
    *pcOutputStreams = 1;

    return S_OK;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTGetStreamCount
//
// Implementation of IMFTransform::GetStreamCount
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::MFTGetStreamCount( 
               DWORD *pcInputStreams,
               DWORD *pcOutputStreams)
{
    HRESULT hr = S_OK;

    if ( NULL == pcInputStreams )
    {
        return E_POINTER;
    }

    if ( NULL == pcOutputStreams )
    {
        return E_POINTER;
    }

    // The plug-in uses one stream in each direction.
    *pcInputStreams = 1;
    *pcOutputStreams = 1;

    return S_OK;
}
[!endif]

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTGetStreamLimits
//
// Implementation of IMFTransform::GetStreamLimits
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::MFTGetStreamLimits(
               DWORD* pdwInputMinimum,
               DWORD* pdwInputMaximum,
               DWORD* pdwOutputMinimum,
               DWORD* pdwOutputMaximum)
{
    if ( NULL == pdwInputMinimum ||
         NULL == pdwInputMaximum ||
         NULL == pdwOutputMinimum ||
         NULL == pdwOutputMaximum )
    {
        return E_POINTER;
    }

    *pdwInputMinimum = 1;
    *pdwInputMaximum = 1;
    *pdwOutputMinimum = 1;
    *pdwOutputMaximum = 1;

    return S_OK;
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputStreamInfo
//
// Implementation of IMediaObject::GetInputStreamInfo
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetInputStreamInfo( 
               DWORD dwInputStreamIndex,
               DWORD *pdwFlags)
{    
    if ( NULL == pdwFlags )
    {
        return E_POINTER;
    }

    // The stream index must be zero.
    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    *pdwFlags = DMO_INPUT_STREAMF_HOLDS_BUFFERS |
                DMO_INPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER |
                DMO_INPUT_STREAMF_WHOLE_SAMPLES |
                DMO_INPUT_STREAMF_FIXED_SAMPLE_SIZE;

    return S_OK;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTGetInputStreamInfo
//
// Implementation of IMFTransform::GetInputStreamInfo
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::MFTGetInputStreamInfo( 
               DWORD dwInputStreamID,
               MFT_INPUT_STREAM_INFO* pStreamInfo)
{    
    // The stream index must be zero.
    if ( 0 != dwInputStreamID )
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if ( NULL == pStreamInfo )
    {
        return E_POINTER;
    }

    pStreamInfo->hnsMaxLatency = 0; // no buffering
    pStreamInfo->dwFlags = MFT_INPUT_STREAM_WHOLE_SAMPLES |
                           MFT_INPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER |
                           MFT_INPUT_STREAM_FIXED_SAMPLE_SIZE;
    pStreamInfo->cbSize = m_dwBufferSize;
    pStreamInfo->cbMaxLookahead = 0; // we don't lookahead
    pStreamInfo->cbAlignment = 0; // no alignment requirements

    return S_OK;
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetOutputStreamInfo
//
// Implementation of IMediaObject::GetOutputStreamInfo
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetOutputStreamInfo( 
               DWORD dwOutputStreamIndex,
               DWORD *pdwFlags)
{
    if ( NULL == pdwFlags )
    {
        return E_POINTER;
    }

    // The stream index must be zero.
    if ( 0 != dwOutputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    // Use the default output stream configuration (a single stream).
    *pdwFlags = 0;

    return S_OK;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTGetOutputStreamInfo
//
// Implementation of IMFTransform::GetOutputStreamInfo
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::MFTGetOutputStreamInfo( 
               DWORD dwOutputStreamID,
               MFT_OUTPUT_STREAM_INFO* pStreamInfo)
{
    if ( 0 != dwOutputStreamID )
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if ( NULL == pStreamInfo )
    {
        return E_POINTER;
    }

    // Populate the info structure.
    pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES |
                           MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER |
                           MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE;
    pStreamInfo->cbSize = m_dwBufferSize; // keep same size as input buffer
    pStreamInfo->cbAlignment = 0; // no alignment requirements

    return S_OK;
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputType
//
// Implementation of IMediaObject::GetInputType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetInputType ( 
               DWORD dwInputStreamIndex,
               DWORD dwTypeIndex,
               DMO_MEDIA_TYPE *pmt)
{
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX ;
    }

    // It is valid for pmt to be NULL. This is used to
    // pre-test whether the dwTypeIndex is in range.
    if( dwTypeIndex >= k_dwValidSubtypesCount )
    {
        return DMO_E_NO_MORE_ITEMS;
    }
    else if ( NULL == pmt ) 
    {
        return S_OK;
    }

    ::ZeroMemory( pmt, sizeof( DMO_MEDIA_TYPE ) );
    pmt->majortype = MEDIATYPE_Video;
    pmt->subtype = *k_guidValidSubtypes[dwTypeIndex];

    return hr;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTGetInputAvailableType
//
// Implementation of IMFTransform::GetInputAvailableType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::MFTGetInputAvailableType( 
               DWORD dwInputStreamID,
               DWORD dwTypeIndex,
               IMFMediaType** ppType)
{
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamID )
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // It is valid for pmt to be NULL. This is used to
    // pre-test whether the dwTypeIndex is in range.
    if( dwTypeIndex >= k_dwValidSubtypesCount )
    {
        return MF_E_NO_MORE_TYPES;
    }
    else if ( NULL == ppType ) 
    {
        return S_OK;
    }

    DMO_MEDIA_TYPE mt;
    ::ZeroMemory( &mt, sizeof( DMO_MEDIA_TYPE ) );
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = *k_guidValidSubtypes[dwTypeIndex];

    return CreateMFMediaType( &mt, ppType );
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetOutputType
//
// Implementation of IMediaObject::GetOutputType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetOutputType( 
               DWORD dwOutputStreamIndex,
               DWORD dwTypeIndex,
               DMO_MEDIA_TYPE *pmt)
{
    HRESULT hr = S_OK;

    if ( 0 != dwOutputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    // It is valid for pmt to be NULL. This is used to
    // pre-test whether the dwTypeIndex is in range.
    if( dwTypeIndex >= k_dwValidSubtypesCount )
    {
        return DMO_E_NO_MORE_ITEMS;
    }
    else if ( NULL == pmt ) 
    {
        return S_OK;
    }

    // if input type has been defined, use that as output type
    if (GUID_NULL != m_mtInput.majortype)
    {
        hr = ::MoCopyMediaType( pmt, &m_mtInput );
    }
    else // otherwise use default for this plug-in
    {
        ::ZeroMemory( pmt, sizeof( DMO_MEDIA_TYPE ) );
        pmt->majortype = MEDIATYPE_Video;
        pmt->subtype = *k_guidValidSubtypes[dwTypeIndex];     
    }

    return hr;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTGetOutputAvailableType
//
// Implementation of IMFTransform::GetOutputAvailableType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::MFTGetOutputAvailableType( 
               DWORD dwOutputStreamID,
               DWORD dwTypeIndex,
               IMFMediaType** ppType)
{
    HRESULT hr = S_OK;

    if ( 0 != dwOutputStreamID )
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // It is valid for pmt to be NULL. This is used to
    // pre-test whether the dwTypeIndex is in range.
    if( dwTypeIndex >= k_dwValidSubtypesCount )
    {
        return MF_E_NO_MORE_TYPES;
    }
    else if ( NULL == ppType ) 
    {
        return S_OK;
    }

    // if input type has been defined, use that as output type
    DMO_MEDIA_TYPE mt;
    if (GUID_NULL != m_mtInput.majortype)
    {
        hr = ::MoCopyMediaType( &mt, &m_mtInput );
        if ( FAILED( hr ) )
        {
            return hr;
        }
    }
    else // otherwise use default for this plug-in
    {
        ::ZeroMemory( &mt, sizeof( DMO_MEDIA_TYPE ) );
        mt.majortype = MEDIATYPE_Video;
        mt.subtype = *k_guidValidSubtypes[dwTypeIndex];
    }

    hr = CreateMFMediaType( &mt, ppType );

    MoFreeMediaType( &mt );

    return hr;
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetInputType
//
// Implementation of IMediaObject::SetInputType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::SetInputType( 
               DWORD dwInputStreamIndex,
               const DMO_MEDIA_TYPE *pmt,
               DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( DMO_SET_TYPEF_CLEAR & dwFlags ) 
    {
        ::MoFreeMediaType(&m_mtInput);
        ::ZeroMemory(&m_mtInput, sizeof(m_mtInput));

        return S_OK;
    }

    if ( NULL == pmt )
    {
       return E_POINTER;
    }

    // validate that the input media type matches our requirements and
    // and matches our output type (if set)
    hr = ValidateMediaType(pmt, &m_mtOutput);

    if ( FAILED( hr ) )
    {
        if( DMO_SET_TYPEF_TEST_ONLY & dwFlags )
        {
            hr = S_FALSE;
        }
        else
        {
            hr = DMO_E_TYPE_NOT_ACCEPTED;
        }
    }
    else if ( 0 == dwFlags )
    {
        // free existing media type
        ::MoFreeMediaType(&m_mtInput);
        ::ZeroMemory(&m_mtInput, sizeof(m_mtInput));

        // copy new media type
        hr = ::MoCopyMediaType( &m_mtInput, pmt );

        m_dwBufferSize = GetSampleSize( &m_mtInput );
    }

    return hr;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTSetInputType
//
// Implementation of IMFTransform::SetInputType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::MFTSetInputType( 
               DWORD dwInputStreamID,
               IMFMediaType* pType,
               DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamID )
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if ( NULL == pType )
    {
        ::MoFreeMediaType(&m_mtInput);
        ::ZeroMemory(&m_mtInput, sizeof(m_mtInput));

        return S_OK;
    }

    DMO_MEDIA_TYPE* pmt;
    hr = pType->GetRepresentation( AM_MEDIA_TYPE_REPRESENTATION, (void**)&pmt );
    if( FAILED( hr ) ) {
        return hr;
    }

    // validate that the input media type matches our requirements and
    // and matches our output type (if set)
    hr = ValidateMediaType(pmt, &m_mtOutput);

    if ( FAILED( hr ) )
    {
        hr = MF_E_INVALIDMEDIATYPE;
    }
    else if ( 0 == (MFT_SET_TYPE_TEST_ONLY & dwFlags) )
    {
        // free existing media type
        ::MoFreeMediaType(&m_mtInput);
        ::ZeroMemory(&m_mtInput, sizeof(m_mtInput));

        // copy new media type
        hr = ::MoCopyMediaType( &m_mtInput, pmt );

        m_dwBufferSize = GetSampleSize( &m_mtInput );
    }

    pType->FreeRepresentation( AM_MEDIA_TYPE_REPRESENTATION, pmt );

    return hr;
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetOutputType
//
// Implementation of IMediaObject::SetOutputType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::SetOutputType( 
               DWORD dwOutputStreamIndex,
               const DMO_MEDIA_TYPE *pmt,
               DWORD dwFlags)
{ 
    HRESULT hr = S_OK;

    if ( 0 != dwOutputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if( DMO_SET_TYPEF_CLEAR & dwFlags )
    {
        ::MoFreeMediaType( &m_mtOutput );
        ::ZeroMemory(&m_mtOutput, sizeof(m_mtOutput));

        return S_OK;
    }

    if ( NULL == pmt )
    {
        return E_POINTER;
    }

    if( GUID_NULL != m_mtInput.majortype )
    {
        // validate that the output media type matches our requirements and
        // and matches our input type (if set)
        hr = ValidateMediaType(pmt, &m_mtInput);
    }
    else
    {
        hr = DMO_E_TYPE_NOT_ACCEPTED;
    }

    if (FAILED(hr))
    {
        if( DMO_SET_TYPEF_TEST_ONLY & dwFlags )
        {
            hr = S_FALSE;
        }
        else
        {
            hr = DMO_E_TYPE_NOT_ACCEPTED;
        }
    }
    else if ( 0 == dwFlags )
    {
        // free existing media type
        ::MoFreeMediaType(&m_mtOutput);
        ::ZeroMemory(&m_mtOutput, sizeof(m_mtOutput));

        // copy new media type
        hr = ::MoCopyMediaType( &m_mtOutput, pmt );
    }

    return hr;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTSetOutputType
//
// Implementation of IMFTransform::SetOutputType
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::MFTSetOutputType( 
               DWORD dwOutputStreamID,
               IMFMediaType* pType,
               DWORD dwFlags)
{ 
    HRESULT hr = S_OK;

    if ( 0 != dwOutputStreamID )
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if( NULL == pType )
    {
        ::MoFreeMediaType( &m_mtOutput );
        ::ZeroMemory(&m_mtOutput, sizeof(m_mtOutput));

        return S_OK;
    }

    DMO_MEDIA_TYPE* pmt;
    hr = pType->GetRepresentation( AM_MEDIA_TYPE_REPRESENTATION, (void**)&pmt );
    if( FAILED( hr ) ) {
        return hr;
    }

    if( GUID_NULL != m_mtInput.majortype )
    {
        // validate that the output media type matches our requirements and
        // and matches our input type (if set)
        hr = ValidateMediaType(pmt, &m_mtInput);
        if( FAILED( hr ) ) {
            hr = MF_E_INVALIDMEDIATYPE;
        }
    }
    else
    {
        hr = MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    if ( SUCCEEDED( hr ) && 0 == (MFT_SET_TYPE_TEST_ONLY & dwFlags) )
    {
        // free existing media type
        ::MoFreeMediaType(&m_mtOutput);
        ::ZeroMemory(&m_mtOutput, sizeof(m_mtOutput));

        // copy new media type
        hr = ::MoCopyMediaType( &m_mtOutput, pmt );
    }

    pType->FreeRepresentation( AM_MEDIA_TYPE_REPRESENTATION, pmt );

    return hr;
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputCurrentType
//
// Implementation of IMediaObject::GetInputCurrentType
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::GetInputCurrentType( 
               DWORD dwInputStreamIndex,
               DMO_MEDIA_TYPE *pmt)
{
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( NULL == pmt )
    {
        return E_POINTER;
    }

    if (GUID_NULL == m_mtInput.majortype)
    {
        return DMO_E_TYPE_NOT_SET;
    }

    ::MoFreeMediaType( pmt );
    hr = ::MoCopyMediaType( pmt, &m_mtInput );

    return hr;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTGetInputCurrentType
//
// Implementation of IMFTransform::GetInputCurrentType
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::MFTGetInputCurrentType( 
               DWORD dwInputStreamID,
               IMFMediaType** ppType)
{
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamID )
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if ( NULL == ppType )
    {
        return E_POINTER;
    }

    if (GUID_NULL == m_mtInput.majortype)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    return CreateMFMediaType( &m_mtInput, ppType );
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetOutputCurrentType
//
// Implementation of IMediaObject::GetOutputCurrentType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetOutputCurrentType( 
               DWORD dwOutputStreamIndex,
               DMO_MEDIA_TYPE *pmt)
{
    HRESULT hr = S_OK;

    if ( 0 != dwOutputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( NULL == pmt )
    {
        return E_POINTER;
    }

    if (GUID_NULL == m_mtOutput.majortype)
    {
        return DMO_E_TYPE_NOT_SET;
    }

    ::MoFreeMediaType( pmt );
    hr = ::MoCopyMediaType( pmt, &m_mtOutput );

    return hr;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTGetOutputCurrentType
//
// Implementation of IMFTransform::GetOutputCurrentType
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::MFTGetOutputCurrentType(
               DWORD dwOutputStreamID,
               IMFMediaType** ppType)
{
    HRESULT hr = S_OK;

    if ( 0 != dwOutputStreamID )
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if ( NULL == ppType )
    {
        return E_POINTER;
    }

    if (GUID_NULL == m_mtOutput.majortype)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    return CreateMFMediaType( &m_mtOutput, ppType );
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputSizeInfo
//
// Implementation of IMediaObject::GetInputSizeInfo
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetInputSizeInfo( 
               DWORD dwInputStreamIndex,
               DWORD *pcbSize,
               DWORD *pcbMaxLookahead,
               DWORD *pcbAlignment)
{
    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( NULL == pcbSize )
    {
       return E_POINTER;
    }

    if ( NULL == pcbMaxLookahead )
    {
        return E_POINTER;
    }

    if ( NULL == pcbAlignment )
    {
       return E_POINTER;
    }

    if (GUID_NULL == m_mtInput.majortype)
    {
        return DMO_E_TYPE_NOT_SET;
    }

    // Return the input sample size, in bytes.
    *pcbSize = m_dwBufferSize;

    // No lookahead for this plug-in.
    *pcbMaxLookahead = 0;

     // No alignment requirement.
    *pcbAlignment = 1;
  
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetOutputSizeInfo
//
// Implementation of IMediaObject::GetOutputSizeInfo
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetOutputSizeInfo( 
               DWORD dwOutputStreamIndex,
               DWORD *pcbSize,
               DWORD *pcbAlignment)
{
    if ( 0 != dwOutputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( NULL == pcbSize )
    {
        return E_POINTER;
    }

    if ( NULL == pcbAlignment )
    {
        return E_POINTER;
    }

    if (GUID_NULL == m_mtOutput.majortype)
    {
        return DMO_E_TYPE_NOT_SET;
    }

    // Return the output sample size, in bytes.
    *pcbSize = m_dwBufferSize;

     // No alignment requirement.
    *pcbAlignment = 1;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputMaxLatency
//
// Implementation of IMediaObject::GetInputMaxLatency
/////////////////////////////////////////////////////////////////////////////
   
STDMETHODIMP C[!output Safe_root]::GetInputMaxLatency( 
               DWORD dwInputStreamIndex,
               REFERENCE_TIME *prtMaxLatency)
{
    return E_NOTIMPL; // Not dealing with latency in this plug-in.
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetInputMaxLatency
//
// Implementation of IMediaObject::SetInputMaxLatency
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::SetInputMaxLatency( 
               DWORD dwInputStreamIndex,
               REFERENCE_TIME rtMaxLatency)
{
    return E_NOTIMPL; // Not dealing with latency in this plug-in.
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Flush
//
// Implementation of IMediaObject::Flush
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::Flush( void )
{
    m_spInputBuffer.Release();  // release smart pointer
[!if DUALMODE]
    m_spMFSample.Release();
[!endif]
    m_bValidTime = false;
    m_bValidLength = false;
    m_rtTimestamp = 0;
    m_rtTimelength = 0;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Discontinuity
//
// Implementation of IMediaObject::Discontinuity
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::Discontinuity( 
               DWORD dwInputStreamIndex)
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::AllocateStreamingResources
//
// Implementation of IMediaObject::AllocateStreamingResources
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::AllocateStreamingResources ( void )
{
    // Allocate any buffers need to process the stream. This plug-in does
    // all processing in-place, so it requires no extra buffers.

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::FreeStreamingResources
//
// Implementation of IMediaObject::FreeStreamingResources
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::FreeStreamingResources( void )
{
    m_spInputBuffer.Release(); // release smart pointer
    m_bValidTime = false;
    m_bValidLength = false;
    m_rtTimestamp = 0;
    m_rtTimelength = 0;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetInputStatus
//
// Implementation of IMediaObject::GetInputStatus
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::GetInputStatus( 
           DWORD dwInputStreamIndex,
           DWORD *pdwFlags)
{ 
    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( NULL == pdwFlags )
    {
        return E_POINTER;
    }

    if ( m_spInputBuffer )
    {
        *pdwFlags = 0; //The buffer still contains data; return zero.
    }
    else
    {
        *pdwFlags = DMO_INPUT_STATUSF_ACCEPT_DATA; // OK to call ProcessInput.
    }

    return S_OK;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTGetInputStatus
//
// Implementation of IMFTransform::GetInputStatus
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::MFTGetInputStatus(
           DWORD dwInputStreamID,
           DWORD* pdwFlags)
{ 
    if ( 0 != dwInputStreamID )
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if ( NULL == pdwFlags )
    {
        return E_POINTER;
    }

    if ( m_spMFSample )
    {
        *pdwFlags = 0; //The buffer still contains data; return zero.
    }
    else
    {
        *pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA; // OK to call ProcessInput.
    }

    return S_OK;
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::ProcessInput
//
// Implementation of IMediaObject::ProcessInput
/////////////////////////////////////////////////////////////////////////////
    
STDMETHODIMP C[!output Safe_root]::ProcessInput( 
               DWORD dwInputStreamIndex,
               IMediaBuffer *pBuffer,
               DWORD dwFlags,
               REFERENCE_TIME rtTimestamp,
               REFERENCE_TIME rtTimelength)
{ 
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamIndex )
    {
        return DMO_E_INVALIDSTREAMINDEX;
    }

    if ( NULL == pBuffer )
    {
        return E_POINTER;
    }

    if ( GUID_NULL == m_mtInput.majortype )
    {
        return DMO_E_TYPE_NOT_SET;
    }

    if ( m_spInputBuffer )
    {
        // Still holding a buffer for processing.
        return DMO_E_NOTACCEPTING;
    }

    // Get a pointer to the actual data and length information.
    BYTE    *pbInputData = NULL;
    DWORD   cbInputLength = 0;

    hr = pBuffer->GetBufferAndLength(&pbInputData, &cbInputLength);

    if ( SUCCEEDED( hr ) )
    {
        // Hold on to the buffer using a smart pointer.
        m_spInputBuffer = pBuffer;

        //Verify that buffer's time stamp is valid.
        if ( dwFlags & DMO_INPUT_DATA_BUFFERF_TIME )
        {
            m_bValidTime = true;
            m_rtTimestamp = rtTimestamp;

        }
        else
        {
            m_bValidTime = false;
        }

        if( dwFlags & DMO_INPUT_DATA_BUFFERF_TIMELENGTH )
        {
            m_bValidLength = true;
            m_rtTimelength = rtTimelength;
        }
        else
        {
            m_bValidLength = false;
        }
    }
 
    return hr;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTProcessInput
//
// Implementation of IMFTransform::ProcessInput
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::MFTProcessInput( 
               DWORD dwInputStreamID,
               IMFSample* pSample,
               DWORD dwFlags)
{ 
    HRESULT hr = S_OK;

    if ( 0 != dwInputStreamID )
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if ( NULL == pSample )
    {
        return E_POINTER;
    }

    if ( GUID_NULL == m_mtInput.majortype )
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    if ( m_spMFSample )
    {
        // Still holding a buffer for processing.
        return MF_E_NOTACCEPTING;
    }

    m_spMFSample = pSample;

    return hr;
}
[!endif]   


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::ProcessOutput
//
// Implementation of IMediaObject::ProcessOutput
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::ProcessOutput( 
               DWORD dwFlags,
               DWORD cOutputBufferCount,
               DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
               DWORD *pdwStatus)
{
    HRESULT hr = S_OK;

    if ( NULL == pOutputBuffers )
    {
        return E_POINTER;
    }

    // this plug-in only supports one output buffer
    if (1 != cOutputBufferCount)
    {
        return E_INVALIDARG;
    }

    if (GUID_NULL == m_mtOutput.majortype)
    {
        return DMO_E_TYPE_NOT_SET;
    }

    if (pdwStatus)
    {
        *pdwStatus = 0;
    }

    // Make sure input and output buffers exist.
    IMediaBuffer *pOutputMediaBuffer = pOutputBuffers[0].pBuffer;

    if ((!m_spInputBuffer.p) || (!pOutputMediaBuffer))
    {
        if (pOutputMediaBuffer)
        {
            pOutputMediaBuffer->SetLength(0);
        }

        pOutputBuffers[0].dwStatus = 0;

        return S_FALSE;
    }

    // Set up the output buffer.
    pOutputBuffers[0].pBuffer->SetLength( m_dwBufferSize );
    pOutputBuffers[0].dwStatus = DMO_OUTPUT_DATA_BUFFERF_TIME | DMO_OUTPUT_DATA_BUFFERF_TIMELENGTH;

    if( m_bValidTime )
    {
        pOutputBuffers[0].rtTimestamp = m_rtTimestamp;
    }

    if( m_bValidLength )
    {
        pOutputBuffers[0].rtTimelength = m_rtTimelength;
    }

    BYTE         *pbOutputData = NULL;
    BYTE         *pbInputData  = NULL;
    DWORD        cbOutputLength = 0;
    DWORD        cbInputLength = 0;
    DWORD        cbBytesProcessed = 0;

    // Get a pointer to the output buffer.
    hr = pOutputMediaBuffer->GetBufferAndLength( &pbOutputData, &cbOutputLength);
    if (FAILED(hr))
    {
        return hr;
    }

    // Get a pointer to the input buffer.
    hr = m_spInputBuffer->GetBufferAndLength(&pbInputData, &cbInputLength);
    if (FAILED(hr))
    {
        return hr;
    }

    // Compare the input buffer to the output buffer.
    // If they aren't the same size or don't match the
    // expected buffer size, return an error.
    if( ( cbOutputLength != cbInputLength ) ||
        ( cbOutputLength != m_dwBufferSize ) ||
        ( cbInputLength  != m_dwBufferSize ) )
    {
        return E_INVALIDARG;
    }
 
    // Call the internal processing method, which returns the no. bytes processed
    hr = DoProcessOutput(pbOutputData, pbInputData, &cbBytesProcessed);
    if (FAILED(hr))
    {
        return hr;
    }

    m_spInputBuffer.Release();   // Release smart pointer
    m_bValidTime = false;
    m_bValidLength = false;
    m_rtTimestamp = 0;
    m_rtTimelength = 0;
 
    return S_OK;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTProcessOutput
//
// Implementation of IMFTransform::ProcessOutput
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::MFTProcessOutput( 
               DWORD dwFlags,
               DWORD cOutputBufferCount,
               MFT_OUTPUT_DATA_BUFFER* pOutputSamples,
               DWORD* pdwStatus)
{
    HRESULT hr = S_OK;

    if (1 != cOutputBufferCount)
    {
        return E_INVALIDARG;
    }

    if ( NULL == pOutputSamples )
    {
        return E_POINTER;
    }

    if (pdwStatus)
    {
        *pdwStatus = 0;
    }

    if (GUID_NULL == m_mtOutput.majortype)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    if (!m_spMFSample.p)
    {
        return MF_E_TRANSFORM_NEED_MORE_INPUT;
    }

    IMFSample* pOutputSample = pOutputSamples[0].pSample;
    if (!pOutputSample)
    {
        return E_INVALIDARG;
    }

    IMFMediaBuffer* pInputBuffer = NULL;
    IMFMediaBuffer* pOutputBuffer = NULL;
    BYTE         *pbOutputData = NULL;
    BYTE         *pbInputData  = NULL;
    DWORD        cbOutputLength = 0;
    DWORD        cbInputLength = 0;
    DWORD        cbInputSize = 0;
    DWORD        cbBytesProcessed = 0;

    do {
        hr = m_spMFSample->ConvertToContiguousBuffer( &pInputBuffer );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pOutputSample->ConvertToContiguousBuffer( &pOutputBuffer );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pInputBuffer->Lock( &pbInputData, &cbInputLength, &cbInputSize);
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pOutputBuffer->Lock( &pbOutputData, &cbOutputLength, NULL);
        if( FAILED( hr ) )
        {
            break;
        }

        if( cbOutputLength < cbInputSize )
        {
            hr = MF_E_BUFFERTOOSMALL;
            break;
        }

        // Call the internal processing method, which returns the no. bytes processed
        hr = DoProcessOutput(pbOutputData, pbInputData, &cbBytesProcessed);
        if (FAILED(hr))
        {
            break;
        }

        pOutputBuffer->SetCurrentLength( cbBytesProcessed );

        // Propagate the timestamp.
        LONGLONG llTimeStamp = 0;
        m_spMFSample->GetSampleTime(&llTimeStamp);
        pOutputSample->SetSampleTime(llTimeStamp);

        // Propagate the duration.
        LONGLONG llDuration = 0;
        m_spMFSample->GetSampleDuration(&llDuration);
        pOutputSample->SetSampleDuration(llDuration);

        m_spMFSample.Release();
    } while (false);

    if( pbInputData != NULL )
    {
        pInputBuffer->Unlock();
    }
    if( pbOutputData != NULL )
    {
        pOutputBuffer->Unlock();
    }
    if( pInputBuffer != NULL )
    {
        pInputBuffer->Release();
    }
    if( pOutputBuffer != NULL )
    {
        pOutputBuffer->Release();
    }
 
    return hr;
}
[!endif]

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::MFTProcessMessage
//
// Implementation of IMFTransform::ProcessMessage
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::MFTProcessMessage( MFT_MESSAGE_TYPE eMessage, ULONG_PTR ulParam ) {
    switch (eMessage) {
        case MFT_MESSAGE_COMMAND_FLUSH:
            return Flush();
    }

    return S_OK;
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Lock
//
// Implementation of IMediaObject::Lock
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::Lock( LONG bLock )
{
    if( bLock )
    {
        Lock();
    }
    else
    {
        Unlock();
    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetEnable
//
// Implementation of IWMPPluginEnable::SetEnable
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::SetEnable( BOOL fEnable )
{
    // This function allows any state or UI associated with the plug-in to reflect the
    // enabled/disable state of the plug-in

    m_bEnabled = fEnable;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetEnable
//
// Implementation of IWMPPluginEnable::GetEnable
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::GetEnable( BOOL *pfEnable )
{
    if ( NULL == pfEnable )
    {
        return E_POINTER;
    }

    *pfEnable = m_bEnabled;

    return S_OK;
}


[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetService
//
// Implementation of IMFGetService::GetService
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP C[!output Safe_root]::GetService( REFGUID guidService, REFIID riid, LPVOID* ppvObject ) {
    // WMP uses the CLSID of the plugin as the service identifier.
    if( guidService == CLSID_[!output Safe_root] )
    {
        // MF wants an interface from us. However, since all supported interfaces
        // are implemented on this object, just do a QI.
        return QueryInterface( riid, ppvObject );
    }

    return MF_E_UNSUPPORTED_SERVICE;
}
[!endif]


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetPages
//
// Implementation of ISpecifyPropertyPages::GetPages
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::GetPages(CAUUID *pPages)
{
    if( NULL == pPages )
    { 
        return E_POINTER;
    }

    // Only one property page is required for the plug-in.
    pPages->cElems = 1;
    pPages->pElems = (GUID *) (CoTaskMemAlloc(sizeof(GUID)));

    // Make sure memory is allocated for pPages->pElems
    if (NULL == pPages->pElems)
    {
        return E_OUTOFMEMORY;
    }

    // Return the property page's class ID
    *(pPages->pElems) = CLSID_[!output Safe_root]PropPage;

    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::DoProcessOutput
//
// Convert the input buffer to the output buffer
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::DoProcessOutput(
                            BYTE *pbOutputData,
                            const BYTE *pbInputData,
                            DWORD *cbBytesProcessed)
{
    HRESULT hr = S_OK;
    // Test whether the plug-in has been disabled by the user.
    if (!m_bEnabled)
    {
        // Just copy the data without changing it. You should
        // also do any neccesary format conversion here.
        memcpy(pbOutputData, pbInputData, m_dwBufferSize);
        *cbBytesProcessed = m_dwBufferSize;
    
        return S_OK;
    }

    // Determine the video format for processing.

    if ( m_mtInput.subtype == MEDIASUBTYPE_NV12 )
    {
        hr = ProcessNV12( (BYTE *)pbInputData, (BYTE *)pbOutputData );
    }
    else if( m_mtInput.subtype == MEDIASUBTYPE_YV12 )
    {
        hr = ProcessYV12( (BYTE *)pbInputData, (BYTE *)pbOutputData );
    }
    else if( m_mtInput.subtype == MEDIASUBTYPE_YUY2 )
    {
        hr = ProcessYUY2( (BYTE *)pbInputData, (BYTE *)pbOutputData );
    }
    else if( m_mtInput.subtype == MEDIASUBTYPE_UYVY )
    {
        hr = ProcessUYVY( (BYTE *)pbInputData, (BYTE *)pbOutputData );
    }
    else if( m_mtInput.subtype == MEDIASUBTYPE_RGB24 )
    {
        hr = Process24Bit( (BYTE *)pbInputData, (BYTE *)pbOutputData );
    }
    else if( m_mtInput.subtype == MEDIASUBTYPE_RGB32 )
    {
        hr = Process32Bit( (BYTE *)pbInputData, (BYTE *)pbOutputData );
    }
    else if( m_mtInput.subtype == MEDIASUBTYPE_RGB555 )
    {
        hr = Process555( (BYTE *)pbInputData, (BYTE *)pbOutputData );
    }
    else if( m_mtInput.subtype == MEDIASUBTYPE_RGB565 )
    {
        hr = Process565( (BYTE *)pbInputData, (BYTE *)pbOutputData );
    }
    else
    {
        ATLTRACE( "Invalid subtype.\n" );
        hr = E_INVALIDARG;
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::ValidateMediaType
//
// Validate that the media type is acceptable
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::ValidateMediaType(const DMO_MEDIA_TYPE *pmtTarget, const DMO_MEDIA_TYPE *pmtPartner)
{
    HRESULT hr = S_OK;

    if ( NULL == pmtTarget ||
         NULL == pmtPartner )
    {
        hr = E_POINTER;
    }

    // Make sure the target media type has the fields we require
    if( MEDIATYPE_Video != pmtTarget->majortype ||
        ( ( FORMAT_VideoInfo != pmtTarget->formattype ) &&
          ( FORMAT_VideoInfo2 != pmtTarget->formattype ) ) )
    {
        hr = DMO_E_TYPE_NOT_ACCEPTED;
    }

    DWORD cSubTypes = k_dwValidSubtypesCount;
    bool bValid = false;

    // Validate the header fields.
    BITMAPINFOHEADER bmiHeaderTarget;
    RECT rcTarget = { 0 };

    if( SUCCEEDED( hr ) )
    {
        hr = GetBitmapInfoHeader( pmtTarget, &bmiHeaderTarget );
    }

    if(SUCCEEDED(hr))
    {
        // Make sure the subtype is a supported format.
        while( cSubTypes-- )
        {
            if( *k_guidValidSubtypes[cSubTypes] == pmtTarget->subtype  )
            {
                if( MEDIASUBTYPE_YV12 == pmtTarget->subtype || MEDIASUBTYPE_NV12 == pmtTarget->subtype )
                {                  
                    if( bmiHeaderTarget.biBitCount != 12 )
                    {
                        bValid = false;
                        break;
                    }                    
                }

                bValid = true;
                break;
            }
        }
    }

    if( !bValid )
    {
        hr = DMO_E_TYPE_NOT_ACCEPTED;
    }
    
    if( SUCCEEDED( hr ) )
    {
        if( 0 == bmiHeaderTarget.biHeight ||
            0 == bmiHeaderTarget.biWidth ||
            sizeof( BITMAPINFOHEADER ) != bmiHeaderTarget.biSize )
        {
            hr = DMO_E_TYPE_NOT_ACCEPTED;
        }
    }

    if( SUCCEEDED( hr ) )
    {   
        hr = GetTargetRECT( pmtTarget, &rcTarget );
    }

    if( SUCCEEDED( hr ) )
    {
        if( ( bmiHeaderTarget.biWidth < ( rcTarget.right - rcTarget.left ) ) ||
            ( abs ( bmiHeaderTarget.biHeight ) < ( rcTarget.bottom - rcTarget.top ) ) )
        {
            hr = DMO_E_TYPE_NOT_ACCEPTED;
        }
    }

    // If the partner media type is configured, make sure it matches the target.
    // This plug-in requires the same input and output types.
    if( SUCCEEDED( hr ) )
    {
        if ( GUID_NULL != pmtPartner->majortype )
        {
            if ( ( pmtTarget->majortype != pmtPartner->majortype ) ||
                 ( pmtTarget->subtype   != pmtPartner->subtype ) ||
                 ( pmtTarget->formattype != pmtPartner->formattype ) ||
                 ( pmtTarget->lSampleSize != pmtPartner->lSampleSize ) )
            {
                hr = DMO_E_TYPE_NOT_ACCEPTED;
            }

            BITMAPINFOHEADER bmiHeaderPartner;

            hr = GetBitmapInfoHeader( pmtPartner, &bmiHeaderPartner );
            
            if( SUCCEEDED( hr ) )
            {
                if( ( bmiHeaderTarget.biWidth != bmiHeaderPartner.biWidth ) ||
                    ( abs( bmiHeaderTarget.biHeight ) != abs( bmiHeaderPartner.biHeight ) ) )
                {
                    hr = DMO_E_TYPE_NOT_ACCEPTED;
                }
            }
            else
            {
                hr = E_UNEXPECTED;
            }
        }
    }
  
    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::get_scale
//
// Property get to retrieve the scale value via the public interface.
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::get_scale(double *pVal)
{
    if ( NULL == pVal )
    {
        return E_POINTER;
    }

    *pVal = m_fScaleFactor;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::put_scale
//
// Property put to store the scale value via the public interface.
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::put_scale(double newVal)
{
    m_fScaleFactor = newVal;
    
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Read
//
// Implementation of IPropertyBag::Read.
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::Read( LPCWSTR pwszPropName, VARIANT *pVar, IErrorLog *pErrorLog )
{
    HRESULT hr = S_OK;
    CComPtr<IStream> pStream;

    if( NULL == pVar )
    {
        return E_POINTER;
    }

    if ( 0 == _wcsicmp( pwszPropName, L"IconStreams" ) )
    {
        // Get the image and load into an IStream.
        hr = LoadResourceImage(&pStream);

        if ( FAILED( hr ) )
        {
            return hr;
        }
        
        // There is only one image. Otherwise, set to VT_ARRAY.
        pVar->vt = VT_UNKNOWN;
        // Return the IStream pointer.
        pVar->punkVal = pStream;
        pVar->punkVal->AddRef();
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// CC[!output Safe_root]::Write
//
// Implementation of IPropertyBag::Write
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::Write( LPCWSTR pwszPropName, VARIANT *pVar )
{
    // E_NOTIMPL is not a valid return code for this method.
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::LoadResourceImage
//
// Load the plug-in image into an IStream and return a pointer.
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP C[!output Safe_root]::LoadResourceImage( IStream **ppStream )
{
    LPVOID pvSrcData   = NULL;
    LPVOID pvDestData   = NULL;
    HRSRC hRsrc = NULL;
    HGLOBAL hGlobal = NULL;
    HGLOBAL hgRawBytes = NULL;
    DWORD dwSize    = 0;
    CComPtr<IStream> spStream;
    HINSTANCE hInst = NULL;
    HRESULT hr = S_OK;

    if( NULL == ppStream )
    {
        return E_POINTER;
    }

    *ppStream = NULL;

    // Get a handle to the plug-in module.
    hInst = _Module.GetResourceInstance();
    if ( NULL == hInst )
    {
        hr = E_HANDLE;
    }

    if( SUCCEEDED( hr ) )
    {
        // Find the resource.
        hRsrc = ::FindResource( hInst, MAKEINTRESOURCE( IDR_ICON ), L"JPG" );
        if ( NULL == hRsrc )
        {
            hr = E_HANDLE;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Determine the size of the resource.
        dwSize = ::SizeofResource( hInst, hRsrc );
        if ( 0 == dwSize )
        {
            hr = E_HANDLE;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Load the resource.
        hGlobal = ::LoadResource( hInst, hRsrc );
        if ( NULL == hGlobal )
        {
            hr = E_HANDLE;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Get the pointer to the image data
        pvSrcData = ::LockResource( hGlobal );
        if ( NULL == pvSrcData )
        {
            hr = E_POINTER;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Allocate enough memory before creating stream
        hgRawBytes = ::GlobalAlloc( GMEM_MOVEABLE |GMEM_NODISCARD | GMEM_ZEROINIT, dwSize );
        if ( NULL == hgRawBytes )
        {
            hr = E_HANDLE;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Get a pointer to the first byte in the buffer.
        pvDestData = ::GlobalLock( hgRawBytes );
        if ( NULL == pvDestData )
        {
            hr = E_POINTER;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        // Copy the data to the buffer.
        memcpy( pvDestData, pvSrcData, dwSize );

        // Create a stream for the data.
        hr = CreateStreamOnHGlobal( hgRawBytes, TRUE, &spStream );
        ::GlobalUnlock( hgRawBytes );
    }

    if ( SUCCEEDED( hr ) &&
         NULL != spStream.p )
    {
        // Set the size for the stream.
        ULARGE_INTEGER ulSize;
        ulSize.QuadPart = dwSize;
        spStream.p->SetSize( ulSize );

        // Return the stream.
        spStream.p->AddRef();
        *ppStream = spStream.p;
    }
    else
    {
        // Free up memory if the stream can't be created.
        // Otherwise IStream owns this memory.
        // It is not necessary to unlock resources because
        // the system automatically delets them when the process
        // that created them terminates.
        if ( hgRawBytes )
        {
            ::GlobalFree( hgRawBytes );
            hgRawBytes = NULL;
        }
    }

    return hr;
}


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetSampleSize
//
// Get the sample size from the media type.
/////////////////////////////////////////////////////////////////////////////

DWORD C[!output Safe_root]::GetSampleSize( const DMO_MEDIA_TYPE *pmt )
{
    DWORD dwSize = 0;

    if( NULL == pmt )
    {
        return E_POINTER;
    }

    if( FORMAT_VideoInfo == pmt->formattype )
    {
        VIDEOINFOHEADER *pvih;
        
        pvih = (VIDEOINFOHEADER *)pmt->pbFormat;

        dwSize = DIBSIZE( pvih->bmiHeader );
    }
    else if( FORMAT_VideoInfo2 == pmt->formattype )
    {
        VIDEOINFOHEADER2 *pvih2;

        pvih2 = (VIDEOINFOHEADER2 *)pmt->pbFormat;

        dwSize = DIBSIZE( pvih2->bmiHeader );
    }
    else
    {
        ATLASSERT( false );
    }

    return dwSize;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetVideoInfoParameters
//
// Helper function to get the important information out of a VIDEOINFOHEADER.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::GetVideoInfoParameters(
    BYTE  * const pbData,    // Pointer to the first address in the buffer.
    DWORD *pdwWidth,         // Returns the width in bytes.
    DWORD *pdwHeight,        // Returns the height in pixels.
    LONG  *plStrideInBytes,  // Add this to a row to get the new row down
    BYTE **ppbTop,           // Returns pointer to the first byte in the top row of pixels.
    bool bYuv )
{
    HRESULT hr = S_OK;
    LONG lStride;

    BITMAPINFOHEADER bmiHeader;
    RECT    rcTarget = { 0 };

    if( NULL == pbData ||
        NULL == pdwWidth ||
        NULL == pdwHeight ||
        NULL == plStrideInBytes ||
        NULL == ppbTop )
    {
        hr = E_POINTER;
    }

    // Get the BITMAPINFOHEADER.
    if( SUCCEEDED( hr ) )
    {
        hr = GetBitmapInfoHeader( &m_mtInput, &bmiHeader );
    }

    if( SUCCEEDED( hr ) )
    {   
        hr = GetTargetRECT( &m_mtInput, &rcTarget );
    }

    if( SUCCEEDED( hr ))
    {
        //  For 'normal' formats, biWidth is in pixels. 
        //  Expand to bytes and round up to a multiple of 4.
        if (bmiHeader.biBitCount != 0 &&
            0 == (7 & bmiHeader.biBitCount)) 
        {
            lStride = (bmiHeader.biWidth * (bmiHeader.biBitCount / 8) + 3) & ~3;
        } 
        else   // Otherwise, biWidth is in bytes.
        {
            lStride = bmiHeader.biWidth;
        }

        //  If rcTarget is empty, use the whole image.
        if (IsRectEmpty(&rcTarget)) 
        {
            *pdwWidth = (DWORD)bmiHeader.biWidth * (bmiHeader.biBitCount) /8;
            *pdwHeight = (DWORD)(abs(bmiHeader.biHeight));
    
            if (bmiHeader.biHeight < 0 || bYuv)   // Top-down bitmap. 
            {
                *plStrideInBytes = lStride; // Stride goes "down"
                *ppbTop           = pbData; // Top row is first.
            } 
            else        // Bottom-up bitmap
            {
                *plStrideInBytes = -lStride;    // Stride goes "up"
                *ppbTop = pbData + lStride * (*pdwHeight - 1);  // Bottom row is first.
            }
        } 
        else   // rcTarget is NOT empty. Use a sub-rectangle in the image.
        {
            *pdwWidth = (DWORD)((rcTarget.right - rcTarget.left) * (bmiHeader.biBitCount)) /8;
            *pdwHeight = (DWORD)(rcTarget.bottom - rcTarget.top);
    
            if (bmiHeader.biHeight < 0 || bYuv)   // Top-down bitmap.
            {
                // Same stride as above, but first pixel is modified down
                // and and over by the target rectangle.
                *plStrideInBytes = lStride;     
                *ppbTop = pbData +
                         lStride * rcTarget.top +
                         (bmiHeader.biBitCount * rcTarget.left) / 8;
            } 
            else  // Bottom-up bitmap.
            {
                *plStrideInBytes = -lStride;
                *ppbTop = pbData +
                         lStride * (bmiHeader.biHeight - rcTarget.top - 1) +
                         (bmiHeader.biBitCount * rcTarget.left) / 8;
            }
        }
    }
    
    return hr;
}

////////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetBitmapInfoHeader
//
// Helper function to get the important information out of a VIDEOINFOHEADER.
////////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::GetBitmapInfoHeader( const DMO_MEDIA_TYPE *pmt,
                                             BITMAPINFOHEADER *pbmih)
{
    HRESULT hr = S_OK;

    if( NULL == pmt ||
        NULL == pbmih )
    {
        hr = E_POINTER;
    }

    if( SUCCEEDED( hr ) )
    {
        if( FORMAT_VideoInfo == pmt->formattype )
        {
            VIDEOINFOHEADER *pvih;
            pvih = (VIDEOINFOHEADER *)pmt->pbFormat;

            *pbmih = pvih->bmiHeader;
        }
        else if( FORMAT_VideoInfo2 == pmt->formattype )
        {
            VIDEOINFOHEADER2 *pvih2;
            pvih2 = (VIDEOINFOHEADER2 *)pmt->pbFormat;

            *pbmih = pvih2->bmiHeader;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetTargetRECT
//
// Helper function to get the important information out of a VIDEOINFOHEADER.
////////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::GetTargetRECT( const DMO_MEDIA_TYPE *pmt,
                                             RECT *prcTarget)
{
    HRESULT hr = S_OK;

    if( NULL == pmt ||
        NULL == prcTarget )
    {
        hr = E_POINTER;
    }

    if( SUCCEEDED( hr ) )
    {
        if( FORMAT_VideoInfo == pmt->formattype )
        {
            VIDEOINFOHEADER *pvih;
            pvih = (VIDEOINFOHEADER *)pmt->pbFormat;

            *prcTarget = pvih->rcTarget;
        }
        else if( FORMAT_VideoInfo2 == pmt->formattype )
        {
            VIDEOINFOHEADER2 *pvih2;
            pvih2 = (VIDEOINFOHEADER2 *)pmt->pbFormat;

            *prcTarget = pvih2->rcTarget;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::ProcessYUY2
//
// Process YUY2 video.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::ProcessYUY2( BYTE *pbInputData, BYTE *pbOutputData)
{
    HRESULT hr = S_OK;

    DWORD dwWidth = 0; // In bytes
    DWORD dwHeight = 0; // In pixels
    LONG lStrideIn = 0;  // Stride in bytes.
    LONG lStrideOut = 0; // Stride in bytes.

    // These pointers will point to the actual image data.
    BYTE *pbSource = NULL;
    BYTE *pbTarget = NULL;

    // Get output info.
    hr = GetVideoInfoParameters( pbOutputData, &dwWidth, &dwHeight,
        &lStrideOut, &pbTarget, true);

    if( SUCCEEDED( hr ) )
    {
        // Get input info.
        hr = GetVideoInfoParameters( pbInputData, &dwWidth, &dwHeight,
            &lStrideIn, &pbSource, true);
    }

    // YUY2 memory layout
    // 
    // Byte Ordering (lowest first)
    // Y0 V0 Y1 U0
    //
    // 1 Macro pixel = 2 image pixels

    if( SUCCEEDED( hr ) )
    {
        // dwWidth and dwHeight came from the input buffer.
        while( dwHeight-- )
        {
            DWORD x = dwWidth; 

            while( x-- )
            {
                // Scale the U and V bytes to 128.
                // Just copy the Y bytes.
                if( x%2 )
                {
                    long temp = (long)((pbSource[x] - 128) * m_fScaleFactor);

                    // Truncate if exceeded full scale.
                    if (temp > 127)
                        temp = 127;
                    if (temp < -128)
                        temp = -128;

                    pbTarget[x] = (BYTE) (temp + 128);
                }
                else
                {
                    pbTarget[x] = pbSource[x];
                }
            }

            // Move the pointers to the next row.
            pbSource += lStrideIn;
            pbTarget += lStrideOut;
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::ProcessYV12
//
// Process YV12 video.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::ProcessYV12( BYTE *pbInputData, BYTE *pbOutputData)
{
    HRESULT hr = S_OK;

    DWORD dwWidth = 0; // In bytes
    DWORD dwHeight = 0; // In pixels
    LONG lStrideIn = 0;  // Stride in bytes.
    LONG lStrideOut = 0; // Stride in bytes.

    // These pointers will point to the actual image data.
    BYTE *pbSource = NULL;
    BYTE *pbTarget = NULL;

    // Get output info.
    hr = GetVideoInfoParameters( pbOutputData, &dwWidth, &dwHeight,
        &lStrideOut, &pbTarget, true);

    if( SUCCEEDED( hr ) )
    {
        // Get input info.
        hr = GetVideoInfoParameters( pbInputData, &dwWidth, &dwHeight,
            &lStrideIn, &pbSource, true);
    }

    // YV12 memory layout is planar
    //
    // Y resolution is 1x1
    // U and V are 2x2
    //
    // 1 Macro pixel = 4 image pixels

    if( SUCCEEDED( hr ) )
    {
        LONG lHalfStrideIn = lStrideIn / 2;
        LONG lHalfStrideOut = lStrideOut / 2;

        // First copy the Y plane.
        memcpy( pbTarget, pbSource, dwHeight * lStrideIn );

        // Move the pointers to the next plane.
        pbSource += dwHeight * lStrideIn;
        pbTarget += dwHeight * lStrideOut;

        // Process the U and V planes.
        DWORD y = dwHeight;

        while( y-- )
        {
            DWORD x = dwWidth/2; 

            while( x-- )
            {
                long temp = (long)((pbSource[x] - 128) * m_fScaleFactor);

                // Truncate if exceeded full scale.
                if (temp > 127)
                    temp = 127;
                if (temp < -128)
                    temp = -128;

                pbTarget[x] = (BYTE) (temp + 128);
            }

            // Move the pointers to the next row.
            pbSource += lHalfStrideIn;
            pbTarget += lHalfStrideOut;
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::ProcessNV12
//
// Process NV12 video.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::ProcessNV12( BYTE *pbInputData, BYTE *pbOutputData)
{
    HRESULT hr = S_OK;

    DWORD dwWidth = 0; // In bytes
    DWORD dwHeight = 0; // In pixels
    LONG lStrideIn = 0;  // Stride in bytes.
    LONG lStrideOut = 0; // Stride in bytes.

    // These pointers will point to the actual image data.
    BYTE *pbSource = NULL;
    BYTE *pbTarget = NULL;

    // Get output info.
    hr = GetVideoInfoParameters( pbOutputData, &dwWidth, &dwHeight,
        &lStrideOut, &pbTarget, true);

    if( SUCCEEDED( hr ) )
    {
        // Get input info.
        hr = GetVideoInfoParameters( pbInputData, &dwWidth, &dwHeight,
            &lStrideIn, &pbSource, true);
    }

    // NV12 memory layout is planar for Y, packed for U and V.
    //
    // Y resolution is 1x1
    // U and V are 2x2
    //
    // 1 Macro pixel = 4 image pixels

    if( SUCCEEDED( hr ) )
    {
        // First copy the Y plane.
        memcpy( pbTarget, pbSource, dwHeight * lStrideIn );

        // Move the pointers to the next plane.
        pbSource += dwHeight * lStrideIn;
        pbTarget += dwHeight * lStrideOut;

        // Process the U and V planes.
        DWORD y = dwHeight/2;

        while( y-- )
        {
            DWORD x = lStrideIn; 

            while( x-- )
            {
                long temp = (long)((pbSource[x] - 128) * m_fScaleFactor);

                // Truncate if exceeded full scale.
                if (temp > 127)
                    temp = 127;
                if (temp < -128)
                    temp = -128;

                pbTarget[x] = (BYTE) (temp + 128);
            }

            // Move the pointers to the next row.
            pbSource += lStrideIn;
            pbTarget += lStrideOut;
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::ProcessUYVY
//
// Process UYVY video.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::ProcessUYVY( BYTE *pbInputData, BYTE *pbOutputData)
{
    HRESULT hr = S_OK;

    DWORD dwWidth = 0; // In bytes
    DWORD dwHeight = 0; // In pixels
    LONG lStrideIn = 0;  // Stride in bytes.
    LONG lStrideOut = 0; // Stride in bytes.

    // These pointers will point to the actual image data.
    BYTE *pbSource = NULL;
    BYTE *pbTarget = NULL;

    // Get output info.
    hr = GetVideoInfoParameters( pbOutputData, &dwWidth, &dwHeight,
        &lStrideOut, &pbTarget, true);

    if( SUCCEEDED( hr ) )
    {
        // Get input info.
        hr = GetVideoInfoParameters( pbInputData, &dwWidth, &dwHeight,
            &lStrideIn, &pbSource, true);
    }

    // UYVY memory layout
    // 
    // Byte Ordering (lowest first)
    // U0 Y0 V0 Y1
    //
    // 1 Macro pixel = 2 image pixels

    if( SUCCEEDED( hr ) )
    {
        // dwWidth and dwHeight came from the input buffer.
        while( dwHeight-- )
        {
            DWORD x = dwWidth; 

            while( x-- )
            {
                // Scale the U and V bytes to 128.
                // Just copy the Y bytes.
                if( x%2 )
                {
                    pbTarget[x] = pbSource[x];
                }
                else
                {
                    long temp = (long)((pbSource[x] - 128) * m_fScaleFactor);

                    // Truncate if exceeded full scale.
                    if (temp > 127)
                        temp = 127;
                    if (temp < -128)
                        temp = -128;

                    pbTarget[x] = (BYTE) (temp + 128);
                }
            }

            // Move the pointers to the next row.
            pbSource += lStrideIn;
            pbTarget += lStrideOut;
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Process24Bit
//
// Process 24-bit RGB video.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::Process24Bit( BYTE *pbInputData, BYTE *pbOutputData)
{
    HRESULT hr = S_OK;

    DWORD dwWidth = 0; // In bytes
    DWORD dwHeight = 0; // In pixels
    LONG lStrideIn = 0;  // Stride in bytes.
    LONG lStrideOut = 0; // Stride in bytes.

    // These pointers will point to the actual image data.
    BYTE *pbSource = NULL;
    BYTE *pbTarget = NULL;

    // Get output info.
    hr = GetVideoInfoParameters( pbOutputData, &dwWidth, &dwHeight,
        &lStrideOut, &pbTarget, true);

    if( SUCCEEDED( hr ) )
    {
        // Get input info.
        hr = GetVideoInfoParameters( pbInputData, &dwWidth, &dwHeight,
            &lStrideIn, &pbSource, true);
    }

    if( SUCCEEDED( hr ) )
    {
        // dwWidth and dwHeight came from the input buffer.

        dwWidth /= 3; // Convert to pixels.

        while( dwHeight-- )
        {
            RGBTRIPLE* pPixelIn = (RGBTRIPLE*)pbSource;
            RGBTRIPLE* pPixelOut = (RGBTRIPLE*)pbTarget;

            for( DWORD x = 0; x < dwWidth; x++ )
            {
                // Get the color bytes.
                long lBlue = (long) pPixelIn[x].rgbtBlue;
                long lGreen = (long) pPixelIn[x].rgbtGreen;
                long lRed = (long) pPixelIn[x].rgbtRed;

                // Compute the average for gray.
                long lAverage = ( lBlue + lGreen + lRed ) / 3;

                // Scale the colors to the average.
                pPixelOut[x].rgbtBlue = (BYTE)( ( lBlue - lAverage ) * m_fScaleFactor  + lAverage );
                pPixelOut[x].rgbtGreen = (BYTE)( ( lGreen - lAverage ) * m_fScaleFactor  + lAverage );
                pPixelOut[x].rgbtRed = (BYTE)( ( lRed - lAverage ) * m_fScaleFactor  + lAverage );          
            }

            // Move the pointers to the next row.
            pbSource += lStrideIn;
            pbTarget += lStrideOut;
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Process32Bit
//
// Process 32-bit RGB video.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::Process32Bit( BYTE *pbInputData, BYTE *pbOutputData)
{
    HRESULT hr = S_OK;

    DWORD dwWidth = 0; // In bytes
    DWORD dwHeight = 0; // In pixels
    LONG lStrideIn = 0;  // Stride in bytes.
    LONG lStrideOut = 0; // Stride in bytes.

    // These pointers will point to the actual image data.
    BYTE *pbSource = NULL;
    BYTE *pbTarget = NULL;

    // Get output info.
    hr = GetVideoInfoParameters( pbOutputData, &dwWidth, &dwHeight,
        &lStrideOut, &pbTarget, true);

    if( SUCCEEDED( hr ) )
    {
        // Get input info.
        hr = GetVideoInfoParameters( pbInputData, &dwWidth, &dwHeight,
            &lStrideIn, &pbSource, true);
    }

    if( SUCCEEDED( hr ) )
    {
        // dwWidth and dwHeight came from the input buffer.
        dwWidth >>= 2; // Convert to pixels.

        while( dwHeight-- )
        {
            RGBQUAD* pPixelIn = (RGBQUAD*)pbSource;
            RGBQUAD* pPixelOut = (RGBQUAD*)pbTarget;

            for( DWORD x = 0; x < dwWidth; x++ )
            {
                // Get the color bytes.
                long lBlue = (long) pPixelIn[x].rgbBlue;
                long lGreen = (long) pPixelIn[x].rgbGreen;
                long lRed = (long) pPixelIn[x].rgbRed;

                // Compute the average for gray.
                long lAverage = ( lBlue + lGreen + lRed ) / 3;

                // Scale the colors to the average.
                pPixelOut[x].rgbBlue = (BYTE)( ( lBlue - lAverage ) * m_fScaleFactor  + lAverage );
                pPixelOut[x].rgbGreen = (BYTE)( ( lGreen - lAverage ) * m_fScaleFactor  + lAverage );
                pPixelOut[x].rgbRed = (BYTE)( ( lRed - lAverage ) * m_fScaleFactor  + lAverage );
                pPixelOut[x].rgbReserved = pPixelIn[x].rgbReserved;

            }

            // Move the pointers to the next row.
            pbSource += lStrideIn;
            pbTarget += lStrideOut;
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Process565
//
// Process 16-bit RGB video.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::Process565( BYTE *pbInputData, BYTE *pbOutputData)
{
    HRESULT hr = S_OK;

    DWORD dwWidth = 0; // In bytes
    DWORD dwHeight = 0; // In pixels
    LONG lStrideIn = 0;  // Stride in bytes.
    LONG lStrideOut = 0; // Stride in bytes.

    // These pointers will point to the actual image data.
    BYTE *pbSource = NULL;
    BYTE *pbTarget = NULL;

    // Get output info.
    hr = GetVideoInfoParameters( pbOutputData, &dwWidth, &dwHeight,
        &lStrideOut, &pbTarget, true);

    if( SUCCEEDED( hr ) )
    {
        // Get input info.
        hr = GetVideoInfoParameters( pbInputData, &dwWidth, &dwHeight,
            &lStrideIn, &pbSource, true);
    }

    if( SUCCEEDED( hr ) )
    {
        // dwWidth and dwHeight came from the input buffer.
        dwWidth >>= 1; // Convert to pixels.

        while( dwHeight-- )
        {
            WORD *pdwPixelIn = (WORD*)pbSource;
            WORD *pdwPixelOut = (WORD*)pbTarget;

            for( DWORD x = 0; x < dwWidth; x++ )
            {
                // Get the color data.
                // Unpack the red and blue data to have a precision matching
                // the green data. This is because the next steps averages
                // and scale the values, so they need to be weighted equally.
                DWORD dwRed = (DWORD)( pdwPixelIn[x] & 0xF800 ) >> 10;
                DWORD dwGreen = (DWORD)( pdwPixelIn[x] & 0x7e0 ) >> 5;
                DWORD dwBlue = (DWORD)( pdwPixelIn[x] & 0x1F ) << 1;

                // Compute the average for gray.
                DWORD dwAverage = ( dwRed + dwGreen + dwBlue ) / 3;

                // Scale the colors to the average.
                dwBlue = (DWORD)( ( dwBlue - dwAverage ) * m_fScaleFactor  + dwAverage );
                dwGreen = (DWORD)( ( dwGreen - dwAverage ) * m_fScaleFactor  + dwAverage );
                dwRed = (DWORD)( ( dwRed - dwAverage ) * m_fScaleFactor  + dwAverage );

                // Return blue and red to their original precision.
                // This effectively drops the rightmost bit before 
                // we repack the colors into a WORD.
                dwBlue >>= 1;
                dwRed >>= 1;

                // Repack the colors.
                // Copy to the output.
                pdwPixelOut[x] =   (WORD)((  dwRed << 11 ) | (  dwGreen << 5 ) |  ( dwBlue) );
            }

            // Move the pointers to the next row.
            pbSource += lStrideIn;
            pbTarget += lStrideOut;
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Process555
//
// Process 16-bit RGB video.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::Process555( BYTE *pbInputData, BYTE *pbOutputData)
{
    HRESULT hr = S_OK;

    DWORD dwWidth = 0; // In bytes
    DWORD dwHeight = 0; // In pixels
    LONG lStrideIn = 0;  // Stride in bytes.
    LONG lStrideOut = 0; // Stride in bytes.

    // These pointers will point to the actual image data.
    BYTE *pbSource = NULL;
    BYTE *pbTarget = NULL;

    // Get output info.
    hr = GetVideoInfoParameters( pbOutputData, &dwWidth, &dwHeight,
        &lStrideOut, &pbTarget, true);

    if( SUCCEEDED( hr ) )
    {
        // Get input info.
        hr = GetVideoInfoParameters( pbInputData, &dwWidth, &dwHeight,
            &lStrideIn, &pbSource, true);
    }

    if( SUCCEEDED( hr ) )
    {
        // dwWidth and dwHeight came from the input buffer.
        dwWidth >>= 1; // Convert to pixels.

        while( dwHeight-- )
        {
            WORD *pdwPixelIn = (WORD*)pbSource;
            WORD *pdwPixelOut = (WORD*)pbTarget;

            for( DWORD x = 0; x < dwWidth; x++ )
            {
                // Get the color data.
                // For RGB555, each set of values is weighted equally.
                DWORD dwRed = (DWORD)( pdwPixelIn[x] & 0xFc00 ) >> 10;
                DWORD dwGreen = (DWORD)( pdwPixelIn[x] & 0x3e0 ) >> 5;
                DWORD dwBlue = (DWORD)( pdwPixelIn[x] & 0x1F );

                // Compute the average for gray.
                DWORD dwAverage = ( dwRed + dwGreen + dwBlue ) / 3;

                // Scale the colors to the average.
                dwBlue = (DWORD)( ( dwBlue - dwAverage ) * m_fScaleFactor  + dwAverage );
                dwGreen = (DWORD)( ( dwGreen - dwAverage ) * m_fScaleFactor  + dwAverage );
                dwRed = (DWORD)( ( dwRed - dwAverage ) * m_fScaleFactor  + dwAverage );

                // Repack the colors.
                // Copy to the output.
                pdwPixelOut[x] =   (WORD)( (  dwRed << 10 ) | (  dwGreen << 5 ) |  ( dwBlue) );
            }

            // Move the pointers to the next row.
            pbSource += lStrideIn;
            pbTarget += lStrideOut;
        }
    }

    return hr;
}

[!if DUALMODE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::CreateMFMediaType
//
// Translates the given DMO_MEDIA_TYPE into a newly created IMFMediaType object.
/////////////////////////////////////////////////////////////////////////////

HRESULT C[!output Safe_root]::CreateMFMediaType( DMO_MEDIA_TYPE* pmtDMOType, IMFMediaType** ppMFType ) {
    HRESULT hr = S_OK;
    IMFMediaType* pMFType = NULL;

    do {
        if( ppMFType == NULL) {
            hr = E_POINTER;
            break;
        }

        hr = MFCreateMediaType( &pMFType );
        if( FAILED( hr ) ) {
            break;
        }

        hr = MFInitMediaTypeFromAMMediaType( pMFType, (AM_MEDIA_TYPE*)pmtDMOType );
        if( FAILED( hr ) ) {
            break;
        }

        *ppMFType = pMFType;
    } while( false );

    if (FAILED(hr) && pMFType) {
        pMFType->Release();
    }

    return hr;
}
[!endif]


