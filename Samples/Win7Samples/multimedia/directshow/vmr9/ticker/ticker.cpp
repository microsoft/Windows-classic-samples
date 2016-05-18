//------------------------------------------------------------------------------
// File: Ticker.cpp
//
// Desc: DirectShow sample code - a simple static image display app.
//       Using the DirectX 9 Video Mixing Renderer, a static image is
//       alpha blended with the image in the corner of the screen.
//       The image moves from right to left near the bottom of the screen.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <tchar.h>
#include <dshow.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <strsafe.h>

#include "ticker.h"
#include "bitmap.h"
#include "resource.h"

// Common files
#include "smartptr.h"
#include "dshowutil.h"


// An application can advertise the existence of its filter graph
// by registering the graph with a global Running Object Table (ROT).
// The GraphEdit application can detect and remotely view the running
// filter graph, allowing you to 'spy' on the graph with GraphEdit.
//
// To enable registration in this sample, define REGISTER_FILTERGRAPH.
//
#define REGISTER_FILTERGRAPH

//
// Global data
//
HWND      ghApp=0;
HMENU     ghMenu=0;
HINSTANCE ghInst=0;
TCHAR     g_szFileName[MAX_PATH]={0};
DWORD     g_dwGraphRegister=0;
RECT      g_rcDest={0};

// DirectShow interfaces
IGraphBuilder *pGB = NULL;
IMediaControl *pMC = NULL;
IMediaEventEx *pME = NULL;
IMediaSeeking *pMS = NULL;
IVMRWindowlessControl9 *pWC = NULL;


HRESULT PlayMovieInWindow(LPTSTR szFile)
{
    HRESULT hr;

    // Check input string
    if (szFile == NULL)
        return E_POINTER;

    // Clear open dialog remnants before calling RenderFile()
    UpdateWindow(ghApp);

    // Get the interface for DirectShow's GraphBuilder
    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                         IID_IGraphBuilder, (void **)&pGB));

    if(SUCCEEDED(hr))
    {
        SmartPtr <IBaseFilter> pVmr;

        // Create the Video Mixing Renderer and add it to the graph
        JIF(InitializeWindowlessVMR(&pVmr));

        // Render the file programmatically to use the VMR9 as renderer.
        // Pass TRUE to create an audio renderer also.
        if (FAILED(hr = RenderFileToVideoRenderer(pGB, szFile, TRUE)))
            return hr;

        // QueryInterface for DirectShow interfaces
        JIF(pGB->QueryInterface(IID_IMediaControl, (void **)&pMC));
        JIF(pGB->QueryInterface(IID_IMediaEventEx, (void **)&pME));
        JIF(pGB->QueryInterface(IID_IMediaSeeking, (void **)&pMS));

        // Is this an audio-only file (no video component)?
        if (CheckVideoVisibility())
        {
            JIF(InitVideoWindow(1, 1));
        }
        else
        {
            // This sample requires a video clip to be loaded
            Msg(TEXT("This sample requires media with a video component.  ")
                TEXT("Please select another file."));
            return E_FAIL;
        }

        // Have the graph signal event via window callbacks for performance
        JIF(pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0));

        // Add the bitmap (static image or dynamic text) to the VMR's input
        if (g_dwTickerFlags & MARK_STATIC_IMAGE)
        {
            hr = BlendApplicationImage(ghApp);
            if (FAILED(hr))
                PostMessage(ghApp, WM_CLOSE, 0, 0);

            CheckMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, MF_CHECKED);
            CheckMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, MF_UNCHECKED);
        }
        else                                        // MARK_DYNAMIC_TEXT
        {
            if (!g_hFont)
                g_hFont = SetTextFont(FALSE);  // Don't display the Windows Font Select dialog

            // If the initial blend fails, post a close message to exit the app
            hr = BlendApplicationText(ghApp, g_szAppText);
            if (FAILED(hr))
                PostMessage(ghApp, WM_CLOSE, 0, 0);

            CheckMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, MF_UNCHECKED);
            CheckMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, MF_CHECKED);
        }

        // Complete the window setup
        ShowWindow(ghApp, SW_SHOWNORMAL);
        UpdateWindow(ghApp);
        SetForegroundWindow(ghApp);
        SetFocus(ghApp);

#ifdef REGISTER_FILTERGRAPH
        if (FAILED(AddGraphToRot(pGB, &g_dwGraphRegister)))
        {
            Msg(TEXT("Failed to register filter graph with ROT!"));
            g_dwGraphRegister = 0;
        }
#endif

        // Run the graph to play the media file
        JIF(pMC->Run());

        // Start animation by default
        PostMessage(ghApp, WM_COMMAND, ID_SLIDE, 0);
    }

    return hr;
}


HRESULT InitVideoWindow(int nMultiplier, int nDivider)
{
    LONG lHeight, lWidth;
    HRESULT hr = S_OK;

    if (!pWC)
        return S_OK;

    // Read the default video size
    hr = pWC->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL);
    if (hr == E_NOINTERFACE)
        return S_OK;

    // Account for requests of normal, half, or double size
    lWidth  = lWidth  * nMultiplier / nDivider;
    lHeight = lHeight * nMultiplier / nDivider;

    int nTitleHeight  = GetSystemMetrics(SM_CYCAPTION);
    int nBorderWidth  = GetSystemMetrics(SM_CXBORDER);
    int nBorderHeight = GetSystemMetrics(SM_CYBORDER);

    // Account for size of title bar and borders for exact match
    // of window client area to default video size
    SetWindowPos(ghApp, NULL, 0, 0, lWidth + 2*nBorderWidth,
                 lHeight + nTitleHeight + 2*nBorderHeight,
                 SWP_NOMOVE | SWP_NOOWNERZORDER);

    GetClientRect(ghApp, &g_rcDest);
    hr = pWC->SetVideoPosition(NULL, &g_rcDest);
    if (FAILED(hr))
        Msg(TEXT("SetVideoPosition FAILED!  hr=0x%x\r\n"), hr);

    return hr;
}


HRESULT InitPlayerWindow(void)
{
    // Reset to a default size for audio and after closing a clip
    SetWindowPos(ghApp, NULL, 0, 0,
                 DEFAULT_PLAYER_WIDTH, DEFAULT_PLAYER_HEIGHT,
                 SWP_NOMOVE | SWP_NOOWNERZORDER);
    return S_OK;
}


void MoveVideoWindow(void)
{
    HRESULT hr;

    // Track the movement of the container window and resize as needed
    if(pWC)
    {
        GetClientRect(ghApp, &g_rcDest);

        hr = pWC->SetVideoPosition(NULL, &g_rcDest);
        if (FAILED(hr))
            Msg(TEXT("SetVideoPosition FAILED!  hr=0x%x\r\n"), hr);
    }
}


BOOL CheckVideoVisibility(void)
{
    HRESULT hr;
    LONG lWidth=0, lHeight=0;

    //
    // Because this sample explicitly loads the VMR9 into the filter graph
    // before rendering a file, the IVMRWindowlessControl interface will exist
    // for all properly rendered files.  As a result, we can't depend on the
    // existence of the pWC interface to determine whether the media file has
    // a video component.  Instead, check the width and height values.
    //
    if (!pWC)
    {
        // Audio-only files have no video interfaces.  This might also
        // be a file whose video component uses an unknown video codec.
        return FALSE;
    }

    hr = pWC->GetNativeVideoSize(&lWidth, &lHeight, 0, 0);
    if (FAILED(hr))
    {
        // If this video is encoded with an unsupported codec,
        // we won't see any video, although the audio will work if it is
        // of a supported format.
        return FALSE;
    }

    // If this is an audio-only clip, width and height will be 0.
    if ((lWidth == 0) && (lHeight == 0))
        return FALSE;

    // Assume that this media file contains a video component
    return TRUE;
}


void OpenClip()
{
    HRESULT hr;

    // If no filename specified by command line, show file open dialog
    if(g_szFileName[0] == L'\0')
    {
        TCHAR szFilename[MAX_PATH];

        InitPlayerWindow();
        SetForegroundWindow(ghApp);

        if (! GetClipFileName(szFilename))
        {
            DWORD dwDlgErr = CommDlgExtendedError();

            // Don't show output if user cancelled the selection (no dlg error)
            if (dwDlgErr)
            {
                Msg(TEXT("GetClipFileName Failed! Error=0x%x\r\n"), GetLastError());
            }
            return;
        }

        // This sample does not support playback of ASX playlists.
        // Since this could be confusing to a user, display a warning
        // message if an ASX file was opened.
        if (_tcsnicmp(szFilename, TEXT(".asx"), 4) == 0)
        {
            Msg(TEXT("ASX Playlists are not supported by this application.\n\n")
                TEXT("Please select a valid media file.\0"));
            return;
        }

        hr = StringCchCopy(g_szFileName, NUMELMS(g_szFileName), szFilename);
        if (FAILED(hr))
        {
            return;
        }
    }

    EnableTickerMenu(TRUE);

    // Start playing the media file
    hr = PlayMovieInWindow(g_szFileName);

    // If we couldn't play the clip, clean up
    if (FAILED(hr))
        CloseClip();
}


BOOL GetClipFileName(LPTSTR szName)
{
    static OPENFILENAME ofn={0};
    static BOOL bSetInitialDir = FALSE;

    // Reset filename
    *szName = 0;

    // Fill in standard structure fields
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = ghApp;
    ofn.lpstrFilter       = NULL;
    ofn.lpstrFilter       = FILE_FILTER_TEXT;
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = szName;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrTitle        = TEXT("Open Image File...\0");
    ofn.lpstrFileTitle    = NULL;
    ofn.lpstrDefExt       = TEXT("*\0");
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_READONLY | OFN_PATHMUSTEXIST;

    // Remember the path of the first selected file
    if (bSetInitialDir == FALSE)
    {
        ofn.lpstrInitialDir = DEFAULT_MEDIA_PATH;
        bSetInitialDir = TRUE;
    }
    else
        ofn.lpstrInitialDir = NULL;

    // Create the standard file open dialog and return its result
    return GetOpenFileName((LPOPENFILENAME)&ofn);
}


void CloseClip()
{
    HRESULT hr;

    // Stop media playback
    if(pMC)
        hr = pMC->Stop();

    // Free DirectShow interfaces
    CloseInterfaces();

    // Clear file name to allow selection of new file with open dialog
    g_szFileName[0] = L'\0';

    // Reset the player window
    RECT rect;
    GetClientRect(ghApp, &rect);
    InvalidateRect(ghApp, &rect, TRUE);

    InitPlayerWindow();
    EnableTickerMenu(FALSE);
}


void CloseInterfaces(void)
{
#ifdef REGISTER_FILTERGRAPH
    if (g_dwGraphRegister)
    {
        RemoveGraphFromRot(g_dwGraphRegister);
        g_dwGraphRegister = 0;
    }
#endif

    // Clear ticker state and timer settings
    ClearTickerState();

    // Release and zero DirectShow interfaces
    SAFE_RELEASE(pME);
    SAFE_RELEASE(pMS);
    SAFE_RELEASE(pMC);
    SAFE_RELEASE(pWC);
    SAFE_RELEASE(pBMP);
    SAFE_RELEASE(pGB);
}


HRESULT HandleGraphEvent(void)
{
    LONG evCode;
	LONG_PTR evParam1, evParam2;
    HRESULT hr=S_OK;

    // Make sure that we don't access the media event interface
    // after it has already been released.
    if (!pME)
        return S_OK;

    // Process all queued events
    while(SUCCEEDED(pME->GetEvent(&evCode, &evParam1, &evParam2, 0)))
    {
        // Free memory associated with callback, since we're not using it
        hr = pME->FreeEventParams(evCode, evParam1, evParam2);

        // If this is the end of the clip, reset to beginning
        if(EC_COMPLETE == evCode)
        {
            LONGLONG pos=0;

            // Reset to first frame of movie
            hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                                   NULL, AM_SEEKING_NoPositioning);
            if (FAILED(hr))
            {
                // If seeking failed, just stop and restart playback
                hr = pMC->Stop();
                hr = pMC->Run();
            }
        }
    }

    return hr;
}



void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    (void)StringCchVPrintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
    va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

    // Display a message box with the formatted string
    MessageBox(NULL, szBuffer, TEXT("VMRTicker Sample"), MB_OK);
}



LRESULT CALLBACK AboutDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            if (wParam == IDOK)
            {
                EndDialog(hWnd, TRUE);
                return TRUE;
            }
            break;
    }

    return FALSE;
}


LRESULT CALLBACK TextDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static TCHAR szSaveText[DYNAMIC_TEXT_SIZE]={0};

    switch (message)
    {
        case WM_INITDIALOG:
            // Save current dynamic text so that we can temporarily modify
            // the dynamic text during font adjustment
            (void)StringCchCopy(szSaveText, NUMELMS(szSaveText), g_szAppText);
            SendMessage(GetDlgItem(hWnd, IDC_EDIT_TEXT), EM_LIMITTEXT, DYNAMIC_TEXT_SIZE, 0L);
            SetWindowText(GetDlgItem(hWnd, IDC_EDIT_TEXT), g_szAppText);
            return TRUE;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDOK:
                {
                    TCHAR szText[DYNAMIC_TEXT_SIZE];
                    GetWindowText(GetDlgItem(hWnd, IDC_EDIT_TEXT), szText, DYNAMIC_TEXT_SIZE);
                    (void)StringCchCopy(g_szAppText, NUMELMS(g_szAppText)-1, szText);
                    BlendApplicationText(ghApp, g_szAppText);
                    EndDialog(hWnd, TRUE);
                    return TRUE;
                }
                break;

                case IDCANCEL:
                    // Restore the original text in case it was modified
                    // and previewed while adjusting the font
                    (void)StringCchCopy(g_szAppText, NUMELMS(g_szAppText)-1, szSaveText);
                    BlendApplicationText(ghApp, g_szAppText);
                    EndDialog(hWnd, TRUE);
                    break;

                case IDC_SET_FONT:
                {
                    TCHAR szTempText[DYNAMIC_TEXT_SIZE]={0};

                    // Change the current font
                    g_hFont = UserSelectFont();   

                    // Start displaying the text that is currently in the edit box
                    GetWindowText(GetDlgItem(hWnd, IDC_EDIT_TEXT), szTempText, DYNAMIC_TEXT_SIZE);
                    BlendApplicationText(ghApp, szTempText);
                }
                break;
            }
            break;
    }
    return FALSE;
}


LRESULT CALLBACK WndMainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_PAINT:
            OnPaint(hWnd);
            break;

        case WM_DISPLAYCHANGE:
            if (pWC)
                pWC->DisplayModeChanged();
            break;
        
        // Resize the video when the window changes
        case WM_MOVE:
        case WM_SIZE:
            if (hWnd == ghApp)
                MoveVideoWindow();
            break;

        // Enforce a minimum size
        case WM_GETMINMAXINFO:
            {
                LPMINMAXINFO lpmm = (LPMINMAXINFO) lParam;
                if (lpmm)
                {
                    lpmm->ptMinTrackSize.x = MINIMUM_VIDEO_WIDTH;
                    lpmm->ptMinTrackSize.y = MINIMUM_VIDEO_HEIGHT;
                }
            }
            break;

        case WM_KEYDOWN:

            switch(toupper((int) wParam))
            {
                case VK_ESCAPE:
                case VK_F12:
                    CloseClip();
                    break;
            }
            break;

        case WM_COMMAND:

            switch(wParam)
            { // Menus

                case ID_FILE_OPENCLIP:
                    // If we have ANY file open, close it and shut down DirectShow
                    CloseClip();                   
                    OpenClip();   // Open the new clip
                    break;

                case ID_FILE_INITCLIP:
                    OpenClip();   // Open the new clip
                    break;

                case ID_FILE_EXIT:
                    CloseClip();
                    PostQuitMessage(0);
                    break;

                case ID_FILE_CLOSE:
                    CloseClip();
                    break;

                case ID_DISABLE:
                    FlipFlag(MARK_DISABLE);
                    DisableTicker(g_dwTickerFlags);
                    break;

                case ID_SLIDE:
                    FlipFlag(MARK_SLIDE);
                    SlideTicker(g_dwTickerFlags);
                    break;

                case ID_TICKER_STATIC_IMAGE:
                    g_dwTickerFlags |= MARK_STATIC_IMAGE;
                    g_dwTickerFlags &= ~(MARK_DYNAMIC_TEXT);
                    BlendApplicationImage(ghApp);
                    CheckMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, MF_CHECKED);
                    CheckMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, MF_UNCHECKED);
                    break;

                case ID_TICKER_DYNAMIC_TEXT:
                    g_dwTickerFlags |= MARK_DYNAMIC_TEXT;
                    g_dwTickerFlags &= ~(MARK_STATIC_IMAGE);
                    BlendApplicationText(ghApp, g_szAppText);
                    CheckMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, MF_UNCHECKED);
                    CheckMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, MF_CHECKED);
                    break;

                case ID_SET_FONT:
                    g_hFont = UserSelectFont();   // Change the current font
                    PostMessage(ghApp, WM_COMMAND, ID_TICKER_DYNAMIC_TEXT, 0);
                    break;

                case ID_SET_TEXT:
                    DialogBox(ghInst, MAKEINTRESOURCE(IDD_DIALOG_TEXT),
                              ghApp,  (DLGPROC) TextDlgProc);
                    break;

                case ID_HELP_ABOUT:
                    DialogBox(ghInst, MAKEINTRESOURCE(IDD_HELP_ABOUT),
                              ghApp,  (DLGPROC) AboutDlgProc);
                    break;

            } // Menus
            break;


        case WM_GRAPHNOTIFY:
            HandleGraphEvent();
            break;

        case WM_CLOSE:
            SendMessage(ghApp, WM_COMMAND, ID_FILE_EXIT, 0);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);

    } // Window msgs handling

    return DefWindowProc(hWnd, message, wParam, lParam);
}


int PASCAL wWinMain(HINSTANCE hInstC, HINSTANCE hInstP, wchar_t * lpCmdLine, int nCmdShow)
{
    MSG msg={0};
    WNDCLASS wc;

    // Initialize COM
    if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
    {
        Msg(TEXT("CoInitialize Failed!\r\n"));
        return FALSE;
    }

    // Verify that the VMR is present on this system
    if(!VerifyVMR9())
        return FALSE;

    // Was a filename specified on the command line?
    if(lpCmdLine[0] != '\0')
    {
        (void)StringCchCopy(g_szFileName, NUMELMS(g_szFileName), lpCmdLine);        
    }

    // Register the window class
    ZeroMemory(&wc, sizeof wc);
    ghInst = wc.hInstance = hInstC;
    wc.lpfnWndProc   = WndMainProc;
    wc.lpszClassName = CLASSNAME;
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = NULL;
    if(!RegisterClass(&wc))
    {
        Msg(TEXT("RegisterClass Failed! Error=0x%x\r\n"), GetLastError());
        CoUninitialize();
        exit(1);
    }

    // Create the main window.  The WS_CLIPCHILDREN style is required.
    ghApp = CreateWindow(CLASSNAME, APPLICATIONNAME,
                         WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN | WS_VISIBLE,
                         CW_USEDEFAULT, CW_USEDEFAULT,
                         DEFAULT_PLAYER_WIDTH, DEFAULT_PLAYER_HEIGHT,
                         0, 0, ghInst, 0);

    if(ghApp)
    {
        // Save menu handle for later use
        ghMenu = GetMenu(ghApp);
        EnableTickerMenu(FALSE);

        // Set default dynamic text if user wants to display dynamic text
        // instead of a static bitmap image
        (void)StringCchCopy(g_szAppText, NUMELMS(g_szAppText), BLEND_TEXT);
        g_dwTickerFlags = DEFAULT_MARK;

        // If a media file was specified on the command line, open it now.
        // (If the first character in the string isn't NULL, post an open clip message.)
        if (g_szFileName[0] != 0)
            PostMessage(ghApp, WM_COMMAND, ID_FILE_INITCLIP, 0);

        // Main message loop
        while(GetMessage(&msg,NULL,0,0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    else
    {
        Msg(TEXT("Failed to create the main window! Error=0x%x\r\n"), GetLastError());
    }

    // Finished with COM
    CoUninitialize();

    return (int) msg.wParam;
}


HRESULT InitializeWindowlessVMR(IBaseFilter **ppVmr9)
{
    IBaseFilter* pVmr = NULL;

    if (!ppVmr9)
        return E_POINTER;
    *ppVmr9 = NULL;

    // Create the VMR and add it to the filter graph.
    HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL,
                     CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVmr);
    if (SUCCEEDED(hr)) 
    {
        hr = pGB->AddFilter(pVmr, L"Video Mixing Renderer 9");
        if (SUCCEEDED(hr)) 
        {
            // Set the rendering mode and number of streams
            SmartPtr <IVMRFilterConfig9> pConfig;

            JIF(pVmr->QueryInterface(IID_IVMRFilterConfig9, (void**)&pConfig));
            JIF(pConfig->SetRenderingMode(VMR9Mode_Windowless));

            hr = pVmr->QueryInterface(IID_IVMRWindowlessControl9, (void**)&pWC);
            if( SUCCEEDED(hr)) 
            {
                hr = pWC->SetVideoClippingWindow(ghApp);
                hr = pWC->SetBorderColor(RGB(0,0,0));
            }

#ifndef BILINEAR_FILTERING
            // Request point filtering (instead of bilinear filtering)
            // to improve the text quality.  In general, if you are 
            // not scaling the app Image, you should use point filtering.
            // This is very important if you are doing source color keying.
            IVMRMixerControl9 *pMix;

            hr = pVmr->QueryInterface(IID_IVMRMixerControl9, (void**)&pMix);
            if( SUCCEEDED(hr)) 
            {
                DWORD dwPrefs=0;
                hr = pMix->GetMixingPrefs(&dwPrefs);

                if (SUCCEEDED(hr))
                {
                    dwPrefs |= MixerPref_PointFiltering;
                    dwPrefs &= ~(MixerPref_BiLinearFiltering);

                    hr = pMix->SetMixingPrefs(dwPrefs);
                }
                pMix->Release();
            }
#endif

            // Get alpha-blended bitmap interface
            hr = pVmr->QueryInterface(IID_IVMRMixerBitmap9, (void**)&pBMP);
        }
        else
            Msg(TEXT("Failed to add VMR to graph!  hr=0x%x\r\n"), hr);

        // Don't release the pVmr interface because we are copying it into
        // the caller's ppVmr9 pointer
        *ppVmr9 = pVmr;
    }
    else
        Msg(TEXT("Failed to create VMR!  hr=0x%x\r\n"), hr);

   return hr;
}


void OnPaint(HWND hwnd) 
{
    HRESULT hr;
    PAINTSTRUCT ps; 
    HDC         hdc; 
    RECT        rcClient; 

    GetClientRect(hwnd, &rcClient); 
    hdc = BeginPaint(hwnd, &ps); 

    if(pWC) 
    { 
        // When using VMR Windowless mode, you must explicitly tell the
        // renderer when to repaint the video in response to WM_PAINT
        // messages.  This is most important when the video is stopped
        // or paused, since the VMR won't be automatically updating the
        // window as the video plays.
        if (pWC)
            hr = pWC->RepaintVideo(hwnd, hdc);  
    } 
    else  // No video image. Just paint the whole client area. 
    { 
        FillRect(hdc, &rcClient, (HBRUSH)(COLOR_BTNFACE + 1)); 
    } 

    EndPaint(hwnd, &ps); 
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