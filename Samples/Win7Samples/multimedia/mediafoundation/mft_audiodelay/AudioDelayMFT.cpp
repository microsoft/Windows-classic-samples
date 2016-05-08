//////////////////////////////////////////////////////////////////////////
//
// AudioDelayMFT.cpp
// Implements an audio effect as a Media Foundation transform.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "AudioDelayMFT.h"
#include "AudioDelayUuids.h"

#define CHECK_HR(hr) IF_FAILED_GOTO(hr, done)


HRESULT ValidatePCMAudioType(IMFMediaType *pmt);

//-------------------------------------------------------------------
// Name: CreateInstance (static method)
// Creates a new instance of the MFT.
// 
// This method is called by the class factory.
// 
// pUnkOuter: Aggregating IUnknown.
// iid: IID of the interface to query for.
// ppv: Receives the interface pointer.
//-------------------------------------------------------------------
HRESULT CDelayMFT::CreateInstance(IUnknown *pUnkOuter, REFIID iid, void **ppv)
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

    CDelayMFT *pMFT = new CDelayMFT();

    if (pMFT == NULL)
    {
        return E_OUTOFMEMORY;
    }

    hr = pMFT->QueryInterface(iid, ppv);

    SAFE_RELEASE(pMFT);

    return hr;
}

//-------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------

CDelayMFT::CDelayMFT() :
    m_nRefCount(1),
    m_pBuffer(NULL),
    m_pSample(NULL),
    m_pMediaType(NULL),
    m_pAttributes(NULL),
    m_pbInputData(NULL),
    m_cbInputLength(0),
    m_rtTimestamp(0),
    m_bValidTime(NULL),
    m_bInputTypeSet(FALSE),
    m_bOutputTypeSet(FALSE),
    m_pbDelayBuffer(NULL),
    m_cbDelayBuffer(0),
    m_pbDelayPtr(NULL),
    m_dwDelay(DEFAULT_DELAY),
    m_bDraining(FALSE),
    m_cbTailSamples(0)
{

}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------

CDelayMFT::~CDelayMFT()
{
    FreeStreamingResources(FALSE);  
    SAFE_RELEASE(m_pMediaType);
    SAFE_RELEASE(m_pAttributes);
}


// IUnknown methods.

ULONG CDelayMFT::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG CDelayMFT::Release()
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

HRESULT CDelayMFT::QueryInterface(REFIID riid, void **ppv)
{
    if (NULL == ppv)
    {
        return E_POINTER;
    }
    else if (riid == __uuidof(IUnknown))
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (riid == __uuidof(IMFTransform))
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


//-------------------------------------------------------------------
// IMFTransform methods
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// Name: GetStreamLimits
// Returns the minimum and maximum number of streams.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetStreamLimits( 
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
// Returns the current number of streams.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetStreamCount(DWORD *pcInputStreams, DWORD *pcOutputStreams)
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
// Returns the stream identifiers.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetStreamIDs(
    DWORD   dwInputIDArraySize,
    DWORD   *pdwInputIDs,
    DWORD   dwOutputIDArraySize,
    DWORD   *pdwOutputIDs
    )
{
    // This MFT has a fixed number of streams and the stream IDs match 
    // the zero-based index of the streams. Therefore, it is not required
    // to implement this method.
    return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: GetInputStreamInfo
// Returns information about the input stream.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO *pStreamInfo)
{
    AutoLock lock(m_critSec);
    
    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }


    pStreamInfo->hnsMaxLatency = 0;

    // Flags
    pStreamInfo->dwFlags = 
        MFT_INPUT_STREAM_WHOLE_SAMPLES |        // The MFT must get complete audio frames. 
        MFT_INPUT_STREAM_PROCESSES_IN_PLACE |   // The MFT can do in-place processing.
        MFT_INPUT_STREAM_FIXED_SAMPLE_SIZE;     // Samples (ie, audio frames) are fixed size.

    pStreamInfo->cbSize = 0;  // If no media type is set, use zero. 
    pStreamInfo->cbMaxLookahead = 0;
    pStreamInfo->cbAlignment = 0;

    // When the media type is set, return the minimum buffer size = one audio frame.
    if (IsInputTypeSet())
    {
        pStreamInfo->cbSize = BlockAlign();
    }

    return S_OK;
}

//-------------------------------------------------------------------
// Name: GetOutputStreamInfo
// Returns information about the output stream.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO *pStreamInfo)
{
    AutoLock lock(m_critSec);

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // Flags
    pStreamInfo->dwFlags = 
        MFT_OUTPUT_STREAM_WHOLE_SAMPLES |         // Output buffers contain complete audio frames. 
        MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES |   // The MFT can allocate output buffers, or use caller-allocated buffers.
        MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE;      // Samples (ie, audio frames) are fixed size. 

    pStreamInfo->cbSize = 0;   // If no media type is set, use zero. 
    pStreamInfo->cbAlignment = 0;

    // When the media type is set, return the minimum buffer size = one audio frame.
    if (m_bOutputTypeSet)
    {
        pStreamInfo->cbSize = BlockAlign();
    }

    return S_OK;
}

//-------------------------------------------------------------------
// Name: GetAttributes
// Returns an attribute store for MFT-wide attributes.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetAttributes(IMFAttributes** ppAttributes)
{
    if (ppAttributes == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    // Lazily create the attribute store.
    CHECK_HR(hr = CreateAttributeStore());

    *ppAttributes = m_pAttributes;
    (*ppAttributes)->AddRef();

done:
    return hr;
}

//-------------------------------------------------------------------
// Name: GetInputStreamAttributes
// Returns an attribute store for input stream attributes.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetInputStreamAttributes(DWORD dwInputStreamID, IMFAttributes **ppAttributes)
{
    // This MFT does not support any stream-level attributes.
    return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: GetOutputStreamAttributes
// Returns an attribute store for output stream attributes.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetOutputStreamAttributes(DWORD dwOutputStreamID, IMFAttributes **ppAttributes)
{
    // This MFT does not support any stream-level attributes.
    return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: DeleteInputStream
// Deletes an input stream.
//-------------------------------------------------------------------

HRESULT CDelayMFT::DeleteInputStream(DWORD dwStreamID)
{
    // This MFT has a fixed number of streams.
    return E_NOTIMPL;
}

//-------------------------------------------------------------------
// Name: AddInputStreams
// Adds one or more input streams.
//-------------------------------------------------------------------

HRESULT CDelayMFT::AddInputStreams(DWORD cStreams, DWORD *adwStreamIDs)
{
    // This MFT has a fixed number of streams.
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// Name: GetInputAvailableType
// Returns a preferred media type for the input stream.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetInputAvailableType(
    DWORD           dwInputStreamID,    // Input stream ID.
    DWORD           dwTypeIndex,        // 0-based index into the list of preferred types.
    IMFMediaType    **ppType            // Receives a pointer to the media type.
    )
{
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
    if (IsOutputTypeSet())
    {
        // Only one preferred type in this case.
        if (dwTypeIndex > 0)
        {
            return MF_E_NO_MORE_TYPES;
        }

//        ASSERT(m_pMediaType != NULL);

        *ppType = m_pMediaType;
        (*ppType)->AddRef();
    }
    else
    {
        // The output type is not set. Create a partial media type.
        hr = GetProposedType(dwTypeIndex, ppType);
    }
    return hr;
}


//-------------------------------------------------------------------
// Name: GetOutputAvailableType
// Returns a preferred media type for the output stream.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetOutputAvailableType(
    DWORD           dwOutputStreamID,   // Output stream ID.
    DWORD           dwTypeIndex,        // 0-based index into the list of preferred types.
    IMFMediaType    **ppType            // Receives a pointer to the media type.
    )
{
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
    if (IsInputTypeSet())
    {
        // Only one preferred type in this case.
        if (dwTypeIndex > 0)
        {
            return MF_E_NO_MORE_TYPES;
        }

        *ppType = m_pMediaType;
        (*ppType)->AddRef();
    }
    else
    {
        // The input type is not set. Create a partial media type.
        hr = GetProposedType(dwTypeIndex, ppType);
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: SetInputType
// Sets the input type.
//-------------------------------------------------------------------

HRESULT CDelayMFT::SetInputType(
      DWORD           dwInputStreamID,
      IMFMediaType    *pType, // Can be NULL to clear the input type.
      DWORD           dwFlags 
      )
{
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

    if ((dwFlags & MFT_SET_TYPE_TEST_ONLY) && (pType == NULL))
    {
        return E_INVALIDARG;
    }

    // If we have output, the client cannot change the type now.
    if (HasPendingOutput())
    {
        return MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
    }


    HRESULT hr = S_OK;

    // Does the caller want us to set the type, or just test it?
    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

    // Validate the type.
    if (pType)
    {
        CHECK_HR(hr = OnCheckInputType(pType));
    }

    // The type is OK. 
    // Set or clear the type, unless the caller was just testing.
    if (bReallySet)
    {
        CHECK_HR(hr = OnSetMediaType(pType, InputStream));
    }

done:
    return hr;
}

//-------------------------------------------------------------------
// Name: SetOutputType
// Sets the output type.
//-------------------------------------------------------------------

HRESULT CDelayMFT::SetOutputType(
       DWORD           dwOutputStreamID,
       IMFMediaType    *pType, // Can be NULL to clear the output type.
       DWORD           dwFlags 
       )
{
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

    if ((dwFlags & MFT_SET_TYPE_TEST_ONLY) && (pType == NULL))
    {
        return E_INVALIDARG;
    }

    // If we have output, the client cannot change the type now.
    if (HasPendingOutput())
    {
        return MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
    }

    HRESULT hr = S_OK;

    // Does the caller want us to set the type, or just test it?
    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

    // Validate the type.
    if (pType)
    {
        CHECK_HR(hr = OnCheckOutputType(pType));
    }

    // The type is OK. 
    // Set or clear the type, unless the caller was just testing.
    if (bReallySet)
    {
        CHECK_HR(hr = OnSetMediaType(pType, OutputStream));
    }

done:
    return hr;
}


//-------------------------------------------------------------------
// Name: GetInputCurrentType
// Returns the current input type.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType **ppType)
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

    if (!IsInputTypeSet())
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    *ppType = m_pMediaType;
    (*ppType)->AddRef();

    return S_OK;
}

//-------------------------------------------------------------------
// Name: GetOutputCurrentType
// Returns the current output type.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType **ppType)
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

    if (!m_bOutputTypeSet)
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    *ppType = m_pMediaType;
    (*ppType)->AddRef();

    return S_OK;
}


//-------------------------------------------------------------------
// Name: GetInputStatus
// Queries whether the MFT can accept more input at this time.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetInputStatus(DWORD dwInputStreamID, DWORD *pdwFlags)
{
    AutoLock lock(m_critSec);

    if (pdwFlags == NULL)
    {
        return E_POINTER;
    }

    if (!IsValidInputStream(dwInputStreamID))
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // If we have output data to give to the client, then we don't accept
    // new input until the client calls ProcessOutput or Flush.
    if (HasPendingOutput())
    {
        *pdwFlags = 0;
    }
    else
    {
        *pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
    }

    return S_OK;
}


//-------------------------------------------------------------------
// Name: GetOutputStatus
// Queries whether the MFT can produce output at this time.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetOutputStatus(DWORD *pdwFlags)
{
    AutoLock lock(m_critSec);

    if (pdwFlags == NULL)
    {
        return E_POINTER;
    }

    if (HasPendingOutput())
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
// Sets the range of timestamps the client needs for output.
//
// Implementation of this method is optional. 
//-------------------------------------------------------------------

HRESULT CDelayMFT::SetOutputBounds(LONGLONG hnsLowerBound, LONGLONG hnsUpperBound)
{
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// Name: ProcessEvent
// Sends a media event to the MFT.
//-------------------------------------------------------------------

HRESULT CDelayMFT::ProcessEvent(DWORD dwInputStreamID, IMFMediaEvent *pEvent)
{
    // This MFT does not respond to any events.
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// Name: ProcessMessage
// Sends a message to the MFT.
//-------------------------------------------------------------------

HRESULT CDelayMFT::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR ulParam)
{
    AutoLock lock(m_critSec);

    HRESULT hr = S_OK;

    switch (eMessage)
    {
    case MFT_MESSAGE_COMMAND_FLUSH:
        FreeStreamingResources(TRUE);
        break;

    case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
        hr = AllocateStreamingResources();
        break;

    case MFT_MESSAGE_NOTIFY_END_STREAMING:
        FreeStreamingResources(FALSE);
        break;

    case MFT_MESSAGE_COMMAND_DRAIN:
        // Drain: Tells the MFT not to accept any more input until 
        // all of the pending output has been processed. 
        hr = OnDrain();
        break;
       
    case MFT_MESSAGE_SET_D3D_MANAGER:
        // The pipeline should never send this message unless the MFT
        // has the MF_SA_D3D_AWARE attribute set to TRUE. However, if we
        // do get this message, it's invalid and we don't implement it.
        hr = E_NOTIMPL;
        break;

    // The remaining messages do not require any action from this MFT.
    case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
    case MFT_MESSAGE_NOTIFY_START_OF_STREAM: 
        break;
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: ProcessInput
// Gives an input sample to the MFT.
//-------------------------------------------------------------------

HRESULT CDelayMFT::ProcessInput(
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

    if (!IsInputTypeSet() || !IsOutputTypeSet())
    {
        // The client must set the input and output types before 
        // calling ProcessInput.
        return MF_E_NOTACCEPTING;   
    }

    if (HasPendingOutput())
    {
        // Not accepting input because there is still data to process.
        return MF_E_NOTACCEPTING;   
    }

    HRESULT hr = S_OK;
    DWORD dwBufferCount = 0;

    // Allocate resources, in case the client did not send the 
    // MFT_MESSAGE_NOTIFY_BEGIN_STREAMING message.
    CHECK_HR(hr = AllocateStreamingResources());

    // Get the input buffer(s) from the sample.
    CHECK_HR(hr = pSample->GetBufferCount(&dwBufferCount));

    // Convert to a single contiguous buffer.
    // NOTE: This does not cause a copy unless there are multiple buffers
    CHECK_HR(hr = pSample->ConvertToContiguousBuffer(&m_pBuffer));

    // Cache the sample.
    m_pSample = pSample;
    m_pSample->AddRef();

    // Save the time stamp, if we got one from the caller.
    // (We don't care about the duration because it is implicit in the size 
    // of the audio data.)

    LONGLONG hnsTime = 0;

    // Ignore failure if the input sample does not have a time stamp. This is 
    // not an error condition. The client may not care about time stamps, and 
    // we don't need them.
    if (SUCCEEDED(pSample->GetSampleTime(&hnsTime)))    
    {
        m_bValidTime = TRUE;
        m_rtTimestamp = hnsTime;
    }
    else
    {
        m_bValidTime = FALSE;
    }


done:
    return hr;
}

//-------------------------------------------------------------------
// Name: ProcessOutput
// Generates output data.
//-------------------------------------------------------------------

HRESULT CDelayMFT::ProcessOutput(
       DWORD                    dwFlags, 
       DWORD                    cOutputBufferCount,
       MFT_OUTPUT_DATA_BUFFER  *pOutputSamples, // one per stream
       DWORD                   *pdwStatus  
       )
{

    AutoLock lock(m_critSec);

    // Check input parameters...

    // The only flag is MFT_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER, which applies 
    // if the MFT has "lazy" or "optional" streams. This MFT does not have any
    // lazy or optional streams, so that flag is not valid.
    if (dwFlags != 0)
    {
        return E_INVALIDARG;
    }

    if (pOutputSamples == NULL || pdwStatus == NULL)
    {
        return E_POINTER;
    }

    // The MFT has exactly one output stream.
    if (cOutputBufferCount != 1)
    {
        return E_INVALIDARG;
    }

    // NOTE: To support in-place processing, we allow pOutputSamples[0].pSample to be NULL. 

    // Check if we are able to deliver any output at this time.
    if (!HasPendingOutput())
    {
        return MF_E_TRANSFORM_NEED_MORE_INPUT;
    }

    // We can deliver output, so one or both of the following is true:
    //
    // 1. We have an input buffer.
    // 2. We are draining the effect tail.
    //
    // If both are true, process the input buffer before the tail.

    assert((m_pBuffer && m_pSample) || (m_bDraining && (m_cbTailSamples > 0)));

    HRESULT hr = S_OK;

    if (m_pBuffer)
    {
        // Process the input.
        hr = InternalProcessOutput(pOutputSamples[0], pdwStatus);
    }
    else
    {
        // Output the effect tail.
        hr = ProcessEffectTail(pOutputSamples[0], pdwStatus);
    }

    return hr;
}


//-------------------------------------------------------------------
// Private methods
//-------------------------------------------------------------------


//-------------------------------------------------------------------
// Name: InternalProcessOutput 
// Does the work of IMFTransform::ProcessOutput, in the case where 
// the MFT has input data.
//
// When the MFT is generating the effect tail, the ProcessEffectTail 
// method is used instead.
//
// All input parameters are validated before this method is called.
//-------------------------------------------------------------------

HRESULT CDelayMFT::InternalProcessOutput(MFT_OUTPUT_DATA_BUFFER& OutputSample, DWORD *pdwStatus)
{
    IMFMediaBuffer *pOutputBuffer = NULL;

    HRESULT     hr = S_OK;
    BYTE        *pbOutputData = NULL;   // Pointer to the memory in the output buffer.
    DWORD       cbOutputLength = 0;     // Size of the output buffer.
    DWORD       cbBytesProcessed = 0;   // How much data we processed.
    BOOL        bComplete = FALSE;      // Are we done with the input buffer?
    LONGLONG    hnsDuration = 0;        // Duration of the output sample.
    UINT32      nBlockAlign = 0; 

    assert(m_pBuffer != NULL);

    nBlockAlign = BlockAlign();
    if (nBlockAlign == 0)
    {
        return E_UNEXPECTED;
    }

    // Lock the input buffer.
    if (m_pbInputData == NULL)
    {
        CHECK_HR(hr = m_pBuffer->Lock(&m_pbInputData, NULL, &m_cbInputLength));
    }

    // If the client provided an output sample, get the output buffer.
    if (OutputSample.pSample != NULL)
    {
        CHECK_HR(hr = OutputSample.pSample->GetBufferByIndex(0, &pOutputBuffer));

        // Lock the output buffer.
        CHECK_HR(hr = pOutputBuffer->Lock(&pbOutputData, &cbOutputLength, NULL));
    }
    else
    {
        // Client did not provide an output sample. Use the input buffer and transform the data in place.
        pbOutputData = m_pbInputData;
        cbOutputLength = m_cbInputLength;

        // Return the input sample as the output sample.
        OutputSample.pSample = m_pSample;
        OutputSample.pSample->AddRef();
    }


    // Calculate how many audio samples we can process.
    if (m_cbInputLength > cbOutputLength)
    {
        cbBytesProcessed = cbOutputLength;
    }
    else
    {
        cbBytesProcessed = m_cbInputLength;
        bComplete = TRUE;
    }

   // Round to the next lowest multiple of nBlockAlign.
   cbBytesProcessed -= (cbBytesProcessed % nBlockAlign);

    // Process the data.
    CHECK_HR(hr = ProcessAudio(pbOutputData, m_pbInputData, cbBytesProcessed / nBlockAlign));

    // Update the output buffer/sample (if provided)
    if (pOutputBuffer)
    {
        // Set the data length on the output buffer.
        CHECK_HR(hr = pOutputBuffer->SetCurrentLength(cbBytesProcessed));

        // Set the time stamp, if we have a valid time from the input sample.
        if (m_bValidTime)
        {
            // Estimate how far along we are...
            hnsDuration = (cbBytesProcessed / AvgBytesPerSec()) * UNITS;

            // Set the time stamp and duration on the output sample.
            CHECK_HR(hr = OutputSample.pSample->SetSampleTime(m_rtTimestamp));
            CHECK_HR(hr = OutputSample.pSample->SetSampleDuration(hnsDuration));

            m_rtTimestamp += hnsDuration;
        }
    }

    // Set status flags.
    OutputSample.dwStatus = 0; 
    *pdwStatus = 0;

    if (bComplete)
    {
        // We are done with this input buffer. Release it.
        CHECK_HR(hr = m_pBuffer->Unlock());

        SAFE_RELEASE(m_pBuffer);
        SAFE_RELEASE(m_pSample);

        m_pbInputData = 0;
        m_cbInputLength = 0;
    }
    else 
    {
        // There is still data in the input buffer.

        // Update the running count of bytes processed.
        m_cbInputLength -= cbBytesProcessed;
        m_pbInputData += cbBytesProcessed;

        // Set the "incomplete" flag to notify the caller that we have more output to produce.
        OutputSample.dwStatus |= MFT_OUTPUT_DATA_BUFFER_INCOMPLETE;
    }

done:
    // Unlock the output buffer.
    if (pOutputBuffer && pbOutputData)
    {
        pOutputBuffer->Unlock();
    }

    SAFE_RELEASE(pOutputBuffer);
    return hr;
}


//-------------------------------------------------------------------
// ProcessEffectTail
// Generates the "tail" of the audio effect. The tail is the portion
// of the delay effect that is heard after the input stream ends.
// 
// To generate the tail, the client must drain the MFT by sending
// the MFT_MESSAGE_COMMAND_DRAIN message and then call ProcessOutput
// to get the tail samples.
//-------------------------------------------------------------------

HRESULT CDelayMFT::ProcessEffectTail(MFT_OUTPUT_DATA_BUFFER& OutputSample, DWORD *pdwStatus)
{
    IMFSample *pSample = NULL;
    IMFMediaBuffer *pOutputBuffer = NULL;
    
    HRESULT     hr = S_OK;
    BYTE        *pbOutputData = NULL;   // Pointer to the memory in the output buffer.
    DWORD       cbOutputLength = 0;     // Size of the output buffer.
    DWORD       cbBytesProcessed = 0;   // How much data we processed.
    LONGLONG    hnsDuration = 0;        // Duration of the output sample.
    UINT32      nBlockAlign = 0;

    nBlockAlign = BlockAlign();
    if (nBlockAlign == 0)
    {
        return E_UNEXPECTED;
    }

    // If the caller provided an output sample, get the output buffer.
    if (OutputSample.pSample != NULL)
    {
        CHECK_HR(hr = OutputSample.pSample->GetBufferByIndex(0, &pOutputBuffer));

        pSample = OutputSample.pSample;
        pSample->AddRef();
    }
    else
    {
        // The caller did not provide an output sample. Allocate one.
        CHECK_HR(hr = MFCreateMemoryBuffer(m_cbTailSamples, &pOutputBuffer));

        CHECK_HR(hr = MFCreateSample(&pSample));

        CHECK_HR(hr = pSample->AddBuffer(pOutputBuffer));

        OutputSample.pSample = pSample;
        OutputSample.pSample->AddRef();
    }

    // Lock the output buffer.
    CHECK_HR(hr = pOutputBuffer->Lock(&pbOutputData, &cbOutputLength, NULL));

    // Calculate how many audio samples we can process.
    cbBytesProcessed = min(m_cbTailSamples, cbOutputLength);

    // Round to the next lowest multiple of nBlockAlign.
    cbBytesProcessed -= (cbBytesProcessed % nBlockAlign);

    // Fill the output buffer with silence, because we are also using it as the input buffer.
    FillBufferWithSilence(pbOutputData, cbBytesProcessed);

    // Process the data.
    CHECK_HR(hr = ProcessAudio(pbOutputData, pbOutputData, cbBytesProcessed / nBlockAlign));

    // Set the data length on the output buffer.
    CHECK_HR(hr = pOutputBuffer->SetCurrentLength(cbBytesProcessed));

    // Set the time stamp, if we have a valid time from the last input sample.
    if (m_bValidTime)
    {
        // Estimate how far along we are...
        hnsDuration = (cbBytesProcessed / AvgBytesPerSec()) * UNITS;

        // Set the time stamp and duration on the output sample.
        CHECK_HR(hr = OutputSample.pSample->SetSampleTime(m_rtTimestamp));
        CHECK_HR(hr = OutputSample.pSample->SetSampleDuration(hnsDuration));

        m_rtTimestamp += hnsDuration;
    }

    // Set status flags.
    OutputSample.dwStatus = 0; 
    *pdwStatus = 0;

    // How many tail samples are left?
    m_cbTailSamples -= cbBytesProcessed;

    if (m_cbTailSamples >= nBlockAlign)
    {
        // Still some data left.
        OutputSample.dwStatus |= MFT_OUTPUT_DATA_BUFFER_INCOMPLETE;
    }
    else
    {
        // Done.
        m_cbTailSamples = 0;
        m_bDraining = FALSE;
    }

done:
    if (pOutputBuffer && pbOutputData)
    {
        pOutputBuffer->Unlock();
    }

    SAFE_RELEASE(pOutputBuffer);
    SAFE_RELEASE(pSample);
    return hr;
}


//-------------------------------------------------------------------
// AllocateStreamingResources
// Allocates resources needed to process data.
//
// This method is called if the client sends the 
// MFT_MESSAGE_NOTIFY_BEGIN_STREAMING message. It is also called from
// ProcessInput(), because the client is not required to send the
// MFT_MESSAGE_NOTIFY_BEGIN_STREAMING message.
//-------------------------------------------------------------------

HRESULT CDelayMFT::AllocateStreamingResources()
{
    if (m_pbDelayBuffer != NULL)
    {
        return S_OK; // Already allocated. Nothing to do.
    }

    // The client must set both media types before allocating streaming resources.
    if (!IsInputTypeSet() || !IsOutputTypeSet())
    {
        return MF_E_TRANSFORM_TYPE_NOT_SET;
    }

    HRESULT hr = S_OK;

    // Lazily create the attribute store, which holds the attribute for the delay length.
    CHECK_HR(hr = CreateAttributeStore());

    // Get the delay length. 
    m_dwDelay = MFGetAttributeUINT32(m_pAttributes, MF_AUDIODELAY_DELAY_LENGTH, DEFAULT_DELAY);

    // A zero-length delay buffer will complicate things, so disallow zero.
    // Use the default instead.
    if (m_dwDelay == 0)
    {
        m_dwDelay = DEFAULT_DELAY;
    }

    // Make sure the delay buffer won't exceed MAXDWORD bytes.
    m_dwDelay = min( m_dwDelay, MAXDWORD / (SamplesPerSec() * BlockAlign()) );


    // Allocate the buffer that holds the delayed samples.
    m_cbDelayBuffer = (m_dwDelay * SamplesPerSec() * BlockAlign()) / 1000;
    m_pbDelayBuffer = (BYTE*)CoTaskMemAlloc(m_cbDelayBuffer);
    
    if (m_pbDelayBuffer == NULL)
    {
        CHECK_HR(hr = E_OUTOFMEMORY);
    }
    
    FillBufferWithSilence(m_pbDelayBuffer, m_cbDelayBuffer);    
    
    m_pbDelayPtr = m_pbDelayBuffer;

done:
    return hr;
}


//-------------------------------------------------------------------
// FreeStreamingResources
// Releases resources allocated during streaming. 
//
// Called for two messages:
// - MFT_MESSAGE_NOTIFY_END_STREAMING
// - MFT_MESSAGE_COMMAND_FLUSH
// 
// The only difference is that "end streaming" frees the delay buffer,
// while "flush" fills the delay buffer with silence.
//-------------------------------------------------------------------

void CDelayMFT::FreeStreamingResources(BOOL bFlush)
{
    // Unlock the input buffer, if locked.
    if (m_pbInputData)
    {
        assert(m_pBuffer != NULL);  // We should never release the buffer while it is still locked.
        m_pBuffer->Unlock();
    }

    // Release the input buffer. 
    SAFE_RELEASE(m_pBuffer);
    SAFE_RELEASE(m_pSample);

    m_pbInputData = NULL;
    m_cbInputLength = 0;

    if (bFlush)
    {
        // Fill the delay buffer with silence.
        FillBufferWithSilence(m_pbDelayBuffer, m_cbDelayBuffer);
    }
    else
    {
        // Free the delay buffer.
        CoTaskMemFree(m_pbDelayBuffer);
        m_pbDelayBuffer = m_pbDelayPtr = NULL;
    }

    m_bValidTime = FALSE;
    m_bDraining = FALSE;
    m_cbTailSamples = 0;

    // Things we do not release in this method:
    // m_pMediaType - The media type should not change unless the caller explicitly changes it.
    // m_pAttributes - We keep the attribute store alive until the destructor is called.
}


//-------------------------------------------------------------------
// CreateAttributeStore
// Creates the MFT's attribute store, if it does not exist yet.
//-------------------------------------------------------------------

HRESULT CDelayMFT::CreateAttributeStore()
{
    HRESULT hr = S_OK;

    if (m_pAttributes == NULL)
    {
        // Create the attribute store.
        CHECK_HR(hr = MFCreateAttributes(&m_pAttributes, ATTRIBUTE_COUNT));

        // Set initial values.
        CHECK_HR(hr = m_pAttributes->SetUINT32(MF_AUDIODELAY_WET_DRY_MIX, DEFAULT_WET_DRY_MIX));
        CHECK_HR(hr = m_pAttributes->SetUINT32(MF_AUDIODELAY_DELAY_LENGTH, m_dwDelay));
    }

done:
    return hr;
}


//-------------------------------------------------------------------
// Name: GetProposedType
// Description: Returns a preferred media type from our list.
//
// dwTypeIndex: Index into the list of peferred media types.
// ppmt: Receives a pointer to the media type.
//-------------------------------------------------------------------

HRESULT CDelayMFT::GetProposedType(DWORD dwTypeIndex, IMFMediaType **ppmt)
{
    if (dwTypeIndex > 1)
    {
        return MF_E_NO_MORE_TYPES;
    }

    IMFMediaType *pType = NULL;
    HRESULT hr = S_OK;

    CHECK_HR(hr = MFCreateMediaType(&pType));

    // The major type is always Audio
    CHECK_HR(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));

    switch (dwTypeIndex)
    {
    case 0:
        // Partial type: PCM audio
        CHECK_HR(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));
        break;

    case 1:
        // Full type: Propose 48 kHz, 16-bit, 2-channel

        const UINT32 SamplesPerSec = 48000;
        const UINT32 BitsPerSample = 16;
        const UINT32 NumChannels = 2;
        const UINT32 BlockAlign = NumChannels * BitsPerSample / 8;

        CHECK_HR(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));
        CHECK_HR(hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, SamplesPerSec));
        CHECK_HR(hr = pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, BitsPerSample));
        CHECK_HR(hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, NumChannels));
        CHECK_HR(hr = pType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, BlockAlign));
        CHECK_HR(hr = pType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, BlockAlign * SamplesPerSec));
        CHECK_HR(hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
        break;
    }

    *ppmt = pType;
    (*ppmt)->AddRef();

done:
    SAFE_RELEASE(pType);
    return hr;
}

//-------------------------------------------------------------------
// OnCheckInputType
// Validate an input media type.
//-------------------------------------------------------------------

HRESULT CDelayMFT::OnCheckInputType(IMFMediaType *pmt)
{
    assert(pmt != NULL);

    HRESULT hr = S_OK;

    // If the output type is set, see if they match.
    if (IsOutputTypeSet())
    {
        DWORD flags = 0;
        hr = pmt->IsEqual(m_pMediaType, &flags);

        // IsEqual can return S_FALSE. Treat this as failure.

        if (hr != S_OK)
        {
            hr = MF_E_INVALIDMEDIATYPE;
        }
    }
    else
    {
        // Output type is not set. Just check this type.
        hr = ValidatePCMAudioType(pmt);
    }
    return hr;
}


//-------------------------------------------------------------------
// Name: OnCheckOutputType
// Validate an output media type.
//
// Note: This function is exactly parallel to OnCheckInputType.
//-------------------------------------------------------------------

HRESULT CDelayMFT::OnCheckOutputType(IMFMediaType *pmt)
{
    assert(pmt != NULL);

    HRESULT hr = S_OK;

    // If the input type is set, see if they match.
    if (IsInputTypeSet())
    {
        DWORD flags = 0;
        hr = pmt->IsEqual(m_pMediaType, &flags);

        // IsEqual can return S_FALSE. Treat this as failure.

        if (hr != S_OK)
        {
            hr = MF_E_INVALIDMEDIATYPE;
        }

    }
    else
    {
        // Input type is not set. Just check this type.
        hr = ValidatePCMAudioType(pmt);
    }

    return hr;    
}

//-------------------------------------------------------------------
// Name: OnSetMediaType
// Set (or clear) the input or output type.
//
// When this method is called, the type has already been validated.
//
// pType: Pointer to the media type to set. If NULL, clear the type.
// dir: Specifies whether to set the input type or the output type.
//-------------------------------------------------------------------

HRESULT CDelayMFT::OnSetMediaType(IMFMediaType *pType, StreamDirection dir)
{
    HRESULT hr = S_OK;
    BOOL bInputType = (dir == InputStream);

    // Note:
    // This MFT requires the input type to match the output type.
    // Therefore, the MFT only stores one type object, and maintains two 
    // Boolean flags to indicate whether the client has set the input or
    // output type.

    if (pType)
    {
        // Store the media type.
        SAFE_RELEASE(m_pMediaType);
        m_pMediaType = pType;
        m_pMediaType->AddRef();

        // Flag the stream (input or output) as set.
        if (bInputType)
        {
            m_bInputTypeSet = TRUE;
        }
        else
        {
            m_bOutputTypeSet = TRUE;
        }
    }
    else
    {
        // Clear the media type.
        if (bInputType)
        {
            m_bInputTypeSet = FALSE;
        }
        else
        {
            m_bOutputTypeSet = FALSE;
        }

        // If both types (input and output) are not set, we can release our media type pointer.
        if (!m_bOutputTypeSet)
        {
            SAFE_RELEASE(m_pMediaType);
        }
    }
    return hr;
}

//-------------------------------------------------------------------
// Name: OnDrain
// Called when the client send the MFT_MESSAGE_COMMAND_DRAIN message.
//
// Swiches the MFT to "drain" mode. In this mode, the MFT does not
// accept input until the client has generated output for the "tail"
// of the delay effect. For details, see ProcessEffectTail().
// (The MFT will also accept new input if the client flushes the MFT.)
//-------------------------------------------------------------------

HRESULT CDelayMFT::OnDrain()
{
    if (m_cbDelayBuffer > 0)
    {
        m_bDraining = TRUE;
        m_cbTailSamples = m_cbDelayBuffer;
    }
    return S_OK;
}


//-------------------------------------------------------------------
// Name: ProcessAudio
// Processes a block of audio data.
//
// pbDest: Destination buffer.
// pbInputData: Buffer that contains the input data.
// dwQuanta: Number of audio samples to process.
//
// Note: pbDest can equal pbInputData, because this MFT supports 
// in-place processing.
//-------------------------------------------------------------------

HRESULT CDelayMFT::ProcessAudio(BYTE *pbDest, const BYTE *pbInputData, DWORD dwQuanta)
{
    assert(m_pbDelayBuffer);
    assert(m_pAttributes);

    int   nWet = 0;  // Wet portion of wet/dry mix
    DWORD sample = 0, channel = 0, cChannels = 0;
    
    cChannels = NumChannels();

    // Get the wet/dry mix.
    nWet = (int)MFGetAttributeUINT32(m_pAttributes, MF_AUDIODELAY_WET_DRY_MIX, DEFAULT_WET_DRY_MIX);
    // Clip the value to [0...100]
    nWet = min(nWet, 100);

    if (Is8Bit())
    {
        for (sample = 0; sample < dwQuanta; ++sample)
        {
            for (channel = 0; channel < cChannels; ++channel)
            {
                // 8-bit sound is 0..255 with 128 == silence
                
                // Get the input sample and normalize to -128 .. 127
                int i = pbInputData[sample * cChannels + channel] - 128;
                
                // Get the delay sample and normalize to -128 .. 127
                int delay = m_pbDelayPtr[0] - 128;
                
                m_pbDelayPtr[0] = static_cast<BYTE>(i + 128);
                IncrementDelayPtr(sizeof(unsigned char));
                
                i = (i * (100 - nWet)) / 100 + (delay * nWet) / 100;
                
                // Truncate
                if (i > 127)
                {
                    i = 127;
                }
                else if (i < -128)
                {
                    i = -128;
                }
                
                pbDest[sample * cChannels + channel] = (unsigned char)(i+128);
                
            }
        }
    }
    else  // 16-bit
    {
        for (sample = 0; sample < dwQuanta; ++sample)
        {
            for (channel = 0; channel < cChannels; ++channel)
            {
                int i = ((short*)pbInputData)[sample * cChannels + channel];
                
                int delay = ((short*)m_pbDelayPtr)[0];

                ((short*)m_pbDelayPtr)[0] = static_cast<short>(i);
                IncrementDelayPtr(sizeof(short));
                
                i = (i * (100 - nWet)) / 100 + (delay * nWet) / 100;
                
                // Truncate
                if (i > 32767)
                {
                    i = 32767;
                }
                else if (i < -32768)
                {
                    i = -32768;
                }
                
                ((short*)pbDest)[sample * cChannels + channel] = (short)i;
                
            }
        }
    }
    return S_OK;
}


//-------------------------------------------------------------------
// Name: FillBufferWithSilence
// Fill a buffer with silence. 
//-------------------------------------------------------------------
void CDelayMFT::FillBufferWithSilence(BYTE *pBuffer, DWORD cb)
{
    if (pBuffer)
    {
        BYTE fill = 0;

        // The definition of 'silence' depends on the audio format.
        if (Is8Bit())
        {
            fill = 0x80;
        }
        else
        {
            fill = 0;
        }
        FillMemory(pBuffer, cb, fill);
    }
}




//-------------------------------------------------------------------
// Name: ValidatePCMAudioType
// Validate a PCM audio media type.
//-------------------------------------------------------------------

HRESULT ValidatePCMAudioType(IMFMediaType *pmt)
{
    HRESULT hr = S_OK;
    GUID majorType = GUID_NULL;
    GUID subtype = GUID_NULL;

    UINT32 nChannels = 0;
    UINT32 nSamplesPerSec = 0;
    UINT32 nAvgBytesPerSec = 0;
    UINT32 nBlockAlign = 0;
    UINT32 wBitsPerSample = 0;

    // Get attributes from the media type.
    // Each of these attributes is required for uncompressed PCM
    // audio, so fail if any are not present.
    CHECK_HR(hr = pmt->GetGUID(MF_MT_MAJOR_TYPE, &majorType));
    CHECK_HR(hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype));
    CHECK_HR(hr = pmt->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &nChannels));
    CHECK_HR(hr = pmt->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &nSamplesPerSec));
    CHECK_HR(hr = pmt->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &nAvgBytesPerSec));
    CHECK_HR(hr = pmt->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &nBlockAlign));
    CHECK_HR(hr = pmt->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &wBitsPerSample));

    // Validate the values. 

    if (nChannels != 1 && nChannels != 2)
    {
        CHECK_HR(hr = MF_E_INVALIDMEDIATYPE);
    }

    if (wBitsPerSample != 8 && wBitsPerSample != 16)
    {
        CHECK_HR(hr = MF_E_INVALIDMEDIATYPE);
    }

    // Make sure block alignment was calculated correctly.
    if (nBlockAlign != nChannels * (wBitsPerSample / 8))
    {   
        CHECK_HR(hr = MF_E_INVALIDMEDIATYPE);
    }

    // Check possible overflow...
    if (nSamplesPerSec  > (DWORD)(MAXDWORD / nBlockAlign))        // Is (nSamplesPerSec * nBlockAlign > MAXDWORD) ?
    {
        CHECK_HR(hr = MF_E_INVALIDMEDIATYPE);
    }

    // Make sure average bytes per second was calculated correctly.
    if (nAvgBytesPerSec != nSamplesPerSec * nBlockAlign)
    {
        CHECK_HR(hr = MF_E_INVALIDMEDIATYPE);
    }

done:
    return hr;
}
