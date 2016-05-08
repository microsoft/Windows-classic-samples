// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Ducking Capture Sample.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Resource.h"
#include "ChatTransport.h"
#include "WaveChat.h"
#include "WasapiChat.h"

#define MAX_LOADSTRING 100

// Global Variables:

//
//  The current "Chat" transport.
//
CChatTransport *g_CurrentChat;
//
// Current instance
//
HINSTANCE g_hInstance;
//
//  UI State information.
//
int g_WaveComboBoxIndex;
int g_WasapiComboBoxIndex = -1;

// Forward declarations of functions included in this code module:
INT_PTR CALLBACK    ChatDialogProc(HWND, UINT, WPARAM, LPARAM);


//
//  This sample only works on Windows 7
//
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

//
//  Program entry point
//
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

    static const INITCOMMONCONTROLSEX commonCtrls =
    {
        sizeof(INITCOMMONCONTROLSEX),
        ICC_STANDARD_CLASSES | ICC_BAR_CLASSES
    };

    HRESULT hr = CoInitialize(NULL);

    InitCommonControlsEx(&commonCtrls);

    g_hInstance = hInstance; // Store instance handle in our global variable

    INT_PTR id = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_CHAT), NULL, ChatDialogProc);

    CoUninitialize();

    return (int) id;
}

enum ChatState
{
    ChatStatePlaying,      // We're currently playing/capturing
    ChatStateNotPlaying,
};
//
//  Makes all of the dialog controls consistent with the current transport and specified chat state
//
void SyncUIState(HWND hWnd, ChatState State)
{
    if (State == ChatStatePlaying)
    {
        //
        //  Sync the UI to the state - Since we're playing, the only thing we can do is to hit the "Stop" button.
        //
        EnableWindow(GetDlgItem(hWnd, IDC_CHATSTART), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_CHATSTOP), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_COMBO_CHAT_TRANSPORT), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CAPTURE), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_RADIO_RENDER), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_CHECK_HIDE_FROM_VOLUME_MIXER), FALSE);
    }
    else if (State == ChatStateNotPlaying)
    {
        //
        //  Sync the UI to the state - since we're not playing all the options except stop become available.
        //
        EnableWindow(GetDlgItem(hWnd, IDC_CHATSTART), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_CHATSTOP), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_COMBO_CHAT_TRANSPORT), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_RADIO_CAPTURE), TRUE);

        //
        //  Now sync the transport options - the wave transport doesn't support output, so disable output device option
        //  when the the current transport is the wave transport.
        //
        //  Otherwise enable the "Use Output" and "hide from volume mixer" options
        //
        //  Note that the "Hide from volume mixer" option is only valid if the "Use Output Device" box is checked.
        //
        if (g_CurrentChat && g_CurrentChat->TransportType() == CChatTransport::ChatTransportWave)
        {
            EnableWindow(GetDlgItem(hWnd, IDC_RADIO_RENDER), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_CHECK_HIDE_FROM_VOLUME_MIXER), FALSE);
            CheckDlgButton(hWnd, IDC_RADIO_CAPTURE, BST_CHECKED);
            CheckDlgButton(hWnd, IDC_RADIO_RENDER, BST_UNCHECKED);
            CheckDlgButton(hWnd, IDC_CHECK_HIDE_FROM_VOLUME_MIXER, BST_UNCHECKED);
        }
        else
        {
            EnableWindow(GetDlgItem(hWnd, IDC_RADIO_RENDER), TRUE);
            EnableWindow(GetDlgItem(hWnd, IDC_CHECK_HIDE_FROM_VOLUME_MIXER), IsDlgButtonChecked(hWnd, IDC_RADIO_RENDER) == BST_CHECKED);
        }
    }
}

//
//  Processes messages for the dialog.
//
INT_PTR CALLBACK ChatDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL handled = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
        {
            //
            //  Start by using the wave transport for "chat".
            //

            //
            //  Allocate the WAVE chat transport.  If we failed to startup, we're done.
            //
            g_CurrentChat = new (std::nothrow) CWaveChat(hWnd);
            if (g_CurrentChat == NULL)
            {
                MessageBox(hWnd, L"Unable to allocate WAVE chat transport", L"Startup Error", MB_OK);
                EndDialog(hWnd, TRUE);
            }
            if (!g_CurrentChat->Initialize(true))
            {
                EndDialog(hWnd, TRUE);
            }

            //
            //  Set up the combobox and initialize the chat options to reflect that we've set the Wave chat transport by default.
            //
            g_WaveComboBoxIndex = ComboBox_InsertString(GetDlgItem(hWnd, IDC_COMBO_CHAT_TRANSPORT), 0, L"WAVE API Transport");
            g_WasapiComboBoxIndex = ComboBox_InsertString(GetDlgItem(hWnd, IDC_COMBO_CHAT_TRANSPORT), 1, L"WASAPI API Transport");
            ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_COMBO_CHAT_TRANSPORT), g_WaveComboBoxIndex);

            //
            //  Simulate a "stop" event to get the UI in sync.
            //
            SyncUIState(hWnd, ChatStateNotPlaying);

            handled = TRUE;
            break;
        }

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
                if (g_CurrentChat)
                {
                    g_CurrentChat->StopChat();
                }
                g_CurrentChat->Shutdown();
                delete g_CurrentChat;
                g_CurrentChat = NULL;

                EndDialog(hWnd, TRUE);
                handled = TRUE;
                break;

            case IDC_CHATSTART:
                //
                //  Start the chat engine.
                //
                if (g_CurrentChat->StartChat(IsDlgButtonChecked(hWnd, IDC_CHECK_HIDE_FROM_VOLUME_MIXER) == BST_CHECKED))
                {
                    SyncUIState(hWnd, ChatStatePlaying);
                }
                handled = TRUE;
                break;

            case IDC_CHATSTOP:
                //
                //  Stop the chat engine.
                //
                g_CurrentChat->StopChat();

                SyncUIState(hWnd, ChatStateNotPlaying);
                handled = TRUE;
                break;

                //
                //  The user's interacting with the chat transport combo box.
                //
            case IDC_COMBO_CHAT_TRANSPORT:
                {
                    switch (wmEvent)
                    {
                    case CBN_SELCHANGE:
                        {
                            int currentSel = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_COMBO_CHAT_TRANSPORT));

                            //
                            //  The user modified the chat transport.  Delete the existing chat transport and create a new one.
                            //
                            g_CurrentChat->Shutdown();
                            delete g_CurrentChat;
                            g_CurrentChat = NULL;

                            if (currentSel == g_WasapiComboBoxIndex)
                            {
                                //
                                //  Instantiate the WASAPI transport.
                                //
                                g_CurrentChat = new (std::nothrow) CWasapiChat(hWnd);
                                if (g_CurrentChat == NULL)
                                {
                                    MessageBox(hWnd, L"Unable to create WASAPI chat transport", L"Error", MB_OK);
                                }
                            }
                            else if (currentSel == g_WaveComboBoxIndex)
                            {
                                //
                                //  Instantiate the wave transport.
                                //
                                g_CurrentChat = new (std::nothrow) CWaveChat(hWnd);
                                if (g_CurrentChat == NULL)
                                {
                                    MessageBox(hWnd, L"Unable to create WAVE chat transport", L"Error", MB_OK);
                                }
                            }

                            //
                            //  Sync the UI to the transport choice
                            //
                            SyncUIState(hWnd, ChatStateNotPlaying);

                            //
                            //  Initialize the chat object
                            //
                            bool useInputDevice = (IsDlgButtonChecked(hWnd, IDC_RADIO_CAPTURE) == BST_CHECKED);
                            if (g_CurrentChat->Initialize(useInputDevice))
                            {
                                //
                                //  Sync the UI to the state again - we're not playing but after initializing the state might change.
                                //
                                SyncUIState(hWnd, ChatStateNotPlaying);
                            }
                            else
                            {
                                MessageBox(hWnd, L"Unable to initialize chat", L"Error", MB_OK);
                            }
                            break;
                        }
                    default:
                        break;
                    }
                    handled = TRUE;
                    break;
                } // IDC_COMBO_CHAT_TRANSPORT
            case IDC_RADIO_CAPTURE:
            case IDC_RADIO_RENDER:
                {
                    int currentSel = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_COMBO_CHAT_TRANSPORT));
                    //
                    //  The radio button selection may change when the transport is changed to Wave because render is not
                    //  an option for Wave.  We detect that here and only rebuild the transport for Wasapi
                    //
                    if ((currentSel == g_WasapiComboBoxIndex) && (g_CurrentChat->TransportType() == CChatTransport::ChatTransportWasapi))
                    {
                        //
                        //  The user switched between render and capture.  Delete the existing chat transport and create a new one.
                        //
                        g_CurrentChat->Shutdown();
                        delete g_CurrentChat;

                        //
                        //  Reinstantiate the WASAPI transport.
                        //
                        //  Also update the state of the rendering options since the WASAPI transport supports them.
                        //
                        g_CurrentChat = new (std::nothrow) CWasapiChat(hWnd);
                        if (g_CurrentChat == NULL)
                        {
                            MessageBox(hWnd, L"Unable to create WASAPI chat transport", L"Error", MB_OK);
                        }
                        else if (g_CurrentChat->Initialize(IsDlgButtonChecked(hWnd, IDC_CHECK_HIDE_FROM_VOLUME_MIXER) == BST_CHECKED))
                        {
                        }
                        else
                        {
                            MessageBox(hWnd, L"Unable to initialize chat", L"Error", MB_OK);
                        }
                    }
                    SyncUIState(hWnd, ChatStateNotPlaying);
                    break;
                }
            default:
                break;
            }   // WM_COMMAND
        }
    default:
        //
        //  If the current chat transport is going to handle this message, pass the message to the transport.
        //
        //  Otherwise just let our caller know that they need to handle it.
        //
        if (g_CurrentChat && g_CurrentChat->HandlesMessage(hWnd, message))
        {
            handled = static_cast<BOOL>(g_CurrentChat->MessageHandler(hWnd, message, wParam, lParam));
        }
        break;
    }
    return handled;
}
