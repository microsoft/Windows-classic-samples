//////////////////////////////////////////////////////////////////////////
//
// decoder.cpp
// Implements the MPEG-1 decoder.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "decoder.h"
#include "BufferLock.h"
#include <strsafe.h>

HRESULT GetDefaultStride(IMFMediaType *pType, LONG *plStride);
void    OurFillRect(PBYTE pbScanline0, DWORD dwWidth, DWORD dwHeight, LONG lStrideInBytes, COLORREF color);
void    DrawOurText(PBYTE pbScanline0, DWORD dwWidth, DWORD dwHeight, LONG lStrideInBytes, LPCTSTR szBuffer);
PBYTE   TextBitmap(LPCTSTR lpsz, SIZE *pSize);


#ifndef LODWORD
#define LODWORD(x)  ((DWORD)(((DWORD_PTR)(x)) & 0xffffffff))
#endif

const UINT32 MAX_VIDEO_WIDTH = 4095;        // per ISO/IEC 11172-2
const UINT32 MAX_VIDEO_HEIGHT = 4095;


HRESULT Decoder_CreateInstance(REFIID riid, void **ppv)
{
    return CDecoder::CreateInstance(riid, ppv);
}

//-------------------------------------------------------------------
// CDecoder class
//-------------------------------------------------------------------


//-------------------------------------------------------------------
// CreateInstance
// Static method to create an instance of the DMO.
//
// This method is used by the class factory.
//
// pUnkOuter: Aggregating IUnknown.
// 
//-------------------------------------------------------------------

HRESULT CDecoder::CreateInstance(REFIID riid, void **ppv)
{
    if (ppv == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    CDecoder *pObj = new CDecoder();

    if (pObj == NULL)
    {
        return E_OUTOFMEMORY;
    }

    *ppv = NULL;

    hr = pObj->QueryInterface(riid, ppv);
    
    SafeRelease(&pObj);

    return hr;
}


CDecoder::CDecoder() :
    m_nRefCount(1),
    m_pInputType(NULL),
    m_pOutputType(NULL),
    m_pBuffer(NULL),
    m_pbData(NULL),
    m_cbData(0),
    m_imageWidthInPixels(0),
    m_imageHeightInPixels(0),
    m_cbImageSize(0),
    m_rtFrame(0),
    m_rtLength(0),
    m_bPicture(false)

{
    DllAddRef();

    InitializeCriticalSection(&m_critSec);

    m_frameRate.Numerator = m_frameRate.Denominator = 0;

}

CDecoder::~CDecoder()
{
    SafeRelease(&m_pBuffer);
    DeleteCriticalSection(&m_critSec);

    DllRelease();
}

// IUnknown methods.

ULONG CDecoder::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG CDecoder::Release()
{
    assert(m_nRefCount >= 0);
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // Return the temporary variable, not the member
    // variable, for thread safety.
    return uCount;
}


HRESULT CDecoder::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDecoder, IMFTransform),
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

// IMFTransform methods. Refer to the Media Foundation SDK documentation for details.

//-------------------------------------------------------------------
// Name: GetStreamLimits
// Returns the minimum and maximum number of streams.
//-------------------------------------------------------------------

HRESULT CDecoder::GetStreamLimits( 
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

HRESULT CDecoder::GetStreamCount(
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

HRESULT CDecoder::GetStreamIDs(
    DWORD   /*dwInputIDArraySize*/,
    DWORD   * /*pdwInputIDs*/,
    DWORD   /*dwOutputIDArraySize*/,
    DWORD   * /*pdwOutputIDs*/
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

HRESULT CDecoder::GetInputStreamInfo(
    DWORD                     dwInputStreamID,
    MFT_INPUT_STREAM_INFO *   pStreamInfo
)
{
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }
    
    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    pStreamInfo->hnsMaxLatency = 0;

    //  We can process data on any boundary.
    pStreamInfo->dwFlags = 0;

    pStreamInfo->cbSize = 1;
    pStreamInfo->cbMaxLookahead = 0;
    pStreamInfo->cbAlignment = 1;

    return S_OK;
}



//-------------------------------------------------------------------
// Name: GetOutputStreamInfo
// Returns information about an output stream. 
//-------------------------------------------------------------------

HRESULT CDecoder::GetOutputStreamInfo(
    DWORD                     dwOutputStreamID, 
    MFT_OUTPUT_STREAM_INFO *  pStreamInfo      
)
{
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    EnterCriticalSection(&m_critSec);

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
        pStreamInfo->cbAlignment = 0;
    }
    else
    {
        pStreamInfo->cbSize = m_cbImageSize;
        pStreamInfo->cbAlignment = 1;
    }
    
    LeaveCriticalSection(&m_critSec);

    return S_OK;
}



//-------------------------------------------------------------------
// Name: GetAttributes
// Returns the attributes for the MFT.
//-------------------------------------------------------------------

HRESULT CDecoder::GetAttributes(IMFAttributes** /*pAttributes*/)
{
    // This MFT does not support any attributes, so the method is not implemented.
    return E_NOTIMPL;   
}



//-------------------------------------------------------------------
// Name: GetInputStreamAttributes
// Returns stream-level attributes for an input stream.
//-------------------------------------------------------------------

HRESULT CDecoder::GetInputStreamAttributes(
    DWORD           /*dwInputStreamID*/,
    IMFAttributes   ** /*ppAttributes*/
)
{
    // This MFT does not support any attributes, so the method is not implemented.
    return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: GetOutputStreamAttributes
// Returns stream-level attributes for an output stream.
//-------------------------------------------------------------------

HRESULT CDecoder::GetOutputStreamAttributes(
    DWORD           /*dwOutputStreamID*/,
    IMFAttributes   ** /*ppAttributes*/
)
{
    // This MFT does not support any attributes, so the method is not implemented.
    return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: DeleteInputStream
//-------------------------------------------------------------------

HRESULT CDecoder::DeleteInputStream(DWORD /*dwStreamID*/)
{
    // This MFT has a fixed number of input streams, so the method is not implemented.
    return E_NOTIMPL; 
}



//-------------------------------------------------------------------
// Name: AddInputStreams
//-------------------------------------------------------------------

HRESULT CDecoder::AddInputStreams( 
    DWORD   /*cStreams*/,
    DWORD   * /*adwStreamIDs*/
)
{
    // This MFT has a fixed number of output streams, so the method is not implemented.
    return E_NOTIMPL; 
}



//-------------------------------------------------------------------
// Name: GetInputAvailableType
// Description: Return a preferred input type.
//-------------------------------------------------------------------

HRESULT CDecoder::GetInputAvailableType(
    DWORD           /*dwInputStreamID*/,
    DWORD           /*dwTypeIndex*/, 
    IMFMediaType    ** /*ppType*/
)
{
    return MF_E_NO_MORE_TYPES;
}



//-------------------------------------------------------------------
// Name: GetOutputAvailableType
// Description: Return a preferred output type.
//-------------------------------------------------------------------

HRESULT CDecoder::GetOutputAvailableType(
    DWORD           dwOutputStreamID,
    DWORD           dwTypeIndex, // 0-based
    IMFMediaType    **ppType
)
{
    if (ppType == NULL)
    {
        return E_INVALIDARG;
    }

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    if (dwTypeIndex != 0)
    {
        return MF_E_NO_MORE_TYPES;
    }

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    IMFMediaType *pOutputType = NULL;

    if (m_pInputType == NULL)
    {
        hr = MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    if (SUCCEEDED(hr))
    {
        hr = MFCreateMediaType(&pOutputType);
    }

    if (SUCCEEDED(hr))
    {
        hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    }

    if (SUCCEEDED(hr))
    {
        hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    }

    if (SUCCEEDED(hr))
    {
        hr = pOutputType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE);
    }

    if (SUCCEEDED(hr))
    {
        hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    }

    if (SUCCEEDED(hr))
    {
        hr = pOutputType->SetUINT32(MF_MT_SAMPLE_SIZE, m_imageHeightInPixels * m_imageWidthInPixels * 4);
    }

    if (SUCCEEDED(hr))
    {
        hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, m_imageWidthInPixels, m_imageHeightInPixels);
    }

    if (SUCCEEDED(hr))
    {
        hr = MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, m_frameRate.Numerator, m_frameRate.Denominator);
    }

    if (SUCCEEDED(hr))
    {
        hr = pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    }

    if (SUCCEEDED(hr))
    {
        hr = MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    }

    if (SUCCEEDED(hr))
    {
        *ppType = pOutputType;
        (*ppType)->AddRef();
    }

    SafeRelease(&pOutputType);
    LeaveCriticalSection(&m_critSec);
    return hr;
}



//-------------------------------------------------------------------
// Name: SetInputType
//-------------------------------------------------------------------

HRESULT CDecoder::SetInputType(
    DWORD           dwInputStreamID,
    IMFMediaType    *pType, // Can be NULL to clear the input type.
    DWORD           dwFlags 
)
{
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

    EnterCriticalSection(&m_critSec);

    // Does the caller want us to set the type, or just test it?
    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

    // If we have an input sample, the client cannot change the type now.
    if (HasPendingOutput())
    {
        hr = MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
    }

    // Validate the type, if non-NULL.
    if (SUCCEEDED(hr))
    {
        if (pType)
        {
            hr = OnCheckInputType(pType);
        }
    }

    if (SUCCEEDED(hr))
    {
        // The type is OK. 
        // Set the type, unless the caller was just testing.
        if (bReallySet)
        {
            hr = OnSetInputType(pType);
        }
    }
            
    LeaveCriticalSection(&m_critSec);
    return hr;
}



//-------------------------------------------------------------------
// Name: SetOutputType
//-------------------------------------------------------------------

HRESULT CDecoder::SetOutputType(
    DWORD           dwOutputStreamID,
    IMFMediaType    *pType, // Can be NULL to clear the output type.
    DWORD           dwFlags 
)
{
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

    EnterCriticalSection(&m_critSec);

    // Does the caller want us to set the type, or just test it?
     BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

    // If we have an input sample, the client cannot change the type now.
    if (HasPendingOutput())
    {
        hr = MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
    }

    // Validate the type, if non-NULL.
    if (SUCCEEDED(hr))
    {
        if (pType)
        {
            hr = OnCheckOutputType(pType);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (bReallySet)
        {
            // The type is OK. 
            // Set the type, unless the caller was just testing.
            hr = OnSetOutputType(pType);
        }
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}



//-------------------------------------------------------------------
// Name: GetInputCurrentType
// Description: Returns the current input type.
//-------------------------------------------------------------------

HRESULT CDecoder::GetInputCurrentType(
    DWORD           dwInputStreamID,
    IMFMediaType    **ppType
)
{
    if (ppType == NULL)
    {
        return E_POINTER;
    }

    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    if (!m_pInputType)
    {
        hr = MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    if (SUCCEEDED(hr))
    {
        *ppType = m_pInputType;
        (*ppType)->AddRef();
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}



//-------------------------------------------------------------------
// Name: GetOutputCurrentType
// Description: Returns the current output type.
//-------------------------------------------------------------------

HRESULT CDecoder::GetOutputCurrentType(
    DWORD           dwOutputStreamID,
    IMFMediaType    **ppType
)
{
    if (ppType == NULL)
    {
        return E_POINTER;
    }

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    if (!m_pOutputType)
    {
        hr = MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    if (SUCCEEDED(hr))
    {
        *ppType = m_pOutputType;
        (*ppType)->AddRef();
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}



//-------------------------------------------------------------------
// Name: GetInputStatus
// Description: Query if the MFT is accepting more input.
//-------------------------------------------------------------------

HRESULT CDecoder::GetInputStatus(
    DWORD           dwInputStreamID,
    DWORD           *pdwFlags 
)
{
    if (pdwFlags == NULL)
    {
        return E_POINTER;
    }

    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    EnterCriticalSection(&m_critSec);

    // If we already have an input sample, we don't accept
    // another one until the client calls ProcessOutput or Flush.
    if (HasPendingOutput())
    {
        *pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
    }
    else
    {
        *pdwFlags = 0;
    }

    LeaveCriticalSection(&m_critSec);
    return S_OK;
}



//-------------------------------------------------------------------
// Name: GetOutputStatus
// Description: Query if the MFT can produce output.
//-------------------------------------------------------------------

HRESULT CDecoder::GetOutputStatus(DWORD *pdwFlags)
{
    if (pdwFlags == NULL)
    {
        return E_POINTER;
    }

    EnterCriticalSection(&m_critSec);

    // We can produce an output sample if (and only if)
    // we have an input sample.
    if (HasPendingOutput())
    {
        *pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
    }
    else
    {
        *pdwFlags = 0;
    }

    LeaveCriticalSection(&m_critSec);
    return S_OK;
}



//-------------------------------------------------------------------
// Name: SetOutputBounds
// Sets the range of time stamps that the MFT will output.
//-------------------------------------------------------------------

HRESULT CDecoder::SetOutputBounds(
    LONGLONG        /*hnsLowerBound*/,
    LONGLONG        /*hnsUpperBound*/
)
{
    // Implementation of this method is optional. 
    return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: ProcessEvent
// Sends an event to an input stream.
//-------------------------------------------------------------------

HRESULT CDecoder::ProcessEvent(
    DWORD              /*dwInputStreamID*/,
    IMFMediaEvent      * /*pEvent */
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

HRESULT CDecoder::ProcessMessage(
    MFT_MESSAGE_TYPE    eMessage,
    ULONG_PTR           /*ulParam*/
)
{
    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    switch (eMessage)
    {
    case MFT_MESSAGE_COMMAND_FLUSH:
        // Flush the MFT.
        hr = OnFlush();
        break;

    case MFT_MESSAGE_COMMAND_DRAIN:
        // Set the discontinuity flag on all of the input.
        hr = OnDiscontinuity();
        break;

    case MFT_MESSAGE_SET_D3D_MANAGER:
        // The pipeline should never send this message unless the MFT
        // has the MF_SA_D3D_AWARE attribute set to TRUE. However, if we
        // do get this message, it's invalid and we don't implement it.
        hr = E_NOTIMPL;
        break;


    case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
        hr = AllocateStreamingResources();
        break;

    case MFT_MESSAGE_NOTIFY_END_STREAMING:
        hr = FreeStreamingResources();
        break;

    // These messages do not require a response.
    case MFT_MESSAGE_NOTIFY_START_OF_STREAM: 
    case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
        break;        

    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}
    


//-------------------------------------------------------------------
// Name: ProcessInput
// Description: Process an input sample.
//-------------------------------------------------------------------

HRESULT CDecoder::ProcessInput(
    DWORD               dwInputStreamID,
    IMFSample           *pSample, 
    DWORD               dwFlags 
)
{
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

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;
    LONGLONG rtTimestamp = 0;

    if (!m_pInputType || !m_pOutputType)
    {
        hr = MF_E_NOTACCEPTING;   // Client must set input and output types.
    }
    else if (HasPendingOutput())
    {
        hr = MF_E_NOTACCEPTING;   // We already have an input sample.
    }

    // Convert to a single contiguous buffer.
    if (SUCCEEDED(hr))
    {   
        // NOTE: This does not cause a copy unless there are multiple buffers
        hr = pSample->ConvertToContiguousBuffer(&m_pBuffer);
    }

    if (SUCCEEDED(hr))
    {   
        hr = m_pBuffer->Lock(&m_pbData, NULL, &m_cbData);
    }

    if (SUCCEEDED(hr))
    {
        // Get the time stamp. It is OK if this call fails.
        if (FAILED(pSample->GetSampleTime(&rtTimestamp)))
        {
            rtTimestamp = INVALID_TIME;
        }

        m_StreamState.TimeStamp(rtTimestamp);

        hr = Process();
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}



//-------------------------------------------------------------------
// Name: ProcessOutput
// Description: Process an output sample.
//-------------------------------------------------------------------

HRESULT CDecoder::ProcessOutput(
    DWORD                   dwFlags, 
    DWORD                   cOutputBufferCount,
    MFT_OUTPUT_DATA_BUFFER  *pOutputSamples, // one per stream
    DWORD                   *pdwStatus  
)
{
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

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;
    DWORD cbData = 0;

    IMFMediaBuffer *pOutput = NULL;

    // If we don't have an input sample, we need some input before
    // we can generate any output.
    if (!HasPendingOutput())
    {
        hr = MF_E_TRANSFORM_NEED_MORE_INPUT;
    }

    // Get the output buffer.

    if (SUCCEEDED(hr))
    {
        hr = pOutputSamples[0].pSample->GetBufferByIndex(0, &pOutput);
    }

    if (SUCCEEDED(hr))
    {
        hr = pOutput->GetMaxLength(&cbData);
    }

    if (SUCCEEDED(hr))
    {
        if (cbData < m_cbImageSize)
        {
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = InternalProcessOutput(pOutputSamples[0].pSample, pOutput);
    }

    if (SUCCEEDED(hr))
    {
        //  Update our state
        m_bPicture = false;

        //  Is there any more data to output at this point?
        if (S_OK == Process()) 
        {
            pOutputSamples[0].dwStatus |= MFT_OUTPUT_DATA_BUFFER_INCOMPLETE;
        }
    }    

    SafeRelease(&pOutput);
    LeaveCriticalSection(&m_critSec);
    return hr;
}

// Private class methods


HRESULT CDecoder::InternalProcessOutput(IMFSample *pSample, IMFMediaBuffer *pOutputBuffer)
{
    VideoBufferLock buffer(pOutputBuffer);

    HRESULT hr = S_OK;

    BYTE *pbData = NULL;
    LONG lDefaultStride = 0;
    LONG lActualStride = 0;

    DWORD dwTimeCode = 0;
    LONGLONG rt = 0;
    TCHAR szBuffer[20];

    hr = GetDefaultStride(m_pOutputType, &lDefaultStride);

    if (SUCCEEDED(hr))
    {
        hr = buffer.LockBuffer(lDefaultStride, m_imageHeightInPixels, &pbData, &lActualStride);
    }

    //  Generate our data
    if (SUCCEEDED(hr))
    {
        rt = m_StreamState.PictureTime(&dwTimeCode);
        
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer),
            TEXT("%2.2d:%2.2d:%2.2d:%2.2d\0"),
             (dwTimeCode >> 19) & 0x1F,
             (dwTimeCode >> 13) & 0x3F,
             (dwTimeCode >> 6) & 0x3F,
              dwTimeCode & 0x3F
              );
    }

    if (SUCCEEDED(hr))
    {
        // Update our bitmap with turquoise
        OurFillRect(pbData, m_imageWidthInPixels, m_imageHeightInPixels, lActualStride, RGB(0xFF,0x80,0x80));

        //  Draw our text
        DrawOurText(pbData, m_imageWidthInPixels, m_imageHeightInPixels, lActualStride, szBuffer);
    }

    if (SUCCEEDED(hr))
    {
        hr = pOutputBuffer->SetCurrentLength(m_cbImageSize);
    }

    // Clean point / key frame
    if (SUCCEEDED(hr))
    {
        hr = pSample->SetUINT32(MFSampleExtension_CleanPoint, TRUE);
    }

    //  Set the timestamp
    //  Uncompressed video must always have a timestamp

    if (SUCCEEDED(hr))
    {
        rt = m_StreamState.PictureTime(&dwTimeCode);
        if (rt == INVALID_TIME) 
        {
            rt = m_rtFrame;
        }
        hr = pSample->SetSampleTime(rt);
    }

    if (SUCCEEDED(hr))
    {
        hr = pSample->SetSampleDuration(m_rtLength);

    }

    if (SUCCEEDED(hr))
    {
        m_rtFrame = rt + m_rtLength;
    }

    return hr;
}


HRESULT CDecoder::OnCheckInputType(IMFMediaType *pmt)
{
    HRESULT hr = S_OK;

    //  Check if the type is already set and if so reject any type that's not identical.
    if (m_pInputType)
    {
        DWORD dwFlags = 0;
        if (S_OK == m_pInputType->IsEqual(pmt, &dwFlags))
        {
            return S_OK;
        }
        else
        {
            return MF_E_INVALIDTYPE;
        }
    }

    GUID majortype = { 0 };
    GUID subtype = { 0 };
    UINT32 width = 0, height = 0;
    MFRatio fps = { 0 };
    UINT32 cbSeqHeader = 0;

    //  We accept MFMediaType_Video, MEDIASUBTYPE_MPEG1Video

    hr = pmt->GetMajorType(&majortype);

    if (SUCCEEDED(hr))
    {
        if (majortype != MFMediaType_Video)
        {
            hr = MF_E_INVALIDTYPE;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype);
    }

    if (SUCCEEDED(hr))
    {
        if (subtype != MEDIASUBTYPE_MPEG1Payload)
        {
            hr = MF_E_INVALIDTYPE;
        }
    }

    // At this point a real decoder would validate the MPEG-1 format.

    // Validate the frame size.

    if (SUCCEEDED(hr))
    {
        hr = MFGetAttributeSize(pmt, MF_MT_FRAME_SIZE, &width, &height);
    }

    if (SUCCEEDED(hr))
    {
        if (width > MAX_VIDEO_WIDTH || height > MAX_VIDEO_HEIGHT)
        {
            hr = MF_E_INVALIDTYPE;
        }
    }

    // Make sure there is a frame rate. 
    // *** A real decoder would also validate this. *** 

    if (SUCCEEDED(hr))
    {
        hr = MFGetAttributeRatio(pmt, MF_MT_FRAME_RATE, (UINT32*)&fps.Numerator, (UINT32*)&fps.Denominator);
    }

    // Check for a sequence header. 
    // We don't actually look at it for this sample.
    // *** A real decoder would validate the sequence header. *** 

    if (SUCCEEDED(hr))
    {
        (void)pmt->GetBlobSize(MF_MT_MPEG_SEQUENCE_HEADER, &cbSeqHeader);

        if (cbSeqHeader < MPEG1_VIDEO_SEQ_HEADER_MIN_SIZE)
        {
            hr = MF_E_INVALIDTYPE;
        }
    }

    return hr;
}


HRESULT CDecoder::OnSetInputType(IMFMediaType *pmt)
{
    HRESULT hr = S_OK;

    SafeRelease(&m_pInputType);

    hr = MFGetAttributeSize(pmt, MF_MT_FRAME_SIZE, &m_imageWidthInPixels, &m_imageHeightInPixels);

    if (SUCCEEDED(hr))
    {
       hr = MFGetAttributeRatio(pmt, MF_MT_FRAME_RATE, (UINT32*)&m_frameRate.Numerator, (UINT32*)&m_frameRate.Denominator);
    }

    // Also store the frame duration, derived from the frame rate.
    if (SUCCEEDED(hr))
    {
        hr = MFFrameRateToAverageTimePerFrame(
            m_frameRate.Numerator, 
            m_frameRate.Denominator, 
            &m_rtLength
            );
    }

    if (SUCCEEDED(hr))
    {
        m_cbImageSize = m_imageWidthInPixels * m_imageHeightInPixels * 4;

        m_pInputType = pmt;
        m_pInputType->AddRef();
    }

    return hr;
}

HRESULT CDecoder::OnCheckOutputType(IMFMediaType *pmt)
{
    //  Check if the type is already set and if so reject any type that's not identical.
    if (m_pOutputType)
    {
        DWORD dwFlags = 0;
        if (S_OK == m_pOutputType->IsEqual(pmt, &dwFlags))
        {
            return S_OK;
        }
        else
        {
            return MF_E_INVALIDTYPE;
        }
    }

    if (m_pInputType == NULL)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET; // Input type must be set first.
    }


    HRESULT hr = S_OK;
    BOOL bMatch = FALSE;

    IMFMediaType *pOurType = NULL;

    // Make sure their type is a superset of our proposed output type.
    hr = GetOutputAvailableType(0, 0, &pOurType);

    if (SUCCEEDED(hr))
    {
        hr = pOurType->Compare(pmt, MF_ATTRIBUTES_MATCH_OUR_ITEMS, &bMatch);
    }

    if (SUCCEEDED(hr))
    {
        if (!bMatch)
        {
            hr = MF_E_INVALIDTYPE;
        }
    }

    SafeRelease(&pOurType);
    return hr;
}


HRESULT CDecoder::OnSetOutputType(IMFMediaType *pmt)
{
    SafeRelease(&m_pOutputType);

    m_pOutputType = pmt;
    m_pOutputType->AddRef();

    return S_OK;
}


HRESULT CDecoder::AllocateStreamingResources()
{
    //  Reinitialize variables
    OnDiscontinuity();

    return S_OK;
}

HRESULT CDecoder::FreeStreamingResources()
{
    return S_OK;
}

HRESULT CDecoder::OnDiscontinuity()
{
    //  Zero our timestamp
    m_rtFrame = 0;

    //  No pictures yet
   m_bPicture   = false;

   //  Reset state machine
   m_StreamState.Reset();
   return S_OK;
}

HRESULT CDecoder::OnFlush()
{
    OnDiscontinuity();

    //  Release buffer
    SafeRelease(&m_pBuffer);

    return S_OK;
}


//  Scan input data until either we're exhausted or we find
//  a picture start code
//  Note GOP time codes as we encounter them so we can
//  output time codes

HRESULT CDecoder::Process()
{
    //  Process bytes and update our state machine
    while (m_cbData && !m_bPicture) 
    {
        m_bPicture = m_StreamState.NextByte(*m_pbData);
        m_cbData--;
        m_pbData++;
    }

    //  Release buffer if we're done with it
    if (m_cbData == 0) 
    {
        m_pBuffer->Unlock();

        m_pbData = NULL;

        SafeRelease(&m_pBuffer);
    }

    //  assert that if have no picture to output then we ate all the data
    assert(m_bPicture || m_cbData == 0);
    return S_OK;
}




//-------------------------------------------------------------------
// CStreamState
//-------------------------------------------------------------------


void CStreamState::TimeStamp(REFERENCE_TIME rt)
{
    DWORD dwIndex = m_cbBytes >= 4 ? 0 : m_cbBytes;

    m_arTS[dwIndex].bValid = true;
    m_arTS[dwIndex].rt = rt;
}

void CStreamState::Reset()
{
    m_cbBytes = 0;
    m_dwNextTimeCode = 0;

    for (int i = 0; i < 4; i++) {
        m_arTS[i].bValid = false;
    }
}

bool CStreamState::NextByte(BYTE bData)
{
    assert(m_arTS[0].bValid);

    switch (m_cbBytes) 
    {
        case 0:
            if (bData == 0) {
                m_cbBytes++;
            }
            return false;

        case 1:
            if (bData == 0) {
                m_cbBytes++;
            } else {
                m_cbBytes = 0;

                //  Pick up new timestamp if there was one
                if (m_arTS[1].bValid) {
                    m_arTS[0].rt = m_arTS[1].rt;
                    m_arTS[1].bValid = false;
                }
            }
            return false;

        case 2:
            if (bData == 1) {
                m_cbBytes++;
            } else {
                if (bData == 0) {
                   if (m_arTS[1].bValid) {
                       m_arTS[0].rt = m_arTS[1].rt;
                       m_arTS[1].bValid = false;
                   }
                   if (m_arTS[2].bValid) {
                       m_arTS[1] = m_arTS[2];
                       m_arTS[2].bValid = false;
                   }
                } else {
                    //  Not 0 or 1, revert
                    m_cbBytes = 0;

                    //  and pick up latest timestamp
                    if (m_arTS[1].bValid) {
                        m_arTS[0].rt = m_arTS[1].rt;
                        m_arTS[1].bValid = false;
                    }
                    if (m_arTS[2].bValid) {
                        m_arTS[0].rt = m_arTS[2].rt;
                        m_arTS[2].bValid = false;
                    }
                }
            }
            return false;

        case 3:
        {
            //  It's a start code
            //  Return the timestamp and reset everything
            m_rt = m_arTS[0].rt;

            //  If it's a picture start code can't use this timestamp again.
            if (0 == bData) {
                m_arTS[0].rt = INVALID_TIME;
                m_cbBytes = 0;
            }

            //  Catch up and clean out timestamps
            for (int i = 1; i < 4; i++) {
                if (m_arTS[i].bValid) {
                    m_arTS[0].rt = m_arTS[i].rt;
                    m_arTS[i].bValid = false;
                }
            }

            // Picture start code?
            if (0 == bData) {
                m_cbBytes = 0;
                m_dwTimeCode = m_dwNextTimeCode;
                m_dwNextTimeCode++;
                return true;
            } else {
                //  Is it a GOP start code?
                if (bData == 0xb8) {
                    m_cbBytes++;
                } else {
                    m_cbBytes = 0;
                }
                return false;
            }
        }

        case 4:
        case 5:
        case 6:
        case 7:
            m_bGOPData[m_cbBytes - 4] = bData;
            m_cbBytes++;

            if (m_cbBytes == 8) {
                m_cbBytes = 0;
                m_dwNextTimeCode = ((DWORD)m_bGOPData[0] << 17) +
                                   ((DWORD)m_bGOPData[1] << 9) +
                                   ((DWORD)m_bGOPData[2] << 1) +
                                   ((DWORD)m_bGOPData[3] >> 7);
            }
            return false;
    }

    // Should never reach here
    return false;
};



//-------------------------------------------------------------------
// Static functions
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// TextBitmap
//
// Draws a string onto a monochrome bitmap and returns a pointer to 
// the bitmap. The caller must free the bitmap by calling 
// CoTaskMemFree.
//
// If the function fails, the return value is NULL.
//-------------------------------------------------------------------

PBYTE TextBitmap(LPCTSTR lpsz, SIZE *pSize) 
{
    HDC hdc = CreateCompatibleDC(NULL);
    if(NULL == hdc) {
        return NULL;
    }
    if(!GetTextExtentPoint32(hdc, lpsz, lstrlen(lpsz), pSize)) {
        return NULL;
    }

    // Create our bitmap
    struct {
        BITMAPINFOHEADER bmiHeader;
        DWORD rgbEntries[2];
    } bmi =
    {
        {
            sizeof(BITMAPINFOHEADER),
            pSize->cx,
            pSize->cy,
            1,  // planes
            1,  // bit count
            BI_RGB,
            0,
            0,
            0
        },
        {
            0x00000000,
            0xFFFFFFFF
        }
    };

    HBITMAP hbm = CreateDIBitmap(hdc, &bmi.bmiHeader, 0, NULL, NULL, 0);
    if(NULL == hbm) {
        DeleteDC(hdc);
        return NULL;
    }

    HGDIOBJ hobj = SelectObject(hdc, hbm);
    if(NULL == hobj) {
        DeleteObject(hbm);
        DeleteDC(hdc);
        return NULL;
    }

    PBYTE pbReturn = NULL;
    BOOL bResult = ExtTextOut(hdc, 0, 0, ETO_OPAQUE | ETO_CLIPPED, NULL, lpsz,
        lstrlen(lpsz), NULL);
    SelectObject(hdc, hobj);

    LONG lLines;
    if(bResult) {
        LONG lWidthInBytes = ((pSize->cx + 31) >> 3) & ~3;
        pbReturn = (BYTE*)CoTaskMemAlloc(lWidthInBytes * pSize->cy);

        if(pbReturn) {
            ZeroMemory(pbReturn, lWidthInBytes * pSize->cy);
            lLines = GetDIBits(hdc, hbm, 0, pSize->cy, (PVOID)pbReturn, (BITMAPINFO *)&bmi, DIB_RGB_COLORS);
        }
    }

    DeleteObject(hbm);
    DeleteDC(hdc);
    return pbReturn;
}


//-------------------------------------------------------------------
// OurFillRect
//
// Fills a bitmap with a specified color.
//
// pvih: Pointer to a VIDEOINFOHEADER structure that describes the 
//       bitmap.
// pbData: Pointer to the start of the bitmap.
// color: Fill color.
//-------------------------------------------------------------------

void OurFillRect(PBYTE pbScanline0, DWORD dwWidth, DWORD dwHeight, LONG lStrideInBytes, COLORREF color)
{
    PBYTE pbPixels = pbScanline0;

    // For just filling we don't care which way up the bitmap is - 
    // we just start at pbData
    for(DWORD dwCount = 0; dwCount < dwHeight; dwCount++) {
        DWORD *pWord = (DWORD *)pbPixels;

        for(DWORD dwPixel = 0; dwPixel < dwWidth; dwPixel++) {
            pWord[dwPixel] = (DWORD)color;  
        }

        //  biWidth is the stride
        pbPixels += lStrideInBytes;
    }
}


//-------------------------------------------------------------------
// DrawOurText
// Helper to get some text into memory.
//-------------------------------------------------------------------

void DrawOurText(PBYTE pbTarget, DWORD dwWidthTarget, DWORD dwHeightTarget, LONG lStrideInBytesTarget, LPCTSTR szBuffer)
{
    //  Copy the data into our real buffer (top lhs)
    SIZE size;

    //  Get a bit map representing our bits
    PBYTE pbBits = TextBitmap(szBuffer, &size);
    if(NULL == pbBits) 
    {
        return;
    }

    //  Now copy the data from the DIB section (which is bottom up)
    //  but first check if it's too big
    if(dwWidthTarget >= (DWORD)size.cx && 
        dwHeightTarget > (DWORD)size.cy && 
        size.cx > 0 && size.cy > 0) {
        DWORD dwSourceStride = ((size.cx + 31) >> 3) & ~3;

        for(DWORD dwY = 0; dwY < (DWORD)size.cy; dwY++) {
            DWORD *pwTarget = (DWORD *)pbTarget;
            PBYTE pbSource = pbBits + dwSourceStride * ((DWORD)size.cy - dwY - 1);

            for(DWORD dwX = 0; dwX < (DWORD)size.cx; dwX++) {
                if(!((0x80 >> (dwX & 7)) & pbSource[dwX >> 3])) {
                    pwTarget[dwX] = 0x0000; // Black
                }
            }
            pbTarget += lStrideInBytesTarget;
        }
    }
    
    CoTaskMemFree(pbBits);
}



HRESULT GetDefaultStride(IMFMediaType *pType, LONG *plStride)
{
    LONG lStride = 0;

    // Try to get the default stride from the media type.
    HRESULT hr = pType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&lStride);
    if (FAILED(hr))
    {
        // Attribute not set. Try to calculate the default stride.
        GUID subtype = GUID_NULL;

        UINT32 width = 0;
        UINT32 height = 0;

        // Get the subtype and the image size.
        hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (SUCCEEDED(hr))
        {
            hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
        }
        if (SUCCEEDED(hr))
        {
            hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &lStride);
        }

        // Set the attribute for later reference.
        if (SUCCEEDED(hr))
        {
            (void)pType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(lStride));
        }
    }

    if (SUCCEEDED(hr))
    {
        *plStride = lStride;
    }
    return hr;
}