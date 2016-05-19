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

//-------------------------------------------------------------------
// InitializeD3D9
//
// Create the Direct3D device.
//-------------------------------------------------------------------

BOOL D3DHelper::InitializeD3D9(HWND hwnd)
{
    HRESULT hr;

    DestroyD3D9();

    hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D);

    if (FAILED(hr))
    {
        DBGMSG(L"Direct3DCreate9Ex failed.\n");
        return FALSE;
    }

    if (!m_bAllowHWDevice)
    {
        m_bAllowSWDevice = RegisterSoftwareRasterizer();
    }
    
    ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));

    if (m_bWindowed)
    {
        m_d3dpp.BackBufferWidth  = 0;
        m_d3dpp.BackBufferHeight = 0;
    }
    else
    {
        m_d3dpp.BackBufferWidth  = GetSystemMetrics(SM_CXSCREEN);
        m_d3dpp.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
    }

    m_d3dpp.BackBufferFormat           = VIDEO_RENDER_TARGET_FORMAT;
    m_d3dpp.BackBufferCount            = BACK_BUFFER_COUNT;
    m_d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
    m_d3dpp.hDeviceWindow              = hwnd;
    m_d3dpp.Windowed                   = m_bWindowed;
    m_d3dpp.Flags                      = D3DPRESENTFLAG_VIDEO;
    m_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    m_d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_ONE;

    if (m_bDXVA_SW)
    {
        m_d3dpp.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    }

    if (m_bAllowHWDevice)
    {
        // First try to create a hardware D3D9 device.
        hr = m_pD3D->CreateDeviceEx(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            hwnd,
            D3DCREATE_FPU_PRESERVE |
            D3DCREATE_MULTITHREADED |
            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            &m_d3dpp,
            NULL,
            &m_pDevice
            );

        if (FAILED(hr))
        {
            DBGMSG(L"CreateDevice(HAL) failed with error 0x%x.\n", hr);
        }
    }

    if ((m_pDevice == NULL) && m_bAllowSWDevice)
    {
        DBGMSG(L"CreateDevice(HAL) failed with error 0x%x.\n", hr);

        // Next try to create a software D3D9 device.
        hr = m_pD3D->CreateDeviceEx(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_SW,
            hwnd,
            D3DCREATE_FPU_PRESERVE |
            D3DCREATE_MULTITHREADED |
            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            &m_d3dpp,
            NULL,
            &m_pDevice
            );

        if (FAILED(hr))
        {
            DBGMSG(L"CreateDevice(SW) failed with error 0x%x.\n", hr);
        }
    }

    if (!m_pDevice)
    {
        return FALSE;
    }
    return TRUE;
}


//-------------------------------------------------------------------
// RegisterSoftwareRasterizer
//
// Register the software Direct3D rasterizer.
//-------------------------------------------------------------------

BOOL D3DHelper::RegisterSoftwareRasterizer()
{
    assert(m_pD3D);

    if (m_hSWRastDLL == NULL)
    {
        // Try to load the SW rasterizer. 
        m_hSWRastDLL = LoadLibrary(L"rgb9rast.dll");

        if (m_hSWRastDLL == NULL)
        {
            return FALSE;
        }

        m_pfnD3D9GetSWInfo = GetProcAddress(m_hSWRastDLL, "D3D9GetSWInfo");

        if (!m_pfnD3D9GetSWInfo)
        {
            return FALSE;
        }
    }

    HRESULT hr = S_OK;

    hr = m_pD3D->RegisterSoftwareDevice(m_pfnD3D9GetSWInfo);

    if (FAILED(hr))
    {
        DBGMSG(L"RegisterSoftwareDevice failed with error 0x%x.\n", hr);
        return FALSE;
    }
    return TRUE;
}


//-------------------------------------------------------------------
// ResetDevice
//
// Reset the Direct3D device.
//
// If bChangeWindowMode is TRUE, switch between full-screen and
// windowed mode.
//-------------------------------------------------------------------

BOOL D3DHelper::ResetDevice(BOOL bChangeWindowMode)
{
    HRESULT hr;
    
    if (bChangeWindowMode)
    {
        if (!SetWindowedMode(!m_bWindowed))
        {
            return FALSE;
        }
    }

    if (m_pDevice)
    {
        if (m_bWindowed)
        {
            m_d3dpp.BackBufferWidth  = 0;
            m_d3dpp.BackBufferHeight = 0;
        }
        else
        {
            m_d3dpp.BackBufferWidth  = GetSystemMetrics(SM_CXSCREEN);
            m_d3dpp.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
        }

        m_d3dpp.Windowed = m_bWindowed;

        // Reset will change the parameters, so use a copy instead.

        D3DPRESENT_PARAMETERS d3dpp = m_d3dpp;

        hr = m_pDevice->Reset(&d3dpp);

        if (FAILED(hr))
        {
            DBGMSG(L"Reset failed with error 0x%x.\n", hr);
        }

        if (SUCCEEDED(hr))
        {
            return TRUE;
        }
    }

    return FALSE;
}


//-------------------------------------------------------------------
// SetWindowedMode
//
// Enable or disable windowed mode.
//-------------------------------------------------------------------

BOOL D3DHelper::SetWindowedMode(BOOL bWindowMode)
{
    if (bWindowMode == m_bWindowed)
    {
        return TRUE; // no-op
    }

    // The API calls below generate WM_SIZE messages. Therefore, mark 
    // the mode change in progress, to prevent the device from being 
    // reset in the WM_SIZE handler. 

    m_bInModeChange = TRUE;

    const HWND& hwnd = m_d3dpp.hDeviceWindow;

    if (!bWindowMode)
    {
        // Fullscreen mode

        // Save the window position.

        if (!GetWindowRect(hwnd, &m_rcWindow))
        {
            DBGMSG(L"GetWindowRect failed with error %d.\n", GetLastError());
            return FALSE;
        }

        if (!SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE))
        {
            DBGMSG(L"SetWindowLongPtr failed with error %d.\n", GetLastError());
            return FALSE;
        }

        if (!SetWindowPos(hwnd,
                          HWND_NOTOPMOST,
                          0,
                          0,
                          GetSystemMetrics(SM_CXSCREEN),
                          GetSystemMetrics(SM_CYSCREEN),
                          SWP_FRAMECHANGED))
        {
            DBGMSG(L"SetWindowPos failed with error %d.\n", GetLastError());
            return FALSE;
        }
    }
    else
    {
        if (!SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE))
        {
            DBGMSG(L"SetWindowLongPtr failed with error %d.\n", GetLastError());
            return FALSE;
        }

        // Restore the window position.

        if (!SetWindowPos(hwnd,
                          HWND_NOTOPMOST,
                          m_rcWindow.left,
                          m_rcWindow.top,
                          m_rcWindow.right - m_rcWindow.left,
                          m_rcWindow.bottom - m_rcWindow.top,
                          SWP_FRAMECHANGED))
        {
            DBGMSG(L"SetWindowPos failed with error %d.\n", GetLastError());
            return FALSE;
        }
    }

    m_bInModeChange = FALSE;
    m_bWindowed = bWindowMode;

    return TRUE;
}

//-------------------------------------------------------------------
// TestCooperativeLevel
// 
// Test the device's cooperative level.
//-------------------------------------------------------------------

HRESULT D3DHelper::TestCooperativeLevel()
{
    if (m_pDevice == NULL)
    {
        return E_FAIL;
    }

    HRESULT hr = m_pDevice->TestCooperativeLevel();

    if (hr == D3DERR_DEVICENOTRESET)
    {
        if (ResetDevice(FALSE))
        {
            hr = D3D_OK;
        }
    }

    return hr;
}