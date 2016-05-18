//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            DSPlay.cpp
//
// Abstract:            Windows Media / DirectShow sample code
//                      A simple DirectShow-based player application
//                      for Windows Media content (ASF, WMV, WMA).  
//
//*****************************************************************************

#include <windows.h>
#include <commdlg.h>
#include <strmif.h>
#include <vfwmsgs.h>
#include <uuids.h>
#include "control.h"
#include "evcode.h"

#include <stdio.h>
#include <tchar.h>
#include <crtdbg.h>
#include <stddef.h> 
#include <strsafe.h>
#include <atlconv.h>

#include <wmsdkidl.h>

#include "dsplay.h"
#include "resource.h"
#include "nserror.h"

// 
// Constants and macros
//
#ifndef NUMELMS
   #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

const int AUDIO=1, VIDEO=2; // Used for enabling playback menu items

#pragma warning(disable: 4100)

//
// Global data
//
HWND      ghApp=0;
HMENU     ghMenu=0;
HINSTANCE ghInst=0;
TCHAR     g_szFileName[MAX_PATH]={0};
BOOL      g_bAudioOnly=FALSE, g_bFullscreen=FALSE;
LONG      g_lVolume=VOLUME_FULL;
PLAYSTATE g_psCurrent=Stopped;

// DirectShow interfaces
IGraphBuilder *pGB = NULL;
IMediaControl *pMC = NULL;
IMediaEventEx *pME = NULL;
IVideoWindow  *pVW = NULL;
IBasicAudio   *pBA = NULL;
IBasicVideo   *pBV = NULL;
IMediaSeeking *pMS = NULL;

//------------------------------------------------------------------------------
// Name: PlayMovieInWindow()
// Desc: Sets up the filter graph and plays the media file.
//
// szFile: Specifies the name of the file.
//
//------------------------------------------------------------------------------
HRESULT PlayMovieInWindow(__in_ecount(MAX_PATH) LPTSTR szFile)
{
    USES_CONVERSION;
    WCHAR wFile[MAX_PATH];
    WCHAR * pszFile = NULL;
    HRESULT hr;

    // Check input string
    if (!szFile)
        return E_POINTER;

    // Clear open dialog remnants before calling RenderFile()
    UpdateWindow(ghApp);

    // Convert filename to wide character string
    pszFile = T2W(szFile);
    if (!pszFile)
        return E_POINTER;
    (void)StringCchCopyW(wFile, NUMELMS(wFile), pszFile);

    // Get the interface for DirectShow's GraphBuilder
    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                         IID_IGraphBuilder, (void **)&pGB));

    // Get the media event interface before building the graph
    JIF(pGB->QueryInterface(IID_IMediaEventEx, (void **)&pME));

    // Have the graph builder construct the appropriate graph automatically
    JIF(pGB->RenderFile(wFile, NULL));

    if( SUCCEEDED( hr ) )
    {
        // QueryInterface for DirectShow interfaces
        JIF(pGB->QueryInterface(IID_IMediaControl, (void **)&pMC));
        JIF(pGB->QueryInterface(IID_IMediaSeeking, (void **)&pMS));

        // Query for video interfaces, which may not be relevant for audio files
        JIF(pGB->QueryInterface(IID_IVideoWindow, (void **)&pVW));
        JIF(pGB->QueryInterface(IID_IBasicVideo,  (void **)&pBV));

        // Query for audio interfaces, which may not be relevant for video-only files
        JIF(pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA));

        // Have the graph signal event via window callbacks for performance
        JIF(pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0));

        // Is this an audio-only file (no video component)?
        CheckVisibility();

        if (!g_bAudioOnly)
        {
            // Setup the video window
            JIF(pVW->put_Owner((OAHWND)ghApp));
            JIF(pVW->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN));
            JIF(InitVideoWindow(1, 1));
        }
        else
        {
            // Initialize the default player window and enable playback menu items
            // that don't involve manipulating video size
            JIF(InitPlayerWindow());
            EnablePlaybackMenu(TRUE, AUDIO);
        }

        // Complete window initialization
        CheckSizeMenu(ID_FILE_SIZE_NORMAL);
        ShowWindow(ghApp, SW_SHOWNORMAL);
        UpdateWindow(ghApp);
        SetForegroundWindow(ghApp);
        UpdateMainTitle();
        g_bFullscreen = FALSE;

        // Run the graph to play the media file
        JIF(pMC->Run());

        g_psCurrent = Running;
        SetFocus(ghApp);
    }

    return hr;
}

//------------------------------------------------------------------------------
// Name: InitVideoWindow()
// Desc: Sets the window size and position.
//
// nMultiplier, nDivider: These parameters give the ratio by which to stretch or
// shrink the video. For example, for 2X size, use (2,1). For 1/2 size, use (1,2).
//------------------------------------------------------------------------------
HRESULT InitVideoWindow(int nMultiplier, int nDivider)
{
    LONG lHeight, lWidth;
    HRESULT hr = S_OK;
    RECT rect;

    if (!pBV)
        return S_OK;

    // Read the default video size
    hr = pBV->GetVideoSize(&lWidth, &lHeight);
    if (hr == E_NOINTERFACE)
        return S_OK;

    // Enable all playback menu items
    EnablePlaybackMenu(TRUE, VIDEO);

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

    // Move the video size/position to within the application window
    GetClientRect(ghApp, &rect);
    JIF(pVW->SetWindowPosition(rect.left, rect.top, rect.right, rect.bottom));

    return hr;
}

//------------------------------------------------------------------------------
// Name: InitPlayerWindow()
// Desc: Sets the window size and position when there is no video.
//------------------------------------------------------------------------------
HRESULT InitPlayerWindow(void)
{
    // Reset to a default size for audio and after closing a clip
    SetWindowPos(ghApp, NULL, 0, 0,
                 DEFAULT_AUDIO_WIDTH,
                 DEFAULT_AUDIO_HEIGHT,
                 SWP_NOMOVE | SWP_NOOWNERZORDER);

    // Check the default 'normal size' menu item
    CheckSizeMenu(ID_FILE_SIZE_NORMAL);

    // Disable the playback menu items
    EnablePlaybackMenu(FALSE, 0);

    return S_OK;
}

//------------------------------------------------------------------------------
// Name: MoveVideoWindow()
// Desc: Track the movement of the container window and resize as needed.
//------------------------------------------------------------------------------
void MoveVideoWindow(void)
{
    HRESULT hr;
    
    
    if(pVW)
    {
        RECT client;
        GetClientRect(ghApp, &client);

        hr = pVW->SetWindowPosition(client.left, client.top, 
                                    client.right, client.bottom);
    }
}

//------------------------------------------------------------------------------
// Name: CheckVisibility()
// Desc: Set global values for presence of video window.
//------------------------------------------------------------------------------
void CheckVisibility(void)
{
    long lVisible;
    HRESULT hr;

    if ((!pVW) || (!pBV))
    {
        // Audio-only files have no video interfaces.  This might also
        // be a file whose video component uses an unknown video codec.
        g_bAudioOnly = TRUE;
        return;
    }
    else
    {
        // Clear the global flag
        g_bAudioOnly = FALSE;
    }

    hr = pVW->get_Visible(&lVisible);
    if (FAILED(hr))
    {
        // If this is an audio-only clip, get_Visible() won't work.
        if (hr == E_NOINTERFACE)
        {
            g_bAudioOnly = TRUE;
        }
        else
        {
            Msg(TEXT("Failed(%08lx) in pVW->get_Visible()!\r\n"), hr);
        }
    }
}

//------------------------------------------------------------------------------
// Name: PauseClip()
// Desc: Toggle play/pause state.
//------------------------------------------------------------------------------
void PauseClip(void)
{
    if (!pMC)
        return;

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

//------------------------------------------------------------------------------
// Name: StopClip()
// Desc: Stop and reset position to beginning.
//------------------------------------------------------------------------------
void StopClip(void)
{
    HRESULT hr;
    
    if ((!pMC) || (!pMS))
        return;

    // Stop and reset position to beginning
    if((g_psCurrent == Paused) || (g_psCurrent == Running))
    {
        hr = pMC->Stop();
        g_psCurrent = Stopped;

        // Seek to the beginning
        LONGLONG pos = 0;
        hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning,
                               NULL, AM_SEEKING_NoPositioning);

        // Display the first frame to indicate the reset condition
        hr = pMC->Pause();
    }

    UpdateMainTitle();
}

//------------------------------------------------------------------------------
// Name: OpenClip()
// Desc: Open the file and play it.
//------------------------------------------------------------------------------
void OpenClip()
{
    HRESULT hr;

    // If no filename was specified by command line, show file open dialog
    if(g_szFileName[0] == L'\0')
    {
        TCHAR szFilename[MAX_PATH];

        UpdateMainTitle();

        // If no filename was specified on the command line, then our video
        // window has not been created or made visible.  Make our main window
        // visible and bring to the front to allow file selection.
        InitPlayerWindow();
        ShowWindow(ghApp, SW_SHOWNORMAL);
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
        if (_tcsstr(szFilename, TEXT(".asx")) || _tcsstr(szFilename, TEXT(".ASX")))
        {
            Msg(TEXT("ASX Playlists are not supported by this application.\n\n")
                TEXT("Please select a valid media file.\0"));
            return;
        }

        (void)StringCchCopy(g_szFileName, NUMELMS(g_szFileName), szFilename);
    }

    // Reset status variables
    g_psCurrent = Stopped;
    g_lVolume = VOLUME_FULL;
    
    // Start playing the media file
    hr = PlayMovieInWindow(g_szFileName);

    // If we couldn't play the clip, clean up
    if (FAILED(hr))
    {
        CloseClip();
        
        if (hr == NS_E_LICENSE_REQUIRED)
            Msg(TEXT("This media file requires a license."));
    }
}


//------------------------------------------------------------------------------
// Name: GetClipFileName()
// Desc: Get a file name from the user.
//------------------------------------------------------------------------------
BOOL GetClipFileName(__out_ecount(MAX_PATH) LPTSTR szName)
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

//------------------------------------------------------------------------------
// Name: CloseClip()
// Desc: Stops playback and does some cleanup.
//------------------------------------------------------------------------------
void CloseClip()
{
    HRESULT hr;

    // Stop media playback
    if(pMC)
        hr = pMC->Stop();

    // Clear global flags
    g_psCurrent = Stopped;
    g_bAudioOnly = TRUE;
    g_bFullscreen = FALSE;

    // Free DirectShow interfaces
    CloseInterfaces();

    // Clear file name to allow selection of new file with open dialog
    g_szFileName[0] = L'\0';

    // No current media state
    g_psCurrent = Init;

    // Reset the player window
    RECT rect;
    GetClientRect(ghApp, &rect);
    InvalidateRect(ghApp, &rect, TRUE);

    UpdateMainTitle();
    InitPlayerWindow();
}

//------------------------------------------------------------------------------
// Name: CloseInterfaces()
// Desc: Releases interfaces.
//------------------------------------------------------------------------------
void CloseInterfaces(void)
{
    HRESULT hr;
    
    // Relinquish ownership (IMPORTANT!) after hiding video window
    if(pVW)
    {
        hr = pVW->put_Visible(0);
        hr = pVW->put_Owner(NULL);
    }

    // Disable event callbacks
    if (pME)
        hr = pME->SetNotifyWindow((OAHWND)NULL, 0, 0);

    // Release and zero DirectShow interfaces
    SAFE_RELEASE(pME);
    SAFE_RELEASE(pMS);
    SAFE_RELEASE(pMC);
    SAFE_RELEASE(pBA);
    SAFE_RELEASE(pBV);
    SAFE_RELEASE(pVW);
    SAFE_RELEASE(pGB);
}

//------------------------------------------------------------------------------
// Name: Msg()
// Desc: Displays the specified message in a Message Box.
//------------------------------------------------------------------------------
void Msg(__in LPCTSTR szFormat, ...)
{
    TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
    
    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    (void)StringCchVPrintf(szBuffer, NUMELMS(szBuffer), szFormat, pArgs);
            
    va_end(pArgs);

    // Display a message box with the formatted string
    MessageBox(NULL, szBuffer, TEXT("DSPlay Sample"), MB_OK);
}

//------------------------------------------------------------------------------
// Name: ToggleMute()
// Desc: Switch between minimum and full volume.
//------------------------------------------------------------------------------
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


//------------------------------------------------------------------------------
// Name: UpdateMainTitle()
// Desc: Displays info about the media file on the window's title bar.
//------------------------------------------------------------------------------
void UpdateMainTitle(void)
{
    TCHAR szTitle[MAX_PATH], szFile[MAX_PATH];

    // If no file is loaded, just show the application title
    if (g_szFileName[0] == L'\0')
    {
        (void)StringCchCopy(szTitle, NUMELMS(szTitle), APPLICATIONNAME);
    }

    // Otherwise, show useful information
    else
    {
        // Get file name without full path
        GetFilename(g_szFileName, szFile);

        // Update the window title to show filename and play state
        (void)StringCchPrintf(szTitle, 
                              NUMELMS(szTitle),
                              TEXT("%s [%s] %s%s"),
                              szFile,
                              g_bAudioOnly ? TEXT("Audio") : TEXT("Video"),
                              (g_lVolume == VOLUME_SILENCE) ? TEXT("(Muted)") : TEXT(""),
                              (g_psCurrent == Paused) ? TEXT("(Paused)") : TEXT(""));
    }

    SetWindowText(ghApp, szTitle);
}

//------------------------------------------------------------------------------
// Name: GetFilename()
// Desc: Strip path and return just the file's name.
//------------------------------------------------------------------------------
void GetFilename(__in LPCTSTR pszFull, __out_ecount(MAX_PATH) LPTSTR pszFile)
{
    int nLength;
    TCHAR szPath[MAX_PATH]={0};
    BOOL bSetFilename=FALSE;

    // Strip path and return just the file's name
    (void)StringCchCopy(szPath, NUMELMS(szPath), pszFull);
    
    nLength = (int) _tcslen(szPath);

    for (int i=nLength-1; i>=0; i--)
    {
        if ((szPath[i] == '\\') || (szPath[i] == '/'))
        {
            szPath[i] = '\0';
            (void)StringCchCopy(pszFile, MAX_PATH, &szPath[i+1]);
            bSetFilename = TRUE;
            break;
        }
    }

    // If there was no path given (just a file name), then
    // just copy the full path to the target path.
    if (!bSetFilename)
        (void)StringCchCopy(pszFile, MAX_PATH, pszFull);
        
    pszFile[MAX_PATH-1] = 0;        // Ensure null-termination
}

//------------------------------------------------------------------------------
// Name: ToggleFullScreen()
// Desc: Switch between fullscreen and windowed mode.
//------------------------------------------------------------------------------
HRESULT ToggleFullScreen(void)
{
    HRESULT hr=S_OK;
    LONG lMode;
    static HWND hDrain=0;

    // Don't bother with full-screen for audio-only files
    if ((g_bAudioOnly) || (!pVW))
        return S_OK;

    // Read current state
    JIF(pVW->get_FullScreenMode(&lMode));

    if (lMode == 0)  /* OAFALSE */
    {
        // Save current message drain
        LIF(pVW->get_MessageDrain((OAHWND *) &hDrain));

        // Set message drain to application main window
        LIF(pVW->put_MessageDrain((OAHWND) ghApp));

        // Switch to full-screen mode
        lMode = -1;  /* OATRUE */
        JIF(pVW->put_FullScreenMode(lMode));
        g_bFullscreen = TRUE;
    }
    else
    {
        // Switch back to windowed mode
        lMode = 0;  /* OAFALSE */
        JIF(pVW->put_FullScreenMode(lMode));

        // Undo change of message drain
        LIF(pVW->put_MessageDrain((OAHWND) hDrain));

        // Reset video window
        LIF(pVW->SetWindowForeground(-1));

        // Reclaim keyboard focus for player application
        UpdateWindow(ghApp);
        SetForegroundWindow(ghApp);
        SetFocus(ghApp);
        g_bFullscreen = FALSE;
    }

    return hr;
}

//------------------------------------------------------------------------------
// Name: HandleGraphEvent()
// Desc: Handle pending filter graph events.
//------------------------------------------------------------------------------
HRESULT HandleGraphEvent(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    // Make sure that we don't access the media event interface
    // after it has already been released.
    if (!pME)
        return S_OK;

    // Process all queued events
    while(SUCCEEDED(pME->GetEvent(&evCode, (LONG_PTR *) &evParam1,
                    (LONG_PTR *) &evParam2, 0)))
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
                // If seeking failed, just stop and restart to reset playback
                if (FAILED(hr = pMC->Stop()))
                {
                    Msg(TEXT("Failed(0x%08lx) to stop media clip!\r\n"), hr);
                    break;
                }

                if (FAILED(hr = pMC->Run()))
                {
                    Msg(TEXT("Failed(0x%08lx) to reset media clip!\r\n"), hr);
                    break;
                }
            }
        }
        else if(EC_ERRORABORT == evCode)
        {
            Msg(TEXT("Playback error (EC_ERRORABORT - 0x%08lx). Aborting...\r\n"), evParam1);

            if (FAILED(hr = pMC->Stop()))
                Msg(TEXT("Failed(0x%08lx) to stop media clip!\r\n"), hr);

            hr = (HRESULT) evParam1;
            break;
        }
    }

    // Free memory associated with callback
    if( FAILED(pME->FreeEventParams(evCode, evParam1, evParam2)) )
    {
        CloseClip();
    }

    return hr;
}

//------------------------------------------------------------------------------
// Name: CheckSizeMenu()
// Desc: Sets or clears checkboxes that specify the size of the video clip.
//------------------------------------------------------------------------------

void CheckSizeMenu(WPARAM wParam)
{
    WPARAM nItems[4] = {ID_FILE_SIZE_HALF,    ID_FILE_SIZE_DOUBLE, 
                        ID_FILE_SIZE_NORMAL,  ID_FILE_SIZE_THREEQUARTER};

    // Set/clear checkboxes that indicate the size of the video clip
    for (int i=0; i<4; i++)
    {
        // Check the selected item
        CheckMenuItem(ghMenu, (UINT) nItems[i],
                     (UINT) (wParam == nItems[i]) ? MF_CHECKED : MF_UNCHECKED);
    }
}

//------------------------------------------------------------------------------
// Name: EnablePlaybackMenu()
// Desc: Sets or clears items on the Control menu.
//------------------------------------------------------------------------------
void EnablePlaybackMenu(BOOL bEnable, int nMediaType)
{
    WPARAM nItems[8] = {ID_FILE_PAUSE,        ID_FILE_STOP,         ID_FILE_MUTE,
                        ID_FILE_SIZE_HALF,    ID_FILE_SIZE_DOUBLE,  
                        ID_FILE_SIZE_NORMAL,  ID_FILE_SIZE_THREEQUARTER, 
                        ID_FILE_FULLSCREEN};

    // Enable/disable menu items related to playback
    EnableMenuItem(ghMenu, (UINT) ID_FILE_PAUSE, 
                  (UINT) (bEnable) ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, (UINT) ID_FILE_STOP, 
                  (UINT) (bEnable) ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, (UINT) ID_FILE_MUTE, 
                  (UINT) (bEnable) ? MF_ENABLED : MF_GRAYED);

    // Enable/disable menu items related to video size
    for (int i=3; i<8; i++)
    {
        // Check the selected item
        EnableMenuItem(ghMenu, (UINT) nItems[i],
                     (UINT) (nMediaType == VIDEO) ? MF_ENABLED : MF_GRAYED);
    }
}

//------------------------------------------------------------------------------
// Name: AboutDlgProc()
// Desc: Message handler for About dialog box.
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
// Name: WndMainProc()
// Desc: Main window procedure.
//------------------------------------------------------------------------------
LRESULT CALLBACK WndMainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        // Resize the video when the window changes
        case WM_MOVE:
        case WM_SIZE:
            if ((hWnd == ghApp) && (!g_bAudioOnly))
                MoveVideoWindow();
            break;

        // Enforce a minimum window size
        case WM_GETMINMAXINFO:
            {
                LPMINMAXINFO lpmm = (LPMINMAXINFO) lParam;
                lpmm->ptMinTrackSize.x = MINIMUM_VIDEO_WIDTH;
                lpmm->ptMinTrackSize.y = MINIMUM_VIDEO_HEIGHT;
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

                case 'F':
                case VK_RETURN:
                    ToggleFullScreen();
                    break;

               case 'H':
                    InitVideoWindow(1,2);
                    CheckSizeMenu(wParam);
                    break;
                case 'N':
                    InitVideoWindow(1,1);
                    CheckSizeMenu(wParam);
                    break;
                case 'D':
                    InitVideoWindow(2,1);
                    CheckSizeMenu(wParam);
                    break;
                case 'T':
                    InitVideoWindow(3,4);
                    CheckSizeMenu(wParam);
                    break;

                case VK_ESCAPE:
                    if (g_bFullscreen)
                        ToggleFullScreen();
                    else
                        CloseClip();
                    break;

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

                case ID_FILE_FULLSCREEN:
                    ToggleFullScreen();
                    break;

                case ID_HELP_ABOUT:
                    DialogBox(ghInst, MAKEINTRESOURCE(IDD_ABOUTBOX), 
                              ghApp,  (DLGPROC) AboutDlgProc);
                    break;

                case ID_FILE_SIZE_HALF:
                    InitVideoWindow(1,2);
                    CheckSizeMenu(wParam);
                    break;
                case ID_FILE_SIZE_NORMAL:
                    InitVideoWindow(1,1);
                    CheckSizeMenu(wParam);
                    break;
                case ID_FILE_SIZE_DOUBLE:
                    InitVideoWindow(2,1);
                    CheckSizeMenu(wParam);
                    break;
                case ID_FILE_SIZE_THREEQUARTER:
                    InitVideoWindow(3,4);
                    CheckSizeMenu(wParam);
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

    // Pass this message to the video window for notification of system changes
    if (pVW)
        pVW->NotifyOwnerMessage((LONG_PTR) hWnd, message, wParam, lParam);

    return DefWindowProc(hWnd, message, wParam, lParam);
}

//------------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.
//------------------------------------------------------------------------------
int PASCAL WinMain(__in HINSTANCE hInstC, __in_opt HINSTANCE hInstP, __in_opt LPSTR lpCmdLine, __in int nCmdShow)
{
    MSG msg={0};
    WNDCLASS wc;

    // Initialize COM
    if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
    {
        Msg(TEXT("CoInitialize Failed!\r\n"));
        exit(1);
    }

    // Was a filename specified on the command line?
    if((NULL != lpCmdLine) && (lpCmdLine[0] != '\0'))
    {
        USES_CONVERSION;
        (void)StringCchCopy(g_szFileName, NUMELMS(g_szFileName), A2T(lpCmdLine) );        
    }
    g_szFileName[MAX_PATH-1] = 0;       // Ensure null-termination    


    // Set initial media state.  This value will update when playing, 
    // paused, or stopped.
    g_psCurrent = Init;

    // Register the window class
    ZeroMemory(&wc, sizeof wc);
    wc.lpfnWndProc = WndMainProc;
    ghInst = wc.hInstance = hInstC;
    wc.lpszClassName = CLASSNAME;
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon(hInstC, MAKEINTRESOURCE(IDI_DSPLAY));
    if(!RegisterClass(&wc))
    {
        Msg(TEXT("RegisterClass Failed! Error=0x%x\r\n"), GetLastError());
        CoUninitialize();
        exit(1);
    }

    // Create the main window.  The WS_CLIPCHILDREN style is required.
    // Because we will resize the application window to fit any media file
    // that is specified on the command line, don't make the window visible.
    // It will be resized and made visible once a media file is rendered.
    ghApp = CreateWindow(CLASSNAME, APPLICATIONNAME,
                    WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN,
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    0, 0, ghInst, 0);

    if(ghApp)
    {
        // Save menu handle for later use
        ghMenu = GetMenu(ghApp);

        // Open the specified media file or prompt for a title
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


