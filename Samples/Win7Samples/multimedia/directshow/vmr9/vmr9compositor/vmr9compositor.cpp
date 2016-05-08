//////////////////////////////////////////////////////////////////////////
// vmr9compositor.cpp : Defines the entry point for the application.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "vmr9compositor.h"
#include "MyCompositor9.h"
#include "MultiSelectFileList.h"
#include <strsafe.h>


#define MAX_LOADSTRING 100

// Maximum number of video streams to render.
const DWORD MAX_VIDEO_STREAMS = 4;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int, HWND& hWnd);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ControlWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HRESULT             ConfigureVMR9(HWND window);
HRESULT             StartGraph(HWND window);

DWORD_PTR                       g_userId = 0xACDCACDC;
HWND                            g_hWnd;
HWND                            g_hWndControl;

SmartPtr<IGraphBuilder>          g_graph;
SmartPtr<IBaseFilter>            g_filter;
SmartPtr<IMediaControl>          g_mediaControl;
SmartPtr<IVMRWindowlessControl9> g_windowlessControl;
SmartPtr<CMyCompositor9>         g_compositor;



int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_VMR9COMPOSITOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow, g_hWnd))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_VMR9COMPOSITOR);

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if( IsWindow(g_hWndControl) && IsDialogMessage(g_hWndControl, &msg)) {
            continue;
        }

        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }


    g_compositor = NULL;
    g_windowlessControl = NULL;
    g_mediaControl = NULL;
    g_filter = NULL;
    g_graph = NULL;
    CoUninitialize();

	return (int) msg.wParam;
}



BOOL GetMoviePath(TCHAR *szBuffer, DWORD cchBuffer)
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    szBuffer[0] = NULL;

    static const TCHAR szFilter[]
                            = TEXT("Video Files (.AVI, .MPG, .MPEG, .VOB, .QT, .WMV)\0*.AVI;*.MPG;*.MPEG;*.VOB;*.QT;*.WMV\0") \
                              TEXT("All Files (*.*)\0*.*\0\0");
    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.hwndOwner           = g_hWnd;
    ofn.hInstance           = NULL;
    ofn.lpstrFilter         = szFilter;
    ofn.nFilterIndex        = 1;
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.lpstrFile           = szBuffer;
    ofn.nMaxFile            = cchBuffer;
    ofn.lpstrFileTitle      = NULL;
    ofn.nMaxFileTitle       = 0;
    ofn.lpstrInitialDir     = NULL;
    ofn.lpstrTitle          = TEXT("Select a video file to play...");
    ofn.Flags               = OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    ofn.nFileOffset         = 0;
    ofn.nFileExtension      = 0;
    ofn.lpstrDefExt         = TEXT("AVI");
    ofn.lCustData           = 0L;
    ofn.lpfnHook            = NULL;
    ofn.lpTemplateName  = NULL;

    return GetOpenFileName(&ofn);
}


HRESULT ConfigureVMR9(HWND window)
{
    assert(g_graph != NULL);

    HRESULT hr = S_OK;
    RECT rect;

    SmartPtr<IVMRFilterConfig9> filterConfig;

    // Create the VMR-9.
    FAIL_RET( CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&g_filter) );

    // Configure the VMR-9.
    // Set the maximum number of video streams, and set windowless mode. 
    FAIL_RET( g_filter->QueryInterface(IID_IVMRFilterConfig9, reinterpret_cast<void**>(&filterConfig)) );

    FAIL_RET( filterConfig->SetNumberOfStreams( MAX_VIDEO_STREAMS ) );
    FAIL_RET( filterConfig->SetRenderingMode( VMR9Mode_Windowless ) );

    FAIL_RET( g_filter->QueryInterface(IID_IVMRWindowlessControl9, reinterpret_cast<void**>(&g_windowlessControl)) );
    FAIL_RET( g_windowlessControl->SetVideoClippingWindow( window ) );

    GetClientRect( window, & rect );
    FAIL_RET( g_windowlessControl->SetVideoPosition(NULL, & rect) );

    // Set the custom compositor on the VMR-9.

    g_compositor.Attach( new CMyCompositor9() );
    FAIL_RET( filterConfig->SetImageCompositor( g_compositor ));

    // Add the VMR-9 to the filter graph.
    FAIL_RET( g_graph->AddFilter(g_filter, L"Video Mixing Renderer 9") );

    return hr;
}


HRESULT StartGraph(HWND window)
{
    HRESULT hr = S_OK;

    const DWORD BUFFER_SIZE = MAX_PATH * MAX_VIDEO_STREAMS;

    MultiSelectFileList<BUFFER_SIZE> selectList;

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.hwndOwner           = g_hWnd;
    ofn.lpstrFilter         = TEXT("Video Files (.AVI, .MPG, .MPEG, .VOB, .WMV)\0*.AVI;*.MPG;*.MPEG;*.VOB;*.WMV\0") 
                              TEXT("All Files (*.*)\0*.*\0\0");
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = selectList.BufferPtr();
    ofn.nMaxFile            = selectList.BufferSizeCch();
    ofn.lpstrTitle          = TEXT("Select a video file to play...");
    ofn.Flags               = OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    ofn.lpstrDefExt         = TEXT("AVI");
    
    // Launch the Open File dialog.
	DWORD result = GetOpenFileName(&ofn);

    // Check for errors.
    if (CommDlgExtendedError() != 0)
    {

        // NOTE: For mult-selection, CommDlgExtendedError can return FNERR_BUFFERTOOSMALL even when
        // GetOpenFileName returns TRUE.

        MessageBox(NULL, TEXT( "Could not open files." ), NULL, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }
    else if (!result)
    {
        // The user cancelled. (No error occurred.)
        return S_OK;
    }

    FAIL_RET(hr = selectList.ParseBuffer());

    // Clear all DirectShow interfaces (COM smart pointers)
    g_compositor            = NULL;
    g_windowlessControl     = NULL;
    g_mediaControl          = NULL;
    g_filter                = NULL;
    g_graph                 = NULL;

    // Create the Filter Graph Manager.
    FAIL_RET( CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&g_graph) );

    // Configure the VMR-9.
    FAIL_RET( ConfigureVMR9(window) );

    // Render every file that the user selected.
    for (DWORD i = 0; i < MAX_VIDEO_STREAMS; i++)
    {
        TCHAR *pFileName = NULL;

        FAIL_RET(hr = selectList.Next(&pFileName));
        if (hr == S_FALSE)
        {
            hr = S_OK;
            break;
        }

		hr = g_graph->RenderFile( pFileName, NULL );

        CoTaskMemFree(pFileName);

        FAIL_RET(hr);

	}

    // Run the graph.

    FAIL_RET( g_graph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&g_mediaControl)) );

    FAIL_RET( g_mediaControl->Run() );

    return hr;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= (LPCTSTR)IDC_VMR9COMPOSITOR;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND& hWnd)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                       100, 100, 600, 500,
                       NULL, NULL, hInstance, NULL);

   if (!hWnd)
      return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hWndControl = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_CONTROL) ,hWnd, (DLGPROC)ControlWndProc );
   ShowWindow( g_hWndControl, nCmdShow );

   return TRUE;
}

void PaintWindow(HWND hWnd)
{
    bool bNeedPaint = false;

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    if( g_mediaControl == NULL ) // we know that there is nothing loaded
    {
        bNeedPaint = true;
    }
    else
    {
        // If we have a movie loaded, we only need to repaint
        // when the graph is stopped
        OAFilterState state;
        if( SUCCEEDED( g_mediaControl->GetState(0, & state ) ) )
        {
            bNeedPaint = ( state == State_Stopped );
        }
    }

    if ( bNeedPaint )
    {
        RECT rc2;
        GetClientRect(hWnd, &rc2);
        FillRect(hdc, &rc2, (HBRUSH)(COLOR_WINDOW+1));
    }

    EndPaint( hWnd, &ps );
}

void MoveWindow()
{
    if( g_windowlessControl == NULL ) {
        return;
    }

    RECT r;
    GetClientRect( g_hWnd, & r );
    g_windowlessControl->SetVideoPosition(NULL, & r);
}


//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr;
    int wmId, wmEvent;

    switch (message)
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam);
            wmEvent = HIWORD(wParam);

            // Parse the menu selections:
            switch (wmId)
            {
                case ID_FILE_START:
                    hr = StartGraph(g_hWnd);
                    if( FAILED(hr) )
                    {
                        return 0;
                    }
                    break;

                case IDM_ABOUT:
                   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
                   break;

                case IDM_EXIT:
                   DestroyWindow(hWnd);
                   break;

                default:
                   return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;

        case WM_PAINT:
            PaintWindow(hWnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_MOVE:
        case WM_SIZE:
            MoveWindow();
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
