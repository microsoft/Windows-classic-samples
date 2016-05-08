//////////////////////////////////////////////////////////////////////////
//
// decoder.h
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

#pragma once

#include <mftransform.h>
#include <mfapi.h>
#include <mferror.h>
#include <shlwapi.h>
#include <uuids.h>      // MPEG-1 subtypes
#include <assert.h>

void DllAddRef();
void DllRelease();

const DWORD MPEG1_VIDEO_SEQ_HEADER_MIN_SIZE = 12;       // Minimum length of the video sequence header.
static const REFERENCE_TIME INVALID_TIME = _I64_MAX;    //  Not really invalid but unlikely enough for sample code.

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

//-------------------------------------------------------------------
// CStreamState class.
// Provides state machine for picture start codes and timestamps.
//
//  We need to know:
//
//    - The time stamp to associate with a picture start code
//    - The next time stamp
//
//-------------------------------------------------------------------

class CStreamState
{
private:

    DWORD m_cbBytes;    // Number of valid bytes of start code so far

    struct 
    {
        bool           bValid;
        REFERENCE_TIME rt;
    } m_arTS[4];        // Array of all the timestamps.
                        // m_cbBytes+1 entries are valid.

    REFERENCE_TIME m_rt;

    BYTE m_bGOPData[4];

    DWORD m_dwTimeCode;
    DWORD m_dwNextTimeCode;

public:
    CStreamState()
    {
        Reset();
    }

    //  Returns true if a start code was identifed
    bool NextByte(BYTE bData);
    void TimeStamp(REFERENCE_TIME rt);
    void Reset();

    REFERENCE_TIME PictureTime(DWORD *pdwTimeCode) const
    {
        *pdwTimeCode = m_dwTimeCode;
        return m_rt;
    }
};



//-------------------------------------------------------------------
// CDecoder
//
// Implements the MPEG-1 "decoder" MFT.
// 
// The decoder outputs RGB-32 only.
//
// Note: This MFT is derived from a sample that used to ship in the
// DirectX SDK. 
//-------------------------------------------------------------------

class CDecoder : public IMFTransform
{
public:
    // Static function to create the object.
    static HRESULT CreateInstance(REFIID iid, void **ppv);

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFTransform methods
    STDMETHODIMP GetStreamLimits( 
        DWORD   *pdwInputMinimum,
        DWORD   *pdwInputMaximum,
        DWORD   *pdwOutputMinimum,
        DWORD   *pdwOutputMaximum
    );
    
    STDMETHODIMP GetStreamCount(
        DWORD   *pcInputStreams,
        DWORD   *pcOutputStreams
    );

    STDMETHODIMP GetStreamIDs(
        DWORD   dwInputIDArraySize,
        DWORD   *pdwInputIDs,
        DWORD   dwOutputIDArraySize,
        DWORD   *pdwOutputIDs
    );
    
    STDMETHODIMP GetInputStreamInfo(
        DWORD                     dwInputStreamID,
        MFT_INPUT_STREAM_INFO *   pStreamInfo
    );
    
    STDMETHODIMP GetOutputStreamInfo(
        DWORD                     dwOutputStreamID, 
        MFT_OUTPUT_STREAM_INFO *  pStreamInfo      
    );

    STDMETHODIMP GetAttributes(IMFAttributes** pAttributes);

    STDMETHODIMP GetInputStreamAttributes(
        DWORD           dwInputStreamID,
        IMFAttributes   **ppAttributes
    );

    STDMETHODIMP GetOutputStreamAttributes(
        DWORD           dwOutputStreamID,
        IMFAttributes   **ppAttributes
    );

    STDMETHODIMP DeleteInputStream(DWORD dwStreamID);

    STDMETHODIMP AddInputStreams( 
        DWORD   cStreams,
        DWORD   *adwStreamIDs
    );

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

    STDMETHODIMP GetInputCurrentType(
        DWORD           dwInputStreamID,
        IMFMediaType    **ppType
    );

    STDMETHODIMP GetOutputCurrentType(
        DWORD           dwOutputStreamID,
        IMFMediaType    **ppType
    );

    STDMETHODIMP GetInputStatus(
        DWORD           dwInputStreamID,
        DWORD           *pdwFlags 
    );

    STDMETHODIMP GetOutputStatus(DWORD *pdwFlags);

    STDMETHODIMP SetOutputBounds(
        LONGLONG        hnsLowerBound,
        LONGLONG        hnsUpperBound
    );

    STDMETHODIMP ProcessEvent(
        DWORD              dwInputStreamID,
        IMFMediaEvent      *pEvent 
    );

    STDMETHODIMP ProcessMessage(
        MFT_MESSAGE_TYPE    eMessage,
        ULONG_PTR           ulParam
    );
        
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

protected:

    CDecoder();
    ~CDecoder();

    // HasPendingOutput: Returns TRUE if the MFT is holding an input sample.
    BOOL HasPendingOutput() const { return m_pBuffer != NULL; }

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

    //  Internal processing routine
    HRESULT InternalProcessOutput(IMFSample *pSample, IMFMediaBuffer *pOutputBuffer);
    HRESULT Process();
    HRESULT OnCheckInputType(IMFMediaType *pmt);
    HRESULT OnCheckOutputType(IMFMediaType *pmt);
    HRESULT OnSetInputType(IMFMediaType *pmt);
    HRESULT OnSetOutputType(IMFMediaType *pmt);
    HRESULT OnDiscontinuity();
    HRESULT AllocateStreamingResources();
    HRESULT FreeStreamingResources();
    HRESULT OnFlush();


protected:

    long                    m_nRefCount;        // reference count

    CRITICAL_SECTION        m_critSec;

    //  Streaming locals
    IMFMediaType            *m_pInputType;              // Input media type.
    IMFMediaType            *m_pOutputType;             // Output media type.

    IMFMediaBuffer          *m_pBuffer;
    BYTE *                  m_pbData;
    DWORD                   m_cbData;

    // Fomat information
    UINT32                      m_imageWidthInPixels;
    UINT32                      m_imageHeightInPixels;
    MFRatio                     m_frameRate;
    DWORD                       m_cbImageSize;              // Image size, in bytes.

    //  Fabricate timestamps based on the average time per from if there isn't one in the stream
    REFERENCE_TIME          m_rtFrame;
    UINT64                  m_rtLength;     // bugbug rename

    //  Current state info
    CStreamState            m_StreamState;
    bool                    m_bPicture;
};

