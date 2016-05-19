//////////////////////////////////////////////////////////////////////
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#include "DXVAHD_Sample.h"
#include "application.h"
#include "resource.h"
#include <new>

HWND InitializeWindow(LPVOID lpParam);

void DisplayError(HWND hwnd, WCHAR *pszMessage)
{
    MessageBox(hwnd, pszMessage, L"Error", MB_OK | MB_ICONERROR);
}

//-------------------------------------------------------------------
// Initialize
//
// Called once at the start of the application.
//-------------------------------------------------------------------

BOOL Application::Initialize()
{
    if (!ParseCommandLine())
    {
        return FALSE;
    }

    if (!m_DWM.Initialize())
    {
        DisplayError(NULL, L"Unable to initialize DWM.");
        return FALSE;
    }

    m_Hwnd = InitializeWindow(this);

    if (!m_Hwnd)
    {
        DisplayError(NULL, L"Unable to create application window.");
        return FALSE;
    }

    if (!m_D3D.InitializeD3D9(m_Hwnd))
    {
        DisplayError(NULL, L"Unable to create Direct3D device.");
        return FALSE;
    }

    if (!InitializeDXVAHD())
    {
        DisplayError(NULL, L"Unable to create DXVA-HD device.");
        return FALSE;
    }

    if (!m_timer.InitializeTimer(VIDEO_MSPF))
    {
        DisplayError(NULL, L"Unable to initialize timer.");
        return FALSE;
    }

    return TRUE;
}



//-------------------------------------------------------------------
// ProcessVideoFrame
//
// Process and render the next video frame.
//-------------------------------------------------------------------

BOOL Application::ProcessVideoFrame()
{
    HRESULT hr = S_OK;

    if (!m_D3D.m_pD3D || !m_pDXVAVP)
    {
        return FALSE;
    }

    RECT client;
    GetClientRect(m_Hwnd, &client);

    if (IsRectEmpty(&client))
    {
        return TRUE;
    }

    // Check the current status of D3D9 device.  
    hr = m_D3D.TestCooperativeLevel();

    switch (hr)
    {
    case D3D_OK :
        break;

    case D3DERR_DEVICELOST :
        return TRUE;

    case D3DERR_DEVICENOTRESET :
        return FALSE;
        break;

    default :
        return FALSE;
    }

    IDirect3DSurface9 *pRT = NULL;  // Render target

    DWORD frame = m_timer.GetFrameNumber();

    RECT dest = { 0 };
    RECT ssdest = { 0 };

    DXVAHD_STREAM_DATA stream_data[2];
    ZeroMemory(&stream_data, sizeof(stream_data));

    // Get the render-target surface.

    hr = m_D3D.m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pRT);

    if (FAILED(hr)) { goto done; }

    // Initialize the stream data structures for the primary video stream 
    // and the substream.

    stream_data[0].Enable = TRUE;
    stream_data[0].OutputIndex = 0;
    stream_data[0].InputFrameOrField = frame;
    stream_data[0].pInputSurface = m_pMainStream;

    stream_data[1].Enable = TRUE;
    stream_data[1].OutputIndex = 0;
    stream_data[1].InputFrameOrField = frame;
    stream_data[1].pInputSurface = m_ppSubStream[0];
 
    AdjustTargetRect(0, 0);

    // Apply the destination rectangle for the main video stream.

    // Scale the destination rectangle to the window client area.
    dest = ScaleRectangle(m_rcMainVideoDestRect, VIDEO_MAIN_RECT, client);

    hr = DXVAHD_SetDestinationRect(
        m_pDXVAVP,
        0,
        TRUE,
        dest
        );

    if (FAILED(hr)) { goto done; }


    // Calculate the substream destination rectangle from the frame number.

    CalculateSubstreamRect(frame, &ssdest);

    // Scale to the window client area.
    ssdest = ScaleRectangle(ssdest, VIDEO_MAIN_RECT, client); 

    hr = DXVAHD_SetDestinationRect(
        m_pDXVAVP,
        1,
        TRUE,
        ssdest
        );

    if (FAILED(hr)) { goto done; }

    // Color-fill the render target if the target rectangle is less
    // than the entire render target.

    if (m_TargetWidthPercent < 100 || m_TargetHeightPercent < 100)
    {
        hr = m_D3D.m_pDevice->ColorFill(pRT, NULL, D3DCOLOR_XRGB(0, 0, 0));

        if (FAILED(hr)) { goto done; }
    }

    // Perform the blit.
    hr = m_pDXVAVP->VideoProcessBltHD(
        pRT,
        frame,
        2,
        stream_data
        );

    if (FAILED(hr)) { goto done; }

    // Enable DWM queuing.
    hr = m_DWM.EnableDwmQueuing(m_Hwnd);

    if (FAILED(hr)) { goto done; }


    // Present the frame.
    hr = m_D3D.m_pDevice->Present(NULL, NULL, NULL, NULL);

done:
    SafeRelease(&pRT);
    return SUCCEEDED(hr);
}



//-------------------------------------------------------------------
// ResetDevice
//
// Resets the Direct3D device.
//-------------------------------------------------------------------

BOOL Application::ResetDevice(BOOL bChangeWindowMode)
{
    // Destroy the DXVA-HD device, because it may be holding D3D9 resources.

    DestroyDXVAHD();

    // Reset the Direct3D device and re-create the DXVA-HD device.

    if (m_D3D.ResetDevice(bChangeWindowMode) && InitializeDXVAHD())
    {
        return TRUE;
    }

    // Either resetting the Direct3D device failed, or initializing
    // the DXVA-HD device failed.

    // Try to recover by recreating the devices from the scratch.

    DestroyDXVAHD();

    if (m_D3D.InitializeD3D9(m_Hwnd) && InitializeDXVAHD())
    {
        return TRUE;
    }


    // Still failed. 
    
    // If we failed to initialize in fullscreen mode, try falling
    // back to windowed mode.

    DestroyDXVAHD();

    if (!m_D3D.m_bWindowed)
    {
        if (m_D3D.SetWindowedMode(TRUE) && m_D3D.InitializeD3D9(m_Hwnd) && InitializeDXVAHD())
        {
            return TRUE;
        }
    }

    // Still failed. Give up.

    return FALSE;
};


//-------------------------------------------------------------------
// InitializeDXVAHD
//
// Create the DXVA-HD video processor device, and initialize
// DXVA-HD resources.
//-------------------------------------------------------------------

BOOL Application::InitializeDXVAHD()
{
    HRESULT hr = S_OK;
    DWORD index = 0;

    D3DFORMAT *pFormats = NULL;
    DXVAHD_VPCAPS *pVPCaps = NULL;

    DXVAHD_CONTENT_DESC desc;

    desc.InputFrameFormat = DXVAHD_FRAME_FORMAT_PROGRESSIVE;
    desc.InputFrameRate.Numerator = VIDEO_FPS;
    desc.InputFrameRate.Denominator = 1;
    desc.InputWidth = VIDEO_MAIN_WIDTH;
    desc.InputHeight = VIDEO_MAIN_HEIGHT;
    desc.OutputFrameRate.Numerator = VIDEO_FPS;
    desc.OutputFrameRate.Denominator = 1;
    desc.OutputWidth = VIDEO_MAIN_WIDTH;
    desc.OutputHeight = VIDEO_MAIN_HEIGHT;

    PDXVAHDSW_Plugin pSWPlugin = NULL;

    if (m_D3D.m_bDXVA_SW)
    {
        // Load the software DXVA-HD device.

        HMODULE hSWPlugin = LoadLibrary(L"dxvahdsw.dll");

        if (hSWPlugin == NULL)
        {
            DBGMSG(L"Could not load dxvahdsw.dll\n");
            return FALSE;
        }

        pSWPlugin = (PDXVAHDSW_Plugin)GetProcAddress(hSWPlugin, "DXVAHDSW_Plugin");

        if (pSWPlugin == NULL)
        {
            DBGMSG(L"Could not get DXVAHDSW_Plugin proc address.\n");
            return FALSE;
        }
    }

    // Create the DXVA-HD device.

    hr = DXVAHD_CreateDevice(
        m_D3D.m_pDevice,
        &desc,
        m_usage,
        pSWPlugin,
        &m_pDXVAHD
        );

    if (FAILED(hr)) { goto done; }

    // Get the DXVA-HD device caps.

    DXVAHD_VPDEVCAPS caps;
    ZeroMemory(&caps, sizeof(caps));

    hr = m_pDXVAHD->GetVideoProcessorDeviceCaps(&caps);

    if (FAILED(hr)) { goto done; }

    if (caps.MaxInputStreams < 1 + SUB_STREAM_COUNT)
    {
        DBGMSG(L"Device only supports %d input streams.\n", caps.MaxInputStreams);
        hr = E_FAIL;
        goto done;
    }

    // Check the output format.

    pFormats = new (std::nothrow) D3DFORMAT[ caps.OutputFormatCount ];
    if (pFormats == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = m_pDXVAHD->GetVideoProcessorOutputFormats(caps.OutputFormatCount, pFormats);

    if (FAILED(hr)) { goto done; }

    for (index = 0; index < caps.OutputFormatCount; index++)
    {
        if (pFormats[index] == VIDEO_RENDER_TARGET_FORMAT)
        {
            break;
        }
    }
    if (index == caps.OutputFormatCount)
    {
        hr = E_FAIL;
        goto done;
    }

    delete [] pFormats;
    pFormats = NULL;

    // Check the input formats.

    pFormats = new (std::nothrow) D3DFORMAT[ caps.InputFormatCount ];
    if (pFormats == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = m_pDXVAHD->GetVideoProcessorInputFormats(caps.InputFormatCount, pFormats);

    if (FAILED(hr)) { goto done; }

    D3DFORMAT inputFormats[] = { VIDEO_MAIN_FORMAT, VIDEO_SUB_FORMAT };

    for (DWORD j = 0; j < 2; j++)
    {
        for (index = 0; index < caps.InputFormatCount; index++)
        {
            if (pFormats[index] == inputFormats[j])
            {
                break;
            }
        }
        if (index == caps.InputFormatCount)
        {
            hr = E_FAIL;
            goto done;
        }
    }

    delete [] pFormats;
    pFormats = NULL;

    // Create the VP device.

    pVPCaps = new (std::nothrow) DXVAHD_VPCAPS[ caps.VideoProcessorCount ];
    if (pVPCaps == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = m_pDXVAHD->GetVideoProcessorCaps(caps.VideoProcessorCount, pVPCaps);

    if (FAILED(hr)) { goto done; }

    hr = m_pDXVAHD->CreateVideoProcessor(&pVPCaps[0].VPGuid, &m_pDXVAVP);

    if (FAILED(hr)) { goto done; }


    // Create the video surface for the primary video stream.
    hr = m_pDXVAHD->CreateVideoSurface(
        VIDEO_MAIN_WIDTH,
        VIDEO_MAIN_HEIGHT,
        VIDEO_MAIN_FORMAT,
        caps.InputPool,
        0,
        DXVAHD_SURFACE_TYPE_VIDEO_INPUT,
        1,
        &m_pMainStream,
        NULL
        );

    if (FAILED(hr)) { goto done; }


    // Set the initial stream states for the primary stream.
    hr = DXVAHD_SetStreamFormat(m_pDXVAVP, 0, VIDEO_MAIN_FORMAT);

    if (FAILED(hr)) { goto done; }

    hr = DXVAHD_SetFrameFormat(m_pDXVAVP, 0, DXVAHD_FRAME_FORMAT_PROGRESSIVE);

    if (FAILED(hr)) { goto done; }

    // Create substream surfaces.

    hr = m_pDXVAHD->CreateVideoSurface(
        VIDEO_SUB_SURF_WIDTH,
        VIDEO_SUB_SURF_HEIGHT,
        VIDEO_SUB_FORMAT,
        caps.InputPool,
        0,
        DXVAHD_SURFACE_TYPE_VIDEO_INPUT,
        SUB_STREAM_COUNT,
        m_ppSubStream,
        NULL
        );

    if (FAILED(hr)) { goto done; }


    // Set the initial stream states for the substream.

    // Video format
    hr = DXVAHD_SetStreamFormat(m_pDXVAVP, 1, VIDEO_SUB_FORMAT);

    if (FAILED(hr)) { goto done; }

    // Frame format (progressive)
    hr = DXVAHD_SetFrameFormat(m_pDXVAVP, 1, DXVAHD_FRAME_FORMAT_PROGRESSIVE);

    if (FAILED(hr)) { goto done; }

    // Luma key
    hr = DXVAHD_SetLumaKey(m_pDXVAVP, 1, TRUE, 0.9f, 1.0f);

    if (FAILED(hr)) { goto done; }

    // Draw the video frame for the primary video stream. 
    // This frame does not change.

    hr = DrawColorBars(m_pMainStream, VIDEO_MAIN_WIDTH, VIDEO_MAIN_HEIGHT);

    if (FAILED(hr)) { goto done; }

    // Load the bitmap onto the substream surface.

    hr = LoadBitmapResourceToAYUVSurface(
        m_ppSubStream[0],
        VIDEO_SUB_SURF_WIDTH,
        VIDEO_SUB_SURF_HEIGHT,
        IDB_BITMAP1,
        m_PixelAlphaValue
        );

    if (FAILED(hr)) { goto done; }

    // Get the image filtering capabilities.

    for (DWORD i = 0; i < NUM_FILTERS; i++)
    {
        if (caps.FilterCaps & (1 << i))
        {
            m_Filters[i].bSupported = TRUE;

            m_pDXVAHD->GetVideoProcessorFilterRange(PROCAMP_FILTERS[i], &m_Filters[i].Range);

            m_Filters[i].CurrentValue = m_Filters[i].Range.Default;

            INT range = m_Filters[i].Range.Maximum - m_Filters[i].Range.Minimum;

            m_Filters[i].Step = range > 32 ? range / 32 : 1;
        }
        else
        {
            m_Filters[i].bSupported = FALSE;
        }
    }

    // Apply the current settings.

    hr = ApplySettings();

    if (FAILED(hr)) { goto done; }


    hr = UpdateVideoSubRect();

done:
    delete [] pFormats;
    delete [] pVPCaps;
    return (SUCCEEDED(hr));
}

//-------------------------------------------------------------------
// DestroyDXVAHD
//
// Releases DXVA-HD resources.
//-------------------------------------------------------------------

void Application::DestroyDXVAHD()
{
    SafeRelease(&m_pMainStream);

    for (DWORD i = 0; i < SUB_STREAM_COUNT; i++)
    {
        SafeRelease(&m_ppSubStream[i]);
    }

    SafeRelease(&m_pDXVAVP);
    SafeRelease(&m_pDXVAHD);
}


//-------------------------------------------------------------------
// MessageLoop
//
// Runs the message loop for the application window.
//
// This method also waits on the timer the controls the frame
// rate for the video.
//-------------------------------------------------------------------

INT Application::MessageLoop()
{
    MSG msg = {0};

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (PreTranslateMessage(msg))
            {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);

            continue;
        }

        // Wait until the timer expires or any message is posted.

        if (WAIT_OBJECT_0 == MsgWaitForMultipleObjects(
                1,
                &m_timer.Handle(),
                FALSE,
                INFINITE,
                QS_ALLINPUT
                ))
        {
            // Draw the next video frame.
            if (!ProcessVideoFrame())
            {
                DestroyWindow(m_Hwnd);
            }
        }
    }

    return INT(msg.wParam);
}


//-------------------------------------------------------------------
// HandleMessage
//
// Handles window messages.
//-------------------------------------------------------------------

LRESULT Application::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_KEYDOWN:
        OnKeyDown((UINT)wParam, TRUE, (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam));
        return 0L;

    case WM_SIZE:
        OnSize();
        return 0L;
    }        
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


//-------------------------------------------------------------------
// OnSize
//
// Handles WM_SIZE messages.
//-------------------------------------------------------------------

void Application::OnSize()
{
    if (m_D3D.m_pDevice == NULL)
    {
        return;
    }

    RECT rect;
    GetClientRect(m_Hwnd, &rect);

    if (IsRectEmpty(&rect))
    {
        return;
    }

    // Do not reset the device while the mode change is in progress.
    if (m_D3D.m_bInModeChange)
    {
        return;
    }

    if (!m_D3D.ResetDevice(FALSE))
    {
        DestroyWindow(m_Hwnd);
        return;
    }

    InvalidateRect(m_Hwnd, NULL, FALSE);
}


//-------------------------------------------------------------------
// OnKeyDown
//
// Handles WM_KEYDOWN messages.
//-------------------------------------------------------------------

void  Application::OnKeyDown(UINT vk, BOOL /* fDown */, int /* cRepeat */, UINT /* flags */)
{
    if (vk == VK_ESCAPE)
    {
        DestroyWindow(m_Hwnd);
        return;
    }
    
    switch (vk)
    {
    case VK_F1:
    case VK_F2:
    case VK_F3:
    case VK_F4:
    case VK_F5:
    case VK_F6:
    case VK_F7:
    case VK_F8:
    case VK_F9:
        m_mode = (ApplicationMode)vk;
        UpdateVideoSubRect();
        break;

    case VK_UP: 
        AdjustSetting(0, 1);
        break;

    case VK_DOWN:
        AdjustSetting(0, -1);
        break;

    case VK_LEFT:
        AdjustSetting(-1, 0);
        break;

    case VK_RIGHT:
        AdjustSetting(1, 0);
        break;

    case VK_HOME :
        ResetSettings();
        break;
    }
}

//-------------------------------------------------------------------
// PreTranslateMessage
//
// Handles window messages before they are sent to TranslateMessage. 
//-------------------------------------------------------------------

BOOL Application::PreTranslateMessage(const MSG& msg)
{
    // Only interested in Alt + Enter.

    if (msg.message != WM_SYSKEYDOWN || msg.wParam != VK_RETURN)
    {
        return FALSE;
    }

    if (!m_D3D.m_pDevice)
    {
        return TRUE;
    }

    RECT rect;
    GetClientRect(msg.hwnd, &rect);

    if (IsRectEmpty(&rect))
    {
        return TRUE;
    }

    // Toggle the window mode (windowed/full screen)
    if (ResetDevice(TRUE))
    {
        return TRUE;
    }

    DestroyWindow(msg.hwnd);
    return TRUE;
}


//-------------------------------------------------------------------
// UpdateVideoSubRect
//
// The application displays a sub-rectangle from the substream
// surface. The substream rectangle is calculated from the current
// mode. (The user can switch modes using the function keys.)
//
// As a result, each mode displays a different portion of the 
// substream bitmap.
//-------------------------------------------------------------------

HRESULT Application::UpdateVideoSubRect()
{
    LONG dx = 0, dy = 0;

    switch (m_mode)
    {
        case Mode1: 
            break;

        case Mode2:
            dx = VIDEO_SUB_WIDTH;
            break;

        case Mode3:
            dx = VIDEO_SUB_WIDTH * 2;
            break;

        case Mode4:
            dy = VIDEO_SUB_HEIGHT;
            break;

        case Mode5:
            dy = VIDEO_SUB_HEIGHT;
            dx = VIDEO_SUB_WIDTH;
            break;

        case Mode6:
            dy = VIDEO_SUB_HEIGHT;
            dx = VIDEO_SUB_WIDTH * 2;
            break;

        case Mode7:
            dy = VIDEO_SUB_HEIGHT * 2;
            break;

        case Mode8:
            dy = VIDEO_SUB_HEIGHT * 2;
            dx = VIDEO_SUB_WIDTH;
            break;

        case Mode9:
            dy = VIDEO_SUB_HEIGHT * 2;
            dx = VIDEO_SUB_WIDTH * 2;
            break;
    }

    SetRect(&m_rcVideoSubRect, 0, 0, VIDEO_SUB_HEIGHT, VIDEO_SUB_WIDTH);
    OffsetRect(&m_rcVideoSubRect, dx, dy);

    // Apply the rectangle to the substream.

    HRESULT hr = S_OK;

    if (m_pDXVAVP)
    {
        hr = DXVAHD_SetSourceRect(
            m_pDXVAVP,
            1,
            TRUE,
            m_rcVideoSubRect
            );
    }

    return hr;

}


HRESULT Application::ApplySettings()
{
    HRESULT hr = S_OK;

    // Substream pixel alpha
    if (m_ppSubStream[0])
    {
        hr = SetAYUVSurfacePixelAlpha(
            m_ppSubStream[0],
            VIDEO_SUB_SURF_WIDTH,
            VIDEO_SUB_SURF_HEIGHT,
            m_PixelAlphaValue
            );

        if (FAILED(hr)) { goto done; }
    }

    if (m_pDXVAVP)
    {
        // Planar alpha

        hr = DXVAHD_SetPlanarAlpha(m_pDXVAVP, 0, TRUE, float(m_PlanarAlphaValue) / 0xFF);

        if (FAILED(hr)) { goto done; }

        // Main video source rectangle.

        hr = DXVAHD_SetSourceRect(
            m_pDXVAVP,
            0,
            TRUE,
            m_rcMainVideoSourceRect
            );

        if (FAILED(hr)) { goto done; }

        // Main video destination rectangle.

        hr = DXVAHD_SetDestinationRect(
            m_pDXVAVP,
            0,
            TRUE,
            m_rcMainVideoDestRect
            );

        if (FAILED(hr)) { goto done; }

        // Target rectangle.
        hr = AdjustTargetRect(0, 0);

        if (FAILED(hr)) { goto done; }

        // Extended color info.
        hr = AdjustExtendedColor(0);

        if (FAILED(hr)) { goto done; }

        // Image filters

        for (DWORD i = 0; i < NUM_FILTERS; i++)
        {
            hr = AdjustFilter((DXVAHD_FILTER)i, 0);

            if (FAILED(hr)) { goto done; }
        }

        // Background color.
        hr = AdjustBackgroundColor(0);

    }

done:
    return hr;

}


//-------------------------------------------------------------------
// ResetSettings
//
// Restores all user-controlled settings to their original state.
//-------------------------------------------------------------------

HRESULT Application::ResetSettings()
{
    HRESULT hr = S_OK;

    m_PixelAlphaValue = DEFAULT_PIXEL_ALPHA_VALUE;
    m_PlanarAlphaValue = DEFAULT_PLANAR_ALPHA_VALUE;

    m_rcMainVideoSourceRect = VIDEO_MAIN_RECT;
    m_rcMainVideoDestRect = VIDEO_MAIN_DEST_RECT;
    m_rcVideoSubRect = VIDEO_SUB_RECT;

    m_iExtendedColor = 0;
    m_iBackgroundColor = 0;

    m_TargetWidthPercent = 100;
    m_TargetHeightPercent = 100;


    for (DWORD i = 0; i < NUM_FILTERS; i++)
    {
        m_Filters[ i ].CurrentValue = m_Filters[i].Range.Default;
    }

    // Substream pixel alpha
    if (m_ppSubStream[0])
    {
        hr = SetAYUVSurfacePixelAlpha(
            m_ppSubStream[0],
            VIDEO_SUB_SURF_WIDTH,
            VIDEO_SUB_SURF_HEIGHT,
            m_PixelAlphaValue
            );

        if (FAILED(hr)) { goto done; }
    }

    hr = ApplySettings();

    if (FAILED(hr)) { goto done; }

    if (m_pDXVAVP)
    {
        // Disable target rectangles

        RECT rc = { 0 };

        hr = DXVAHD_SetTargetRect(m_pDXVAVP, FALSE, rc);
    }

done:
    return hr;
}

//-------------------------------------------------------------------
// AdjustSetting
//
// Adjusts a user-controlled setting, depending on the current mode.
// For details, see the code comment at the start of winmain.cpp
//-------------------------------------------------------------------

HRESULT Application::AdjustSetting(int dx, int dy)
{
    HRESULT hr = S_OK;

    if (m_pDXVAVP == NULL)
    {
        return S_OK;
    }

    switch (m_mode)
    {
        case Mode1: 
            // Increment and decrement alpha values.    
            hr = AdjustAlphaSetting(dx, dy);
            break;

        case Mode2:
            // Scale the main video source rectangle.
            InflateRectBounded(
                &m_rcMainVideoSourceRect, 
                -dx * 8, -dy * 8, 
                VIDEO_MAIN_RECT
                );

            hr = DXVAHD_SetSourceRect(
                m_pDXVAVP,
                0,
                TRUE,
                m_rcMainVideoSourceRect
                );
            break;

        case Mode3:
            // Move the main video source rectangle.
            MoveRectBounded(
                &m_rcMainVideoSourceRect, 
                dx * 8, -dy * 8, 
                VIDEO_MAIN_RECT
                );

            hr = DXVAHD_SetSourceRect(
                m_pDXVAVP,
                0,
                TRUE,
                m_rcMainVideoSourceRect
                );
            break;

        case Mode4:
            // Scale the main video destination rectangle
            InflateRectBounded(
                &m_rcMainVideoDestRect, 
                dx * 8, dy * 8, 
                VIDEO_MAIN_RECT
                );

            hr = DXVAHD_SetDestinationRect(
                m_pDXVAVP,
                0,
                TRUE,
                m_rcMainVideoDestRect
                );
            break;

        case Mode5:
            // Move the main video destination rectangle
            MoveRectBounded(
                &m_rcMainVideoDestRect, 
                dx * 8, -dy * 8, 
                VIDEO_MAIN_RECT
                );

            hr = DXVAHD_SetDestinationRect(
                m_pDXVAVP,
                0,
                TRUE,
                m_rcMainVideoDestRect
                );            
            break;

        case Mode6:
            if (dy != 0)
            {
                hr = AdjustExtendedColor(dy);
            }
            if (dx != 0)
            {
                hr = AdjustBackgroundColor(dx);
            }
            break;

        case Mode7:
            if (dy != 0)
            {
                hr = AdjustFilter(DXVAHD_FILTER_BRIGHTNESS, dy);
            }
            if (dx != 0)
            {
                hr = AdjustFilter(DXVAHD_FILTER_CONTRAST, dx);
            }
            break;

        case Mode8:
            if (dy != 0)
            {
                hr = AdjustFilter(DXVAHD_FILTER_HUE, dy);
            }
            if (dx != 0)
            {
                hr = AdjustFilter(DXVAHD_FILTER_SATURATION, dx);
            }
            break;

        case Mode9:
            hr = AdjustTargetRect(dx, dy);
            break;

    }

    return hr;
}



//-------------------------------------------------------------------
// AdjustAlphaSetting
//
// Adjusts the planar alpha for the primary video stream, or the
// per-pixel alpha for the substream.
//-------------------------------------------------------------------

HRESULT Application::AdjustAlphaSetting(int dx, int dy)
{
    HRESULT hr = S_OK;

    if (dy != 0)
    {
        // Adjust planar alpha
        if (dy > 0)
        {
            m_PlanarAlphaValue = min(m_PlanarAlphaValue + 8, 0xFF);
        }
        else
        {
            m_PlanarAlphaValue = m_PlanarAlphaValue < 8 ? 0 : m_PlanarAlphaValue - 8;
        }
        hr = DXVAHD_SetPlanarAlpha(m_pDXVAVP, 0, TRUE, float(m_PlanarAlphaValue) / 0xFF);

        if (FAILED(hr)) { goto done; }

    }

    if (dx != 0)
    {
        // Adjust per-pixel alpha. This is done by writing new values to 
        // the video surface.

        if (dx > 0)
        {
            m_PixelAlphaValue = min(m_PixelAlphaValue + 8, 0xFF);
        }
        else 
        {
            m_PixelAlphaValue = m_PixelAlphaValue < 8 ? 0 : m_PixelAlphaValue - 8;      
        }
        
        hr = SetAYUVSurfacePixelAlpha(
            m_ppSubStream[0],
            VIDEO_SUB_SURF_WIDTH,
            VIDEO_SUB_SURF_HEIGHT,
            m_PixelAlphaValue
            );
    }
done:
    return hr;
}


//-------------------------------------------------------------------
// AdjustExtendedColor
//
// Changes the extended color information.
//-------------------------------------------------------------------

HRESULT Application::AdjustExtendedColor(int dy)
{
    // Cycle through extended color settings

    HRESULT hr = S_OK;

    if (dy > 0)
    {
        if (++m_iExtendedColor > NUM_EX_COLORS - 1)
        {
            m_iExtendedColor = 0;
        }
    }
    else if (dy < 0)
    {
        if (--m_iExtendedColor < 0)
        {
            m_iExtendedColor = NUM_EX_COLORS - 1;
        }
    }
    
    // Output color space.
    hr = DXVAHD_SetOutputColorSpace(
        m_pDXVAVP,
        TRUE,       // Playback
        EX_COLOR_INFO[m_iExtendedColor].bRgbRange16_235,
        EX_COLOR_INFO[m_iExtendedColor].bBT709,
        0
        );

    if (FAILED(hr)) { goto done; }

    // Input color space
    for (DWORD i = 0; i < 1 + SUB_STREAM_COUNT; i++)
    {
        hr = DXVAHD_SetInputColorSpace(
            m_pDXVAVP,
            i,
            TRUE,       // Playback
            EX_COLOR_INFO[m_iExtendedColor].bRgbRange16_235,
            EX_COLOR_INFO[m_iExtendedColor].bBT709,
            0
            );

        if (FAILED(hr)) 
        { 
            break; 
        }
    }

done:
    return hr;
}


//-------------------------------------------------------------------
// AdjustBackgroundColor
//
// Changes the background color.
//-------------------------------------------------------------------

HRESULT Application::AdjustBackgroundColor(int dx)
{
    // Cycle through the background colors

    HRESULT hr = S_OK;

    if (dx > 0)
    {
        if (++m_iBackgroundColor > NUM_BACKGROUND_COLORS - 1)
        {
            m_iBackgroundColor = 0;
        }
    }
    else if (dx < 0)
    {
        if (--m_iBackgroundColor < 0)
        {
            m_iBackgroundColor = NUM_BACKGROUND_COLORS - 1;
        }
    }

    DXVAHD_COLOR clr;

    clr.RGB = BACKGROUND_COLORS[ m_iBackgroundColor ];
    
    hr = DXVAHD_SetBackgroundColor(
        m_pDXVAVP,
        FALSE,       // YCbCr?
        clr
        );

    return hr;
}


//-------------------------------------------------------------------
// AdjustFilter
//
// Changes one of the ProcAmp filter values.
//-------------------------------------------------------------------

HRESULT Application::AdjustFilter(DXVAHD_FILTER filter, int dy)
{
    HRESULT hr = S_OK;

    if (filter > NUM_FILTERS)
    {
        return E_UNEXPECTED;
    }

    if (!m_Filters[ filter ].bSupported)
    {
        return S_OK; // Unsupported filter. Ignore.
    }

    const INT step = m_Filters[ filter ].Step;
    const INT minimum = m_Filters[ filter ].Range.Minimum;
    const INT maximum = m_Filters[ filter ].Range.Maximum;

    INT val = m_Filters[ filter ].CurrentValue + dy * step;

    if (val >= minimum && val <= maximum)
    {
        hr = DXVAHD_SetFilterValue(
            m_pDXVAVP,
            0, 
            filter,
            TRUE,
            val
        );

        if (FAILED(hr))
        {
            // Try the default.

            val = m_Filters[ filter ].Range.Default;

            hr = DXVAHD_SetFilterValue(
                m_pDXVAVP,
                0, 
                filter,
                TRUE,
                val
                );

        }

        if (SUCCEEDED(hr))
        {
            m_Filters[ filter ].CurrentValue = val;
        }
    
    }

    return hr;
}


//-------------------------------------------------------------------
// AdjustTargetRect
//
// Changes the target rectangle. 
//-------------------------------------------------------------------

HRESULT Application::AdjustTargetRect(int dx, int dy)
{
    HRESULT hr = S_OK;

    if (dy > 0)
    {
        m_TargetHeightPercent = min(100, m_TargetHeightPercent + 4);
    }
    else if (dy < 0)
    {
        m_TargetHeightPercent = max(0, m_TargetHeightPercent - 4);
    }

    if (dx > 0)
    {
        m_TargetWidthPercent = min(100, m_TargetWidthPercent + 4);
    }
    else if (dx < 0)
    {
        m_TargetWidthPercent = max(0, m_TargetWidthPercent - 4);
    }

    RECT client;
    RECT target;

    GetClientRect(m_Hwnd, &client);

    target.left   = client.left   + (client.right  - client.left) / 2 * (100 - m_TargetWidthPercent)  / 100;
    target.right  = client.right  - (client.right  - client.left) / 2 * (100 - m_TargetWidthPercent)  / 100;
    target.top    = client.top    + (client.bottom - client.top)  / 2 * (100 - m_TargetHeightPercent) / 100;
    target.bottom = client.bottom - (client.bottom - client.top)  / 2 * (100 - m_TargetHeightPercent) / 100;

    hr = DXVAHD_SetTargetRect(m_pDXVAVP, TRUE, target);

    return hr;
}


//-------------------------------------------------------------------
// ParseCommandLine
//
// Reads the command-line arguments.
//-------------------------------------------------------------------

BOOL Application::ParseCommandLine()
{
    LPWSTR *szArglist = NULL;
    int nArgs = 0;

    BOOL bValid = TRUE;

    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (szArglist == NULL)
    {
        return FALSE;
    }

    for( int i = 1; i < nArgs; i++)
    {
        LPWSTR szArg = szArglist[i];

        if (_wcsicmp(szArg, L"-hh") == 0)
        {
            m_D3D.m_bDXVA_SW = FALSE;
            m_D3D.m_bAllowSWDevice = FALSE; // Force HW Direct3D device
        }
        else if (_wcsicmp(szArg, L"-hs") == 0)
        {
            m_D3D.m_bDXVA_SW = TRUE;
            m_D3D.m_bAllowSWDevice = FALSE; // Force HW Direct3D device
        }
        else if (_wcsicmp(szArg, L"-ss") == 0)
        {
            m_D3D.m_bDXVA_SW = TRUE;
            m_D3D.m_bAllowHWDevice = FALSE; // Force SW Direct3D device
        }
        else if (_wcsicmp(szArg, L"-u") == 0)
        {
            // Device usage

            if (i >= nArgs - 1)
            {
                bValid = FALSE;
                break;
            }

            szArg = szArglist[++i];

            if (_wcsicmp(szArg, L"0") == 0)
            {
                m_usage = DXVAHD_DEVICE_USAGE_PLAYBACK_NORMAL;
            }
            else if (_wcsicmp(szArg, L"1") == 0)
            {
                m_usage = DXVAHD_DEVICE_USAGE_OPTIMAL_SPEED;
            }
            else if (_wcsicmp(szArg, L"2") == 0)
            {
                m_usage = DXVAHD_DEVICE_USAGE_OPTIMAL_QUALITY;
            }
            else
            {
                bValid = FALSE;
                break;
            }
        }
        else
        {
            bValid = FALSE;
            break;
        }

    }

    if (!bValid)
    {
        MessageBox(
            NULL, 
            L"Usage:\n"
            L"-hh : Hardware Direct3D device; DXVA-HD\n"
            L"-hs : Hardware Direct3D device; software DXVA-HD\n"
            L"-ss : Software Direct3D device; software DXVA-HD\n"
            L"-u [0 | 1 | 2]: DXVA-HD device usage",
            L"Invalid Command Line",
            MB_OK | MB_ICONERROR
            );
    }

    // Free memory allocated for CommandLineToArgvW arguments.
    LocalFree(szArglist);

   return bValid;
}


//-------------------------------------------------------------------
// CalculateSubstreamRect
//
// Calculates the destination rectangle for the substream, based
// on the current frame number.
//-------------------------------------------------------------------

void Application::CalculateSubstreamRect(INT frame, RECT *prect)
{
    INT x, y, wx, wy;

    x = frame * VIDEO_SUB_VX;
    wx = VIDEO_MAIN_WIDTH - VIDEO_SUB_WIDTH;
    x = (x / wx) & 0x1 ? wx - (x % wx) : x % wx;

    y = frame * VIDEO_SUB_VY;
    wy = VIDEO_MAIN_HEIGHT - VIDEO_SUB_HEIGHT;
    y = (y / wy) & 0x1 ? wy - (y % wy) : y % wy;

    SetRect(prect, x, y, x + VIDEO_SUB_WIDTH, y + VIDEO_SUB_HEIGHT);
}



