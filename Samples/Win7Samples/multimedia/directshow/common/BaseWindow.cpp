//////////////////////////////////////////////////////////////////////////
// BaseWindow.cpp: Abstract window class.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "wincontrol.h"
#include "BaseWindow.h"


//--------------------------------------------------------------------------------------
// BaseWindow constructor.
//--------------------------------------------------------------------------------------

BaseWindow::BaseWindow() : m_hwnd(NULL), m_hInstance(NULL)
{
}


//--------------------------------------------------------------------------------------
// BaseWindow::Register
// Description: Registers the window class.
//--------------------------------------------------------------------------------------

HRESULT BaseWindow::Register()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInstance;
	wcex.hIcon			= NULL; 
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MenuName();
	wcex.lpszClassName	= ClassName();
	wcex.hIconSm		= NULL;

	ATOM atom = RegisterClassEx(&wcex);

	if (atom == 0)
	{
		return __HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		return S_OK;
	}
}

//--------------------------------------------------------------------------------------
// BaseWindow::Create
// Description: Creates an instance of the window.
//--------------------------------------------------------------------------------------

HRESULT BaseWindow::Create(HINSTANCE hInstance)
{
	m_hInstance = hInstance;

	HRESULT hr = Register();
	if (SUCCEEDED(hr))
	{
		HWND hwnd = CreateWindow(
			ClassName(),
			WindowName(), 
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
			NULL, 
			NULL, 
			m_hInstance, 
			this);

		if (hwnd == 0)
		{
			hr =  __HRESULT_FROM_WIN32(GetLastError());
		}
	}

	return hr;
}

//--------------------------------------------------------------------------------------
// BaseWindow::Show
// Description: Show or hide the window.
//--------------------------------------------------------------------------------------

HRESULT BaseWindow::Show(int nCmdShow)
{
	ShowWindow(m_hwnd, nCmdShow);
	UpdateWindow(m_hwnd);
	return S_OK;
}


//--------------------------------------------------------------------------------------
// BaseWindow::WindowProc
// Description: Window procedure.
//--------------------------------------------------------------------------------------

LRESULT CALLBACK BaseWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BaseWindow *pWin = NULL;

	if (uMsg == WM_NCCREATE) 
	{
        // When we create the window, we pass in a pointer to this class 
        // as part of the CREATESTRUCT structure.
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		pWin = (BaseWindow*)lpcs->lpCreateParams;
		
        // Set the window handle.
        pWin->m_hwnd = hwnd;

        // Set the pointer to the class as user data.

        _SetWindowLongPtr(hwnd, GWLP_USERDATA, pWin);
	} 
	else 
	{
        // Get the pointer to the class.
		pWin = _GetWindowLongPtr<BaseWindow*>(hwnd, GWLP_USERDATA);
	}

	if (pWin) 
	{
		return pWin->OnReceiveMessage(uMsg, wParam, lParam);
	} 
	else 
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

//--------------------------------------------------------------------------------------
// BaseWindow::OnReceiveMessage
// Description: Handle window messages other than WM_NCCREATE.
//--------------------------------------------------------------------------------------

LRESULT BaseWindow::OnReceiveMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
	case WM_NCDESTROY:
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

	case WM_PAINT:
		OnPaint();
		return 0;

	}
	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
