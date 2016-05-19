// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include "stdafx.h"
#include <commctrl.h>
#include <shlwapi.h>
#include "MediaPlayer.h"
#include "Resource.h"


#define progressTimer   0x1000

// Global Variables:
HINSTANCE g_hInstance;
CMediaPlayer *g_MediaPlayer;

// Forward declarations of functions included in this code module:
INT_PTR CALLBACK MediaPlayerDialogProc(HWND, UINT, WPARAM, LPARAM);

bool IsWin7OrLater()
{
    bool bWin7OrLater = true;

    OSVERSIONINFO ver = {};
    ver.dwOSVersionInfoSize = sizeof(ver);

    if (GetVersionEx(&ver))
    {
        bWin7OrLater = (ver.dwMajorVersion > 6) ||
                       ((ver.dwMajorVersion == 6) && (ver.dwMinorVersion >= 1));
    }

    return bWin7OrLater;
}

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE /*hPrevInstance*/,
                      LPWSTR    /*lpCmdLine*/,
                      int       /*nCmdShow*/)
{
    if (!IsWin7OrLater())
    {
        MessageBox(NULL, L"This sample requires Windows 7 or later", L"Incompatible OS Version", MB_OK);
        return 0;
    }

    CoInitialize(NULL);

    static const INITCOMMONCONTROLSEX commonCtrls =
    {
        sizeof(INITCOMMONCONTROLSEX),
        ICC_STANDARD_CLASSES | ICC_BAR_CLASSES
    };
    InitCommonControlsEx(&commonCtrls);

    g_hInstance = hInstance; // Store instance handle in our global variable

    INT_PTR id = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MEDIAPLAYER), NULL, MediaPlayerDialogProc);
    CoUninitialize();

    return (int) id;
}

//
//  FUNCTION: MediaPlayerDialogProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
INT_PTR CALLBACK MediaPlayerDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL handled = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        SendMessage(GetDlgItem(hWnd, IDC_SLIDER_PLAYBACKPOS), TBM_SETRANGE, TRUE, MAKELONG(0, 1000));
        SendMessage(GetDlgItem(hWnd, IDC_SLIDER_PLAYBACKPOS), TBM_SETTICFREQ, 10, 0);

        SendMessage(GetDlgItem(hWnd, IDC_SLIDER_VOLUME), TBM_SETRANGE, TRUE, MAKELONG(0, 100));
        SendMessage(GetDlgItem(hWnd, IDC_SLIDER_VOLUME), TBM_SETTICFREQ, 10, 0);

        EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PLAY), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_STOP), FALSE);

        SHAutoComplete(GetDlgItem(hWnd, IDC_EDIT_FILENAME), SHACF_FILESYSTEM | SHACF_USETAB);

        g_MediaPlayer = new (std::nothrow) CMediaPlayer(hWnd);
        if (g_MediaPlayer == NULL)
        {
            MessageBox(hWnd, L"Unable to allocate media player", L"Initialization Failure", MB_OK);
            EndDialog(hWnd, -1);
			break;
        }
        HRESULT hr = g_MediaPlayer->Initialize();
        if (FAILED(hr))
        {
            MessageBox(hWnd, L"Unable to initialize media player", L"Initialization Failure", MB_OK);
            EndDialog(hWnd, -1);
			break;
        }

        float volume = g_MediaPlayer->GetVolume();
        SendMessage(GetDlgItem(hWnd, IDC_SLIDER_VOLUME), TBM_SETPOS, TRUE, static_cast<int>(volume * 100.0f));

        bool mute = g_MediaPlayer->GetMute();
        CheckDlgButton(hWnd, IDC_CHECK_MUTE, mute ? BST_CHECKED : BST_UNCHECKED);
        break;
    }

    case WM_DESTROY:
        g_MediaPlayer->Shutdown();
        g_MediaPlayer->Release();
        g_MediaPlayer = NULL;
        break;

    case WM_COMMAND:
    {
        int wmId    = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        // Parse the menu selections:
        switch (wmId)
        {
        case IDOK:
        case IDCANCEL:
            //
            //  Stop on Cancel/OK.
            //
            EndDialog(hWnd, TRUE);
            handled = TRUE;
            break;
            
        //
        //  The user checked the "Pause on Duck" option - ask the media player to sync to that state.
        //
        case IDC_CHECK_PAUSE_ON_DUCK:
            g_MediaPlayer->SyncPauseOnDuck(IsDlgButtonChecked(hWnd, IDC_CHECK_PAUSE_ON_DUCK) == BST_CHECKED);
            break;
            
        //
        //  The user checked the "Opt Out" option - ask the media player to sync to that state.
        //
        case IDC_CHECK_DUCKING_OPT_OUT:
            g_MediaPlayer->SyncDuckingOptOut(IsDlgButtonChecked(hWnd, IDC_CHECK_DUCKING_OPT_OUT) == BST_CHECKED);
            break;

        //
        //  See if the user navigated away from the filename edit control - if so, load the file in the filename edit control.
        //
        case IDC_EDIT_FILENAME:
            if (wmEvent == EN_KILLFOCUS)
            {
                wchar_t fileName[MAX_PATH];
                //
                //  If we're playing (the stop button is enabled), stop playing.
                //
                if (IsWindowEnabled(GetDlgItem(hWnd, IDC_BUTTON_STOP)))
                {
                    g_MediaPlayer->Stop();
                    KillTimer(hWnd, progressTimer);
                    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PLAY), TRUE);
                    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), FALSE);
                    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_STOP), FALSE);
                }

                GetDlgItemText(hWnd, IDC_EDIT_FILENAME, fileName, ARRAYSIZE(fileName));
                if (g_MediaPlayer->SetFileName(fileName))
                {
                    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PLAY), TRUE);
                    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), FALSE);
                    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_STOP), FALSE);
                }
            }
            break;

        //
        //  If the user hit the "Browse" button, bring up the file common dialog box.
        //
        //  If the user hit "OK" to the dialog then update the edit control to include the filename and load
        //  the file into the player.
        //
        case IDC_BUTTON_BROWSE:
        {
            wchar_t fileName[MAX_PATH];
            OPENFILENAME openFileName = {0};
            openFileName.lStructSize = sizeof(openFileName);
            openFileName.lpstrFile = fileName;
            openFileName.nMaxFile = ARRAYSIZE(fileName);
            fileName[0] = L'\0';
            openFileName.Flags = OFN_ENABLESIZING | OFN_FILEMUSTEXIST;
            openFileName.hInstance = g_hInstance;
            openFileName.hwndOwner = hWnd;
            
            if (GetOpenFileName(&openFileName))
            {
                SetDlgItemText(hWnd, IDC_EDIT_FILENAME, openFileName.lpstrFile);
                //
                //  If we're playing (the stop button is enabled), stop playing.
                //
                if (IsWindowEnabled(GetDlgItem(hWnd, IDC_BUTTON_STOP)))
                {
                    g_MediaPlayer->Stop();
                    KillTimer(hWnd, progressTimer);
                    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PLAY), TRUE);
                    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), FALSE);
                    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_STOP), FALSE);
                }

                g_MediaPlayer->SetFileName(fileName);
                EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PLAY), TRUE);
                EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), FALSE);
                EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_STOP), FALSE);
            }
            break;
        }
        //
        //  The user hit the "Play" button.
        //
        //  Sync the "Pause On Duck" and "Ducking Opt Out" buttons with the player and then start playback.
        //
        //
        //  Then disable the "Play" button and enable the "Pause" and "Stop" buttons.
        //
        case IDC_BUTTON_PLAY:
        {
            g_MediaPlayer->SyncPauseOnDuck(IsDlgButtonChecked(hWnd, IDC_CHECK_PAUSE_ON_DUCK) == BST_CHECKED);
            g_MediaPlayer->SyncDuckingOptOut(IsDlgButtonChecked(hWnd, IDC_CHECK_DUCKING_OPT_OUT) == BST_CHECKED);
            g_MediaPlayer->Play();
            SetTimer(hWnd, progressTimer, 40, NULL);
            EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PLAY), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), TRUE);
            EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_STOP), TRUE);
            break;
        }
        //
        //  The user hit the "Pause/Continue" button.
        //
        //  Toggle the "Pause" state in the player and update the button text as appropriate.
        //
        case IDC_BUTTON_PAUSE:
        {
            if (g_MediaPlayer->TogglePauseState())
            {
                SetWindowText(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), L"Continue");
                KillTimer(hWnd, progressTimer);
            }
            else
            {
                SetWindowText(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), L"Pause");
                SetTimer(hWnd, progressTimer, 40, NULL);
            }
            break;
        }
        //
        //  The user hit the "Stop" button.
        //
        //  Stop the player and stop the progress timer and enable the "Play" button.
        //
        case IDC_BUTTON_STOP:
        {
            g_MediaPlayer->Stop();
            KillTimer(hWnd, progressTimer);
            EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PLAY), TRUE);
            EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_STOP), FALSE);
            break;
        }
        case IDC_CHECK_MUTE:
        {
            g_MediaPlayer->SetMute(IsDlgButtonChecked(hWnd, IDC_CHECK_MUTE) == BST_CHECKED);
        }
        default:
            break;
        }
        break;
    }
    case WM_HSCROLL:
    {
        //
        //  This horizontal scroll notification comes from the volume slider.  Update the volume for the media player.
        //
        if (reinterpret_cast<HWND>(lParam) == GetDlgItem(hWnd, IDC_SLIDER_VOLUME))
        {
            int volumePosition = static_cast<int>(SendMessage(GetDlgItem(hWnd, IDC_SLIDER_VOLUME), TBM_GETPOS, 0, 0));
            g_MediaPlayer->SetVolume(static_cast<float>(volumePosition) / 100.0f);
        }

        break;
    }
    case WM_TIMER:
    {
        if (wParam == progressTimer)
        {
            //
            //  Update the progress slider to match the current playback position.
            //
            long position = g_MediaPlayer->GetPosition();
            SendMessage(GetDlgItem(hWnd, IDC_SLIDER_PLAYBACKPOS), TBM_SETPOS, TRUE, position);
        }
        break;
    }

    //
    //  Let the media player know about the DShow graph event.  If we come to the end of the track, reset the slider to the beginning of the track.
    //
    case WM_APP_GRAPHNOTIFY:
    {
        if (g_MediaPlayer->HandleGraphEvent())
        {
            //  Reset the slider and timer, we're at the end of the track.
            SendMessage(GetDlgItem(hWnd, IDC_SLIDER_PLAYBACKPOS), TBM_SETPOS, TRUE, 0);

            KillTimer(hWnd, progressTimer);
            EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PLAY), TRUE);
            EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_STOP), FALSE);
        }
        break;
    }

    //
    //  Called when the media player receives a ducking notification.  Lets the media player know that the session has been ducked and pauses the player.
    //
    case WM_APP_SESSION_DUCKED:
        if (g_MediaPlayer->Pause())
        {
            SetWindowText(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), L"Continue");
            KillTimer(hWnd, progressTimer);
        }
        break;
    //
    //  Called when the media player receives an unduck notification.  Lets the media player know that the session has been unducked and continues the player.
    //
    case WM_APP_SESSION_UNDUCKED:
        if (g_MediaPlayer->Continue())
        {
            SetWindowText(GetDlgItem(hWnd, IDC_BUTTON_PAUSE), L"Pause");
            SetTimer(hWnd, progressTimer, 40, NULL);
        }
        break;
    //
    //  Process a session volume changed notification.  Sync the UI elements with the values in the notification.
    //
    //  The caller passes the new Mute state in wParam and the new volume value in lParam.
    //
    case WM_APP_SESSION_VOLUME_CHANGED:
        {
            BOOL newMute = (BOOL)wParam;
            float newVolume = LPARAM2FLOAT(lParam);
            int volumePos = static_cast<int>(newVolume * 100);
            SendMessage(GetDlgItem(hWnd, IDC_SLIDER_VOLUME), TBM_SETPOS, TRUE, volumePos);
            CheckDlgButton(hWnd, IDC_CHECK_MUTE, newMute ? BST_CHECKED : BST_UNCHECKED);
            break;
        }

    default:
        //
        //  If the current chat transport is going to handle this message, pass the message to the transport.
        //
        //  Otherwise just let our caller know that they need to handle it.
        //
       break;
    }
    return handled;
}
