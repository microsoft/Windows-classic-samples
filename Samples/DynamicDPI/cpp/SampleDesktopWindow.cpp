//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "SampleDesktopWindow.h"
#include <ShellScalingApi.h>

CSampleDesktopWindow::CSampleDesktopWindow()
{
    // Set member variables to zero or NULL defaults.
    m_visible = FALSE; 
    m_occlusion = DWORD(0.0);
}

CSampleDesktopWindow::~CSampleDesktopWindow()
{
    if (m_deviceResources)
    {
        m_deviceResources->SetWindow(nullptr, 96.0F);
        m_deviceResources.reset();
    }
}

// <summary>
// These functions are used to initialize and configure the main
// application window and message pumps.
// </summary>
HRESULT
CSampleDesktopWindow::Initialize(
    _In_    RECT bounds,
    _In_    std::wstring title
    )
{
    // Create device resources required to render content.
    m_deviceResources = std::make_shared<DeviceResources>();
    if (!m_deviceResources)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    CreateDeviceIndependentResources();
    CreateDeviceResources();

    // Create main application window.
    m_hWnd = __super::Create(nullptr, bounds, title.c_str());
    if (!m_hWnd)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Initialize member variables with default values.
    m_visible = TRUE; 
    m_occlusion = DWORD(0.0);

    // Enable mouse to act as pointing device for this application.
    if(!EnableMouseInPointer(TRUE))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

// <summary>
// This method checks the current DPI against what the application has stored.
// If the DPI has changed, update DPI for D2D resources.
// </summary>
void
CSampleDesktopWindow::SetNewDpi(_In_ float newPerMonitorDpi)
{
    if (m_deviceResources && m_deviceResources->GetDpi() != newPerMonitorDpi)
    {
        m_deviceResources->SetDpi(newPerMonitorDpi);
    }
}

// Main message loop for application.
HRESULT
CSampleDesktopWindow::Run()
{
    HRESULT hr = S_OK;

    MSG message = { };
    do
    {
        if (m_visible)
        {
            hr = Render();
        }
        else
        {
            WaitMessage();
        }

        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE) && (message.message != WM_QUIT))
        {
			TranslateMessage(&message);
            
            DispatchMessage(&message);
        }
    } while (message.message != WM_QUIT);

    return hr;
}

// <summary>
// This method is called in response to message handlers and
// as part of the main message loop.
// </summary>
HRESULT
CSampleDesktopWindow::Render()
{
    HRESULT hr = S_OK;
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    d2dContext->BeginDraw();

    // Draw window background.
    d2dContext->Clear(D2D1::ColorF(0.8764F, 0.8764F, 0.8882F));

    // Draw client area as implemented by any derived classes.
    Draw();

    hr = d2dContext->EndDraw();
    if (FAILED(hr))
    {
        return hr;
    }

    if (!m_deviceResources->Present())
    {
        hr = m_deviceResources->GetDxgiFactory()->RegisterOcclusionStatusWindow(m_hWnd, WM_USER, &m_occlusion);
        if (FAILED(hr))
        {
            return hr;
        }
        else
        {
            m_visible = false;
        }
    }

    return hr;
}

// <summary>
// These functions will be called as messages are processed by message map
// defined in the Desktop Window class.
// </summary>
LRESULT
CSampleDesktopWindow::OnCreate(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM lParam,
    _Out_ BOOL &bHandled
    )
{
    auto cs = reinterpret_cast<CREATESTRUCT *>(lParam);

    auto monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	UINT dpix;
	UINT dpiy;
	if (FAILED(GetDpiForMonitor(monitor, MONITOR_DPI_TYPE::MDT_EFFECTIVE_DPI, &dpix, &dpiy)))
	{
		dpix = 96;
		dpiy = 96;
	}
    auto windowDpi = static_cast<float>(dpix);

    // Store a reference to the hWnd so DirectX can render to this surface.
    m_deviceResources->SetWindow(m_hWnd, windowDpi);

    // Set styles needed to avoid drawing over any child or sibling windows.
    cs->style |= ( WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );

    // Set styles required to avoid overdraw. 
    cs->dwExStyle |= ( WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP );

    // Apply selected styles.
    SetWindowLong(GWL_STYLE, cs->style);
    SetWindowLong(GWL_EXSTYLE, cs->dwExStyle);
    ASSERT(SetWindowPos(nullptr, 0, 0, 0, 0,
        SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER));

    bHandled = TRUE;
    return 0;
}

//
// Destroying this window will also quit the application. 
//
LRESULT
CSampleDesktopWindow::OnDestroy(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM,
    _Out_ BOOL &bHandled
    )
{
    m_visible = false;

    PostQuitMessage(0);

    bHandled = TRUE;
    return 0;
}

LRESULT
CSampleDesktopWindow::OnPaint(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM,
    _Out_ BOOL &bHandled
    )
{
    HDC hDC;
    PAINTSTRUCT ps;

    hDC = BeginPaint(&ps);
    if (hDC)
    {
        Render();

        EndPaint(&ps);

        bHandled = TRUE;
        return 0;
    }
    else
    {
        bHandled = FALSE;
    }

    return 0;
}

LRESULT
CSampleDesktopWindow::OnWindowPosChanged(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM lparam,
    _Out_ BOOL &bHandled
    )
{
    RECT clientRect;
    auto windowPos = reinterpret_cast<WINDOWPOS *>(lparam);
    GetClientRect(&clientRect);
    if (!(windowPos->flags & SWP_NOSIZE))
    {
        DeviceResources::Size size;
        size.Width = static_cast<float>(clientRect.right - clientRect.left) / (m_deviceResources->GetDpi() / 96.0F);
        size.Height = static_cast<float>(clientRect.bottom - clientRect.top) / (m_deviceResources->GetDpi() / 96.0F);
        m_deviceResources->SetLogicalSize(size);
        Render();
    }

    bHandled = TRUE;
    return 0;
}

LRESULT
CSampleDesktopWindow::OnDisplayChange(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM,
    _Out_ BOOL &bHandled
    )
{
    Render();
    OnDisplayChange();
    bHandled = TRUE;
    return 0;
}

LRESULT
CSampleDesktopWindow::OnGetMinMaxInfo(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM lparam,
    _Out_ BOOL &bHandled
    )
{
    auto minMaxInfo = reinterpret_cast<MINMAXINFO *>(lparam);

    minMaxInfo->ptMinTrackSize.y = 200;

    bHandled = TRUE;
    return 0;
}

LRESULT
CSampleDesktopWindow::OnActivate(
    _In_ UINT, 
    _In_ WPARAM wparam, 
    _In_ LPARAM, 
    _Out_ BOOL &bHandled
    )
{
    m_visible = !HIWORD(wparam);

    bHandled = TRUE;
    return 0;
}

LRESULT
CSampleDesktopWindow::OnPowerBroadcast(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM lparam,
    _Out_ BOOL &bHandled
    )
{
    if (lparam > 0)
    {
        ASSERT(lparam);
        auto const ps = reinterpret_cast<POWERBROADCAST_SETTING *>(lparam);
        ASSERT(GUID_SESSION_DISPLAY_STATUS == ps->PowerSetting);

        ASSERT(sizeof(DWORD) == ps->DataLength);
        ASSERT(ps->Data);
        m_visible = 0 != *reinterpret_cast<DWORD const *>(ps->Data);
    }

    if (m_visible)
    {
        PostMessage(WM_NULL);
    }

    bHandled = TRUE;
    return TRUE;
}

LRESULT
CSampleDesktopWindow::OnOcclusion(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM,
    _Out_ BOOL &bHandled
    )
{
    ASSERT(m_occlusion);

    if (S_OK == m_deviceResources->GetSwapChain()->Present(0, DXGI_PRESENT_TEST))
    {
        m_deviceResources->GetDxgiFactory()->UnregisterOcclusionStatus(m_occlusion);
        m_occlusion = 0;
        m_visible = true;
    }

    bHandled = TRUE;
    return 0;
}

LRESULT
CSampleDesktopWindow::OnPointerDown(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM lparam,
    _Out_ BOOL &bHandled
    )
{
    auto x = GET_X_LPARAM(lparam);
    auto y = GET_Y_LPARAM(lparam);

    POINT pt;
    pt.x = x;
    pt.y = y;

    ScreenToClient(&pt);

    auto localx = static_cast<float>(pt.x) / (m_deviceResources->GetDpi() / 96.0F);
    auto localy = static_cast<float>(pt.y) / (m_deviceResources->GetDpi() / 96.0F);

    // Call handler implemented by derived class for WM_POINTERDOWN message.
    OnPointerDown(localx, localy);

    bHandled = TRUE;
    return 0;
}

LRESULT
CSampleDesktopWindow::OnPointerUp(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM lparam,
    _Out_ BOOL &bHandled
    )
{
    auto x = GET_X_LPARAM(lparam);
    auto y = GET_Y_LPARAM(lparam);

    POINT pt;
    pt.x = x;
    pt.y = y;

    ScreenToClient(&pt);

    auto localX = static_cast<float>(pt.x) / (m_deviceResources->GetDpi() / 96.0F);
    auto localY = static_cast<float>(pt.y) / (m_deviceResources->GetDpi() / 96.0F);

    // Call handler implemented by derived class for WM_POINTERUP message.
    OnPointerUp(localX, localY);

    bHandled = TRUE;
    return 0;
}

LRESULT
CSampleDesktopWindow::OnEnterSizeMove(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM,
    _Out_ BOOL &bHandled
    )
{
    // Call handler implemented by derived class for WM_ENTERSIZEMOVE message.
    OnEnterSizeMove();

    bHandled = TRUE;
    return 0;
}

LRESULT
CSampleDesktopWindow::OnExitSizeMove(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM,
    _Out_ BOOL &bHandled
    )
{
    // Call handler implemented by derived class for WM_EXITSIZEMOVE message.
    OnExitSizeMove();

    bHandled = TRUE;
    return 0;
}

LRESULT
CSampleDesktopWindow::OnDpiChange(
    _In_ UINT,
    _In_ WPARAM wparam,
    _In_ LPARAM lparam,
    _Out_ BOOL &bHandled
    )
{
    auto lprcNewScale = reinterpret_cast<LPRECT>(lparam);

    // Call handler implemented by derived class for WM_DPICHANGED message.
    OnDpiChange(LOWORD(wparam), lprcNewScale);

    bHandled = TRUE;
    return 0;
}

LRESULT
 CSampleDesktopWindow::OnPointerUpdate(
    _In_ UINT,
    _In_ WPARAM,
    _In_ LPARAM lparam,
    _Out_ BOOL &bHandled
    )
{
    auto x = GET_X_LPARAM(lparam);
    auto y = GET_Y_LPARAM(lparam);

    POINT pt;
    pt.x = x;
    pt.y = y;

    ScreenToClient(&pt);

    auto localx = static_cast<float>(pt.x) / (m_deviceResources->GetDpi() / 96.0F);
    auto localy = static_cast<float>(pt.y) / (m_deviceResources->GetDpi() / 96.0F);

    // Call handler implemented by derived class for WM_POINTERUPDATE message.
    OnPointerUpdate(localx, localy);

    bHandled = TRUE;
    return 0;
}
