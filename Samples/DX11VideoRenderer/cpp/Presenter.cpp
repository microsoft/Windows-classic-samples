#include "Presenter.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CPresenter class. - Presents samples using DX11.
//
// Notes:
// - Most public methods calls CheckShutdown. This method fails if the presenter was shut down.
//
/////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------
// CPresenter constructor.
//-------------------------------------------------------------------

DX11VideoRenderer::CPresenter::CPresenter(void) :
    m_nRefCount(1),
    m_critSec(), // default ctor
    m_IsShutdown(FALSE),
    m_pDXGIFactory2(NULL),
    m_pD3D11Device(NULL),
    m_pD3DImmediateContext(NULL),
    m_pDXGIManager(NULL),
    m_pDXGIOutput1(NULL),
    m_pSampleAllocatorEx(NULL),
    m_pDCompDevice(NULL),
    m_pHwndTarget(NULL),
    m_pRootVisual(NULL),
    m_bSoftwareDXVADeviceInUse(FALSE),
    m_hwndVideo(NULL),
    m_pMonitors(NULL),
    m_lpCurrMon(NULL),
    m_DeviceResetToken(0),
    m_DXSWSwitch(0),
    m_useXVP(1),
    m_useDCompVisual(0),
    m_useDebugLayer(D3D11_CREATE_DEVICE_VIDEO_SUPPORT),
    m_pDX11VideoDevice(NULL),
    m_pVideoProcessorEnum(NULL),
    m_pVideoProcessor(NULL),
    m_pSwapChain1(NULL),
    m_bDeviceChanged(FALSE),
    m_bResize(TRUE),
    m_b3DVideo(FALSE),
    m_bStereoEnabled(FALSE),
    m_vp3DOutput(MFVideo3DSampleFormat_BaseView),
    m_bFullScreenState(FALSE),
    m_bCanProcessNextSample(TRUE),
    m_displayRect(), // default ctor
    m_imageWidthInPixels(0),
    m_imageHeightInPixels(0),
    m_uiRealDisplayWidth(0),
    m_uiRealDisplayHeight(0),
    m_rcSrcApp(), // default ctor
    m_rcDstApp(), // default ctor
    m_pXVP(NULL),
    m_pXVPControl(NULL)
{
    ZeroMemory(&m_rcSrcApp, sizeof(m_rcSrcApp));
    ZeroMemory(&m_rcDstApp, sizeof(m_rcDstApp));
}

//-------------------------------------------------------------------
// CPresenter destructor.
//-------------------------------------------------------------------

DX11VideoRenderer::CPresenter::~CPresenter(void)
{
    SafeDelete(m_pMonitors);
}

// IUnknown
ULONG DX11VideoRenderer::CPresenter::AddRef(void)
{
    return InterlockedIncrement(&m_nRefCount);
}

// IUnknown
HRESULT DX11VideoRenderer::CPresenter::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IMFVideoDisplayControl*>(this));
    }
    else if (iid == __uuidof(IMFVideoDisplayControl))
    {
        *ppv = static_cast<IMFVideoDisplayControl*>(this);
    }
    else if (iid == __uuidof(IMFGetService))
    {
        *ppv = static_cast<IMFGetService*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

// IUnknown
ULONG  DX11VideoRenderer::CPresenter::Release(void)
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

// IMFVideoDisplayControl
HRESULT DX11VideoRenderer::CPresenter::GetFullscreen(__RPC__out BOOL* pfFullscreen)
{
    CAutoLock lock(&m_critSec);

    HRESULT hr = CheckShutdown();
    if (FAILED(hr))
    {
        return hr;
    }

    if (pfFullscreen == NULL)
    {
        return E_POINTER;
    }

    *pfFullscreen = m_bFullScreenState;

    return S_OK;
}

// IMFVideoDisplayControl
HRESULT DX11VideoRenderer::CPresenter::SetFullscreen(BOOL fFullscreen)
{
    CAutoLock lock(&m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        m_bFullScreenState = fFullscreen;

        SafeRelease(m_pDX11VideoDevice);
        SafeRelease(m_pVideoProcessorEnum);
        SafeRelease(m_pVideoProcessor);
    }

    return hr;
}

// IMFVideoDisplayControl
HRESULT DX11VideoRenderer::CPresenter::SetVideoWindow(__RPC__in HWND hwndVideo)
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_critSec);

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        if (!IsWindow(hwndVideo))
        {
            hr = E_INVALIDARG;
            break;
        }

        m_pMonitors = new CMonitorArray();
        if (!m_pMonitors)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = SetVideoMonitor(hwndVideo);
        if (FAILED(hr))
        {
            break;
        }

        CheckDecodeSwitchRegKey();

        m_hwndVideo = hwndVideo;

        hr = CreateDXGIManagerAndDevice();
        if (FAILED(hr))
        {
            break;
        }

        if (m_useXVP)
        {
            hr = CreateXVP();
            if (FAILED(hr))
            {
                break;
            }
        }
    }
    while(FALSE);

    return hr;
}

//-------------------------------------------------------------------------
// Name: GetService
// Description: IMFGetService
//-------------------------------------------------------------------------

HRESULT DX11VideoRenderer::CPresenter::GetService(__RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID* ppvObject)
{
    HRESULT hr = S_OK;

    if (guidService == MR_VIDEO_ACCELERATION_SERVICE)
    {
        if (riid == __uuidof(IMFDXGIDeviceManager))
        {
            if (NULL != m_pDXGIManager)
            {
                *ppvObject = (void*) static_cast<IUnknown*>(m_pDXGIManager);
                ((IUnknown*) *ppvObject)->AddRef();
            }
            else
            {
                hr = E_NOINTERFACE;
            }
        }
        else if (riid == __uuidof(IMFVideoSampleAllocatorEx))
        {
            if (NULL == m_pSampleAllocatorEx)
            {
                hr = MFCreateVideoSampleAllocatorEx(IID_IMFVideoSampleAllocatorEx, (LPVOID*)&m_pSampleAllocatorEx);
                if (SUCCEEDED(hr) && NULL != m_pDXGIManager)
                {
                    hr = m_pSampleAllocatorEx->SetDirectXManager(m_pDXGIManager);
                }
            }
            if (SUCCEEDED(hr))
            {
                hr = m_pSampleAllocatorEx->QueryInterface(riid, ppvObject);
            }
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }
    else if (guidService == MR_VIDEO_RENDER_SERVICE)
    {
        hr = QueryInterface(riid, ppvObject);
    }
    else
    {
        hr = MF_E_UNSUPPORTED_SERVICE;
    }

    return hr;
}

BOOL DX11VideoRenderer::CPresenter::CanProcessNextSample(void)
{
    return m_bCanProcessNextSample;
}

HRESULT DX11VideoRenderer::CPresenter::Flush(void)
{
    CAutoLock lock(&m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr) && m_useXVP)
    {
        hr = m_pXVP->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    }

    m_bCanProcessNextSample = TRUE;

    return hr;
}

HRESULT DX11VideoRenderer::CPresenter::GetMonitorRefreshRate(DWORD* pdwRefreshRate)
{
    if (pdwRefreshRate == NULL)
    {
        return E_POINTER;
    }

    if (m_lpCurrMon == NULL)
    {
        return MF_E_INVALIDREQUEST;
    }

    *pdwRefreshRate = m_lpCurrMon->dwRefreshRate;

    return S_OK;
}

HRESULT DX11VideoRenderer::CPresenter::IsMediaTypeSupported(IMFMediaType* pMediaType, DXGI_FORMAT dxgiFormat)
{
    HRESULT hr = S_OK;
    UINT32 uiNumerator = 30000, uiDenominator = 1001;
    UINT32 uimageWidthInPixels, uimageHeightInPixels = 0;

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        if (pMediaType == NULL)
        {
            hr = E_POINTER;
            break;
        }

        if (!m_pDX11VideoDevice)
        {
            hr = m_pD3D11Device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&m_pDX11VideoDevice);
            if (FAILED(hr))
            {
                break;
            }
        }

        hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &uimageWidthInPixels, &uimageHeightInPixels);

        if (FAILED(hr))
        {
            break;
        }

        MFGetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, &uiNumerator, &uiDenominator);

        //Check if the format is supported

        D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc;
        ZeroMemory( &ContentDesc, sizeof( ContentDesc ) );
        ContentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
        ContentDesc.InputWidth = (DWORD)uimageWidthInPixels;
        ContentDesc.InputHeight = (DWORD)uimageHeightInPixels;
        ContentDesc.OutputWidth = (DWORD)uimageWidthInPixels;
        ContentDesc.OutputHeight = (DWORD)uimageHeightInPixels;
        ContentDesc.InputFrameRate.Numerator = uiNumerator;
        ContentDesc.InputFrameRate.Denominator = uiDenominator;
        ContentDesc.OutputFrameRate.Numerator = uiNumerator;
        ContentDesc.OutputFrameRate.Denominator = uiDenominator;
        ContentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

        SafeRelease(m_pVideoProcessorEnum);
        hr = m_pDX11VideoDevice->CreateVideoProcessorEnumerator(&ContentDesc, &m_pVideoProcessorEnum);
        if (FAILED(hr))
        {
            break;
        }

        UINT uiFlags;
        hr = m_pVideoProcessorEnum->CheckVideoProcessorFormat(dxgiFormat, &uiFlags);
        if (FAILED(hr) || 0 == (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT))
        {
            hr = MF_E_UNSUPPORTED_D3D_TYPE;
            break;
        }

        if (m_useXVP)
        {
            hr = m_pXVP->SetInputType(0, pMediaType, MFT_SET_TYPE_TEST_ONLY);
            if (FAILED(hr))
            {
                break;
            }
        }
    }
    while (FALSE);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     PresentFrame
//
//  Synopsis:   Present the current outstanding frame in the DX queue
//
//--------------------------------------------------------------------------

HRESULT DX11VideoRenderer::CPresenter::PresentFrame(void)
{
    HRESULT hr = S_OK;

    CAutoLock lock(&m_critSec);

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        if (NULL == m_pSwapChain1)
        {
            break;
        }

        RECT rcDest;
        ZeroMemory(&rcDest, sizeof(rcDest));
        if (CheckEmptyRect(&rcDest))
        {
            hr = S_OK;
            break;
        }

        hr = m_pSwapChain1->Present( 0, 0 );
        if (FAILED(hr))
        {
            break;
        }

        m_bCanProcessNextSample = TRUE;
    }
    while (FALSE);

    return hr;
}

//-------------------------------------------------------------------
// Name: ProcessFrame
// Description: Present one media sample.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CPresenter::ProcessFrame(IMFMediaType* pCurrentType, IMFSample* pSample, UINT32* punInterlaceMode, BOOL* pbDeviceChanged, BOOL* pbProcessAgain, IMFSample** ppOutputSample)
{
    HRESULT hr = S_OK;
    BYTE* pData = NULL;
    DWORD dwSampleSize = 0;
    IMFMediaBuffer* pBuffer = NULL;
    IMFMediaBuffer* pEVBuffer = NULL;
    DWORD cBuffers = 0;
    ID3D11Texture2D* pTexture2D = NULL;
    IMFDXGIBuffer* pDXGIBuffer  = NULL;
    ID3D11Texture2D* pEVTexture2D = NULL;
    IMFDXGIBuffer* pEVDXGIBuffer  = NULL;
    ID3D11Device* pDeviceInput = NULL;
    UINT dwViewIndex = 0;
    UINT dwEVViewIndex = 0;

    CAutoLock lock(&m_critSec);

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        if (punInterlaceMode == NULL || pCurrentType == NULL || pSample == NULL || pbDeviceChanged == NULL || pbProcessAgain == NULL)
        {
            hr = E_POINTER;
            break;
        }

        *pbProcessAgain = FALSE;
        *pbDeviceChanged = FALSE;

        hr = pSample->GetBufferCount( &cBuffers );
        if (FAILED(hr))
        {
            break;
        }

        if (1 == cBuffers)
        {
            hr = pSample->GetBufferByIndex(0, &pBuffer);
        }
        else if (2 == cBuffers && m_b3DVideo && 0 != m_vp3DOutput)
        {
            hr = pSample->GetBufferByIndex(0, &pBuffer);
            if (FAILED(hr))
            {
                break;
            }

            hr = pSample->GetBufferByIndex(1, &pEVBuffer);
        }
        else
        {
            hr = pSample->ConvertToContiguousBuffer(&pBuffer);
        }

        if (FAILED(hr))
        {
            break;
        }

        hr = CheckDeviceState(pbDeviceChanged);
        if (FAILED(hr))
        {
            break;
        }

        RECT rcDest;
        ZeroMemory(&rcDest, sizeof(rcDest));
        if (CheckEmptyRect(&rcDest))
        {
            hr = S_OK;
            break;
        }

        MFVideoInterlaceMode unInterlaceMode = (MFVideoInterlaceMode) MFGetAttributeUINT32( pCurrentType, MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive );

        //
        // Check the per-sample attributes
        //
        if (MFVideoInterlace_MixedInterlaceOrProgressive == unInterlaceMode)
        {
            BOOL fInterlaced = MFGetAttributeUINT32( pSample, MFSampleExtension_Interlaced, FALSE );
            if ( !fInterlaced )
            {
                // Progressive sample
                *punInterlaceMode = MFVideoInterlace_Progressive;
            }
            else
            {
                BOOL fBottomFirst = MFGetAttributeUINT32( pSample, MFSampleExtension_BottomFieldFirst, FALSE );
                if ( fBottomFirst )
                {
                    *punInterlaceMode = MFVideoInterlace_FieldInterleavedLowerFirst;
                }
                else
                {
                    *punInterlaceMode = MFVideoInterlace_FieldInterleavedUpperFirst;
                }
            }
        }

        hr = pBuffer->QueryInterface(__uuidof(IMFDXGIBuffer), (LPVOID*)&pDXGIBuffer);
        if (FAILED(hr))
        {
            break;
        }

        hr = pDXGIBuffer->GetResource(__uuidof(ID3D11Texture2D), (LPVOID*)&pTexture2D);
        if (FAILED(hr))
        {
            break;
        }

        hr = pDXGIBuffer->GetSubresourceIndex(&dwViewIndex);
        if (FAILED(hr))
        {
            break;
        }

        if (m_b3DVideo && 0 != m_vp3DOutput)
        {
            if (pEVBuffer && MFVideo3DSampleFormat_MultiView == m_vp3DOutput)
            {
                hr = pEVBuffer->QueryInterface(__uuidof(IMFDXGIBuffer), (LPVOID*)&pEVDXGIBuffer);
                if (FAILED(hr))
                {
                    break;
                }

                hr = pEVDXGIBuffer->GetResource(__uuidof(ID3D11Texture2D), (LPVOID*)&pEVTexture2D);
                if (FAILED(hr))
                {
                    break;
                }

                hr = pEVDXGIBuffer->GetSubresourceIndex(&dwEVViewIndex);
                if (FAILED(hr))
                {
                    break;
                }
            }
        }

        pTexture2D->GetDevice(&pDeviceInput);
        if ((NULL == pDeviceInput) || (pDeviceInput !=  m_pD3D11Device))
        {
            break;
        }

        if (m_useXVP)
        {
            BOOL bInputFrameUsed = FALSE;

            hr = ProcessFrameUsingXVP( pCurrentType, pSample, pTexture2D, rcDest, ppOutputSample, &bInputFrameUsed );

            if (SUCCEEDED(hr) && !bInputFrameUsed)
            {
                *pbProcessAgain = TRUE;
            }
        }
        else
        {
            hr = ProcessFrameUsingD3D11( pTexture2D, pEVTexture2D, dwViewIndex, dwEVViewIndex, rcDest, *punInterlaceMode, ppOutputSample );

            LONGLONG hnsDuration = 0;
            LONGLONG hnsTime = 0;
            DWORD dwSampleFlags = 0;

            if (ppOutputSample != NULL && *ppOutputSample != NULL)
            {
                if (SUCCEEDED(pSample->GetSampleDuration(&hnsDuration)))
                {
                    (*ppOutputSample)->SetSampleDuration(hnsDuration);
                }

                if (SUCCEEDED(pSample->GetSampleTime(&hnsTime)))
                {
                    (*ppOutputSample)->SetSampleTime(hnsTime);
                }

                if (SUCCEEDED(pSample->GetSampleFlags(&dwSampleFlags)))
                {
                    (*ppOutputSample)->SetSampleFlags(dwSampleFlags);
                }
            }
        }
    }
    while (FALSE);

    SafeRelease(pTexture2D);
    SafeRelease(pDXGIBuffer);
    SafeRelease(pEVTexture2D);
    SafeRelease(pEVDXGIBuffer);
    SafeRelease(pDeviceInput);
    SafeRelease(pBuffer);
    SafeRelease(pEVBuffer);

    return hr;
}

HRESULT DX11VideoRenderer::CPresenter::SetCurrentMediaType(IMFMediaType* pMediaType)
{
    HRESULT hr = S_OK;
    IMFAttributes* pAttributes = NULL;

    CAutoLock lock(&m_critSec);

    do
    {
        hr = CheckShutdown();
        if (FAILED(hr))
        {
            break;
        }

        hr = pMediaType->QueryInterface(IID_IMFAttributes, reinterpret_cast<void**>(&pAttributes));
        if (FAILED(hr))
        {
            break;
        }

        HRESULT hr1 = pAttributes->GetUINT32(MF_MT_VIDEO_3D, (UINT32*)&m_b3DVideo);
        if (SUCCEEDED(hr1))
        {
            hr = pAttributes->GetUINT32(MF_MT_VIDEO_3D_FORMAT, (UINT32*)&m_vp3DOutput);
            if (FAILED(hr))
            {
                break;
            }
        }

        //Now Determine Correct Display Resolution
        if (SUCCEEDED(hr))
        {
            UINT32 parX = 0, parY = 0;
            int PARWidth = 0, PARHeight = 0;
            MFVideoArea videoArea = {0};
            ZeroMemory(&m_displayRect, sizeof(RECT));

            if (FAILED(MFGetAttributeSize(pMediaType, MF_MT_PIXEL_ASPECT_RATIO, &parX, &parY)))
            {
                parX = 1;
                parY = 1;
            }

            hr = GetVideoDisplayArea(pMediaType, &videoArea);
            if (FAILED(hr))
            {
                break;
            }

            m_displayRect = MFVideoAreaToRect(videoArea);

            PixelAspectToPictureAspect(
                videoArea.Area.cx,
                videoArea.Area.cy,
                parX,
                parY,
                &PARWidth,
                &PARHeight);

            SIZE szVideo = videoArea.Area;
            SIZE szPARVideo = {PARWidth, PARHeight};
            AspectRatioCorrectSize(&szVideo, szPARVideo, videoArea.Area, FALSE);
            m_uiRealDisplayWidth = szVideo.cx;
            m_uiRealDisplayHeight = szVideo.cy;
        }

        if (SUCCEEDED(hr) && m_useXVP)
        {
            // set the input type on the XVP
            hr = m_pXVP->SetInputType(0, pMediaType, 0);
            if (FAILED(hr))
            {
                break;
            }
        }
    }
    while (FALSE);

    SafeRelease(pAttributes);

    return hr;
}

//-------------------------------------------------------------------
// Name: Shutdown
// Description: Releases resources held by the presenter.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CPresenter::Shutdown(void)
{
    CAutoLock lock(&m_critSec);

    HRESULT hr = MF_E_SHUTDOWN;

    m_IsShutdown = TRUE;

    SafeRelease(m_pDXGIManager);
    SafeRelease(m_pDXGIFactory2);
    SafeRelease(m_pD3D11Device);
    SafeRelease(m_pD3DImmediateContext);
    SafeRelease(m_pDXGIOutput1);
    SafeRelease(m_pSampleAllocatorEx);
    SafeRelease(m_pDCompDevice);
    SafeRelease(m_pHwndTarget);
    SafeRelease(m_pRootVisual);
    SafeRelease(m_pXVPControl);
    SafeRelease(m_pXVP);
    SafeRelease(m_pDX11VideoDevice);
    SafeRelease(m_pVideoProcessor);
    SafeRelease(m_pVideoProcessorEnum);
    SafeRelease(m_pSwapChain1);

    return hr;
}

/// Private methods

//+-------------------------------------------------------------------------
//
//  Function:   AspectRatioCorrectSize
//
//  Synopsis:   Corrects the supplied size structure so that it becomes the same shape
//              as the specified aspect ratio, the correction is always applied in the
//              horizontal axis
//
//--------------------------------------------------------------------------

void DX11VideoRenderer::CPresenter::AspectRatioCorrectSize(
    LPSIZE lpSizeImage,     // size to be aspect ratio corrected
    const SIZE& sizeAr,     // aspect ratio of image
    const SIZE& sizeOrig,   // original image size
    BOOL ScaleXorY          // axis to correct in
    )
{
    int cxAR = sizeAr.cx;
    int cyAR = sizeAr.cy;
    int cxOr = sizeOrig.cx;
    int cyOr = sizeOrig.cy;
    int sx   = lpSizeImage->cx;
    int sy   = lpSizeImage->cy;

    // MulDiv rounds correctly.
    lpSizeImage->cx = MulDiv((sx * cyOr), cxAR, (cyAR * cxOr));

    if (ScaleXorY && lpSizeImage->cx < cxOr)
    {
        lpSizeImage->cx = cxOr;
        lpSizeImage->cy = MulDiv((sy * cxOr), cyAR, (cxAR * cyOr));
    }
}

void DX11VideoRenderer::CPresenter::CheckDecodeSwitchRegKey(void)
{
    const TCHAR* lpcszDXSW = TEXT("DXSWSwitch");
    const TCHAR* lpcszInVP = TEXT("XVP");
    const TCHAR* lpcszDComp = TEXT("DComp");
    const TCHAR* lpcszDebugLayer = TEXT("Dbglayer");
    const TCHAR* lpcszREGKEY = TEXT("SOFTWARE\\Microsoft\\Scrunch\\CodecPack\\MSDVD");
    HKEY hk = NULL;
    DWORD dwData;
    DWORD cbData = sizeof(DWORD);
    DWORD cbType;

    if(0 == RegOpenKeyEx(HKEY_CURRENT_USER, lpcszREGKEY, 0, KEY_READ, &hk))
    {
        if (0 == RegQueryValueEx(hk, lpcszDXSW, 0, &cbType, (LPBYTE)&dwData, &cbData))
        {
            m_DXSWSwitch = dwData;
        }

        dwData = 0;
        cbData = sizeof(DWORD);
        if (0 == RegQueryValueEx(hk, lpcszInVP, 0, &cbType, (LPBYTE)&dwData, &cbData))
        {
            m_useXVP = dwData;
        }

        dwData = 0;
        cbData = sizeof(DWORD);
        if (0 == RegQueryValueEx(hk, lpcszDComp, 0, &cbType, (LPBYTE)&dwData, &cbData))
        {
            m_useDCompVisual = dwData;
        }

        dwData = 0;
        cbData = sizeof(DWORD);
        if (0 == RegQueryValueEx(hk, lpcszDebugLayer, 0, &cbType, (LPBYTE)&dwData, &cbData))
        {
            m_useDebugLayer = dwData;
        }
    }

    if(NULL != hk)
    {
        RegCloseKey(hk);
    }

    return;
}
        
HRESULT DX11VideoRenderer::CPresenter::CheckDeviceState(BOOL* pbDeviceChanged)
{
    if (pbDeviceChanged == NULL)
    {
        return E_POINTER;
    }

    static int deviceStateChecks = 0;
    static D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

    HRESULT hr = SetVideoMonitor(m_hwndVideo);
    if (FAILED(hr))
    {
        return hr;
    }

    if (m_pD3D11Device != NULL)
    {
        // Lost/hung device. Destroy the device and create a new one.
        if (S_FALSE == hr || (m_DXSWSwitch > 0 && deviceStateChecks == m_DXSWSwitch))
        {
            if (m_DXSWSwitch > 0 && deviceStateChecks == m_DXSWSwitch)
            {
                (driverType == D3D_DRIVER_TYPE_HARDWARE) ? driverType = D3D_DRIVER_TYPE_WARP : driverType = D3D_DRIVER_TYPE_HARDWARE;
            }

            hr = CreateDXGIManagerAndDevice(driverType);
            if (FAILED(hr))
            {
                return hr;
            }

            *pbDeviceChanged = TRUE;

            SafeRelease(m_pDX11VideoDevice);
            SafeRelease(m_pVideoProcessorEnum);
            SafeRelease(m_pVideoProcessor);
            SafeRelease(m_pSwapChain1);

            deviceStateChecks = 0;
        }
        deviceStateChecks++;
    }

    return hr;
}

BOOL DX11VideoRenderer::CPresenter::CheckEmptyRect(RECT* pDst)
{
    GetClientRect(m_hwndVideo, pDst);

    return IsRectEmpty(pDst);
}

HRESULT DX11VideoRenderer::CPresenter::CheckShutdown(void) const
{
    if (m_IsShutdown)
    {
        return MF_E_SHUTDOWN;
    }
    else
    {
        return S_OK;
    }
}

HRESULT DX11VideoRenderer::CPresenter::CreateDCompDeviceAndVisual(void)
{
    HRESULT hr = S_OK;
    IDXGIDevice* pDXGIDevice = NULL;

    do
    {
        hr = m_pD3D11Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDXGIDevice));
        if (FAILED(hr))
        {
            break;
        }

        hr = DCompositionCreateDevice(pDXGIDevice, __uuidof(IDCompositionDevice), reinterpret_cast<void**>(&m_pDCompDevice));
        if (FAILED(hr))
        {
            break;
        }

        hr = m_pDCompDevice->CreateTargetForHwnd(m_hwndVideo, TRUE, &m_pHwndTarget);
        if (FAILED(hr))
        {
            break;
        }

        hr = m_pDCompDevice->CreateVisual(reinterpret_cast<IDCompositionVisual**>(&m_pRootVisual));
        if (FAILED(hr))
        {
            break;
        }

        hr = m_pHwndTarget->SetRoot(m_pRootVisual);
        if (FAILED(hr))
        {
            break;
        }
    }
    while(FALSE);

    SafeRelease(pDXGIDevice);

    return hr;
}

//-------------------------------------------------------------------
// Name: CreateDXGIManagerAndDevice
// Description: Creates D3D11 device and manager.
//
// Note: This method is called once when SetVideoWindow is called using
//       IDX11VideoRenderer.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CPresenter::CreateDXGIManagerAndDevice(D3D_DRIVER_TYPE DriverType)
{
    HRESULT hr = S_OK;

    IDXGIAdapter* pTempAdapter = NULL;
    ID3D10Multithread* pMultiThread = NULL;
    IDXGIDevice1* pDXGIDev = NULL;
    IDXGIAdapter1* pAdapter = NULL;
    IDXGIOutput* pDXGIOutput = NULL;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };
    D3D_FEATURE_LEVEL featureLevel;
    UINT resetToken;

    do
    {
        SafeRelease(m_pD3D11Device);
        if (D3D_DRIVER_TYPE_WARP == DriverType)
        {
            ID3D11Device* pD3D11Device = NULL;

            hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, m_useDebugLayer, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &pD3D11Device, &featureLevel, NULL);

            if (SUCCEEDED(hr))
            {
                m_pD3D11Device = new CPrivate_ID3D11Device(pD3D11Device);
                if (NULL == m_pD3D11Device)
                {
                    E_OUTOFMEMORY;
                }
            }
            // SafeRelease(pD3D11Device);
        }
        else
        {
            for (DWORD dwCount = 0; dwCount < ARRAYSIZE(featureLevels); dwCount++)
            {
                hr = D3D11CreateDevice(NULL, DriverType, NULL, m_useDebugLayer, &featureLevels[dwCount], 1, D3D11_SDK_VERSION, &m_pD3D11Device, &featureLevel, NULL);
                if (SUCCEEDED(hr))
                {
                    ID3D11VideoDevice* pDX11VideoDevice = NULL;
                    hr = m_pD3D11Device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&pDX11VideoDevice);
                    SafeRelease(pDX11VideoDevice);

                    if (SUCCEEDED(hr))
                    {
                        break;
                    }
                    SafeRelease(m_pD3D11Device);
                }
            }
        }

        if (FAILED(hr))
        {
            break;
        }

        if (NULL == m_pDXGIManager)
        {
            hr = MFCreateDXGIDeviceManager(&resetToken, &m_pDXGIManager);
            if (FAILED(hr))
            {
                break;
            }
            m_DeviceResetToken = resetToken;
        }

        hr = m_pDXGIManager->ResetDevice(m_pD3D11Device, m_DeviceResetToken);
        if (FAILED(hr))
        {
            break;
        }

        SafeRelease(m_pD3DImmediateContext);
        m_pD3D11Device->GetImmediateContext(&m_pD3DImmediateContext);

        // Need to explitly set the multithreaded mode for this device
        hr = m_pD3DImmediateContext->QueryInterface(__uuidof(ID3D10Multithread), (void**)&pMultiThread);
        if (FAILED(hr))
        {
            break;
        }

        pMultiThread->SetMultithreadProtected(TRUE);

        hr = m_pD3D11Device->QueryInterface( __uuidof( IDXGIDevice1 ), ( LPVOID* )&pDXGIDev );
        if (FAILED(hr))
        {
            break;
        }

        hr = pDXGIDev->GetAdapter(&pTempAdapter);
        if (FAILED(hr))
        {
            break;
        }

        hr = pTempAdapter->QueryInterface( __uuidof( IDXGIAdapter1 ), (LPVOID*) &pAdapter ) ;
        if (FAILED(hr))
        {
            break;
        }

        SafeRelease(m_pDXGIFactory2);
        hr = pAdapter->GetParent( __uuidof( IDXGIFactory2 ), (LPVOID*) &m_pDXGIFactory2 );
        if (FAILED(hr))
        {
            break;
        }

        hr = pAdapter->EnumOutputs(0, &pDXGIOutput);
        if (FAILED(hr))
        {
            break;
        }

        SafeRelease(m_pDXGIOutput1);
        hr = pDXGIOutput->QueryInterface( __uuidof(IDXGIOutput1), (LPVOID*) &m_pDXGIOutput1 );
        if (FAILED(hr))
        {
            break;
        }

        if (m_useDCompVisual)
        {
            hr = CreateDCompDeviceAndVisual();
            if (FAILED(hr))
            {
                break;
            }
        }
    }
    while(FALSE);

    SafeRelease(pTempAdapter);
    SafeRelease(pMultiThread);
    SafeRelease(pDXGIDev);
    SafeRelease(pAdapter);
    SafeRelease(pDXGIOutput);

    return hr;
}

//-------------------------------------------------------------------
// Name: CreateXVP
// Description: Creates a new instance of the XVP MFT.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CPresenter::CreateXVP(void)
{
    HRESULT hr = S_OK;
    IMFAttributes* pAttributes = NULL;

    do
    {
        hr = CoCreateInstance(CLSID_VideoProcessorMFT, nullptr, CLSCTX_INPROC_SERVER, IID_IMFTransform, (void**)&m_pXVP);
        if (FAILED(hr))
        {
            break;
        }

        hr = m_pXVP->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, ULONG_PTR(m_pDXGIManager));
        if (FAILED(hr))
        {
            break;
        }

        // Tell the XVP that we are the swapchain allocator
        hr = m_pXVP->GetAttributes(&pAttributes);
        if (FAILED(hr))
        {
            break;
        }

        hr = pAttributes->SetUINT32(MF_XVP_PLAYBACK_MODE, TRUE);
        if (FAILED(hr))
        {
            break;
        }

        hr = m_pXVP->QueryInterface(IID_PPV_ARGS(&m_pXVPControl));
        if (FAILED(hr))
        {
            break;
        }
    }
    while (FALSE);

    SafeRelease(pAttributes);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     FindBOBProcessorIndex
//
//  Synopsis:   Find the BOB video processor. BOB does not require any
//              reference frames and can be used with both Progressive
//              and interlaced video
//
//--------------------------------------------------------------------------

HRESULT DX11VideoRenderer::CPresenter::FindBOBProcessorIndex(DWORD* pIndex)
{
    HRESULT hr = S_OK;
    D3D11_VIDEO_PROCESSOR_CAPS caps = {};
    D3D11_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS convCaps = {};

    *pIndex = 0;
    hr = m_pVideoProcessorEnum->GetVideoProcessorCaps(&caps);
    if (FAILED(hr))
    {
        return hr;
    }
    for (DWORD i = 0; i < caps.RateConversionCapsCount; i++)
    {
        hr = m_pVideoProcessorEnum->GetVideoProcessorRateConversionCaps(i, &convCaps);
        if (FAILED(hr))
        {
            return hr;
        }

        // Check the caps to see which deinterlacer is supported
        if ((convCaps.ProcessorCaps & D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_BOB) != 0)
        {
            *pIndex = i;
            return hr;
        }
    }

    return E_FAIL;
}

//-------------------------------------------------------------------
// Name: GetVideoDisplayArea
// Description: get the display area from the media type.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CPresenter::GetVideoDisplayArea(IMFMediaType* pType, MFVideoArea* pArea)
{
    HRESULT hr = S_OK;
    BOOL bPanScan = FALSE;
    UINT32 uimageWidthInPixels = 0, uimageHeightInPixels = 0;

    hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &uimageWidthInPixels, &uimageHeightInPixels);
    if (FAILED(hr))
    {
        return hr;
    }

    if (uimageWidthInPixels != m_imageWidthInPixels || uimageHeightInPixels != m_imageHeightInPixels)
    {
        SafeRelease(m_pVideoProcessorEnum);
        SafeRelease(m_pVideoProcessor);
        SafeRelease(m_pSwapChain1);
    }

    m_imageWidthInPixels = uimageWidthInPixels;
    m_imageHeightInPixels = uimageHeightInPixels;

    bPanScan = MFGetAttributeUINT32(pType, MF_MT_PAN_SCAN_ENABLED, FALSE);

    // In pan/scan mode, try to get the pan/scan region.
    if (bPanScan)
    {
        hr = pType->GetBlob(
            MF_MT_PAN_SCAN_APERTURE,
            (UINT8*)pArea,
            sizeof(MFVideoArea),
            NULL
            );
    }

    // If not in pan/scan mode, or the pan/scan region is not set,
    // get the minimimum display aperture.

    if (!bPanScan || hr == MF_E_ATTRIBUTENOTFOUND)
    {
        hr = pType->GetBlob(
            MF_MT_MINIMUM_DISPLAY_APERTURE,
            (UINT8*)pArea,
            sizeof(MFVideoArea),
            NULL
            );

        if (hr == MF_E_ATTRIBUTENOTFOUND)
        {
            // Minimum display aperture is not set.

            // For backward compatibility with some components,
            // check for a geometric aperture.

            hr = pType->GetBlob(
                MF_MT_GEOMETRIC_APERTURE,
                (UINT8*)pArea,
                sizeof(MFVideoArea),
                NULL
                );
        }

        // Default: Use the entire video area.

        if (hr == MF_E_ATTRIBUTENOTFOUND)
        {
            *pArea = MakeArea(0.0, 0.0, m_imageWidthInPixels, m_imageHeightInPixels);
            hr = S_OK;
        }
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   LetterBoxDstRectPixelAspectToPictureAspect
//
//  Synopsis:
//
// Takes a src rectangle and constructs the largest possible destination
// rectangle within the specifed destination rectangle such that
// the video maintains its current shape.
//
// This function assumes that pels are the same shape within both the src
// and dst rectangles.
//
//--------------------------------------------------------------------------

void DX11VideoRenderer::CPresenter::LetterBoxDstRect(
    LPRECT lprcLBDst,     // output letterboxed rectangle
    const RECT& rcSrc,    // input source rectangle
    const RECT& rcDst     // input destination rectangle
    )
{
    // figure out src/dest scale ratios
    int iSrcWidth  = rcSrc.right - rcSrc.left;
    int iSrcHeight = rcSrc.bottom - rcSrc.top;

    int iDstWidth  = rcDst.right - rcDst.left;
    int iDstHeight = rcDst.bottom - rcDst.top;

    int iDstLBWidth  = 0;
    int iDstLBHeight = 0;

    //
    // work out if we are Column or Row letter boxing
    //

    if (MulDiv(iSrcWidth, iDstHeight, iSrcHeight) <= iDstWidth)
    {
        //
        // column letter boxing - we add border color bars to the
        // left and right of the video image to fill the destination
        // rectangle.
        //
        iDstLBWidth  = MulDiv(iDstHeight, iSrcWidth, iSrcHeight);
        iDstLBHeight = iDstHeight;
    }
    else
    {
        //
        // row letter boxing - we add border color bars to the top
        // and bottom of the video image to fill the destination
        // rectangle
        //
        iDstLBWidth  = iDstWidth;
        iDstLBHeight = MulDiv(iDstWidth, iSrcHeight, iSrcWidth);
    }

    //
    // now create a centered LB rectangle within the current destination rect
    //
    lprcLBDst->left   = rcDst.left + ((iDstWidth - iDstLBWidth) / 2);
    lprcLBDst->right  = lprcLBDst->left + iDstLBWidth;

    lprcLBDst->top    = rcDst.top + ((iDstHeight - iDstLBHeight) / 2);
    lprcLBDst->bottom = lprcLBDst->top + iDstLBHeight;
}

//+-------------------------------------------------------------------------
//
//  Function:   PixelAspectToPictureAspect
//
//  Synopsis:   Converts a pixel aspect ratio to a picture aspect ratio
//
//--------------------------------------------------------------------------

void DX11VideoRenderer::CPresenter::PixelAspectToPictureAspect(
    int Width,
    int Height,
    int PixelAspectX,
    int PixelAspectY,
    int* pPictureAspectX,
    int* pPictureAspectY
    )
{
    //
    // sanity check - if any inputs are 0, return 0
    //
    if (PixelAspectX == 0 || PixelAspectY == 0 || Width == 0 || Height == 0)
    {
        *pPictureAspectX = 0;
        *pPictureAspectY = 0;
        return;
    }

    //
    // start by reducing both ratios to lowest terms
    //
    ReduceToLowestTerms(Width, Height, &Width, &Height);
    ReduceToLowestTerms(PixelAspectX, PixelAspectY, &PixelAspectX, &PixelAspectY);

    //
    // Make sure that none of the values are larger than 2^16, so we don't
    // overflow on the last operation.   This reduces the accuracy somewhat,
    // but it's a "hail mary" for incredibly strange aspect ratios that don't
    // exist in practical usage.
    //
    while (Width > 0xFFFF || Height > 0xFFFF)
    {
        Width  >>= 1;
        Height >>= 1;
    }

    while (PixelAspectX > 0xFFFF || PixelAspectY > 0xFFFF)
    {
        PixelAspectX >>= 1;
        PixelAspectY >>= 1;
    }

    ReduceToLowestTerms(
        PixelAspectX * Width,
        PixelAspectY * Height,
        pPictureAspectX,
        pPictureAspectY
        );
}

HRESULT DX11VideoRenderer::CPresenter::ProcessFrameUsingD3D11( ID3D11Texture2D* pLeftTexture2D, ID3D11Texture2D* pRightTexture2D, UINT dwLeftViewIndex, UINT dwRightViewIndex, RECT rcDest, UINT32 unInterlaceMode, IMFSample** ppVideoOutFrame )
{
    HRESULT hr = S_OK;
    ID3D11VideoContext* pVideoContext = NULL;
    ID3D11VideoProcessorInputView* pLeftInputView = NULL;
    ID3D11VideoProcessorInputView* pRightInputView = NULL;
    ID3D11VideoProcessorOutputView* pOutputView = NULL;
    ID3D11Texture2D* pDXGIBackBuffer = NULL;
    ID3D11RenderTargetView* pRTView = NULL;
    IMFSample* pRTSample = NULL;
    IMFMediaBuffer* pBuffer = NULL;
    D3D11_VIDEO_PROCESSOR_CAPS vpCaps = {0};
    LARGE_INTEGER lpcStart,lpcEnd;

    do
    {
        if (!m_pDX11VideoDevice)
        {
            hr = m_pD3D11Device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&m_pDX11VideoDevice);
            if (FAILED(hr))
            {
                break;
            }
        }

        hr = m_pD3DImmediateContext->QueryInterface(__uuidof( ID3D11VideoContext ), (void**)&pVideoContext);
        if (FAILED(hr))
        {
            break;
        }

        // remember the original rectangles
        RECT TRectOld = m_rcDstApp;
        RECT SRectOld = m_rcSrcApp;
        UpdateRectangles(&TRectOld, &SRectOld);

        //Update destination rect with current client rect
        m_rcDstApp = rcDest;

        D3D11_TEXTURE2D_DESC surfaceDesc;
        pLeftTexture2D->GetDesc(&surfaceDesc);

        if (!m_pVideoProcessorEnum || !m_pVideoProcessor || m_imageWidthInPixels != surfaceDesc.Width || m_imageHeightInPixels != surfaceDesc.Height)
        {
            SafeRelease(m_pVideoProcessorEnum);
            SafeRelease(m_pVideoProcessor);

            m_imageWidthInPixels = surfaceDesc.Width;
            m_imageHeightInPixels = surfaceDesc.Height;

            D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc;
            ZeroMemory( &ContentDesc, sizeof( ContentDesc ) );
            ContentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
            ContentDesc.InputWidth = surfaceDesc.Width;
            ContentDesc.InputHeight = surfaceDesc.Height;
            ContentDesc.OutputWidth = surfaceDesc.Width;
            ContentDesc.OutputHeight = surfaceDesc.Height;
            ContentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

            hr = m_pDX11VideoDevice->CreateVideoProcessorEnumerator(&ContentDesc, &m_pVideoProcessorEnum);
            if (FAILED(hr))
            {
                break;
            }

            UINT uiFlags;
            DXGI_FORMAT VP_Output_Format = DXGI_FORMAT_B8G8R8A8_UNORM;

            hr = m_pVideoProcessorEnum->CheckVideoProcessorFormat(VP_Output_Format, &uiFlags);
            if (FAILED(hr) || 0 == (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT))
            {
                hr = MF_E_UNSUPPORTED_D3D_TYPE;
                break;
            }

            m_rcSrcApp.left = 0;
            m_rcSrcApp.top = 0;
            m_rcSrcApp.right = m_uiRealDisplayWidth;
            m_rcSrcApp.bottom = m_uiRealDisplayHeight;

            DWORD index;
            hr = FindBOBProcessorIndex(&index);
            if (FAILED(hr))
            {
                break;
            }

            hr = m_pDX11VideoDevice->CreateVideoProcessor(m_pVideoProcessorEnum, index, &m_pVideoProcessor);
            if (FAILED(hr))
            {
                break;
            }

            if (m_b3DVideo)
            {
                hr = m_pVideoProcessorEnum->GetVideoProcessorCaps(&vpCaps);
                if (FAILED(hr))
                {
                    break;
                }

                if (vpCaps.FeatureCaps & D3D11_VIDEO_PROCESSOR_FEATURE_CAPS_STEREO)
                {
                    m_bStereoEnabled = TRUE;
                }

                DXGI_MODE_DESC1 modeFilter = { 0 };
                modeFilter.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                modeFilter.Width = surfaceDesc.Width;
                modeFilter.Height = surfaceDesc.Height;
                modeFilter.Stereo = m_bStereoEnabled;

                DXGI_MODE_DESC1 matchedMode;
                if (m_bFullScreenState)
                {
                    hr = m_pDXGIOutput1->FindClosestMatchingMode1(&modeFilter, &matchedMode, m_pD3D11Device);
                    if (FAILED(hr))
                    {
                        break;
                    }
                }
            }
        }

        // now create the input and output media types - these need to reflect
        // the src and destination rectangles that we have been given.
        RECT TRect = m_rcDstApp;
        RECT SRect = m_rcSrcApp;
        UpdateRectangles(&TRect, &SRect);

        const BOOL fDestRectChanged = !EqualRect(&TRect, &TRectOld);

        if (!m_pSwapChain1 || fDestRectChanged)
        {
            hr = UpdateDXGISwapChain();
            if (FAILED(hr))
            {
                break;
            }
        }

        m_bCanProcessNextSample = FALSE;

        // Get Backbuffer
        hr = m_pSwapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pDXGIBackBuffer);
        if (FAILED(hr))
        {
            break;
        }

        // create the output media sample
        hr = MFCreateSample(&pRTSample);
        if (FAILED(hr))
        {
            break;
        }

        hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), pDXGIBackBuffer, 0, FALSE, &pBuffer);
        if (FAILED(hr))
        {
            break;
        }

        hr = pRTSample->AddBuffer(pBuffer);
        if (FAILED(hr))
        {
            break;
        }

        if (m_b3DVideo && 0 != m_vp3DOutput)
        {
            SafeRelease(pBuffer);

            hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), pDXGIBackBuffer, 1, FALSE, &pBuffer);
            if (FAILED(hr))
            {
                break;
            }

            hr = pRTSample->AddBuffer(pBuffer);
            if (FAILED(hr))
            {
                break;
            }
        }

        QueryPerformanceCounter(&lpcStart);

        QueryPerformanceCounter(&lpcEnd);

        //
        // Create Output View of Output Surfaces.
        //
        D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC OutputViewDesc;
        ZeroMemory( &OutputViewDesc, sizeof( OutputViewDesc ) );
        if (m_b3DVideo && m_bStereoEnabled)
        {
            OutputViewDesc.ViewDimension =  D3D11_VPOV_DIMENSION_TEXTURE2DARRAY;
        }
        else
        {
            OutputViewDesc.ViewDimension =  D3D11_VPOV_DIMENSION_TEXTURE2D;
        }
        OutputViewDesc.Texture2D.MipSlice = 0;
        OutputViewDesc.Texture2DArray.MipSlice = 0;
        OutputViewDesc.Texture2DArray.FirstArraySlice = 0;
        if (m_b3DVideo && 0 != m_vp3DOutput)
        {
            OutputViewDesc.Texture2DArray.ArraySize = 2; // STEREO
        }

        QueryPerformanceCounter(&lpcStart);

        hr  = m_pDX11VideoDevice->CreateVideoProcessorOutputView(pDXGIBackBuffer, m_pVideoProcessorEnum, &OutputViewDesc, &pOutputView);
        if (FAILED(hr))
        {
            break;
        }

        D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC InputLeftViewDesc;
        ZeroMemory( &InputLeftViewDesc, sizeof( InputLeftViewDesc ) );
        InputLeftViewDesc.FourCC = 0;
        InputLeftViewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
        InputLeftViewDesc.Texture2D.MipSlice = 0;
        InputLeftViewDesc.Texture2D.ArraySlice = dwLeftViewIndex;

        hr = m_pDX11VideoDevice->CreateVideoProcessorInputView(pLeftTexture2D, m_pVideoProcessorEnum, &InputLeftViewDesc, &pLeftInputView);
        if (FAILED(hr))
        {
            break;
        }

        if (m_b3DVideo && MFVideo3DSampleFormat_MultiView == m_vp3DOutput && pRightTexture2D)
        {
            D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC InputRightViewDesc;
            ZeroMemory( &InputRightViewDesc, sizeof( InputRightViewDesc ) );
            InputRightViewDesc.FourCC = 0;
            InputRightViewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
            InputRightViewDesc.Texture2D.MipSlice = 0;
            InputRightViewDesc.Texture2D.ArraySlice = dwRightViewIndex;

            hr = m_pDX11VideoDevice->CreateVideoProcessorInputView(pRightTexture2D, m_pVideoProcessorEnum, &InputRightViewDesc, &pRightInputView);
            if (FAILED(hr))
            {
                break;
            }
        }
        QueryPerformanceCounter(&lpcEnd);

        QueryPerformanceCounter(&lpcStart);

        SetVideoContextParameters(pVideoContext, &SRect, &TRect, unInterlaceMode);

        // Enable/Disable Stereo
        if (m_b3DVideo)
        {
            pVideoContext->VideoProcessorSetOutputStereoMode(m_pVideoProcessor, m_bStereoEnabled);

            D3D11_VIDEO_PROCESSOR_STEREO_FORMAT vpStereoFormat = D3D11_VIDEO_PROCESSOR_STEREO_FORMAT_SEPARATE;
            if (MFVideo3DSampleFormat_Packed_LeftRight == m_vp3DOutput)
            {
                vpStereoFormat = D3D11_VIDEO_PROCESSOR_STEREO_FORMAT_HORIZONTAL;
            }
            else if (MFVideo3DSampleFormat_Packed_TopBottom == m_vp3DOutput)
            {
                vpStereoFormat = D3D11_VIDEO_PROCESSOR_STEREO_FORMAT_VERTICAL;
            }

            pVideoContext->VideoProcessorSetStreamStereoFormat(m_pVideoProcessor,
                0, m_bStereoEnabled, vpStereoFormat, TRUE, TRUE, D3D11_VIDEO_PROCESSOR_STEREO_FLIP_NONE, 0);
        }

        QueryPerformanceCounter(&lpcEnd);

        QueryPerformanceCounter(&lpcStart);

        D3D11_VIDEO_PROCESSOR_STREAM StreamData;
        ZeroMemory( &StreamData, sizeof( StreamData ) );
        StreamData.Enable = TRUE;
        StreamData.OutputIndex = 0;
        StreamData.InputFrameOrField = 0;
        StreamData.PastFrames = 0;
        StreamData.FutureFrames = 0;
        StreamData.ppPastSurfaces = NULL;
        StreamData.ppFutureSurfaces = NULL;
        StreamData.pInputSurface = pLeftInputView;
        StreamData.ppPastSurfacesRight = NULL;
        StreamData.ppFutureSurfacesRight = NULL;

        if (m_b3DVideo && MFVideo3DSampleFormat_MultiView == m_vp3DOutput && pRightTexture2D)
        {
            StreamData.pInputSurfaceRight = pRightInputView;
        }

        hr = pVideoContext->VideoProcessorBlt(m_pVideoProcessor, pOutputView, 0, 1, &StreamData );
        if (FAILED(hr))
        {
            break;
        }
        QueryPerformanceCounter(&lpcEnd);

        if (ppVideoOutFrame != NULL)
        {
            *ppVideoOutFrame = pRTSample;
            (*ppVideoOutFrame)->AddRef();
        }
    }
    while (FALSE);

    SafeRelease(pBuffer);
    SafeRelease(pRTSample);
    SafeRelease(pDXGIBackBuffer);
    SafeRelease(pOutputView);
    SafeRelease(pLeftInputView);
    SafeRelease(pRightInputView);
    SafeRelease(pVideoContext);

    return hr;
}

HRESULT DX11VideoRenderer::CPresenter::ProcessFrameUsingXVP( IMFMediaType* pCurrentType, IMFSample* pVideoFrame, ID3D11Texture2D* pTexture2D, RECT rcDest, IMFSample** ppVideoOutFrame, BOOL* pbInputFrameUsed )
{
    HRESULT hr = S_OK;
    ID3D11VideoContext* pVideoContext = NULL;
    ID3D11Texture2D* pDXGIBackBuffer = NULL;
    IMFSample* pRTSample = NULL;
    IMFMediaBuffer* pBuffer = NULL;
    IMFAttributes*  pAttributes = NULL;
    D3D11_VIDEO_PROCESSOR_CAPS vpCaps = {0};

    do
    {
        if (!m_pDX11VideoDevice)
        {
            hr = m_pD3D11Device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&m_pDX11VideoDevice);
            if (FAILED(hr))
            {
                break;
            }
        }

        hr = m_pD3DImmediateContext->QueryInterface(__uuidof( ID3D11VideoContext ), (void**)&pVideoContext);
        if (FAILED(hr))
        {
            break;
        }

        // remember the original rectangles
        RECT TRectOld = m_rcDstApp;
        RECT SRectOld = m_rcSrcApp;
        UpdateRectangles(&TRectOld, &SRectOld);

        //Update destination rect with current client rect
        m_rcDstApp = rcDest;

        D3D11_TEXTURE2D_DESC surfaceDesc;
        pTexture2D->GetDesc(&surfaceDesc);

        BOOL fTypeChanged = FALSE;
        if (!m_pVideoProcessorEnum || !m_pSwapChain1 || m_imageWidthInPixels != surfaceDesc.Width || m_imageHeightInPixels != surfaceDesc.Height)
        {
            SafeRelease(m_pVideoProcessorEnum);
            SafeRelease(m_pSwapChain1);

            m_imageWidthInPixels = surfaceDesc.Width;
            m_imageHeightInPixels = surfaceDesc.Height;
            fTypeChanged = TRUE;

            D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc;
            ZeroMemory( &ContentDesc, sizeof( ContentDesc ) );
            ContentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
            ContentDesc.InputWidth = surfaceDesc.Width;
            ContentDesc.InputHeight = surfaceDesc.Height;
            ContentDesc.OutputWidth = surfaceDesc.Width;
            ContentDesc.OutputHeight = surfaceDesc.Height;
            ContentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

            hr = m_pDX11VideoDevice->CreateVideoProcessorEnumerator(&ContentDesc, &m_pVideoProcessorEnum);
            if (FAILED(hr))
            {
                break;
            }

            m_rcSrcApp.left = 0;
            m_rcSrcApp.top = 0;
            m_rcSrcApp.right = m_uiRealDisplayWidth;
            m_rcSrcApp.bottom = m_uiRealDisplayHeight;

            if (m_b3DVideo)
            {
                hr = m_pVideoProcessorEnum->GetVideoProcessorCaps(&vpCaps);
                if (FAILED(hr))
                {
                    break;
                }

                if (vpCaps.FeatureCaps & D3D11_VIDEO_PROCESSOR_FEATURE_CAPS_STEREO)
                {
                    m_bStereoEnabled = TRUE;
                }

                DXGI_MODE_DESC1 modeFilter = { 0 };
                modeFilter.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                modeFilter.Width = surfaceDesc.Width;
                modeFilter.Height = surfaceDesc.Height;
                modeFilter.Stereo = m_bStereoEnabled;

                DXGI_MODE_DESC1 matchedMode;
                if (m_bFullScreenState)
                {
                    hr = m_pDXGIOutput1->FindClosestMatchingMode1(&modeFilter, &matchedMode, m_pD3D11Device);
                    if (FAILED(hr))
                    {
                        break;
                    }
                }

                hr = m_pXVP->GetAttributes(&pAttributes);
                if (FAILED(hr))
                {
                    break;
                }

                hr = pAttributes->SetUINT32(MF_ENABLE_3DVIDEO_OUTPUT, (0 != m_vp3DOutput) ? MF3DVideoOutputType_Stereo : MF3DVideoOutputType_BaseView);
                if (FAILED(hr))
                {
                    break;
                }
            }
        }

        // now create the input and output media types - these need to reflect
        // the src and destination rectangles that we have been given.
        RECT TRect = m_rcDstApp;
        RECT SRect = m_rcSrcApp;
        UpdateRectangles(&TRect, &SRect);

        const BOOL fDestRectChanged = !EqualRect(&TRect, &TRectOld);
        const BOOL fSrcRectChanged = !EqualRect(&SRect, &SRectOld);

        if (!m_pSwapChain1 || fDestRectChanged)
        {
            hr = UpdateDXGISwapChain();
            if (FAILED(hr))
            {
                break;
            }
        }

        if (fTypeChanged || fSrcRectChanged || fDestRectChanged)
        {
            // stop streaming to avoid multiple start\stop calls internally in XVP
            hr = m_pXVP->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0);
            if (FAILED(hr))
            {
                break;
            }

            if (fTypeChanged)
            {
                hr = SetXVPOutputMediaType(pCurrentType, DXGI_FORMAT_B8G8R8A8_UNORM);
                if (FAILED(hr))
                {
                    break;
                }
            }

            if (fDestRectChanged)
            {
                hr = m_pXVPControl->SetDestinationRectangle(&m_rcDstApp);
                if (FAILED(hr))
                {
                    break;
                }
            }

            if (fSrcRectChanged)
            {
                hr = m_pXVPControl->SetSourceRectangle(&SRect);
                if (FAILED(hr))
                {
                    break;
                }
            }

            hr = m_pXVP->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
            if (FAILED(hr))
            {
                break;
            }
        }

        m_bCanProcessNextSample = FALSE;

        // Get Backbuffer
        hr = m_pSwapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pDXGIBackBuffer);
        if (FAILED(hr))
        {
            break;
        }

        // create the output media sample
        hr = MFCreateSample(&pRTSample);
        if (FAILED(hr))
        {
            break;
        }

        hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), pDXGIBackBuffer, 0, FALSE, &pBuffer);
        if (FAILED(hr))
        {
            break;
        }

        hr = pRTSample->AddBuffer(pBuffer);
        if (FAILED(hr))
        {
            break;
        }

        if (m_b3DVideo && 0 != m_vp3DOutput)
        {
            SafeRelease(pBuffer);

            hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), pDXGIBackBuffer, 1, FALSE, &pBuffer);
            if (FAILED(hr))
            {
                break;
            }

            hr = pRTSample->AddBuffer(pBuffer);
            if (FAILED(hr))
            {
                break;
            }
        }

        DWORD dwStatus = 0;
        MFT_OUTPUT_DATA_BUFFER outputDataBuffer = {};
        outputDataBuffer.pSample = pRTSample;
        hr = m_pXVP->ProcessOutput(0, 1, &outputDataBuffer, &dwStatus);
        if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
        {
            //call process input on the MFT to deliver the YUV video sample
            // and the call process output to extract of newly processed frame
            hr = m_pXVP->ProcessInput(0, pVideoFrame, 0);
            if (FAILED(hr))
            {
                break;
            }

            *pbInputFrameUsed = TRUE;

            hr = m_pXVP->ProcessOutput(0, 1, &outputDataBuffer, &dwStatus);
            if (FAILED(hr))
            {
                break;
            }
        }
        else
        {
            *pbInputFrameUsed = FALSE;
        }

        if (ppVideoOutFrame != NULL)
        {
            *ppVideoOutFrame = pRTSample;
            (*ppVideoOutFrame)->AddRef();
        }
    }
    while (FALSE);

    SafeRelease(pAttributes);
    SafeRelease(pBuffer);
    SafeRelease(pRTSample);
    SafeRelease(pDXGIBackBuffer);
    SafeRelease(pVideoContext);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   ReduceToLowestTerms
//
//  Synopsis:   reduces a numerator and denominator pair to their lowest terms
//
//--------------------------------------------------------------------------

void DX11VideoRenderer::CPresenter::ReduceToLowestTerms(
    int NumeratorIn,
    int DenominatorIn,
    int* pNumeratorOut,
    int* pDenominatorOut
    )
{
    int GCD = gcd(NumeratorIn, DenominatorIn);

    *pNumeratorOut = NumeratorIn / GCD;
    *pDenominatorOut = DenominatorIn / GCD;
}

HRESULT DX11VideoRenderer::CPresenter::SetMonitor(UINT adapterID)
{
    HRESULT hr = S_OK;
    DWORD dwMatchID = 0;

    CAutoLock lock(&m_critSec);

    do
    {
        hr = m_pMonitors->MatchGUID(adapterID, &dwMatchID);
        if (FAILED(hr))
        {
            break;
        }

        if (hr == S_FALSE)
        {
            hr = E_INVALIDARG;
            break;
        }

        m_lpCurrMon = &(*m_pMonitors)[dwMatchID];
        m_ConnectionGUID = adapterID;
    }
    while (FALSE);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     SetVideoContextParameters
//
//  Synopsis:   Updates the various parameters used for VpBlt call
//
//--------------------------------------------------------------------------

void DX11VideoRenderer::CPresenter::SetVideoContextParameters(ID3D11VideoContext* pVideoContext, const RECT* pSRect, const RECT* pTRect, UINT32 unInterlaceMode)
{
    D3D11_VIDEO_FRAME_FORMAT FrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
    if ( MFVideoInterlace_FieldInterleavedUpperFirst == unInterlaceMode ||  MFVideoInterlace_FieldSingleUpper == unInterlaceMode ||  MFVideoInterlace_MixedInterlaceOrProgressive == unInterlaceMode )
    {
        FrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
    }
    else if ( MFVideoInterlace_FieldInterleavedLowerFirst == unInterlaceMode ||  MFVideoInterlace_FieldSingleLower == unInterlaceMode )
    {
        FrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST;
    }

    // input format
    pVideoContext->VideoProcessorSetStreamFrameFormat(m_pVideoProcessor, 0, FrameFormat);

    // Output rate (repeat frames)
    pVideoContext->VideoProcessorSetStreamOutputRate(m_pVideoProcessor, 0, D3D11_VIDEO_PROCESSOR_OUTPUT_RATE_NORMAL, TRUE, NULL);

    // Source rect
    pVideoContext->VideoProcessorSetStreamSourceRect(m_pVideoProcessor, 0, TRUE, pSRect);

    // Stream dest rect
    pVideoContext->VideoProcessorSetStreamDestRect(m_pVideoProcessor, 0, TRUE, pTRect);

    pVideoContext->VideoProcessorSetOutputTargetRect(m_pVideoProcessor, TRUE, &m_rcDstApp);

    // Stream color space
    D3D11_VIDEO_PROCESSOR_COLOR_SPACE colorSpace = {};
    colorSpace.YCbCr_xvYCC = 1;
    pVideoContext->VideoProcessorSetStreamColorSpace(m_pVideoProcessor, 0, &colorSpace);

    // Output color space
    pVideoContext->VideoProcessorSetOutputColorSpace(m_pVideoProcessor, &colorSpace);

    // Output background color (black)
    D3D11_VIDEO_COLOR backgroundColor = {};
    backgroundColor.RGBA.A = 1.0F;
    backgroundColor.RGBA.R = 1.0F * static_cast<float>(GetRValue(0)) / 255.0F;
    backgroundColor.RGBA.G = 1.0F * static_cast<float>(GetGValue(0)) / 255.0F;
    backgroundColor.RGBA.B = 1.0F * static_cast<float>(GetBValue(0)) / 255.0F;

    pVideoContext->VideoProcessorSetOutputBackgroundColor(m_pVideoProcessor, FALSE, &backgroundColor);
}

HRESULT DX11VideoRenderer::CPresenter::SetVideoMonitor(HWND hwndVideo)
{
    HRESULT hr = S_OK;
    CAMDDrawMonitorInfo* pMonInfo = NULL;
    HMONITOR hMon = NULL;

    if (!m_pMonitors)
    {
        return E_UNEXPECTED;
    }

    hMon = MonitorFromWindow(hwndVideo, MONITOR_DEFAULTTONULL);

    do
    {
        if (NULL != hMon)
        {
            m_pMonitors->TerminateDisplaySystem();
            m_lpCurrMon = NULL;

            hr = m_pMonitors->InitializeDisplaySystem(hwndVideo);
            if (FAILED(hr))
            {
                break;
            }

            pMonInfo = m_pMonitors->FindMonitor(hMon);
            if (NULL != pMonInfo && pMonInfo->uDevID != m_ConnectionGUID)
            {
                hr = SetMonitor(pMonInfo->uDevID);
                if (FAILED(hr))
                {
                    break;
                }
                hr = S_FALSE;
            }
        }
        else
        {
            hr = E_POINTER;
            break;
        }
    }
    while(FALSE);

    return hr;
}

//-------------------------------------------------------------------
// Name: SetXVPOutputMediaType
// Description: Tells the XVP about the size of the destination surface
// and where within the surface we should be writing.
//-------------------------------------------------------------------

HRESULT DX11VideoRenderer::CPresenter::SetXVPOutputMediaType(IMFMediaType* pType, DXGI_FORMAT vpOutputFormat)
{
    HRESULT hr = S_OK;
    IMFVideoMediaType* pMTOutput = NULL;
    MFVIDEOFORMAT mfvf = {};

    if (SUCCEEDED(hr))
    {
        hr = MFInitVideoFormat_RGB(
            &mfvf,
            m_rcDstApp.right,
            m_rcDstApp.bottom,
            MFMapDXGIFormatToDX9Format(vpOutputFormat));
    }

    if (SUCCEEDED(hr))
    {
        hr = MFCreateVideoMediaType(&mfvf, &pMTOutput);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pXVP->SetOutputType(0, pMTOutput, 0);
    }

    SafeRelease(pMTOutput);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     UpdateDXGISwapChain
//
//  Synopsis:   Creates SwapChain for HWND or DComp or Resizes buffers in case of resolution change
//
//--------------------------------------------------------------------------

_Post_satisfies_(this->m_pSwapChain1 != NULL)
HRESULT DX11VideoRenderer::CPresenter::UpdateDXGISwapChain(void)
{
    HRESULT hr = S_OK;

    // Get the DXGISwapChain1
    DXGI_SWAP_CHAIN_DESC1 scd;
    ZeroMemory(&scd, sizeof(scd));
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    scd.Scaling = DXGI_SCALING_STRETCH;
    scd.Width = m_rcDstApp.right;
    scd.Height = m_rcDstApp.bottom;
    scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.Stereo = m_bStereoEnabled;
    scd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.Flags = m_bStereoEnabled ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0; //opt in to do direct flip;
    scd.BufferCount = 4;

    do
    {
        if (m_pSwapChain1)
        {
            // Resize our back buffers for the desired format.
            hr = m_pSwapChain1->ResizeBuffers
                (
                4,
                m_rcDstApp.right,
                m_rcDstApp.bottom,
                scd.Format,
                scd.Flags
                );

            break;
        }

        if (!m_useDCompVisual)
        {
            hr = m_pDXGIFactory2->CreateSwapChainForHwnd(m_pD3D11Device, m_hwndVideo, &scd, NULL, NULL, &m_pSwapChain1);
            if (FAILED(hr))
            {
                break;
            }

            if (m_bFullScreenState)
            {
                hr = m_pSwapChain1->SetFullscreenState(TRUE, NULL);
                if (FAILED(hr))
                {
                    break;
                }
            }
            else
            {
                hr = m_pSwapChain1->SetFullscreenState(FALSE, NULL);
                if (FAILED(hr))
                {
                    break;
                }
            }
        }
        else
        {
            // Create a swap chain for composition
            hr = m_pDXGIFactory2->CreateSwapChainForComposition(m_pD3D11Device, &scd, NULL, &m_pSwapChain1);
            if (FAILED(hr))
            {
                break;
            }

            hr = m_pRootVisual->SetContent(m_pSwapChain1);
            if (FAILED(hr))
            {
                break;
            }

            hr = m_pDCompDevice->Commit();
            if (FAILED(hr))
            {
                break;
            }
        }
    }
    while (FALSE);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     UpodateRectangles
//
//  Synopsis:   Figures out the real source and destination rectangles
//              to use when drawing the video frame into the clients
//              destination location.  Takes into account pixel aspect
//              ration correction, letterboxing and source rectangles.
//
//--------------------------------------------------------------------------

void DX11VideoRenderer::CPresenter::UpdateRectangles(RECT* pDst, RECT* pSrc)
{
    // take the given src rect and reverse map it into the native video
    // image rectange.  For example, consider a video with a buffer size of
    // 720x480 and an active area of 704x480 - 8,0 with a picture aspect
    // ratio of 4:3.  The user sees the native video size as 640x480.
    //
    // If the user gave us a src rectangle of (180, 135, 540, 405)
    // then this gets reversed mapped to
    //
    // 8 + (180 * 704 / 640) = 206
    // 0 + (135 * 480 / 480) = 135
    // 8 + (540 * 704 / 640) = 602
    // 0 + (405 * 480 / 480) = 405

    RECT Src = *pSrc;

    pSrc->left = m_displayRect.left + MulDiv(pSrc->left, (m_displayRect.right - m_displayRect.left), m_uiRealDisplayWidth);
    pSrc->right = m_displayRect.left + MulDiv(pSrc->right, (m_displayRect.right - m_displayRect.left), m_uiRealDisplayWidth);

    pSrc->top = m_displayRect.top + MulDiv(pSrc->top, (m_displayRect.bottom - m_displayRect.top), m_uiRealDisplayHeight);
    pSrc->bottom = m_displayRect.top + MulDiv(pSrc->bottom, (m_displayRect.bottom - m_displayRect.top), m_uiRealDisplayHeight);

    LetterBoxDstRect(pDst, Src, m_rcDstApp);
}
