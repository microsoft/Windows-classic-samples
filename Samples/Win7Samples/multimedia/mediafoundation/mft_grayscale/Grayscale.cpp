//////////////////////////////////////////////////////////////////////////
//
// winmain.cpp : Defines the entry point for the application.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#define CHECK_HR(hr) if (FAILED(hr)) { goto done; }

#include "MFT_Grayscale.h"
#include "Grayscale.h"
#include "logmediatype.h"

#include <uuids.h>      // DirectShow GUIDs
#include <assert.h>
#include <evr.h>


// This sample implements a Media Foundation transform (MFT) that 
// converts YUV video frames to grayscale. The conversion is done
// simply by setting all of the U and V bytes to zero (0x80).

// NOTES:
// 1-in, 1-out
// Fixed streams
// Formats: UYVY, YUY2, NV12

// Assumptions:
// 1. If the MFT is holding an input sample, SetInputType and SetOutputType 
//    return MF_E_UNSUPPORTED_MEDIATYPE
// 2. If the input type is set, the output type must match (and vice versa).
// 3. If both types are set, no type can be set until the current type is 
//    cleared.
// 4. Preferred input types: 
//    (a) If the output type is set, that's the preferred type.
//    (b) Otherwise. the preferred types are partial types, constructed from 
//        a list of supported video subtypes. 
// 5. Preferred output types: As above.
 
// Video FOURCC codes.
const FOURCC FOURCC_YUY2 = MAKEFOURCC('Y', 'U', 'Y', '2');
const FOURCC FOURCC_UYVY = MAKEFOURCC('U', 'Y', 'V', 'Y');
const FOURCC FOURCC_NV12 = MAKEFOURCC('N', 'V', '1', '2');

// Static array of media types (preferred and accepted).
const GUID* g_MediaSubtypes[] = 
{
    & MEDIASUBTYPE_NV12,
    & MEDIASUBTYPE_YUY2,
    & MEDIASUBTYPE_UYVY
};

// Number of media types in the aray.
DWORD g_cNumSubtypes = ARRAY_SIZE(g_MediaSubtypes);

// GetImageSize: Returns the size of a video frame, in bytes.
HRESULT GetImageSize(FOURCC fcc, UINT32 width, UINT32 height, DWORD* pcbImage);

//-------------------------------------------------------------------
// Name: TransformImage_UYVY
// Description: Converts an image in UYVY format to grayscale.
//
// The image conversion functions take the following parameters:
//
// pDest:            Pointer to the destination buffer.
// lDestStride:      Stride of the destination buffer, in bytes.
// pSrc:             Pointer to the source buffer.
// lSrcStride:       Stride of the source buffer, in bytes.
// dwWidthInPixels:  Frame width in pixels.
// dwHeightInPixels: Frame height, in pixels.
//-------------------------------------------------------------------

void TransformImage_UYVY(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    )
{
    for (DWORD y = 0; y < dwHeightInPixels; y++)
    {
        WORD *pSrc_Pixel = (WORD*)pSrc;
        WORD *pDest_Pixel = (WORD*)pDest;
        
        for (DWORD x = 0; x < dwWidthInPixels; x++)
        {
            // Byte order is U0 Y0 V0 Y1
            // Each WORD is a byte pair (U/V, Y)
            // Windows is little-endian so the order appears reversed.
            
            WORD pixel = pSrc_Pixel[x] & 0xFF00;
            pixel |= 0x0080;
            pDest_Pixel[x] = pixel;
        }
        pDest += lDestStride;
        pSrc += lSrcStride;
    }
}


//-------------------------------------------------------------------
// Name: TransformImage_YUY2
// Description: Converts an image in YUY2 format to grayscale.
//-------------------------------------------------------------------

void TransformImage_YUY2(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    )
{
    for (DWORD y = 0; y < dwHeightInPixels; y++)
    {
        WORD *pSrc_Pixel = (WORD*)pSrc;
        WORD *pDest_Pixel = (WORD*)pDest;
        
        for (DWORD x = 0; x < dwWidthInPixels; x++)
        {
            // Byte order is Y0 U0 Y1 V0 
            // Each WORD is a byte pair (Y, U/V)
            // Windows is little-endian so the order appears reversed.
            
            WORD pixel = pSrc_Pixel[x] & 0x00FF;
            pixel |= 0x8000;
            pDest_Pixel[x] = pixel;
        }
        pDest += lDestStride;
        pSrc += lSrcStride;
    }
}



//-------------------------------------------------------------------
// Name: TransformImage_NV12
// Description: Converts an image in NV12 format to grayscale.
//-------------------------------------------------------------------

void TransformImage_NV12(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    )
{
    // NV12 is planar: Y plane, followed by packed U-V plane.

    // Y plane
    for (DWORD y = 0; y < dwHeightInPixels; y++)
    {
        CopyMemory(pDest, pSrc, dwWidthInPixels);
        pDest += lDestStride;
        pSrc += lSrcStride;
    }

    // U-V plane
    for (DWORD y = 0; y < dwHeightInPixels/2; y++)
    {
        FillMemory(pDest, dwWidthInPixels, 0x80);
        pDest += lDestStride;
    }
}


//-------------------------------------------------------------------
// Name: CreateInstance
// Description: Static method to create an instance of the source.
//
// pUnkOuter:   Aggregating object or NULL.
// iid:         IID of the requested interface on the source.
// ppSource:    Receives a ref-counted pointer to the source.
//-------------------------------------------------------------------

HRESULT CGrayscale::CreateInstance(IUnknown *pUnkOuter, REFIID iid, void **ppMFT)
{
    if (ppMFT == NULL)
    {
        return E_POINTER;
    }

    // This object does not support aggregation.
    if (pUnkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    HRESULT hr = S_OK;
    CGrayscale *pMFT = new CGrayscale(hr);
    if (pMFT == NULL)
    {
        CHECK_HR(hr = E_OUTOFMEMORY);
    }

    CHECK_HR(hr = pMFT->QueryInterface(iid, ppMFT));

done:
    SAFE_RELEASE(pMFT);
    return hr;
}


//-------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------

CGrayscale::CGrayscale(HRESULT& hr) :
    m_nRefCount(1),     
    m_pSample(NULL),
    m_pInputType(NULL),
    m_pOutputType(NULL),
    m_pTransformFn(NULL),
    m_videoFOURCC(0),
    m_imageWidthInPixels(0),
    m_imageHeightInPixels(0),
    m_cbImageSize(0)
{
}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------

CGrayscale::~CGrayscale()
{
    assert(m_nRefCount == 0);

    SAFE_RELEASE(m_pInputType);
    SAFE_RELEASE(m_pOutputType);
    SAFE_RELEASE(m_pSample);
}

// IUnknown methods

ULONG CGrayscale::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG CGrayscale::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT CGrayscale::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == __uuidof(IMFTransform))
    {
        *ppv = static_cast<IMFTransform*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}


// IMFTransform methods. Refer to the Media Foundation SDK documentation for details.

//-------------------------------------------------------------------
// Name: GetStreamLimits
// Returns the minimum and maximum number of streams.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetStreamLimits( 
    DWORD   *pdwInputMinimum,
    DWORD   *pdwInputMaximum,
    DWORD   *pdwOutputMinimum,
    DWORD   *pdwOutputMaximum
)
{

    if ((pdwInputMinimum == NULL) ||
        (pdwInputMaximum == NULL) ||
        (pdwOutputMinimum == NULL) ||
        (pdwOutputMaximum == NULL))
    {
        return E_POINTER;
    }


    // This MFT has a fixed number of streams.
    *pdwInputMinimum = 1;
    *pdwInputMaximum = 1;
    *pdwOutputMinimum = 1;
    *pdwOutputMaximum = 1;

    return S_OK;
}


//-------------------------------------------------------------------
// Name: GetStreamCount
// Returns the actual number of streams.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetStreamCount(
    DWORD   *pcInputStreams,
    DWORD   *pcOutputStreams
)
{
    if ((pcInputStreams == NULL) || (pcOutputStreams == NULL))
        
    {
        return E_POINTER;
    }

    // This MFT has a fixed number of streams.
    *pcInputStreams = 1;
    *pcOutputStreams = 1;

    return S_OK;
}



//-------------------------------------------------------------------
// Name: GetStreamIDs
// Returns stream IDs for the input and output streams.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetStreamIDs(
    DWORD   dwInputIDArraySize,
    DWORD   *pdwInputIDs,
    DWORD   dwOutputIDArraySize,
    DWORD   *pdwOutputIDs
)
{
    // Do not need to implement, because this MFT has a fixed number of 
    // streams and the stream IDs match the stream indexes.
    return E_NOTIMPL;   
}


//-------------------------------------------------------------------
// Name: GetInputStreamInfo
// Returns information about an input stream. 
//-------------------------------------------------------------------

HRESULT CGrayscale::GetInputStreamInfo(
    DWORD                     dwInputStreamID,
    MFT_INPUT_STREAM_INFO *   pStreamInfo
)
{
    TRACE((L"GetInputStreamInfo\n"));

    AutoLock lock(m_critSec);
    
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }
    
    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // NOTE: This method should succeed even when there is no media type on the
    //       stream. If there is no media type, we only need to fill in the dwFlags 
    //       member of MFT_INPUT_STREAM_INFO. The other members depend on having a
    //       a valid media type.

    pStreamInfo->hnsMaxLatency = 0;
    pStreamInfo->dwFlags = MFT_INPUT_STREAM_WHOLE_SAMPLES | MFT_INPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER ;

    if (m_pInputType == NULL)
    {
        pStreamInfo->cbSize = 0;
    }
    else
    {
        pStreamInfo->cbSize = m_cbImageSize;
    }

    pStreamInfo->cbMaxLookahead = 0;
    pStreamInfo->cbAlignment = 0;

    return S_OK;
}



//-------------------------------------------------------------------
// Name: GetOutputStreamInfo
// Returns information about an output stream. 
//-------------------------------------------------------------------

HRESULT CGrayscale::GetOutputStreamInfo(
    DWORD                     dwOutputStreamID, 
    MFT_OUTPUT_STREAM_INFO *  pStreamInfo      
)
{
    AutoLock lock(m_critSec);

    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // NOTE: This method should succeed even when there is no media type on the
    //       stream. If there is no media type, we only need to fill in the dwFlags 
    //       member of MFT_OUTPUT_STREAM_INFO. The other members depend on having a
    //       a valid media type.

    pStreamInfo->dwFlags = 
        MFT_OUTPUT_STREAM_WHOLE_SAMPLES | 
        MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER |
        MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE ;

    if (m_pOutputType == NULL)
    {
        pStreamInfo->cbSize = 0;
    }
    else
    {
        pStreamInfo->cbSize = m_cbImageSize;
    }
    
    pStreamInfo->cbAlignment = 0;

    return S_OK;
}



//-------------------------------------------------------------------
// Name: GetAttributes
// Returns the attributes for the MFT.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetAttributes(IMFAttributes** pAttributes)
{
    // This MFT does not support any attributes, so the method is not implemented.
    return E_NOTIMPL;   
}



//-------------------------------------------------------------------
// Name: GetInputStreamAttributes
// Returns stream-level attributes for an input stream.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetInputStreamAttributes(
    DWORD           dwInputStreamID,
    IMFAttributes   **ppAttributes
)
{
    // This MFT does not support any attributes, so the method is not implemented.
    return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: GetOutputStreamAttributes
// Returns stream-level attributes for an output stream.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetOutputStreamAttributes(
    DWORD           dwOutputStreamID,
    IMFAttributes   **ppAttributes
)
{
    // This MFT does not support any attributes, so the method is not implemented.
    return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: DeleteInputStream
//-------------------------------------------------------------------

HRESULT CGrayscale::DeleteInputStream(DWORD dwStreamID)
{
    // This MFT has a fixed number of input streams, so the method is not implemented.
    return E_NOTIMPL; 
}



//-------------------------------------------------------------------
// Name: AddInputStreams
//-------------------------------------------------------------------

HRESULT CGrayscale::AddInputStreams( 
    DWORD   cStreams,
    DWORD   *adwStreamIDs
)
{
    // This MFT has a fixed number of output streams, so the method is not implemented.
    return E_NOTIMPL; 
}



//-------------------------------------------------------------------
// Name: GetInputAvailableType
// Description: Return a preferred input type.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetInputAvailableType(
    DWORD           dwInputStreamID,
    DWORD           dwTypeIndex, // 0-based
    IMFMediaType    **ppType
)
{
    TRACE((L"GetInputAvailableType (stream = %d, type index = %d)\n", dwInputStreamID, dwTypeIndex));

    AutoLock lock(m_critSec);

    if (ppType == NULL)
    {
        return E_INVALIDARG;
    }

    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr = S_OK;

    // If the output type is set, return that type as our preferred input type.
    if (this->m_pOutputType)
    {
        if (dwTypeIndex > 0)
        {
            return MF_E_NO_MORE_TYPES;
        }

        *ppType = m_pOutputType;
        (*ppType)->AddRef();
    }
    else
    {
        // The output type is not set. Create a partial media type.
        hr = OnGetPartialType(dwTypeIndex, ppType);
    }
    return hr;
}



//-------------------------------------------------------------------
// Name: GetOutputAvailableType
// Description: Return a preferred output type.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetOutputAvailableType(
    DWORD           dwOutputStreamID,
    DWORD           dwTypeIndex, // 0-based
    IMFMediaType    **ppType
)
{
    TRACE((L"GetOutputAvailableType (stream = %d, type index = %d)\n", dwOutputStreamID, dwTypeIndex));

    AutoLock lock(m_critSec);

    if (ppType == NULL)
    {
        return E_INVALIDARG;
    }

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr = S_OK;

    // If the input type is set, return that type as our preferred output type.
    if (this->m_pInputType)
    {
        if (dwTypeIndex > 0)
        {
            return MF_E_NO_MORE_TYPES;
        }

        *ppType = m_pInputType;
        (*ppType)->AddRef();
    }
    else
    {
        // The input type is not set. Create a partial media type.
        hr = OnGetPartialType(dwTypeIndex, ppType);
    }

    return hr;
}



//-------------------------------------------------------------------
// Name: SetInputType
//-------------------------------------------------------------------

HRESULT CGrayscale::SetInputType(
    DWORD           dwInputStreamID,
    IMFMediaType    *pType, // Can be NULL to clear the input type.
    DWORD           dwFlags 
)
{
    TRACE((L"CGrayscale::SetInputType\n"));

    AutoLock lock(m_critSec);

    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // Validate flags.
    if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    // Does the caller want us to set the type, or just test it?
    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

    // If we have an input sample, the client cannot change the type now.
    if (HasPendingOutput())
    {
        CHECK_HR(hr = MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING);
    }

    // Validate the type, if non-NULL.
    if (pType)
    {
        CHECK_HR(hr = OnCheckInputType(pType));
    }

    // The type is OK. 
    // Set the type, unless the caller was just testing.
    if (bReallySet)
    {
        CHECK_HR(hr = OnSetInputType(pType));
    }

done:
    return hr;
}



//-------------------------------------------------------------------
// Name: SetOutputType
//-------------------------------------------------------------------

HRESULT CGrayscale::SetOutputType(
    DWORD           dwOutputStreamID,
    IMFMediaType    *pType, // Can be NULL to clear the output type.
    DWORD           dwFlags 
)
{
    TRACE((L"CGrayscale::SetOutputType\n"));

    AutoLock lock(m_critSec);

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // Validate flags.
    if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;


    // Does the caller want us to set the type, or just test it?
     BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

    // If we have an input sample, the client cannot change the type now.
    if (HasPendingOutput())
    {
        CHECK_HR(hr = MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING);
    }

    // Validate the type, if non-NULL.
    if (pType)
    {
        CHECK_HR(hr = OnCheckOutputType(pType));
    }

    if (bReallySet)
    {
        // The type is OK. 
        // Set the type, unless the caller was just testing.
        CHECK_HR(hr = OnSetOutputType(pType));
    }

done:
    return hr;
}



//-------------------------------------------------------------------
// Name: GetInputCurrentType
// Description: Returns the current input type.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetInputCurrentType(
    DWORD           dwInputStreamID,
    IMFMediaType    **ppType
)
{
    AutoLock lock(m_critSec);

    if (ppType == NULL)
    {
        return E_POINTER;
    }

    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (!m_pInputType)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    *ppType = m_pInputType;
    (*ppType)->AddRef();

    return S_OK;

}



//-------------------------------------------------------------------
// Name: GetOutputCurrentType
// Description: Returns the current output type.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetOutputCurrentType(
    DWORD           dwOutputStreamID,
    IMFMediaType    **ppType
)
{
    AutoLock lock(m_critSec);

    if (ppType == NULL)
    {
        return E_POINTER;
    }

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (!m_pOutputType)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    *ppType = m_pOutputType;
    (*ppType)->AddRef();

    return S_OK;

}



//-------------------------------------------------------------------
// Name: GetInputStatus
// Description: Query if the MFT is accepting more input.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetInputStatus(
    DWORD           dwInputStreamID,
    DWORD           *pdwFlags 
)
{
    TRACE((L"GetInputStatus\n"));

    AutoLock lock(m_critSec);

    if (pdwFlags == NULL)
    {
        return E_POINTER;
    }

    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // If we already have an input sample, we don't accept
    // another one until the client calls ProcessOutput or Flush.
    if (m_pSample == NULL)
    {
        *pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
    }
    else
    {
        *pdwFlags = 0;
    }

    return S_OK;
}



//-------------------------------------------------------------------
// Name: GetOutputStatus
// Description: Query if the MFT can produce output.
//-------------------------------------------------------------------

HRESULT CGrayscale::GetOutputStatus(DWORD *pdwFlags)
{
    TRACE((L"GetOutputStatus\n"));

    AutoLock lock(m_critSec);

    if (pdwFlags == NULL)
    {
        return E_POINTER;
    }

    // We can produce an output sample if (and only if)
    // we have an input sample.
    if (m_pSample != NULL)
    {
        *pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
    }
    else
    {
        *pdwFlags = 0;
    }

    return S_OK;
}



//-------------------------------------------------------------------
// Name: SetOutputBounds
// Sets the range of time stamps that the MFT will output.
//-------------------------------------------------------------------

HRESULT CGrayscale::SetOutputBounds(
    LONGLONG        hnsLowerBound,
    LONGLONG        hnsUpperBound
)
{
    // Implementation of this method is optional. 
    return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: ProcessEvent
// Sends an event to an input stream.
//-------------------------------------------------------------------

HRESULT CGrayscale::ProcessEvent(
    DWORD              dwInputStreamID,
    IMFMediaEvent      *pEvent 
)
{
    // This MFT does not handle any stream events, so the method can 
    // return E_NOTIMPL. This tells the pipeline that it can stop 
    // sending any more events to this MFT.
    return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: ProcessMessage
//-------------------------------------------------------------------

HRESULT CGrayscale::ProcessMessage(
    MFT_MESSAGE_TYPE    eMessage,
    ULONG_PTR           ulParam
)
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    switch (eMessage)
    {
    case MFT_MESSAGE_COMMAND_FLUSH:
        // Flush the MFT.
        hr = OnFlush();
        break;

    case MFT_MESSAGE_COMMAND_DRAIN:
        // Drain: Tells the MFT not to accept any more input until 
        // all of the pending output has been processed. That is our 
        // default behevior already, so there is nothing to do.
    break;

    case MFT_MESSAGE_SET_D3D_MANAGER:
        // The pipeline should never send this message unless the MFT
        // has the MF_SA_D3D_AWARE attribute set to TRUE. However, if we
        // do get this message, it's invalid and we don't implement it.
        hr = E_NOTIMPL;
        break;

    // The remaining messages do not require any action from this MFT.
    case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
    case MFT_MESSAGE_NOTIFY_END_STREAMING:
    case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
    case MFT_MESSAGE_NOTIFY_START_OF_STREAM: 
        break;
    }

    return hr;
}
    


//-------------------------------------------------------------------
// Name: ProcessInput
// Description: Process an input sample.
//-------------------------------------------------------------------

HRESULT CGrayscale::ProcessInput(
    DWORD               dwInputStreamID,
    IMFSample           *pSample, 
    DWORD               dwFlags 
)
{
    AutoLock lock(m_critSec);

    if (pSample == NULL)
    {
        return E_POINTER;
    }

    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (dwFlags != 0)
    {
        return E_INVALIDARG; // dwFlags is reserved and must be zero.
    }

    if (!m_pInputType || !m_pOutputType)
    {
        return MF_E_NOTACCEPTING;   // Client must set input and output types.
    }

    if (m_pSample != NULL)
    {
        return MF_E_NOTACCEPTING;   // We already have an input sample.
    }

    HRESULT hr = S_OK;
    DWORD dwBufferCount = 0;

    // Validate the number of buffers. There should only be a single buffer to hold the video frame. 
    CHECK_HR(hr = pSample->GetBufferCount(&dwBufferCount));

    if (dwBufferCount == 0)
    {
        CHECK_HR(hr = E_FAIL);
    }
    if (dwBufferCount > 1)
    {
        CHECK_HR(hr = MF_E_SAMPLE_HAS_TOO_MANY_BUFFERS);
    }

    // Cache the sample. We do the actual work in ProcessOutput.
    m_pSample = pSample;
    pSample->AddRef();  // Hold a reference count on the sample.

done:
    return hr;
}



//-------------------------------------------------------------------
// Name: ProcessOutput
// Description: Process an output sample.
//-------------------------------------------------------------------

HRESULT CGrayscale::ProcessOutput(
    DWORD                   dwFlags, 
    DWORD                   cOutputBufferCount,
    MFT_OUTPUT_DATA_BUFFER  *pOutputSamples, // one per stream
    DWORD                   *pdwStatus  
)
{
    AutoLock lock(m_critSec);

    // Check input parameters...

    // There are no flags that we accept in this MFT.
    // The only defined flag is MFT_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER. This 
    // flag only applies when the MFT marks an output stream as lazy or optional.
    // However there are no lazy or optional streams on this MFT, so the flag is
    // not valid.
    if (dwFlags != 0)
    {
        return E_INVALIDARG;
    }

    if (pOutputSamples == NULL || pdwStatus == NULL)
    {
        return E_POINTER;
    }

    // Must be exactly one output buffer.
    if (cOutputBufferCount != 1)
    {
        return E_INVALIDARG;
    }

    // It must contain a sample.
    if (pOutputSamples[0].pSample == NULL)
    {
        return E_INVALIDARG;
    }

    // If we don't have an input sample, we need some input before
    // we can generate any output.
    if (m_pSample == NULL)
    {
        return MF_E_TRANSFORM_NEED_MORE_INPUT;
    }

    HRESULT hr = S_OK;

    IMFMediaBuffer *pInput = NULL;
    IMFMediaBuffer *pOutput = NULL;

    // Get the input buffer.
    CHECK_HR(hr = m_pSample->ConvertToContiguousBuffer(&pInput));

    // Get the output buffer.
    CHECK_HR(hr = pOutputSamples[0].pSample->ConvertToContiguousBuffer(&pOutput));

    CHECK_HR(hr = OnProcessOutput(pInput, pOutput));

    // Set status flags.
    pOutputSamples[0].dwStatus = 0; 
    *pdwStatus = 0;


    // Copy the duration and time stamp from the input sample,
    // if present.
     
    LONGLONG hnsDuration = 0;
    LONGLONG hnsTime = 0;

    if (SUCCEEDED(m_pSample->GetSampleDuration(&hnsDuration)))
    {
        CHECK_HR(hr = pOutputSamples[0].pSample->SetSampleDuration(hnsDuration));
    }

    if (SUCCEEDED(m_pSample->GetSampleTime(&hnsTime)))
    {
        CHECK_HR(hr = pOutputSamples[0].pSample->SetSampleTime(hnsTime));
    }

done:
    
    SAFE_RELEASE(m_pSample);   // Release our input sample.
    SAFE_RELEASE(pInput);
    SAFE_RELEASE(pOutput);
    return hr;
}



/// PRIVATE METHODS

//-------------------------------------------------------------------
// Name: OnGetPartialType
// Description: Returns a partial media type from our list.
//
// dwTypeIndex: Index into the list of peferred media types.
// ppmt: Receives a pointer to the media type.
//-------------------------------------------------------------------

HRESULT CGrayscale::OnGetPartialType(DWORD dwTypeIndex, IMFMediaType **ppmt)
{
    HRESULT hr = S_OK;

    if (dwTypeIndex >= g_cNumSubtypes)
    {
        return MF_E_NO_MORE_TYPES;
    }

    IMFMediaType *pmt = NULL;

    CHECK_HR(hr = MFCreateMediaType(&pmt));

    CHECK_HR(hr = pmt->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));

    CHECK_HR(hr = pmt->SetGUID(MF_MT_SUBTYPE, *g_MediaSubtypes[dwTypeIndex]));

    *ppmt = pmt;
    (*ppmt)->AddRef();

done:
    SAFE_RELEASE(pmt);
    return hr;
}


//-------------------------------------------------------------------
// Name: OnCheckInputType
// Description: Validate an input media type.
//-------------------------------------------------------------------

HRESULT CGrayscale::OnCheckInputType(IMFMediaType *pmt)
{
    TRACE((L"OnCheckInputType\n"));
    assert(pmt != NULL);

    HRESULT hr = S_OK;

    // If the output type is set, see if they match.
    if (m_pOutputType != NULL)
    {
        DWORD flags = 0;
        hr = pmt->IsEqual(m_pOutputType, &flags);

        // IsEqual can return S_FALSE. Treat this as failure.

        if (hr != S_OK)
        {
            hr = MF_E_INVALIDMEDIATYPE;
        }
    }
    else
    {
        // Output type is not set. Just check this type.
        hr = OnCheckMediaType(pmt);
    }

    return hr;
}




//-------------------------------------------------------------------
// Name: OnCheckOutputType
// Description: Validate an output media type.
//-------------------------------------------------------------------

HRESULT CGrayscale::OnCheckOutputType(IMFMediaType *pmt)
{
    TRACE((L"OnCheckOutputType\n"));
    assert(pmt != NULL);

    HRESULT hr = S_OK;

    // If the input type is set, see if they match.
    if (m_pInputType != NULL)
    {
        DWORD flags = 0;
        hr = pmt->IsEqual(m_pInputType, &flags);

        // IsEqual can return S_FALSE. Treat this as failure.

        if (hr != S_OK)
        {
            hr = MF_E_INVALIDMEDIATYPE;
        }

    }
    else
    {
        // Input type is not set. Just check this type.
        hr = OnCheckMediaType(pmt);
    }

    return hr;    
}



//-------------------------------------------------------------------
// Name: OnCheckMediaType
// Description: Validates a media type for this transform.
//-------------------------------------------------------------------

HRESULT CGrayscale::OnCheckMediaType(IMFMediaType *pmt)
{
    LogMediaType(pmt);

    GUID major_type = GUID_NULL;
    GUID subtype = GUID_NULL;
    MFVideoInterlaceMode interlace = MFVideoInterlace_Unknown;
    UINT32 val = 0;
    BOOL bFoundMatchingSubtype = FALSE;

    HRESULT hr = S_OK;

    // Major type must be video.
    CHECK_HR(hr = pmt->GetGUID(MF_MT_MAJOR_TYPE, &major_type));

    if (major_type != MFMediaType_Video)
    {
        CHECK_HR(hr = MF_E_INVALIDMEDIATYPE);
    }

    // Subtype must be one of the subtypes in our global list.

    // Get the subtype GUID.
    CHECK_HR(hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype));

    // Look for the subtype in our list of accepted types.
    for (DWORD i = 0; i < g_cNumSubtypes; i++)
    {
        if (subtype == *g_MediaSubtypes[i])
        {
            bFoundMatchingSubtype = TRUE;
            break;
        }
    }

    if (!bFoundMatchingSubtype)
    {
        CHECK_HR(hr = MF_E_INVALIDMEDIATYPE);
    }       

    // Video must be progressive frames.
    CHECK_HR(hr = pmt->GetUINT32(MF_MT_INTERLACE_MODE, (UINT32*)&interlace));
    if (interlace != MFVideoInterlace_Progressive)
    {
        CHECK_HR(hr = MF_E_INVALIDMEDIATYPE);
    }

done:
    return hr;
}



//-------------------------------------------------------------------
// Name: OnSetInputType
// Description: Sets or clears the input media type.
//
// Prerequisite:
// The input type has already been validated.
//-------------------------------------------------------------------

HRESULT CGrayscale::OnSetInputType(IMFMediaType *pmt)
{
    TRACE((L"CGrayscale::OnSetInputType\n"));

    // if pmt is NULL, clear the type.
    // if pmt is non-NULL, set the type.

    SAFE_RELEASE(m_pInputType);
    m_pInputType = pmt;
    if (m_pInputType)
    {
        m_pInputType->AddRef();
    }

    // Update the format information.
    UpdateFormatInfo();

    return S_OK;
}




//-------------------------------------------------------------------
// Name: OnSetOutputType
// Description: Sets or clears the output media type.
//
// Prerequisite:
// The output type has already been validated.
//-------------------------------------------------------------------

HRESULT CGrayscale::OnSetOutputType(IMFMediaType *pmt)
{
    TRACE((L"CGrayscale::OnSetOutputType\n"));

    // if pmt is NULL, clear the type.
    // if pmt is non-NULL, set the type.
    
    SAFE_RELEASE(m_pOutputType);
    m_pOutputType = pmt;
    if (m_pOutputType)
    {
        m_pOutputType->AddRef();
    }

    return S_OK;
}




//-------------------------------------------------------------------
// Name: OnProcessOutput
// Description: Generates output data.
//-------------------------------------------------------------------

HRESULT CGrayscale::OnProcessOutput(IMFMediaBuffer *pIn, IMFMediaBuffer *pOut)
{
    HRESULT hr = S_OK;

    BYTE *pDest = NULL;         // Destination buffer.
    LONG lDestStride = 0;       // Destination stride.

    BYTE *pSrc = NULL;          // Source buffer.
    LONG lSrcStride = 0;        // Source stride.

    // Helper objects to lock the buffers.
    VideoBufferLock inputLock(pIn);
    VideoBufferLock outputLock(pOut);

    // Stride if the buffer does not support IMF2DBuffer
    LONG lDefaultStride = 0;
    
    CHECK_HR(hr = GetDefaultStride(m_pInputType, &lDefaultStride));

    // Lock the input buffer.
    CHECK_HR(hr = inputLock.LockBuffer(lDefaultStride, this->m_imageHeightInPixels, &pSrc, &lSrcStride));

    // Lock the output buffer.
    CHECK_HR(hr = outputLock.LockBuffer(lDefaultStride, m_imageHeightInPixels, &pDest, &lDestStride));

    // Invoke the image transform function.
    assert (m_pTransformFn != NULL); 
    if (m_pTransformFn)
    {
        (*m_pTransformFn)( pDest, lDestStride, pSrc, lSrcStride, 
            m_imageWidthInPixels, m_imageHeightInPixels);
    }
    else
    {
        CHECK_HR(hr = E_UNEXPECTED);
    }


    // Set the data size on the output buffer.
    CHECK_HR(hr = pOut->SetCurrentLength(m_cbImageSize));

    // The VideoBufferLock class automatically unlocks the buffers.
done:
    return S_OK;
}


//-------------------------------------------------------------------
// Name: OnFlush
// Description: Flush the MFT.
//-------------------------------------------------------------------

HRESULT CGrayscale::OnFlush()
{
    // For this MFT, flushing just means releasing the input sample.
    SAFE_RELEASE(m_pSample);
    return S_OK;
}




//-------------------------------------------------------------------
// Name: UpdateFormatInfo
// Description: After the input type is set, update our format 
//              information.
//-------------------------------------------------------------------

HRESULT CGrayscale::UpdateFormatInfo()
{
    HRESULT hr = S_OK;

    GUID subtype = GUID_NULL;

    m_imageWidthInPixels = 0;
    m_imageHeightInPixels = 0;
    m_videoFOURCC = 0;
    m_cbImageSize = 0;

    m_pTransformFn = NULL;

    if (m_pInputType != NULL)
    {
        CHECK_HR(hr = m_pInputType->GetGUID(MF_MT_SUBTYPE, &subtype));

        m_videoFOURCC = subtype.Data1;

        switch (m_videoFOURCC)
        {
        case FOURCC_YUY2:
            m_pTransformFn = TransformImage_YUY2;
            break;

        case FOURCC_UYVY:
            m_pTransformFn = TransformImage_UYVY;
            break;

        case FOURCC_NV12:
            m_pTransformFn = TransformImage_NV12;
            break;

        default:
            CHECK_HR(hr = E_UNEXPECTED);
        }

        CHECK_HR(hr = MFGetAttributeSize(
                m_pInputType, 
                MF_MT_FRAME_SIZE, 
                &m_imageWidthInPixels, 
                &m_imageHeightInPixels
                ));

        TRACE((L"Frame size: %d x %d\n", m_imageWidthInPixels, m_imageHeightInPixels));

        // Calculate the image size (not including padding)
        CHECK_HR(hr = GetImageSize(m_videoFOURCC, m_imageWidthInPixels, m_imageHeightInPixels, &m_cbImageSize));
    }

done:
    return hr;
}


//-------------------------------------------------------------------
// Name: GetImageSize
// Description: 
// Calculates the buffer size needed, based on the video format.
//-------------------------------------------------------------------

HRESULT GetImageSize(FOURCC fcc, UINT32 width, UINT32 height, DWORD* pcbImage)
{
    HRESULT hr = S_OK;

    switch (fcc)
    {
    case FOURCC_YUY2:
    case FOURCC_UYVY:
        // check overflow
        if ((width > MAXDWORD / 2) ||
            (width * 2 > MAXDWORD / height))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            // 16 bpp
            *pcbImage = width * height * 2;
        }
        break;
        

    case FOURCC_NV12:
        // check overflow
        if ((height/2 > MAXDWORD - height) ||
            ((height + height/2) > MAXDWORD / width))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            // 12 bpp
            *pcbImage = width * (height + (height/2));
        }
        break;

    default:
        hr = E_FAIL;    // Unsupported type.
    }

    return hr;
}

