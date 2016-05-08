//------------------------------------------------------------------------------
// File: vmr9allocator.cpp
//
// Desc: DirectShow sample code - main implementation file
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#pragma warning(disable: 4710)  // "function not inlined"

#include "stdafx.h"
#include "resource.h"
#include "util.h"
#include "Allocator.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                       
TCHAR szTitle[MAX_LOADSTRING];         
TCHAR szWindowClass[MAX_LOADSTRING];  

// Forward declarations of functions included in this code module:
BOOL                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND& );
void                PaintWindow(HWND hWnd);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HRESULT             StartGraph(HWND window);
HRESULT             CloseGraph(HWND window);
HRESULT             SetAllocatorPresenter( IBaseFilter *filter, HWND window );
BOOL                VerifyVMR9(void);

DWORD_PTR                       g_userId = 0xACDCACDC;
HWND                            g_hWnd;

// DirectShow interfaces
SmartPtr<IGraphBuilder>          g_graph;
SmartPtr<IBaseFilter>            g_filter;
SmartPtr<IMediaControl>          g_mediaControl;
SmartPtr<IVMRSurfaceAllocator9>  g_allocator;


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    int ret = -1;

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    // Verify that the VMR9 is present on this system
    if(!VerifyVMR9())
    {
        CoUninitialize();
        return FALSE;
    }

    __try 
    {
        MSG msg;
        HACCEL hAccelTable;

        // Initialize global strings
        LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
        LoadString(hInstance, IDC_ALLOCATOR9, szWindowClass, MAX_LOADSTRING);
        MyRegisterClass(hInstance);

        // Perform application initialization:
        if (!InitInstance (hInstance, nCmdShow, g_hWnd)) 
        {
            // Exit the try block gracefully.  Returning FALSE here would
            // lead to a performance penalty unwinding the stack.
            __leave;
        }

        hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_ALLOCATOR9);

        // Main message loop:
        while (GetMessage(&msg, NULL, 0, 0)) 
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        ret = (int) msg.wParam;
    }
    __finally
    {
        // make sure to release everything at the end
        // regardless of what's happening
        CloseGraph(g_hWnd);
        CoUninitialize();
    }

    return ret;
}

BSTR GetMoviePath()
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    TCHAR  szBuffer[MAX_PATH];
    szBuffer[0] = NULL;

    static const TCHAR szFilter[]  
                            = TEXT("Video Files (.ASF, .AVI, .MPG, .MPEG, .VOB, .QT, .WMV)\0*.ASF;*.AVI;*.MPG;*.MPEG;*.VOB;*.QT;*.WMV\0") \
                              TEXT("All Files (*.*)\0*.*\0\0");
    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.hwndOwner           = g_hWnd;
    ofn.hInstance           = NULL;
    ofn.lpstrFilter         = szFilter;
    ofn.nFilterIndex        = 1;
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.lpstrFile           = szBuffer;
    ofn.nMaxFile            = MAX_PATH;
    ofn.lpstrFileTitle      = NULL;
    ofn.nMaxFileTitle       = 0;
    ofn.lpstrInitialDir     = NULL;
    ofn.lpstrTitle          = TEXT("Select a video file to play...");
    ofn.Flags               = OFN_HIDEREADONLY;
    ofn.nFileOffset         = 0;
    ofn.nFileExtension      = 0;
    ofn.lpstrDefExt         = TEXT("AVI");
    ofn.lCustData           = 0L;
    ofn.lpfnHook            = NULL;
    ofn.lpTemplateName  = NULL; 
    
    if (GetOpenFileName (&ofn))  // user specified a file
    {
        return SysAllocString( szBuffer );
    }

   return NULL;
}

HRESULT             
CloseGraph(HWND window)
{
    if( g_mediaControl != NULL ) 
    {
        OAFilterState state;
        do {
            g_mediaControl->Stop();
            g_mediaControl->GetState(0, & state );
        } while( state != State_Stopped ) ;
    }

    g_allocator    = NULL;        
    g_mediaControl = NULL;        
    g_filter       = NULL;        
    g_graph        = NULL;
    ::InvalidateRect( window, NULL, true );
    return S_OK;
}

HRESULT StartGraph(HWND window)
{
    // Clear DirectShow interfaces (COM smart pointers)
    CloseGraph(window);

    SmartPtr<IVMRFilterConfig9> filterConfig;


    BSTR path = GetMoviePath();
    if( ! path )
    {
        return E_FAIL;
    }

    HRESULT hr;
    
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&g_graph);

    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&g_filter);
    }

    if (SUCCEEDED(hr))
    {
        hr = g_filter->QueryInterface(IID_IVMRFilterConfig9, reinterpret_cast<void**>(&filterConfig));
    }

    if (SUCCEEDED(hr))
    {
        hr = filterConfig->SetRenderingMode( VMR9Mode_Renderless );

    }

    if (SUCCEEDED(hr))
    {
        hr = filterConfig->SetNumberOfStreams(2);

    }

    if (SUCCEEDED(hr))
    {
        hr = SetAllocatorPresenter( g_filter, window );
    }

    if (SUCCEEDED(hr))
    {
        hr = g_graph->AddFilter(g_filter, L"Video Mixing Renderer 9");
    }

    if (SUCCEEDED(hr))
    {
        hr = g_graph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&g_mediaControl));
    }

    if (SUCCEEDED(hr))
    {
        hr = g_graph->RenderFile( path, NULL );
    }

    if (SUCCEEDED(hr))
    {
        hr = g_mediaControl->Run();
    }

    SysFreeString(path);

    return hr;
}

HRESULT SetAllocatorPresenter( IBaseFilter *filter, HWND window )
{
    if( filter == NULL )
    {
        return E_FAIL;
    }

    HRESULT hr;

    SmartPtr<IVMRSurfaceAllocatorNotify9> lpIVMRSurfAllocNotify;
    FAIL_RET( filter->QueryInterface(IID_IVMRSurfaceAllocatorNotify9, reinterpret_cast<void**>(&lpIVMRSurfAllocNotify)) );

    // create our surface allocator
    g_allocator.Attach(new CAllocator( hr, window ));
    if( FAILED( hr ) )
    {
        g_allocator = NULL;
        return hr;
    }

    // let the allocator and the notify know about each other
    FAIL_RET( lpIVMRSurfAllocNotify->AdviseSurfaceAllocator( g_userId, g_allocator ) );
    FAIL_RET( g_allocator->AdviseNotify(lpIVMRSurfAllocNotify) );
    
    return hr;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
BOOL MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));

    // Set the members of the window class structure.
    //
    // Don't provide a background brush, because we process the WM_PAINT
    // messages in OnPaint().  If a movie is active, we tell the VMR to
    // repaint the window; otherwise, we repaint with COLOR_WINDOW+1.
    // If a background brush is provided, you will see a white flicker
    // whenever you resize the main application window, because Windows
    // will repaint the window before the application also repaints.
    wc.hInstance     = hInstance;
    wc.lpfnWndProc   = WndProc;
    wc.lpszClassName = szWindowClass;
    wc.lpszMenuName  = (LPCTSTR)IDC_ALLOCATOR9;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hbrBackground = NULL;        // No background brush
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = NULL;
    if(!RegisterClass(&wc))
    {
        return FALSE;
    }

    return TRUE;
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

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
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
                case IDM_PLAY_FILE:
                    hr = StartGraph(g_hWnd);
                    if( FAILED(hr) )
                    {
                        return 0;
                    }
                    break;

                case ID_FILE_CLOSE:
                    CloseGraph( hWnd );
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

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
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



//----------------------------------------------------------------------------
//  VerifyVMR9
//
//  Verifies that VMR9 COM objects exist on the system and that the VMR9
//  can be instantiated.
//
//  Returns: FALSE if the VMR9 can't be created
//----------------------------------------------------------------------------

BOOL VerifyVMR9(void)
{
    HRESULT hr;

    // Verify that the VMR exists on this system
    IBaseFilter* pBF = NULL;
    hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL,
                          CLSCTX_INPROC,
                          IID_IBaseFilter,
                          (LPVOID *)&pBF);
    if(SUCCEEDED(hr))
    {
        pBF->Release();
        return TRUE;
    }
    else
    {
        MessageBox(NULL,
            TEXT("This application requires the VMR-9.\r\n\r\n")

            TEXT("The VMR-9 is not enabled when viewing through a Remote\r\n")
            TEXT(" Desktop session. You can run VMR-enabled applications only\r\n") 
            TEXT("on your local computer.\r\n\r\n")

            TEXT("\r\nThis sample will now exit."),

            TEXT("Video Mixing Renderer (VMR9) capabilities are required"), MB_OK);

        return FALSE;
    }
}