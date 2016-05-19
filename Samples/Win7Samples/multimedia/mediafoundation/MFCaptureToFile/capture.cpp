//////////////////////////////////////////////////////////////////////////
//
// capture.cpp: Manages video capture.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include <new>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Wmcodecdsp.h>
#include <assert.h>
#include <Dbt.h>
#include <shlwapi.h>

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

#include "capture.h"

HRESULT CopyAttribute(IMFAttributes *pSrc, IMFAttributes *pDest, const GUID& key);


void DeviceList::Clear()
{
    for (UINT32 i = 0; i < m_cDevices; i++)
    {
        SafeRelease(&m_ppDevices[i]);
    }
    CoTaskMemFree(m_ppDevices);
    m_ppDevices = NULL;

    m_cDevices = 0;
}

HRESULT DeviceList::EnumerateDevices()
{
    HRESULT hr = S_OK;
    IMFAttributes *pAttributes = NULL;

    Clear();

    // Initialize an attribute store. We will use this to 
    // specify the enumeration parameters.

    hr = MFCreateAttributes(&pAttributes, 1);

    // Ask for source type = video capture devices
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, 
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
            );
    }

    // Enumerate devices.
    if (SUCCEEDED(hr))
    {
        hr = MFEnumDeviceSources(pAttributes, &m_ppDevices, &m_cDevices);
    }

    SafeRelease(&pAttributes);

    return hr;
}


HRESULT DeviceList::GetDevice(UINT32 index, IMFActivate **ppActivate)
{
    if (index >= Count())
    {
        return E_INVALIDARG;
    }

    *ppActivate = m_ppDevices[index];
    (*ppActivate)->AddRef();

    return S_OK;
}

HRESULT DeviceList::GetDeviceName(UINT32 index, WCHAR **ppszName)
{
    if (index >= Count())
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    hr = m_ppDevices[index]->GetAllocatedString(
        MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, 
        ppszName, 
        NULL
        );

    return hr;
}



//-------------------------------------------------------------------
//  CreateInstance
//
//  Static class method to create the CCapture object.
//-------------------------------------------------------------------

HRESULT CCapture::CreateInstance(
    HWND     hwnd,       // Handle to the window to receive events
    CCapture **ppCapture // Receives a pointer to the CCapture object.
    )
{
    if (ppCapture == NULL)
    {
        return E_POINTER;
    }

    CCapture *pCapture = new (std::nothrow) CCapture(hwnd);

    if (pCapture == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // The CCapture constructor sets the ref count to 1.
    *ppCapture = pCapture;

    return S_OK;
}


//-------------------------------------------------------------------
//  constructor
//-------------------------------------------------------------------

CCapture::CCapture(HWND hwnd) : 
    m_pReader(NULL),
    m_pWriter(NULL),
    m_hwndEvent(hwnd),
    m_nRefCount(1),
    m_bFirstSample(FALSE),
    m_llBaseTime(0),
    m_pwszSymbolicLink(NULL)
{
    InitializeCriticalSection(&m_critsec);
}

//-------------------------------------------------------------------
//  destructor
//-------------------------------------------------------------------

CCapture::~CCapture()
{
    assert(m_pReader == NULL);
    assert(m_pWriter == NULL);
    DeleteCriticalSection(&m_critsec);
}



/////////////// IUnknown methods ///////////////

//-------------------------------------------------------------------
//  AddRef
//-------------------------------------------------------------------

ULONG CCapture::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}


//-------------------------------------------------------------------
//  Release
//-------------------------------------------------------------------

ULONG CCapture::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    return uCount;
}



//-------------------------------------------------------------------
//  QueryInterface
//-------------------------------------------------------------------

HRESULT CCapture::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CCapture, IMFSourceReaderCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


/////////////// IMFSourceReaderCallback methods ///////////////

//-------------------------------------------------------------------
// OnReadSample
//
// Called when the IMFMediaSource::ReadSample method completes.
//-------------------------------------------------------------------

HRESULT CCapture::OnReadSample(
    HRESULT hrStatus,
    DWORD /*dwStreamIndex*/,
    DWORD /*dwStreamFlags*/,
    LONGLONG llTimeStamp,
    IMFSample *pSample      // Can be NULL
    )
{
    EnterCriticalSection(&m_critsec);

    if (!IsCapturing())
    {
        LeaveCriticalSection(&m_critsec);
        return S_OK;
    }

    HRESULT hr = S_OK;

    if (FAILED(hrStatus))
    {
        hr = hrStatus;
        goto done;
    }

    if (pSample)
    {
        if (m_bFirstSample)
        {
            m_llBaseTime = llTimeStamp;
            m_bFirstSample = FALSE;
        }

        // rebase the time stamp
        llTimeStamp -= m_llBaseTime;

        hr = pSample->SetSampleTime(llTimeStamp);

        if (FAILED(hr)) { goto done; }

        hr = m_pWriter->WriteSample(0, pSample);

        if (FAILED(hr)) { goto done; }
    }

    // Read another sample.
    hr = m_pReader->ReadSample(
        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        0,
        NULL,   // actual
        NULL,   // flags
        NULL,   // timestamp
        NULL    // sample
        );

done:
    if (FAILED(hr))
    {
        NotifyError(hr);
    }

    LeaveCriticalSection(&m_critsec);
    return hr;
}


//-------------------------------------------------------------------
// OpenMediaSource
//
// Set up preview for a specified media source. 
//-------------------------------------------------------------------

HRESULT CCapture::OpenMediaSource(IMFMediaSource *pSource)
{
    HRESULT hr = S_OK;

    IMFAttributes *pAttributes = NULL;

    hr = MFCreateAttributes(&pAttributes, 2);

    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
    }

    if (SUCCEEDED(hr))
    {
        hr = MFCreateSourceReaderFromMediaSource(
            pSource,
            pAttributes,
            &m_pReader
            );
    }

    SafeRelease(&pAttributes);
    return hr;
}


//-------------------------------------------------------------------
// StartCapture
//
// Start capturing.
//-------------------------------------------------------------------

HRESULT CCapture::StartCapture(
    IMFActivate *pActivate, 
    const WCHAR *pwszFileName, 
    const EncodingParameters& param
    )
{
    HRESULT hr = S_OK;

    IMFMediaSource *pSource = NULL;

    EnterCriticalSection(&m_critsec);

    // Create the media source for the device.
    hr = pActivate->ActivateObject(
        __uuidof(IMFMediaSource), 
        (void**)&pSource
        );

    // Get the symbolic link. This is needed to handle device-
    // loss notifications. (See CheckDeviceLost.)

    if (SUCCEEDED(hr))
    {
        hr = pActivate->GetAllocatedString(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
            &m_pwszSymbolicLink,
            NULL
            );
    }

    if (SUCCEEDED(hr))
    {
        hr = OpenMediaSource(pSource);
    }

    // Create the sink writer 
    if (SUCCEEDED(hr))
    {
        hr = MFCreateSinkWriterFromURL(
            pwszFileName,
            NULL,
            NULL,
            &m_pWriter
            );
    }    

    // Set up the encoding parameters.
    if (SUCCEEDED(hr))
    {
        hr = ConfigureCapture(param);
    }

    if (SUCCEEDED(hr))
    {
        m_bFirstSample = TRUE;
        m_llBaseTime = 0;

        // Request the first video frame.

        hr = m_pReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0,
            NULL,
            NULL,
            NULL,
            NULL
            );
    }

    SafeRelease(&pSource);
    LeaveCriticalSection(&m_critsec);
    return hr;
}


//-------------------------------------------------------------------
// EndCaptureSession
//
// Stop the capture session. 
//
// NOTE: This method resets the object's state to State_NotReady.
// To start another capture session, call SetCaptureFile.
//-------------------------------------------------------------------

HRESULT CCapture::EndCaptureSession()
{
    EnterCriticalSection(&m_critsec);

    HRESULT hr = S_OK;

    if (m_pWriter)
    {
        hr = m_pWriter->Finalize();
    }

    SafeRelease(&m_pWriter);
    SafeRelease(&m_pReader);

    LeaveCriticalSection(&m_critsec);

    return hr;
}


BOOL CCapture::IsCapturing() 
{ 
    EnterCriticalSection(&m_critsec);
    
    BOOL bIsCapturing = (m_pWriter != NULL);

    LeaveCriticalSection(&m_critsec);

    return bIsCapturing;
}



//-------------------------------------------------------------------
//  CheckDeviceLost
//  Checks whether the video capture device was removed.
//
//  The application calls this method when is receives a 
//  WM_DEVICECHANGE message.
//-------------------------------------------------------------------

HRESULT CCapture::CheckDeviceLost(DEV_BROADCAST_HDR *pHdr, BOOL *pbDeviceLost)
{
    if (pbDeviceLost == NULL)
    {
        return E_POINTER;
    }

    EnterCriticalSection(&m_critsec);

    DEV_BROADCAST_DEVICEINTERFACE *pDi = NULL;

    *pbDeviceLost = FALSE;
    
    if (!IsCapturing())
    {
        goto done;
    }
    if (pHdr == NULL)
    {
        goto done;
    }
    if (pHdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
    {
        goto done;
    }

    // Compare the device name with the symbolic link.

    pDi = (DEV_BROADCAST_DEVICEINTERFACE*)pHdr;

    if (m_pwszSymbolicLink)
    {
        if (_wcsicmp(m_pwszSymbolicLink, pDi->dbcc_name) == 0)
        {
            *pbDeviceLost = TRUE;
        }
    }

done:
    LeaveCriticalSection(&m_critsec);
    return S_OK;
}


/////////////// Private/protected class methods ///////////////



//-------------------------------------------------------------------
//  ConfigureSourceReader
//
//  Sets the media type on the source reader.
//-------------------------------------------------------------------

HRESULT ConfigureSourceReader(IMFSourceReader *pReader)
{
    // The list of acceptable types.
    GUID subtypes[] = { 
        MFVideoFormat_NV12, MFVideoFormat_YUY2, MFVideoFormat_UYVY,
        MFVideoFormat_RGB32, MFVideoFormat_RGB24, MFVideoFormat_IYUV
    };

    HRESULT hr = S_OK;
    BOOL    bUseNativeType = FALSE;

    GUID subtype = { 0 };

    IMFMediaType *pType = NULL;

    // If the source's native format matches any of the formats in 
    // the list, prefer the native format.

    // Note: The camera might support multiple output formats, 
    // including a range of frame dimensions. The application could
    // provide a list to the user and have the user select the
    // camera's output format. That is outside the scope of this
    // sample, however.

    hr = pReader->GetNativeMediaType(
        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        0,  // Type index
        &pType
        );

    if (FAILED(hr)) { goto done; }

    hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);

    if (FAILED(hr)) { goto done; }

    for (UINT32 i = 0; i < ARRAYSIZE(subtypes); i++)
    {
        if (subtype == subtypes[i])
        {
            hr = pReader->SetCurrentMediaType(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 
                NULL, 
                pType
                );

            bUseNativeType = TRUE;
            break;
        }
    }

    if (!bUseNativeType)
    {
        // None of the native types worked. The camera might offer 
        // output a compressed type such as MJPEG or DV.

        // Try adding a decoder.

        for (UINT32 i = 0; i < ARRAYSIZE(subtypes); i++)
        {
            hr = pType->SetGUID(MF_MT_SUBTYPE, subtypes[i]);

            if (FAILED(hr)) { goto done; }

            hr = pReader->SetCurrentMediaType(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 
                NULL, 
                pType
                );

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
    }

done:
    SafeRelease(&pType);
    return hr;
}

HRESULT ConfigureEncoder(
    const EncodingParameters& params, 
    IMFMediaType *pType, 
    IMFSinkWriter *pWriter,
    DWORD *pdwStreamIndex
    )
{
    HRESULT hr = S_OK;

    IMFMediaType *pType2 = NULL;

    hr = MFCreateMediaType(&pType2);   

    if (SUCCEEDED(hr))
    {
        hr = pType2->SetGUID( MF_MT_MAJOR_TYPE, MFMediaType_Video );     
    }

    if (SUCCEEDED(hr))
    {
        hr = pType2->SetGUID(MF_MT_SUBTYPE, params.subtype);
    }

    if (SUCCEEDED(hr))
    {
        hr = pType2->SetUINT32(MF_MT_AVG_BITRATE, params.bitrate);
    }

    if (SUCCEEDED(hr))
    {
        hr = CopyAttribute(pType, pType2, MF_MT_FRAME_SIZE);
    }

    if (SUCCEEDED(hr))
    {
        hr = CopyAttribute(pType, pType2, MF_MT_FRAME_RATE);
    }

    if (SUCCEEDED(hr))
    {
        hr = CopyAttribute(pType, pType2, MF_MT_PIXEL_ASPECT_RATIO);
    }

    if (SUCCEEDED(hr))
    {
        hr = CopyAttribute(pType, pType2, MF_MT_INTERLACE_MODE);
    }

    if (SUCCEEDED(hr))
    {
        hr = pWriter->AddStream(pType2, pdwStreamIndex);
    }

    SafeRelease(&pType2);
    return hr;
}

//-------------------------------------------------------------------
// ConfigureCapture
//
// Configures the capture session.
//
//-------------------------------------------------------------------

HRESULT CCapture::ConfigureCapture(const EncodingParameters& param)
{
    HRESULT hr = S_OK;
    DWORD sink_stream = 0;

    IMFMediaType *pType = NULL;

    hr = ConfigureSourceReader(m_pReader);

    if (SUCCEEDED(hr))
    {
        hr = m_pReader->GetCurrentMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 
            &pType
            );
    }

    if (SUCCEEDED(hr))
    {
        hr = ConfigureEncoder(param, pType, m_pWriter, &sink_stream);
    }


    if (SUCCEEDED(hr))
    {
        // Register the color converter DSP for this process, in the video 
        // processor category. This will enable the sink writer to enumerate
        // the color converter when the sink writer attempts to match the
        // media types.

        hr = MFTRegisterLocalByCLSID(
            __uuidof(CColorConvertDMO),
            MFT_CATEGORY_VIDEO_PROCESSOR,
            L"",
            MFT_ENUM_FLAG_SYNCMFT,
            0,
            NULL,
            0,
            NULL
            );
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pWriter->SetInputMediaType(sink_stream, pType, NULL);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pWriter->BeginWriting();
    }

    SafeRelease(&pType);
    return hr;
}


//-------------------------------------------------------------------
// EndCaptureInternal
//
// Stops capture. 
//-------------------------------------------------------------------

HRESULT CCapture::EndCaptureInternal()
{
    HRESULT hr = S_OK;

    if (m_pWriter)
    {
        hr = m_pWriter->Finalize();
    }

    SafeRelease(&m_pWriter);
    SafeRelease(&m_pReader);

    CoTaskMemFree(m_pwszSymbolicLink);
    m_pwszSymbolicLink = NULL;

    return hr;
}




//-------------------------------------------------------------------
// CopyAttribute
//
// Copy an attribute value from one attribute store to another.
//-------------------------------------------------------------------

HRESULT CopyAttribute(IMFAttributes *pSrc, IMFAttributes *pDest, const GUID& key)
{
    PROPVARIANT var;
    PropVariantInit(&var);

    HRESULT hr = S_OK;
    
    hr = pSrc->GetItem(key, &var);
    if (SUCCEEDED(hr))
    {
        hr = pDest->SetItem(key, var);
    }
    
    PropVariantClear(&var);
    return hr;
}
