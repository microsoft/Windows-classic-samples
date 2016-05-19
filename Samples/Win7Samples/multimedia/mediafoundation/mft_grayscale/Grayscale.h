//////////////////////////////////////////////////////////////////////////
//
// Grayscale.h: Defines the transform class.
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

// Function pointer for the function that transforms the image.
typedef void (*IMAGE_TRANSFORM_FN)(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    );

// CGrayscale class:
// Implements a grayscale video effect.

class CGrayscale : BaseObject, public IMFTransform
{
public:
    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID iid, void **ppSource);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFTransform
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


private:

    // Constructor is private. Client should use static CreateInstance method.
    CGrayscale(HRESULT &hr);

    // Destructor is private. The object deletes itself when the reference count is zero.
    ~CGrayscale();


    // HasPendingOutput: Returns TRUE if the MFT is holding an input sample.
    BOOL HasPendingOutput() const { return m_pSample != NULL; }

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


    HRESULT OnGetPartialType(DWORD dwTypeIndex, IMFMediaType **ppmt);
    HRESULT OnCheckInputType(IMFMediaType *pmt);
    HRESULT OnCheckOutputType(IMFMediaType *pmt);
    HRESULT OnCheckMediaType(IMFMediaType *pmt);
    HRESULT OnSetInputType(IMFMediaType *pmt);
    HRESULT OnSetOutputType(IMFMediaType *pmt);
    HRESULT OnProcessOutput(IMFMediaBuffer *pIn, IMFMediaBuffer *pOut);
    HRESULT OnFlush();

    HRESULT UpdateFormatInfo();

    long                        m_nRefCount;                // reference count
    CritSec                     m_critSec;

    IMFSample                   *m_pSample;                 // Input sample.
    IMFMediaType                *m_pInputType;              // Input media type.
    IMFMediaType                *m_pOutputType;             // Output media type.

    // Fomat information
    FOURCC                      m_videoFOURCC;
    UINT32                      m_imageWidthInPixels;
    UINT32                      m_imageHeightInPixels;
    DWORD                       m_cbImageSize;              // Image size, in bytes.

    // Image transform function. (Changes based on the media type.)
    IMAGE_TRANSFORM_FN          m_pTransformFn;

};