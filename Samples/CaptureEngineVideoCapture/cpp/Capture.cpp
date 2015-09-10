// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "Capture.h"
#include "resource.h"

IMFDXGIDeviceManager* g_pDXGIMan = NULL;
ID3D11Device*         g_pDX11Device = NULL;
UINT                  g_ResetToken = 0;

STDMETHODIMP CaptureManager::CaptureEngineCB::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CaptureEngineCB, IMFCaptureEngineOnEventCallback),
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}      

STDMETHODIMP_(ULONG) CaptureManager::CaptureEngineCB::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CaptureManager::CaptureEngineCB::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

// Callback method to receive events from the capture engine.
STDMETHODIMP CaptureManager::CaptureEngineCB::OnEvent( _In_ IMFMediaEvent* pEvent)
{
    // Post a message to the application window, so the event is handled 
    // on the application's main thread. 

    if (m_fSleeping && m_pManager != NULL)
    {
        // We're about to fall asleep, that means we've just asked the CE to stop the preview
        // and record.  We need to handle it here since our message pump may be gone.
        GUID    guidType;
        HRESULT hrStatus;
        HRESULT hr = pEvent->GetStatus(&hrStatus);
        if (FAILED(hr))
        {
            hrStatus = hr;
        }

        hr = pEvent->GetExtendedType(&guidType);
        if (SUCCEEDED(hr))
        {
            if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STOPPED)
            {
                m_pManager->OnPreviewStopped(hrStatus);
                SetEvent(m_pManager->m_hEvent);
            }
            else if (guidType == MF_CAPTURE_ENGINE_RECORD_STOPPED)
            {
                m_pManager->OnRecordStopped(hrStatus);
                SetEvent(m_pManager->m_hEvent);
            }
            else
            {
                // This is an event we don't know about, we don't really care and there's
                // no clean way to report the error so just set the event and fall through.
                SetEvent(m_pManager->m_hEvent);
            }
        }

        return S_OK;
    }
    else
    {
        pEvent->AddRef();  // The application will release the pointer when it handles the message.
        PostMessage(m_hwnd, WM_APP_CAPTURE_EVENT, (WPARAM)pEvent, 0L);
    }

    return S_OK;
}

HRESULT CreateDX11Device(_Out_ ID3D11Device** ppDevice, _Out_ ID3D11DeviceContext** ppDeviceContext, _Out_ D3D_FEATURE_LEVEL* pFeatureLevel )
{
    HRESULT hr = S_OK;
    static const D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,  
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    
    hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
        levels,
        ARRAYSIZE(levels),
        D3D11_SDK_VERSION,
        ppDevice,
        pFeatureLevel,
        ppDeviceContext
        );
    
    if(SUCCEEDED(hr))
    {
        ID3D10Multithread* pMultithread;
        hr =  ((*ppDevice)->QueryInterface(IID_PPV_ARGS(&pMultithread)));

        if(SUCCEEDED(hr))
        {
            pMultithread->SetMultithreadProtected(TRUE);
        }

        SafeRelease(&pMultithread);
        
    }

    return hr;
}

HRESULT CreateD3DManager()
{
    HRESULT hr = S_OK;
    D3D_FEATURE_LEVEL FeatureLevel;
    ID3D11DeviceContext* pDX11DeviceContext;
    
    hr = CreateDX11Device(&g_pDX11Device, &pDX11DeviceContext, &FeatureLevel);

    if(SUCCEEDED(hr))
    {
        hr = MFCreateDXGIDeviceManager(&g_ResetToken, &g_pDXGIMan);
    }

    if(SUCCEEDED(hr))
    {
        hr = g_pDXGIMan->ResetDevice(g_pDX11Device, g_ResetToken);
    }
    
    SafeRelease(&pDX11DeviceContext);
    
    return hr;
}

HRESULT
CaptureManager::InitializeCaptureManager(HWND hwndPreview, IUnknown* pUnk)
{
    HRESULT                         hr = S_OK;
    IMFAttributes*                  pAttributes = NULL;
    IMFCaptureEngineClassFactory*   pFactory = NULL;

    DestroyCaptureEngine();

    m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == m_hEvent)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    m_pCallback = new (std::nothrow) CaptureEngineCB(m_hwndEvent);
    if (m_pCallback == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    m_pCallback->m_pManager = this;
    m_hwndPreview = hwndPreview;

    //Create a D3D Manager
    hr = CreateD3DManager();
    if (FAILED(hr))
    {
        goto Exit;
    }
    hr = MFCreateAttributes(&pAttributes, 1); 
    if (FAILED(hr))
    {
        goto Exit;
    }
    hr = pAttributes->SetUnknown(MF_CAPTURE_ENGINE_D3D_MANAGER, g_pDXGIMan);
    if (FAILED(hr))
    {
        goto Exit;
    }

    // Create the factory object for the capture engine.
    hr = CoCreateInstance(CLSID_MFCaptureEngineClassFactory, NULL, 
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
    if (FAILED(hr))
    {
        goto Exit;
    }

    // Create and initialize the capture engine.
    hr = pFactory->CreateInstance(CLSID_MFCaptureEngine, IID_PPV_ARGS(&m_pEngine));
    if (FAILED(hr))
    {
        goto Exit;
    }
    hr = m_pEngine->Initialize(m_pCallback, pAttributes, NULL, pUnk);
    if (FAILED(hr))
    {
        goto Exit;
    }

Exit:
    if (NULL != pAttributes)
    {
        pAttributes->Release();
        pAttributes = NULL;
    }
    if (NULL != pFactory)
    {
        pFactory->Release();
        pFactory = NULL;
    }
    return hr;
}

// Handle an event from the capture engine. 
// NOTE: This method is called from the application's UI thread. 
HRESULT CaptureManager::OnCaptureEvent(WPARAM wParam, LPARAM lParam)
{
    GUID guidType;
    HRESULT hrStatus;

    IMFMediaEvent *pEvent = reinterpret_cast<IMFMediaEvent*>(wParam);

    HRESULT hr = pEvent->GetStatus(&hrStatus);
    if (FAILED(hr))
    {
        hrStatus = hr;
    }

    hr = pEvent->GetExtendedType(&guidType);
    if (SUCCEEDED(hr))
    {

#ifdef _DEBUG
        LPOLESTR str;
        if (SUCCEEDED(StringFromCLSID(guidType, &str)))
        {
            DBGMSG((L"MF_CAPTURE_ENGINE_EVENT: %s (hr = 0x%X)\n", str, hrStatus));
            CoTaskMemFree(str);
        }
#endif

        if (guidType == MF_CAPTURE_ENGINE_INITIALIZED)
        {
            OnCaptureEngineInitialized(hrStatus);
            SetErrorID(hrStatus, IDS_ERR_INITIALIZE);
        }
        else if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STARTED)
        {
            OnPreviewStarted(hrStatus);
            SetErrorID(hrStatus, IDS_ERR_PREVIEW);
        }
        else if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STOPPED)
        {
            OnPreviewStopped(hrStatus);
            SetErrorID(hrStatus, IDS_ERR_PREVIEW);
        }
        else if (guidType == MF_CAPTURE_ENGINE_RECORD_STARTED)
        {
            OnRecordStarted(hrStatus);
            SetErrorID(hrStatus, IDS_ERR_RECORD);
        }
        else if (guidType == MF_CAPTURE_ENGINE_RECORD_STOPPED)
        {
            OnRecordStopped(hrStatus);
            SetErrorID(hrStatus, IDS_ERR_RECORD);
        }
        else if (guidType == MF_CAPTURE_ENGINE_PHOTO_TAKEN)
        {
            m_bPhotoPending = false;
            SetErrorID(hrStatus, IDS_ERR_PHOTO);
        }
        else if (guidType == MF_CAPTURE_ENGINE_ERROR)
        {
            DestroyCaptureEngine();
            SetErrorID(hrStatus, IDS_ERR_CAPTURE);
        }
        else if (FAILED(hrStatus))
        {
            SetErrorID(hrStatus, IDS_ERR_CAPTURE);
        }
    }

    pEvent->Release();
    SetEvent(m_hEvent);
    return hrStatus;
}


void CaptureManager::OnCaptureEngineInitialized(HRESULT& hrStatus)
{
    if (hrStatus == MF_E_NO_CAPTURE_DEVICES_AVAILABLE)
    {
        hrStatus = S_OK;  // No capture device. Not an application error.
    }
}

void CaptureManager::OnPreviewStarted(HRESULT& hrStatus)
{
    m_bPreviewing = SUCCEEDED(hrStatus);
}

void CaptureManager::OnPreviewStopped(HRESULT& hrStatus)
{
    m_bPreviewing = false;
}

void CaptureManager::OnRecordStarted(HRESULT& hrStatus)
{
    m_bRecording = SUCCEEDED(hrStatus);
}

void CaptureManager::OnRecordStopped(HRESULT& hrStatus)
{
    m_bRecording = false;
}


HRESULT CaptureManager::StartPreview()
{
    if (m_pEngine == NULL)
    {
        return MF_E_NOT_INITIALIZED;
    }

    if (m_bPreviewing == true)
    {
        return S_OK;
    }

    IMFCaptureSink *pSink = NULL;
    IMFMediaType *pMediaType = NULL;
    IMFMediaType *pMediaType2 = NULL;
    IMFCaptureSource *pSource = NULL;

    HRESULT hr = S_OK;
    
    // Get a pointer to the preview sink.
    if (m_pPreview == NULL)
    {
        hr = m_pEngine->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_PREVIEW, &pSink);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = pSink->QueryInterface(IID_PPV_ARGS(&m_pPreview));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = m_pPreview->SetRenderHandle(m_hwndPreview);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = m_pEngine->GetSource(&pSource);
        if (FAILED(hr))
        {
            goto done;
        }

        // Configure the video format for the preview sink.
        hr = pSource->GetCurrentDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_PREVIEW , &pMediaType);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = CloneVideoMediaType(pMediaType, MFVideoFormat_RGB32, &pMediaType2);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = pMediaType2->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
        if (FAILED(hr))
        {
            goto done;
        }

        // Connect the video stream to the preview sink.
        DWORD dwSinkStreamIndex;
        hr = m_pPreview->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_PREVIEW,  pMediaType2, NULL, &dwSinkStreamIndex);        
        if (FAILED(hr))
        {
            goto done;
        }
    }


    hr = m_pEngine->StartPreview();
    if (!m_fPowerRequestSet && m_hpwrRequest != INVALID_HANDLE_VALUE)
    {
        // NOTE:  By calling this, on SOC systems (AOAC enabled), we're asking the system to not go
        // into sleep/connected standby while we're streaming.  However, since we don't want to block
        // the device from ever entering connected standby/sleep, we're going to latch ourselves to
        // the monitor on/off notification (RegisterPowerSettingNotification(GUID_MONITOR_POWER_ON)).
        // On SOC systems, this notification will fire when the user decides to put the device in
        // connected standby mode--we can trap this, turn off our media streams and clear this
        // power set request to allow the device to go into the lower power state.
        m_fPowerRequestSet = (TRUE == PowerSetRequest(m_hpwrRequest, PowerRequestExecutionRequired));
    }
done:
    SafeRelease(&pSink);
    SafeRelease(&pMediaType);
    SafeRelease(&pMediaType2);
    SafeRelease(&pSource);

    return hr;
}

HRESULT CaptureManager::StopPreview()
{
    HRESULT hr = S_OK;

    if (m_pEngine == NULL)
    {
        return MF_E_NOT_INITIALIZED;
    }

    if (!m_bPreviewing)
    {
        return S_OK;
    }
    hr = m_pEngine->StopPreview();
    if (FAILED(hr))
    {
        goto done;
    }
    WaitForResult();

    if (m_fPowerRequestSet && m_hpwrRequest != INVALID_HANDLE_VALUE)
    {
        PowerClearRequest(m_hpwrRequest, PowerRequestExecutionRequired);
        m_fPowerRequestSet = false;
    }
done:
    return hr;
}

// Helper function to get the frame size from a video media type.
inline HRESULT GetFrameSize(IMFMediaType *pType, UINT32 *pWidth, UINT32 *pHeight)
{    return MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, pWidth, pHeight);}

// Helper function to get the frame rate from a video media type.
inline HRESULT GetFrameRate(
    IMFMediaType *pType, 
    UINT32 *pNumerator, 
    UINT32 *pDenominator
    )
{
    return MFGetAttributeRatio(
        pType, 
        MF_MT_FRAME_RATE, 
        pNumerator, 
        pDenominator
        );
}


HRESULT GetEncodingBitrate(IMFMediaType *pMediaType, UINT32 *uiEncodingBitrate)
{
    UINT32 uiWidth;
    UINT32 uiHeight;
    float uiBitrate;
    UINT32 uiFrameRateNum;
    UINT32 uiFrameRateDenom;

    HRESULT hr = GetFrameSize(pMediaType, &uiWidth, &uiHeight);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetFrameRate(pMediaType, &uiFrameRateNum, &uiFrameRateDenom);
    if (FAILED(hr))
    {
        goto done;
    }

    uiBitrate = uiWidth / 3.0f * uiHeight * uiFrameRateNum / uiFrameRateDenom;
    
    *uiEncodingBitrate = (UINT32) uiBitrate;

done:

    return hr;
}

HRESULT ConfigureVideoEncoding(IMFCaptureSource *pSource, IMFCaptureRecordSink *pRecord, REFGUID guidEncodingType)
{
    IMFMediaType *pMediaType = NULL;
    IMFMediaType *pMediaType2 = NULL;
    GUID guidSubType = GUID_NULL;

    // Configure the video format for the recording sink.
    HRESULT hr = pSource->GetCurrentDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD , &pMediaType);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CloneVideoMediaType(pMediaType, guidEncodingType, &pMediaType2);
    if (FAILED(hr))
    {
        goto done;
    }


    hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &guidSubType);
    if(FAILED(hr))
    {
        goto done;
    }

    if(guidSubType == MFVideoFormat_H264_ES || guidSubType == MFVideoFormat_H264)
    {
        //When the webcam supports H264_ES or H264, we just bypass the stream. The output from Capture engine shall be the same as the native type supported by the webcam
        hr = pMediaType2->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    }
    else
    {    
        UINT32 uiEncodingBitrate;
        hr = GetEncodingBitrate(pMediaType2, &uiEncodingBitrate);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = pMediaType2->SetUINT32(MF_MT_AVG_BITRATE, uiEncodingBitrate);
    }

    if (FAILED(hr))
    {
        goto done;
    }

    // Connect the video stream to the recording sink.
    DWORD dwSinkStreamIndex;
    hr = pRecord->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD, pMediaType2, NULL, &dwSinkStreamIndex);

done:
    SafeRelease(&pMediaType);
    SafeRelease(&pMediaType2);
    return hr;
}

HRESULT ConfigureAudioEncoding(IMFCaptureSource *pSource, IMFCaptureRecordSink *pRecord, REFGUID guidEncodingType)
{
    IMFCollection *pAvailableTypes = NULL;
    IMFMediaType *pMediaType = NULL;
    IMFAttributes *pAttributes = NULL;

    // Configure the audio format for the recording sink.

    HRESULT hr = MFCreateAttributes(&pAttributes, 1);
    if(FAILED(hr))
    {
        goto done;
    }

    // Enumerate low latency media types
    hr = pAttributes->SetUINT32(MF_LOW_LATENCY, TRUE);
    if(FAILED(hr))
    {
        goto done;
    }


    // Get a list of encoded output formats that are supported by the encoder.
    hr = MFTranscodeGetAudioOutputAvailableTypes(guidEncodingType, MFT_ENUM_FLAG_ALL | MFT_ENUM_FLAG_SORTANDFILTER,
        pAttributes, &pAvailableTypes);
    if (FAILED(hr))
    {
        goto done;
    }

    // Pick the first format from the list.
    hr = GetCollectionObject(pAvailableTypes, 0, &pMediaType); 
    if (FAILED(hr))
    {
        goto done;
    }

    // Connect the audio stream to the recording sink.
    DWORD dwSinkStreamIndex;
    hr = pRecord->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_AUDIO, pMediaType, NULL, &dwSinkStreamIndex);
    if(hr == MF_E_INVALIDSTREAMNUMBER)
    {
        //If an audio device is not present, allow video only recording
        hr = S_OK;
    }
done:
    SafeRelease(&pAvailableTypes);
    SafeRelease(&pMediaType);
    SafeRelease(&pAttributes);
    return hr;
}

HRESULT CaptureManager::StartRecord(PCWSTR pszDestinationFile)
{
    if (m_pEngine == NULL)
    {
        return MF_E_NOT_INITIALIZED;
    }

    if (m_bRecording == true)
    {
        return MF_E_INVALIDREQUEST;
    }

    PWSTR pszExt = PathFindExtension(pszDestinationFile);

    GUID guidVideoEncoding;
    GUID guidAudioEncoding;

    if (wcscmp(pszExt, L".mp4") == 0)
    {
        guidVideoEncoding = MFVideoFormat_H264;
        guidAudioEncoding = MFAudioFormat_AAC;
    }
    else if (wcscmp(pszExt, L".wmv") == 0)
    {
        guidVideoEncoding = MFVideoFormat_WMV3;
        guidAudioEncoding = MFAudioFormat_WMAudioV9;
    }
    else if (wcscmp(pszExt, L".wma") == 0)
    {
        guidVideoEncoding = GUID_NULL;
        guidAudioEncoding = MFAudioFormat_WMAudioV9;
    }
    else
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    IMFCaptureSink *pSink = NULL;
    IMFCaptureRecordSink *pRecord = NULL;
    IMFCaptureSource *pSource = NULL;

    HRESULT hr = m_pEngine->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_RECORD, &pSink);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pSink->QueryInterface(IID_PPV_ARGS(&pRecord));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_pEngine->GetSource(&pSource);
    if (FAILED(hr))
    {
        goto done;
    }

    // Clear any existing streams from previous recordings.
    hr = pRecord->RemoveAllStreams();
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pRecord->SetOutputFileName(pszDestinationFile);
    if (FAILED(hr))
    {
        goto done;
    }

    // Configure the video and audio streams.
    if (guidVideoEncoding != GUID_NULL)
    {
        hr = ConfigureVideoEncoding(pSource, pRecord, guidVideoEncoding);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (guidAudioEncoding != GUID_NULL)
    {
        hr = ConfigureAudioEncoding(pSource, pRecord, guidAudioEncoding);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = m_pEngine->StartRecord();
    if (FAILED(hr))
    {
        goto done;
    }

    m_bRecording = true;
    
done:
    SafeRelease(&pSink);
    SafeRelease(&pSource);
    SafeRelease(&pRecord);
    
    return hr;
}

HRESULT CaptureManager::StopRecord()
{
    HRESULT hr = S_OK;

    if (m_bRecording)
    {
        hr = m_pEngine->StopRecord(TRUE, FALSE);
        WaitForResult();
    }

    return hr;
}


HRESULT CaptureManager::TakePhoto(PCWSTR pszFileName)
{
    IMFCaptureSink *pSink = NULL;
    IMFCapturePhotoSink *pPhoto = NULL;
    IMFCaptureSource *pSource;
    IMFMediaType *pMediaType = 0;
    IMFMediaType *pMediaType2 = 0;
    bool bHasPhotoStream = true;

    // Get a pointer to the photo sink.
    HRESULT hr = m_pEngine->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_PHOTO, &pSink);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pSink->QueryInterface(IID_PPV_ARGS(&pPhoto));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_pEngine->GetSource(&pSource);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pSource->GetCurrentDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_PHOTO , &pMediaType);     
    if (FAILED(hr))
    {
        goto done;
    }

    //Configure the photo format
    hr = CreatePhotoMediaType(pMediaType, &pMediaType2);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pPhoto->RemoveAllStreams();
    if (FAILED(hr))
    {
        goto done;
    }

    DWORD dwSinkStreamIndex;
    // Try to connect the first still image stream to the photo sink
    if(bHasPhotoStream)
    {
        hr = pPhoto->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_PHOTO,  pMediaType2, NULL, &dwSinkStreamIndex);        
    }    

    if(FAILED(hr))
    {
        goto done;
    }

    hr = pPhoto->SetOutputFileName(pszFileName);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_pEngine->TakePhoto();
    if (FAILED(hr))
    {
        goto done;
    }

    m_bPhotoPending = true;

done:
    SafeRelease(&pSink);
    SafeRelease(&pPhoto);
    SafeRelease(&pSource);
    SafeRelease(&pMediaType);
    SafeRelease(&pMediaType2);
    return hr;
}




