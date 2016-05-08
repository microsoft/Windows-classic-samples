//////////////////////////////////////////////////////////////////////////
//
// AudioDelayMFT.h 
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

#pragma once

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mftransform.h>
#include <mferror.h>
#include <assert.h>

// Common sample files.
#include "common.h"
using namespace MediaFoundationSamples;

const DWORD UNITS = 10000000;            // 1 sec = 1 * UNITS
const DWORD DEFAULT_WET_DRY_MIX = 25;    // Percentage of "wet" (delay) audio in the mix.
const DWORD DEFAULT_DELAY = 1000;        // Delay in msec
const UINT32 ATTRIBUTE_COUNT = 2;        // Initial size of our attribute store. 



//-------------------------------------------------------------------
// Name: CDelayMFT
// Description: Implements the MFT.
//-------------------------------------------------------------------
class CDelayMFT : BaseObject, public IMFTransform
{
private:
    CDelayMFT();
    virtual ~CDelayMFT();

public:
    // Static function to create the object.
    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID iid, void **ppv);

    // IUnknown methods.
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFTransform methods.
    STDMETHODIMP GetStreamLimits( 
        DWORD   *pdwInputMinimum,
        DWORD   *pdwInputMaximum,
        DWORD   *pdwOutputMinimum,
        DWORD   *pdwOutputMaximum
    );
    
    STDMETHODIMP GetStreamCount(DWORD *pcInputStreams, DWORD *pcOutputStreams);

    STDMETHODIMP GetStreamIDs(
        DWORD   dwInputIDArraySize,
        DWORD   *pdwInputIDs,
        DWORD   dwOutputIDArraySize,
        DWORD   *pdwOutputIDs
    );
    
    STDMETHODIMP GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO *pStreamInfo);
    STDMETHODIMP GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO *pStreamInfo);
    STDMETHODIMP GetAttributes(IMFAttributes** ppAttributes);
    STDMETHODIMP GetInputStreamAttributes(DWORD dwInputStreamID, IMFAttributes **ppAttributes);
    STDMETHODIMP GetOutputStreamAttributes(DWORD dwOutputStreamID, IMFAttributes **ppAttributes);
    STDMETHODIMP DeleteInputStream(DWORD dwStreamID);
    STDMETHODIMP AddInputStreams(DWORD cStreams, DWORD *adwStreamIDs);

    STDMETHODIMP GetInputAvailableType(
        DWORD           dwInputStreamID,
        DWORD           dwTypeIndex, // 0-based
        IMFMediaType    **ppType
    );

    STDMETHODIMP GetOutputAvailableType(
        DWORD           dwOutputStreamID,
        DWORD           dwTypeIndex, // 0-based
        IMFMediaType    **ppType
    );

    STDMETHODIMP SetInputType(
        DWORD           dwInputStreamID,
        IMFMediaType    *pType,
        DWORD           dwFlags 
    );

    STDMETHODIMP SetOutputType(
        DWORD           dwOutputStreamID,
        IMFMediaType    *pType,
        DWORD           dwFlags 
    );

    STDMETHODIMP GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType **ppType);
    STDMETHODIMP GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType **ppType);
    STDMETHODIMP GetInputStatus(DWORD dwInputStreamID, DWORD *pdwFlags);
    STDMETHODIMP GetOutputStatus(DWORD *pdwFlags);
    STDMETHODIMP SetOutputBounds(LONGLONG hnsLowerBound, LONGLONG hnsUpperBound);
    STDMETHODIMP ProcessEvent(DWORD dwInputStreamID, IMFMediaEvent *pEvent);
    STDMETHODIMP ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR ulParam);
    STDMETHODIMP ProcessInput(
        DWORD               dwInputStreamID,
        IMFSample           *pSample, 
        DWORD               dwFlags 
    );

    STDMETHODIMP ProcessOutput(
        DWORD                   dwFlags, 
        DWORD                   cOutputBufferCount,
        MFT_OUTPUT_DATA_BUFFER  *pOutputSamples, // one per stream
        DWORD                   *pdwStatus  
    );

private:

    long                    m_nRefCount;        // reference count
    IMFMediaBuffer          *m_pBuffer;         // Current input buffer.
    IMFSample               *m_pSample;         // Current input sample.
    IMFMediaType            *m_pMediaType;      // Media type (for both input and output).
    IMFAttributes           *m_pAttributes;     // Attribute store.

    CritSec                 m_critSec;
    BYTE                    *m_pbInputData;     // Pointer to the data in the input buffer.
    DWORD                   m_cbInputLength;    // Length of the data.

    REFERENCE_TIME          m_rtTimestamp;      // Most recent timestamp
    BOOL                    m_bValidTime;       // Is timestamp valid?

    BOOL                    m_bInputTypeSet;    // Is the input type set?
    BOOL                    m_bOutputTypeSet;   // Is the output type set?

    BYTE                    *m_pbDelayBuffer;   // circular buffer for delay samples
    DWORD                   m_cbDelayBuffer;    // size of the delay buffer
    BYTE                    *m_pbDelayPtr;      // ptr to next delay sample

    BOOL                    m_bDraining;        // Is the MFT draining?
    DWORD                   m_cbTailSamples;    // How many bytes of "tail" samples left to produce.

    DWORD                   m_dwDelay;          // Delay in ms

private:

    enum StreamDirection { InputStream, OutputStream };

    // Format information. (Do not call unless the media type is set.)
    UINT32 BlockAlign() const {     assert(m_pMediaType);   return MFGetAttributeUINT32(m_pMediaType, MF_MT_AUDIO_BLOCK_ALIGNMENT, 0); }
    UINT32 AvgBytesPerSec() const { assert(m_pMediaType);   return MFGetAttributeUINT32(m_pMediaType, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 0); }
    UINT32 SamplesPerSec() const {  assert(m_pMediaType);   return MFGetAttributeUINT32(m_pMediaType, MF_MT_AUDIO_SAMPLES_PER_SECOND, 0); }
    UINT32 NumChannels() const {    assert(m_pMediaType);   return MFGetAttributeUINT32(m_pMediaType, MF_MT_AUDIO_NUM_CHANNELS, 0); }
    UINT32 BitsPerSample() const {  assert(m_pMediaType);   return MFGetAttributeUINT32(m_pMediaType, MF_MT_AUDIO_BITS_PER_SAMPLE, 0); }

    BOOL Is8Bit() const { return (BitsPerSample() == 8); }

    // IncrementDelayPtr: Moves the delay pointer around the circular buffer
    void IncrementDelayPtr(size_t size) 
    {
        m_pbDelayPtr += size;
        if (m_pbDelayPtr + size > m_pbDelayBuffer + m_cbDelayBuffer)
        {
            m_pbDelayPtr = m_pbDelayBuffer;
        }
    }   

    // IsValidInputStream: Returns TRUE if dwInputStreamID is a valid input stream identifier.
    BOOL IsValidInputStream(DWORD dwInputStreamID) const 
    {
        return dwInputStreamID == 0;
    }

    // IsValidOutputStream: Returns TRUE if dwOutputStreamID is a valid output stream identifier.
    BOOL IsValidOutputStream(DWORD dwOutputStreamID) const
    {
        return dwOutputStreamID == 0;
    }

    // HasPendingOutput: Returns TRUE if the MFT can produce output.
    BOOL HasPendingOutput() const 
    { 
        // Two cases: The MFT has an input buffer, OR the MFT is producing an effect tail.
        return ((m_pBuffer != NULL) || (m_bDraining)); 
    }

    BOOL IsOutputTypeSet() const
    {
        // If m_bOutputTypeSet it TRUE, then m_pMediaType must be non-NULL.
        assert(!(m_bOutputTypeSet && (m_pMediaType == NULL)));
        return (m_bOutputTypeSet && m_pMediaType);
    }

    BOOL IsInputTypeSet() const
    {
        // If m_bInputTypeSet it TRUE, then m_pMediaType must be non-NULL.
        assert(!(m_bInputTypeSet && (m_pMediaType == NULL)));
        return (m_bInputTypeSet && (m_pMediaType != NULL));
    }

    HRESULT AllocateStreamingResources();
    void    FreeStreamingResources(BOOL bFlush);  
    HRESULT CreateAttributeStore();

    HRESULT InternalProcessOutput(MFT_OUTPUT_DATA_BUFFER& OutputSample, DWORD *pdwStatus);
    
    HRESULT ProcessAudio(
                BYTE *pbData,            // Pointer to the output buffer
                const BYTE *pbInputData, // Pointer to the input buffer
                DWORD dwQuantaToProcess  // Number of quanta to process
                );

    HRESULT ProcessEffectTail(MFT_OUTPUT_DATA_BUFFER& OutputSample, DWORD *pdwStatus);
    void    FillBufferWithSilence(BYTE *pBuffer, DWORD cb);

    HRESULT GetProposedType(DWORD dwTypeIndex, IMFMediaType **ppmt);
    HRESULT OnCheckInputType(IMFMediaType *pmt);
    HRESULT OnCheckOutputType(IMFMediaType *pmt);
    HRESULT OnCheckMediaType(IMFMediaType *pmt);
    HRESULT OnSetMediaType(IMFMediaType *pmt, StreamDirection dir);
    HRESULT OnDrain();
};