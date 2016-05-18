//------------------------------------------------------------------------------
// File: Watermark.cpp
//
// Desc: DirectShow sample code - a simple audio/video media file player app.
//       Using the DirectX 9 Video Mixing Renderer, a static image is
//       alpha blended with the running video in the corner of the screen.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <tchar.h>
#include <dshow.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <strsafe.h>

#include "watermark.h"
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
LONG      g_lVolume=VOLUME_FULL;
DWORD     g_dwGraphRegister=0;
PLAYSTATE g_psCurrent=Init;
RECT      g_rcDest={0};

// DirectShow interfaces
IGraphBuilder *pGB = NULL;
IMediaControl *pMC = NULL;
IMediaEventEx *pME = NULL;
IBasicAudio   *pBA = NULL;
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
        JIF(pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA));

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

        // Add the bitmap to the VMR's input
        BlendApplicationImage(ghApp);

        // Have the graph signal event via window callbacks for performance
        JIF(pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0));

        // Complete the window setup
        ShowWindow(ghApp, SW_SHOWNORMAL);
        UpdateWindow(ghApp);
        SetForegroundWindow(ghApp);
        SetFocus(ghApp);
        UpdateMainTitle();

#ifdef REGISTER_FILTERGRAPH
        hr = AddGraphToRot(pGB, &g_dwGraphRegister);
        if (FAILED(hr))
        {
            Msg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
            g_dwGraphRegister = 0;
        }
#endif

        // Run the graph to play the media file
        JIF(pMC->Run());
        g_psCurrent=Running;
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

    EnablePlaybackMenu(TRUE);

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

    return hr;
}


HRESULT InitPlayerWindow(void)
{
    // Reset to a default size for audio and after closing a clip
    SetWindowPos(ghApp, NULL, 0, 0,
                 DEFAULT_AUDIO_WIDTH,
                 DEFAULT_AUDIO_HEIGHT,
                 SWP_NOMOVE | SWP_NOOWNERZORDER);

    EnablePlaybackMenu(FALSE);
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


void PauseClip(void)
{
    if (!pMC)
        return;

    // Toggle play/pause behavior
    if((g_psCurrent == Paused) || (g_psCurrent == Stopped))
    {
        if (SUCCEEDED(pMC->Run()))
            g_psCurrent = Running;
    }
    else
    {
        if (SUCCEEDED(pMC->Pause()))
            g_psCurrent = Paused;
    }

    UpdateMainTitle();
}


void StopClip(void)
{
    HRESULT hr;

    if ((!pMC) || (!pMS))
        return;

    // Stop and reset postion to beginning
    if((g_psCurrent == Paused) || (g_psCurrent == Running))
    {
        LONGLONG pos = 0;
        hr = pMC->Stop();
        g_psCurrent = Stopped;

        // Seek to the beginning
        hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                               NULL, AM_SEEKING_NoPositioning);

        // Display the first frame to indicate the reset condition
        hr = pMC->Pause();
    }

    UpdateMainTitle();
}


void OpenClip()
{
    HRESULT hr;

    // If no filename specified by command line, show file open dialog
    if(g_szFileName[0] == L'\0')
    {
        TCHAR szFilename[MAX_PATH];

        UpdateMainTitle();

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

        StringCchCopy(g_szFileName, NUMELMS(g_szFileName), szFilename);
    }

    // Reset status variables
    g_psCurrent = Stopped;
    g_lVolume = VOLUME_FULL;
    EnableWatermarkMenu(TRUE);

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
    ofn.lpstrTitle        = TEXT("Open Media File...\0");
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

    // Clear global flags
    g_psCurrent = Init;     // No current media state

    // Free DirectShow interfaces
    CloseInterfaces();

    // Clear file name to allow selection of new file with open dialog
    g_szFileName[0] = L'\0';

    // Reset the player window
    RECT rect;
    GetClientRect(ghApp, &rect);
    InvalidateRect(ghApp, &rect, TRUE);

    EnableWatermarkMenu(FALSE);
    UpdateMainTitle();
    InitPlayerWindow();
}


void CloseInterfaces(void)
{
    HRESULT hr;

    // Disable event callbacks
    if (pME)
        hr = pME->SetNotifyWindow((OAHWND)NULL, 0, 0);

#ifdef REGISTER_FILTERGRAPH
    if (g_dwGraphRegister)
    {
        RemoveGraphFromRot(g_dwGraphRegister);
        g_dwGraphRegister = 0;
    }
#endif

    // Clear watermark state and timer settings
    ClearWatermarkState();

    // Release and zero DirectShow interfaces
    SAFE_RELEASE(pME);
    SAFE_RELEASE(pMS);
    SAFE_RELEASE(pMC);
    SAFE_RELEASE(pBA);
    SAFE_RELEASE(pBMP);
    SAFE_RELEASE(pWC);
    SAFE_RELEASE(pGB);
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
    MessageBox(NULL, szBuffer, TEXT("Watermark Sample"), MB_OK);
}


HRESULT ToggleMute(void)
{
    HRESULT hr=S_OK;

    if ((!pGB) || (!pBA))
        return S_OK;

    // Read current volume
    hr = pBA->get_Volume(&g_lVolume);
    if (hr == E_NOTIMPL)
    {
        // Fail quietly if this is a video-only media file
        return S_OK;
    }
    else if (FAILED(hr))
    {
        Msg(TEXT("Failed to read audio volume!  hr=0x%x\r\n"), hr);
        return hr;
    }

    // Switch volume levels
    if (g_lVolume == VOLUME_FULL)
        g_lVolume = VOLUME_SILENCE;
    else
        g_lVolume = VOLUME_FULL;

    // Set new volume
    JIF(pBA->put_Volume(g_lVolume));

    UpdateMainTitle();
    return hr;
}


void UpdateMainTitle(void)
{
    TCHAR szTitle[MAX_PATH]={0}, szFile[MAX_PATH]={0};
    HRESULT hr;

    // If no file is loaded, just show the application title
    if (g_szFileName[0] == L'\0')
    {
        hr = StringCchCopy(szTitle, NUMELMS(szTitle), APPLICATIONNAME);
    }

    // Otherwise, show useful information
    else
    {
        // Get file name without full path
        GetFilename(g_szFileName, szFile);

        // Update the window title to show filename and play state
        hr = StringCchPrintf(szTitle, NUMELMS(szTitle), TEXT("Watermark - %s %s%s\0\0"),
                szFile,
                (g_lVolume == VOLUME_SILENCE) ? TEXT("(Muted)\0") : TEXT("\0"),
                (g_psCurrent == Paused) ? TEXT("(Paused)\0") : TEXT("\0"));
    }

    SetWindowText(ghApp, szTitle);
}


void GetFilename(TCHAR *pszFull, TCHAR *pszFile)
{
    int nLength;
    TCHAR szPath[MAX_PATH]={0};
    BOOL bSetFilename=FALSE;

    // Strip path and return just the file's name
    (void)StringCchCopy(szPath, MAX_PATH, pszFull);
    szPath[MAX_PATH-1] = 0;

    nLength = (int) _tcslen(szPath);

    for (int i=nLength-1; i>=0; i--)
    {
        if ((szPath[i] == '\\') || (szPath[i] == '/'))
        {
            szPath[i] = '\0';
            StringCchCopy(pszFile, MAX_PATH, &szPath[i+1]);
            bSetFilename = TRUE;
            break;
        }
    }

    // If there was no path given (just a file name), then
    // just copy the full path to the target path.
    if (!bSetFilename)
        (void)StringCchCopy(pszFile, MAX_PATH, pszFull );

    pszFile[MAX_PATH-1] = 0;        // Ensure null-termination
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

        // Free memory associated with callback, since we're not using it
        hr = pME->FreeEventParams(evCode, evParam1, evParam2);
    }

    return hr;
}


void EnablePlaybackMenu(BOOL bEnable)
{
    const int NUM_PLAYBACK_ITEMS=3;
    WPARAM nItems[NUM_PLAYBACK_ITEMS] =
                 {ID_FILE_PAUSE, ID_FILE_STOP, ID_FILE_MUTE};

    // Set/clear checkboxes that indicate the size of the video clip
    for (int i=0; i<NUM_PLAYBACK_ITEMS; i++)
    {
        // Check the selected item
        EnableMenuItem(ghMenu, (UINT) nItems[i],
                      (UINT) (bEnable) ? MF_ENABLED : MF_GRAYED);
    }
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
                case 'P':
                    PauseClip();
                    break;

                case 'S':
                    StopClip();
                    break;

                case 'M':
                    ToggleMute();
                    break;

                case VK_ESCAPE:
                case VK_F12:
                case 'Q':
                case 'X':
                    CloseClip();
                    break;
            }
            break;

        case WM_COMMAND:

            switch(wParam)
            { // Menus

                case ID_FILE_OPENCLIP:
                    // If we have ANY file open, close it and shut down DirectShow
                    if (g_psCurrent != Init)
                        CloseClip();

                    // Open the new clip
                    OpenClip();
                    break;

                case ID_FILE_EXIT:
                    CloseClip();
                    PostQuitMessage(0);
                    break;

                case ID_FILE_PAUSE:
                    PauseClip();
                    break;

                case ID_FILE_STOP:
                    StopClip();
                    break;

                case ID_FILE_CLOSE:
                    CloseClip();
                    break;

                case ID_FILE_MUTE:
                    ToggleMute();
                    break;

                case ID_FLIP:
                    FlipFlag(MARK_FLIP);
                    FlipWatermark(g_dwWatermarkFlags);
                    break;

                case ID_MIRROR:
                    FlipFlag(MARK_MIRROR);
                    MirrorWatermark(g_dwWatermarkFlags);
                    break;

                case ID_DISABLE:
                    FlipFlag(MARK_DISABLE);
                    DisableWatermark(g_dwWatermarkFlags);
                    break;

                case ID_ANIMATE:
                    FlipFlag(MARK_ANIMATE);
                    AnimateWatermark(g_dwWatermarkFlags);
                    break;

                case ID_SLIDE:
                    FlipFlag(MARK_SLIDE);
                    SlideWatermark(g_dwWatermarkFlags);
                    break;

                case ID_STROBE:
                    FlipFlag(MARK_STROBE);
                    StrobeWatermark(g_dwWatermarkFlags);
                    break;

                case ID_ALL_EFFECTS:
                    SetAllEffects();
                    break;

                case ID_NO_EFFECTS:
                    ClearAllEffects();
                    break;

                case ID_HELP_ABOUT:
                    DialogBox(ghInst, MAKEINTRESOURCE(IDD_ABOUTBOX),
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


int PASCAL wWinMain(HINSTANCE hInstC, HINSTANCE hInstP, LPWSTR lpCmdLine, int nCmdShow)
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

    // Set initial media state
    g_psCurrent = Init;

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
                         DEFAULT_AUDIO_WIDTH, DEFAULT_AUDIO_HEIGHT,
                         0, 0, ghInst, 0);

    if(ghApp)
    {
        // Save menu handle for later use
        ghMenu = GetMenu(ghApp);
        EnablePlaybackMenu(FALSE);

        // If a media file was specified on the command line, open it now.
        // (If the first character in the string isn't NULL, post an open clip message.)
        if (g_szFileName[0] != 0)
            PostMessage(ghApp, WM_COMMAND, ID_FILE_OPENCLIP, 0);

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

            // Get alpha-blended bitmap interface
            hr = pVmr->QueryInterface(IID_IVMRMixerBitmap9, (void**)&pBMP);

        }

        // Don't release the pVmr interface because we are copying it into
        // the caller's ppVmr9 pointer
        *ppVmr9 = pVmr;
    }

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