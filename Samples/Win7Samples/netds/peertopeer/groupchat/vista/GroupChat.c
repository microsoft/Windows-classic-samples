/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    GroupChat.c

Abstract:

    This C file includes sample code for a chat application built
    with the Peer-to-Peer Grouping API.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

Note:
    This peer to peer application requires global IPv6 connectivity.

--********************************************************************/

#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4306)   // conversion to object of greater size

#include "GroupChat.h"

// Global Variables:
//
HINSTANCE  g_hInst            = NULL;  // application instance
HWND       g_hwndMain         = NULL;  // main window
HWND       g_hwndText         = NULL;  // text to send for the user
HWND       g_hwndChatLabel    = NULL;  // static text of message area
HWND       g_hwndMsg          = NULL;  // message area
HWND       g_hwndMembers      = NULL;  // list view of active members
HWND       g_hwndMembersLabel = NULL;  // static text of active members
HWND       g_hwndSend         = NULL;  // "Send" button
HWND       g_hwndStatus       = NULL;  // status bar window
HGROUP     g_hGroup           = NULL;  // group object
HPEEREVENT g_hPeerEvent       = NULL;  // PeerEvent handle
HANDLE     g_hEvent           = NULL;  // event handle
HANDLE     g_hWait            = NULL;  // wait handle
ULONGLONG  g_ullConnectionId  = 0;     // 64-bit integer that identifies the direct connection

DWORD      g_dwAuth = PEER_GROUP_GMC_AUTHENTICATION;  // Authentication scheme (password or GMC)
BOOL       g_fGlobalScope = TRUE;      // global (TRUE) or local (FALSE) group scope
WCHAR      g_wzDCName[MAX_USERNAME];
WCHAR      g_wzName[MAX_USERNAME];

// The unique identifier for chat messages
GUID RECORD_TYPE_CHAT_MESSAGE =
    {0x4d5b2f11, 0x6522, 0x433b, {0x84, 0xef, 0xa2, 0x98, 0xe6, 0x7, 0x57, 0xb0}};
// The unique identifier for Whisper (Private chat) messages
GUID DATA_TYPE_WHISPER_MESSAGE =
    { 0x4d5b2f11, 0x6522, 0x433b, { 0x84, 0xef, 0xa2, 0x98, 0xe6, 0x7, 0xbb, 0xbb } };

// File Extensions
const PCWSTR c_wzFileExtInv = L"inv"; // Group Invitation
const PCWSTR c_wzFileExtIdt = L"idt"; // Peer Identity

// Size of controls
const int c_dxBorder  = 5;
const int c_dyBorder  = 5;
const int c_dxMembers = 140;
const int c_dyStatic  = 20;
const int c_dyText    = 78;
const int c_dxButton  = 70;
const int c_dyButton  = 40;

// Window sizing
const int c_dxMin = 300; // Minimum width of the main window
const int c_dyMin = 200; // Minimum height of the main window

// Local functions
HRESULT InitSystem(int nCmdShow);
BOOL ProcessSpecialKeys(MSG * pMsg);


//-----------------------------------------------------------------------------
// Function: WinMain
//
// Purpose:  This is the main entry point for the application.
//
// Returns:  0
//
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, int nCmdShow)
{
    //Unreferenced parameter
    lpCmdLine;
    hPrevInstance;

    g_hInst = hInstance;

    if (SUCCEEDED(InitSystem(nCmdShow)))
    {
        MSG msg;
        HACCEL hAccel = LoadAccelerators(g_hInst, (LPCTSTR)IDC_ACCEL);

        // Main message loop:
        while (GetMessage(&msg, NULL, 0, 0))
        {
            HWND hwndTranslate;
            if ((msg.hwnd == g_hwndMain) || IsChild (g_hwndMain, msg.hwnd))
            {
                hwndTranslate = g_hwndMain;
            }
            else
            {
                hwndTranslate = msg.hwnd;
            }

            if (!TranslateAccelerator(hwndTranslate, hAccel, &msg))
            {
                if (!ProcessSpecialKeys(&msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
    }
    else
    {
        MessageBox(NULL, L"One or more Group Chat components could not be initialized.  Make sure Peer-to-Peer is installed and enabled.", L"Group Chat Error", MB_OK | MB_ICONWARNING);
    }
    // Cleanup
    CleanupGroup( );
    PeerGroupShutdown( );

    return 0;
}

//-----------------------------------------------------------------------------
// Function: InitUI
//
// Purpose:  Initialize the User Interface for the main window.
//
// Returns:  S_OK if the main window was created successfully
//
HRESULT InitUI(void)
{
    HRESULT hr = S_OK;
    WNDCLASSEX wcex;
    WCHAR szTitle[MAX_LOADSTRING];
    WCHAR szWindowClass[MAX_LOADSTRING];

    InitCommonControls( );

    // The title bar text
    LoadString(g_hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

    // the main window class name
    LoadString(g_hInst, IDS_APP_CLASS, szWindowClass, MAX_LOADSTRING);

    // Register the window class
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize         = sizeof(wcex);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)MainProc;
    wcex.hInstance      = g_hInst;
    wcex.hIcon          = LoadIcon(g_hInst, (LPCTSTR)IDI_APP);
    wcex.hIconSm        = wcex.hIcon;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);
    wcex.lpszMenuName   = (LPCTSTR)IDC_MENU;
    wcex.lpszClassName  = szWindowClass;
    RegisterClassEx(&wcex);

    // Create the main window
    CreateWindow(szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL, NULL, g_hInst, NULL);

    if (g_hwndMain == NULL)  // g_hwndMain is set by WM_CREATE
        hr = E_FAIL;

    return hr;
}


//-----------------------------------------------------------------------------
// Function: InitSystem
//
// Purpose:  Initialize the main system (Peer-to-Peer, windows, controls, etc.)
//
// Returns:  S_OK if the system was successfully initialized
//
HRESULT InitSystem(int nCmdShow)
{
    HRESULT hr = S_OK;
    PEER_VERSION_DATA peerVersion;
    WSADATA wsaData = {0};

    // Setup Winsock
    hr = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (FAILED(hr))
    {
        DisplayHrError(L"Unable to Intialize WSA.", hr);
        return E_UNEXPECTED;
    }

    hr = PeerGroupStartup(PEER_GROUP_VERSION, &peerVersion);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = (InitUI( ));
    if (FAILED(hr))
    {
        return hr;
    }

    ShowWindow(g_hwndMain, nCmdShow);
    UpdateWindow(g_hwndMain);

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
// M A I N   W I N D O W


//-----------------------------------------------------------------------------
// Function: ProcessSendButton
//
// Purpose:  Get the text and send it as a chat message.
//
// Returns:  nothing
//
void ProcessSendButton(void)
{
    WCHAR  wzMessage[MAX_CHAT_MESSAGE];

    if (NULL == g_hGroup)
    {
        return;
    }

    if (0 == GetWindowText(g_hwndText, wzMessage, celems(wzMessage)))
    {
        return;
    }

    if (FAILED(AddChatRecord(wzMessage)))
    {
        return;
    }

    // Clear the text box and prepare for the next line
    SetWindowText(g_hwndText, L"");
    SetFocus(g_hwndText);
}


//-----------------------------------------------------------------------------
// Function: ProcessSpecialKeys
//
// Purpose:  Handle the TABs for the window.
//
// Returns:  BOOL
//
BOOL ProcessSpecialKeys(MSG * pMsg)
{
    BOOL fShift;
    HWND hwnd;

    switch (pMsg->message)
    {
        case WM_CHAR:
            switch (pMsg->wParam)
            {
                case VK_TAB:
                    fShift = 0 > GetKeyState(VK_SHIFT);
                    hwnd = GetFocus( );
                    if (hwnd == g_hwndText)
                    {
                        hwnd = fShift ? g_hwndMsg : g_hwndSend;
                    }
                    else if (hwnd == g_hwndSend)
                    {
                        hwnd = fShift ? g_hwndText : g_hwndMembers;
                    }
                    else if (hwnd == g_hwndMembers)
                    {
                        hwnd = fShift ?  g_hwndSend : g_hwndMsg;
                    }
                    else // if (hwnd == g_hwndMsg)
                    {
                        hwnd = fShift ? g_hwndMembers : g_hwndText;
                    }
                    SetFocus(hwnd);
                    return TRUE;

                case VK_RETURN:
                    ProcessSendButton( );
                    return TRUE;

                default:
                    // Any typing (except ctrl+C) moves the focus to the text window
                    if (((pMsg->hwnd == g_hwndMsg) && (pMsg->wParam != 0x03))
                            || (pMsg->hwnd == g_hwndMain))
                    {
                        SetFocus(g_hwndText);
                        SendMessage(g_hwndText, pMsg->message, pMsg->wParam, pMsg->lParam);
                        return TRUE;
                    }
                    break;
            }
            break; /* WM_CHAR */

        default:
            break;
    }

    return FALSE;
}


//-----------------------------------------------------------------------------
// Function: ProcessResizing
//
// Purpose:  Ensures the dialog size stays within limits.
//
// Returns:  Nothing
//
void ProcessResizing(LPRECT pRect)
{
    if ((pRect->right - pRect->left) < c_dxMin)
    {
        pRect->right = pRect->left + c_dxMin;
    }

    if ((pRect->bottom - pRect->top) < c_dyMin)
    {
        pRect->bottom = pRect->top + c_dyMin;
    }
}


//-----------------------------------------------------------------------------
// Function: ResizeMainWindow
//
// Purpose:  Resizes the window to the specified width and height.
//
// Returns:  Nothing
//
void ResizeMainWindow(int dxMain, int dyMain)
{
    int  dyStatus;  // Height of the status bar
    RECT rect;      // temporary rectangle

    HDWP hDWP = BeginDeferWindowPos(7);     // 7 windows will be repositioned

    GetWindowRect(g_hwndStatus, &rect);
    dyStatus = rect.bottom - rect.top;
    hDWP = DeferWindowPos(hDWP, g_hwndStatus, NULL,
            0, dyMain - dyStatus, dxMain, dyStatus,
            SWP_NOZORDER | SWP_NOOWNERZORDER);
    dyMain -= (dyStatus + c_dyBorder);


    hDWP = DeferWindowPos(hDWP, g_hwndChatLabel, NULL,
            c_dxBorder, c_dyBorder,
            dxMain - (c_dxMembers + 3*c_dxBorder), c_dyStatic,
            SWP_NOZORDER | SWP_NOOWNERZORDER);
    hDWP = DeferWindowPos(hDWP, g_hwndMsg, NULL,
            c_dxBorder, c_dyBorder + c_dyStatic,
            dxMain - (c_dxMembers + 3*c_dxBorder), dyMain - (c_dyText + 2*c_dyBorder),
            SWP_NOZORDER | SWP_NOOWNERZORDER);
    hDWP = DeferWindowPos(hDWP, g_hwndText, NULL,
            c_dxBorder, dyMain - c_dyText + c_dyStatic,
            dxMain - (c_dxMembers + 3*c_dxBorder), c_dyText - c_dyStatic,
            SWP_NOZORDER | SWP_NOOWNERZORDER);


    hDWP = DeferWindowPos(hDWP, g_hwndMembersLabel, NULL,
            (dxMain - (c_dxMembers + 2*c_dxBorder)) + c_dxBorder, c_dyBorder,
            c_dxMembers, c_dyStatic,
            SWP_NOZORDER | SWP_NOOWNERZORDER);
    hDWP = DeferWindowPos(hDWP, g_hwndMembers, NULL,
            (dxMain - (c_dxMembers + 2*c_dxBorder)) + c_dxBorder, c_dyBorder + c_dyStatic,
            c_dxMembers, dyMain - (c_dyText + 2*c_dyBorder),
            SWP_NOZORDER | SWP_NOOWNERZORDER);

    hDWP = DeferWindowPos(hDWP, g_hwndSend, NULL,
            (dxMain - (c_dxMembers + c_dxBorder)) + (c_dxMembers - c_dxButton)/2,
            (dyMain - c_dyText) + (c_dyText + c_dyStatic - c_dyButton)/2,
            0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE);

    EndDeferWindowPos(hDWP);
}


//-----------------------------------------------------------------------------
// Function: AddControls
//
// Purpose:  Add the controls to the main window
//
// Returns:  nothing
//
void AddControls(void)
{
    HFONT   hFont;
    LOGFONT lf;

    int  rgStatusBarWidths[2] = {
        100, // SB_PART_STATUS  - "Listening"/"Connected"
        -1}; // SB_PART_MESSAGE - generic messages

    // Create a font for the "Send" button
    ZeroMemory(&lf, sizeof(lf));
    hFont = CreateFontIndirect(&lf);

    // Create the "Send" push button
    g_hwndSend = CreateWindowEx(
        0, WC_BUTTON, L"Send",
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP |
        BS_PUSHBUTTON,
        0, 0, c_dxButton, c_dyButton,
        g_hwndMain, (HMENU) IDC_SEND, NULL, NULL);
    SetWindowFont(g_hwndSend, hFont, FALSE);

    // Create the edit control for the user to enter text
    g_hwndText = CreateWindowEx(
        0, WC_EDIT, L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_VSCROLL |
        ES_MULTILINE | ES_AUTOVSCROLL,
        c_dxBorder, 0, 0, c_dyText,
        g_hwndMain, (HMENU) IDC_TEXTBOX, NULL, NULL);
    SendMessage(g_hwndText, EM_SETLIMITTEXT, MAX_CHAT_MESSAGE - 1, 0);
    SetWindowFont(g_hwndText, hFont, FALSE);

    // Create the static text "Chat Members"
    g_hwndMembersLabel = CreateWindowEx(
        0, WC_STATIC, L"Chat Members",
        WS_CHILD | WS_VISIBLE | ES_READONLY | SS_CENTER,
        0, c_dyBorder, c_dxMembers, c_dyStatic,
        g_hwndMain, (HMENU) IDC_STATIC_MEMBERS, NULL, NULL);
    SetWindowFont(g_hwndMembersLabel, hFont, FALSE);

    // Create the listbox of active members
    g_hwndMembers = CreateWindowEx(
        0, WC_LISTBOX, L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_VSCROLL |
        LBS_DISABLENOSCROLL | LBS_SORT | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
        0, c_dyBorder, c_dxMembers, 0,
        g_hwndMain, (HMENU) IDC_MEMBERS, NULL, NULL);
    SetWindowFont(g_hwndMembers, (HFONT) GetStockObject(DEFAULT_GUI_FONT), FALSE);

    // Create the static text "Conversation"
    g_hwndChatLabel = CreateWindowEx(
        0, WC_STATIC, L"Offline",
        WS_CHILD | WS_VISIBLE | ES_READONLY | SS_CENTER,
        c_dxBorder, c_dyBorder, 0, c_dyStatic,
        g_hwndMain, (HMENU) IDC_STATIC_MEMBERS, NULL, NULL);
    SetWindowFont(g_hwndChatLabel, hFont, FALSE);

    // Create the message area
    g_hwndMsg = CreateWindowEx(
        0, WC_EDIT, L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_VSCROLL |
        ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        c_dxBorder, c_dyBorder, 0, 0,
        g_hwndMain, (HMENU) IDC_MESSAGES, NULL, NULL);
    SetWindowFont(g_hwndMsg, hFont, FALSE);
    SendMessage(g_hwndMsg,  EM_SETLIMITTEXT, 0, 0);

    // Create the status bar
    g_hwndStatus = CreateWindowEx(
        0, STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0,
        g_hwndMain, (HMENU) IDC_STATUS, NULL, NULL);
    SendMessage(g_hwndStatus, (UINT) SB_SETPARTS, celems(rgStatusBarWidths), (LPARAM) &rgStatusBarWidths[0]);
}


//-----------------------------------------------------------------------------
// Function: MainProc
//
// Purpose:  Processes messages for the main window.
//
// Returns:  LRESULT (depends on message)
//
LRESULT CALLBACK MainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
            g_hwndMain = hWnd;
            AddControls( );
            SetFocus(g_hwndText);
            break;

        case WM_SIZING:
            ProcessResizing((LPRECT) lParam);
            return TRUE; // the size may have been changed

        case WM_SIZE:
            ResizeMainWindow(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_CTLCOLORSTATIC:
            // draw the read-only control with a normal window background
            if (lParam == (LPARAM) g_hwndMsg)
            {
                return (LRESULT) GetSysColorBrush(COLOR_WINDOW);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDM_CLEARTEXT:
                    SetWindowText(g_hwndMsg, L"");
                    break;

                case IDM_EXIT:
                    PostMessage(g_hwndMain, WM_CLOSE, 0, 0);
                    break;

                case IDM_ABOUT:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), g_hwndMain, AboutProc);
                    break;

                case IDM_CREATEGROUP:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CREATEGROUP), g_hwndMain, NewGroupProc);
                    break;

                case IDM_OPENGROUP:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_OPENGROUP), g_hwndMain, OpenGroupProc);
                    break;

                case IDM_JOINGROUP:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_JOINGROUP), g_hwndMain, JoinGroupProc);
                    break;

                case IDM_DELETEGROUP:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DELETEGROUP), g_hwndMain, DeleteGroupProc);
                    break;

                case IDM_CLOSEGROUP:
                    CmdCloseGroup( );
                    break;

                case IDM_SAVEIDENTITYINFO:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_SAVEIDENTITYINFO), g_hwndMain, SaveIdentityInfoProc);
                    break;

                case IDM_CREATEIDENTITY:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_NEWIDENTITY), g_hwndMain, NewIdentityProc);
                    break;

                case IDM_DELETEIDENTITY:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DELETEIDENTITY), g_hwndMain, DeleteIdentityProc);
                    break;

                case IDM_CREATEINVITATION:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CREATEINVITATION), g_hwndMain, CreateInvitationProc);
                    break;

                case IDC_SEND:
                    ProcessSendButton( );
                    break;

                case IDC_MEMBERS:
                    if (HIWORD(wParam) == LBN_DBLCLK)
                    {
                        PCWSTR pwzIdentity = GetSelectedChatMember();
                        if (SUCCEEDED(SetupDirectConnection(pwzIdentity)))
                        {
                            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_WHISPERMESSAGE), g_hwndMain, WhisperMessageProc);
                        }
                    }
                    break;

                default:
                    break;
            }
            break; /* WM_COMMAND */

        default:
            break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


///////////////////////////////////////////////////////////////////////////////
//  A B O U T  B O X

//-----------------------------------------------------------------------------
// Function: AboutProc
//
// Purpose:  DialogProc for the About box
//
// Returns:  Returns TRUE if it processed the message, and FALSE if it did not.
//
LRESULT CALLBACK AboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    //Unreferenced parameter
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                case IDOK:
                    EndDialog(hDlg, LOWORD(wParam));
                    break;

                default:
                    break;
            }
            break; /* WM_COMMAND */

        default:
            break;
    }
    return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
//  N E W   G R O U P

//-----------------------------------------------------------------------------
// Function: NewGroupProc
//
// Purpose:  DialogProc to create a new group
//
// Returns:  Returns TRUE if it processed the message, and FALSE if it did not.
//
LRESULT CALLBACK NewGroupProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PEER_NAME_PAIR **ppNamePairs = NULL;
    BOOL fPass = FALSE;

    //Unreferenced parameter
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:

            // Use Non-password based invitations by default
            SendDlgItemMessage(hDlg, IDC_RADIO_AUTH_INVITE, BM_SETCHECK, BST_CHECKED, 0);
            SendDlgItemMessage(hDlg, IDC_EDT_GROUPNAME, EM_SETLIMITTEXT, MAX_GROUPNAME - 1, 0);         
            RefreshIdentityCombo(GetDlgItem(hDlg, IDC_CB_IDENTITY), TRUE, &ppNamePairs);

            // Use global scope by default
            SendDlgItemMessage(hDlg, IDC_RADIO_GLOBAL_SCOPE, BM_SETCHECK, BST_CHECKED, 0);
            return TRUE;

        case WM_DESTROY:
            if (ppNamePairs != NULL)
            {
                PeerFreeData(ppNamePairs);
                ppNamePairs = NULL;
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    if (SUCCEEDED(HandleNewGroup(hDlg)))
                    {
                        SetStatus(L"Group created");
                        EndDialog(hDlg, IDOK);

                    }
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDC_BTN_NEW_IDENTITY:
                    if (IDOK == DialogBox(g_hInst, MAKEINTRESOURCE(IDD_NEWIDENTITY), hDlg, NewIdentityProc))
                    {
                        if (ppNamePairs != NULL)
                        {
                            PeerFreeData(ppNamePairs);
                            ppNamePairs = NULL;
                        }
                        RefreshIdentityCombo(GetDlgItem(hDlg, IDC_CB_IDENTITY), TRUE, &ppNamePairs);
                    }
                    break;
                case IDC_RADIO_AUTH_PASSW:
                case IDC_RADIO_AUTH_INVITE:
                    fPass = (SendDlgItemMessage(hDlg, IDC_RADIO_AUTH_PASSW, BM_GETCHECK, BST_CHECKED, 0) == BST_CHECKED);
                    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_PASSWORD), fPass);
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASSWORD), fPass);
                    break;
                default:
                    break;
            }
            break; /* WM_COMMAND */

        default:
            break;
    }

    return FALSE;
}


//-----------------------------------------------------------------------------
// Function: HandleNewGroup
//
// Purpose:  Extracts the information from the dialog and calls
//           CreateGroup to do the actual work.
//
// Returns:  HRESULT
//
HRESULT HandleNewGroup(HWND hDlg)
{
    HRESULT hr = S_OK;
    WCHAR wzFriendlyName[MAX_GROUPNAME];
    WCHAR wzPassword[MAX_PASSWORD] = {0};

    LRESULT iCurSel = SendDlgItemMessage(hDlg, IDC_CB_IDENTITY, CB_GETCURSEL, 0, 0);
    PWSTR pwzIdentity = (PWSTR) SendDlgItemMessage(hDlg, IDC_CB_IDENTITY, CB_GETITEMDATA, (WPARAM) iCurSel, 0);
    
    // Retrieve user's selection on authentication method
    if (SendDlgItemMessage(hDlg, IDC_RADIO_AUTH_PASSW, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        SendDlgItemMessage(hDlg, IDC_EDIT_PASSWORD, WM_GETTEXT, celems(wzPassword), (LPARAM) wzPassword);
        g_dwAuth = PEER_GROUP_PASSWORD_AUTHENTICATION;
    }
    else
    {
        g_dwAuth = PEER_GROUP_GMC_AUTHENTICATION;
    }

    g_fGlobalScope = (SendDlgItemMessage(hDlg, IDC_RADIO_GLOBAL_SCOPE, BM_GETCHECK, 0, 0) == BST_CHECKED);

    SendDlgItemMessage(hDlg, IDC_EDT_GROUPNAME, WM_GETTEXT, celems(wzFriendlyName), (LPARAM) wzFriendlyName);

    hr = CreateGroup(wzFriendlyName, pwzIdentity, wzPassword);

    return hr;
}

//-----------------------------------------------------------------------------
// Function: CreateGroup
//
// Purpose:  Creates a new group with the friendly name.
//
// Parameters:
//      pwzName     [in] : Friendly name of group
//      pwzIdentity [in] : Path to identity file of group creator
//      pwzPassword [in] : Password of group (if applicable)
//
// Returns:  HRESULT
//
HRESULT CreateGroup(PCWSTR pwzName, PCWSTR pwzIdentity, PWSTR pwzPassword)
{
    HRESULT hr = S_OK;
    PEER_GROUP_PROPERTIES props = {0};

    if (SUCCEEDED(hr))
    {
        if ((NULL == pwzName) || (0 == *pwzName))
        {
            hr = E_INVALIDARG;
            DisplayHrError(L"Please enter a group name.", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        CleanupGroup( );

        props.dwSize = sizeof(props);
        props.pwzClassifier = L"SampleChatGroup";
        props.pwzFriendlyName = (PWSTR) pwzName;
        props.pwzCreatorPeerName = (PWSTR) pwzIdentity;
        props.dwAuthenticationSchemes = g_dwAuth;
        if (g_dwAuth == PEER_GROUP_PASSWORD_AUTHENTICATION)
        {
            props.groupPasswordRole = PEER_GROUP_ROLE_ADMIN;
            props.pwzGroupPassword = (PWSTR) pwzPassword;
        }

        if (g_fGlobalScope)
        {
            props.pwzCloud = NULL;
            hr = PeerGroupCreate(&props, &g_hGroup);
        }
        else
        {
            WCHAR pwzCloudName[MAX_CLOUD_NAME] = {0};
            
            hr = GetLocalCloudName(celems(pwzCloudName), pwzCloudName);
            if (SUCCEEDED(hr))
            {
                props.pwzCloud = pwzCloudName;
                hr = PeerGroupCreate(&props, &g_hGroup);
            }
        } 
        
        if (FAILED(hr))
        {
            if (hr == PEER_E_PASSWORD_DOES_NOT_MEET_POLICY)
            {
                DisplayHrError(L"Password does not meet local policy.", hr);
            }
            else 
            {
                DisplayHrError(L"Failed to create a new group.", hr);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = PrepareToChat( );
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//  O P E N   G R O U P

//-----------------------------------------------------------------------------
// Function: OpenGroupProc
//
// Purpose:  DialogProc to open an existing group
//
// Returns:  Returns TRUE if it processed the message, and FALSE if it did not.
//
LRESULT CALLBACK OpenGroupProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PEER_NAME_PAIR **ppIdentities = NULL;
    static PEER_NAME_PAIR **ppGroups = NULL;

    //Unreferenced parameter
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:
            if (SUCCEEDED(RefreshIdentityCombo(GetDlgItem(hDlg, IDC_CB_IDENTITY), TRUE, &ppIdentities)))
            {
                PCWSTR pwzIdentity = GetSelectedIdentity(hDlg);
                RefreshGroupCombo(GetDlgItem(hDlg, IDC_CB_GROUP), pwzIdentity, &ppGroups);
            }
            return TRUE;

        case WM_DESTROY:
            if (ppGroups != NULL)
            {
                PeerFreeData(ppGroups);
                ppGroups = NULL;
            }
            if (ppIdentities != NULL)
            {
                PeerFreeData(ppIdentities);
                ppIdentities = NULL;
            }
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    if (SUCCEEDED(HandleOpenGroup(hDlg)))
                    {
                        SetStatus(L"Group opened");
                        EndDialog(hDlg, IDOK);
                    }
                    break;
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDC_CB_IDENTITY:

                    // When identities change, refresh the group list.
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        PCWSTR pwzIdentity = GetSelectedIdentity(hDlg);

                        if (ppGroups != NULL)
                        {
                            PeerFreeData(ppGroups);
                            ppGroups = NULL;
                        }
                        RefreshGroupCombo(GetDlgItem(hDlg, IDC_CB_GROUP), pwzIdentity, &ppGroups);
                    }
                    break;

                default:
                    break;
            }
            break; /* WM_COMMAND */

        default:
            break;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
// Function: HandleOpenGroup
//
// Purpose:  Extracts the information from the dialog and calls
//           OpenGroup to do the actual work.
//
// Returns:  HRESULT
//
HRESULT HandleOpenGroup(HWND hDlg)
{
    PCWSTR  pwzGroup = GetSelectedGroup(hDlg);
    PCWSTR  pwzIdentity = GetSelectedIdentity(hDlg);
    HRESULT hr = OpenGroup(pwzIdentity, pwzGroup);

    if (SUCCEEDED(hr))
    {
        GetFriendlyNameForIdentity(pwzIdentity, g_wzName, celems(g_wzName));
    }
    
    return hr;
}

//-----------------------------------------------------------------------------
// Function: OpenGroup
//
// Purpose:  Open an existing group for a particular identity.
//           Displays a message if there was an error.
//
// Returns:  HRESULT
//
HRESULT OpenGroup(PCWSTR pwzIdentity, PCWSTR pwzName)
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        if ((NULL == pwzName) || (0 == *pwzName))
        {
            hr = E_INVALIDARG;
            DisplayHrError(L"Please select a group.", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Release any previous group resources
        CleanupGroup( );

        // The NULL parameter indicates that we'll let grouping try to 
        // automatically determine which cloud to use
        hr = PeerGroupOpen(pwzIdentity, pwzName, NULL, &g_hGroup);
        
        // If NULL does not work, we try the first link local cloud
        if (FAILED(hr))
        {
            WCHAR wzCloudName[MAX_CLOUD_NAME] = {0};

            hr = GetLocalCloudName(celems(wzCloudName), wzCloudName);
            if (SUCCEEDED(hr))
            {
                hr = PeerGroupOpen(pwzIdentity, pwzName, wzCloudName, &g_hGroup);
            }
        }
        // Otherwise, return failure
        if (FAILED(hr))
        {
            DisplayHrError(L"Failed to open group.", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = PrepareToChat( );
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//  N E W   I D E N T I T Y

//-----------------------------------------------------------------------------
// Function: NewIdentityProc
//
// Purpose:  DialogProc to create a new identity
//
// Returns:  Returns TRUE if it processed the message, and FALSE if it did not.
//
LRESULT CALLBACK NewIdentityProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    //Unreferenced parameter
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:
            SendDlgItemMessage(hDlg, IDC_EDT_FRIENDLYNAME, EM_SETLIMITTEXT, MAX_IDENTITY - 1, 0);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    if (SUCCEEDED(HandleCreateIdentity(hDlg)))
                        EndDialog(hDlg, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;
            }
            break; /* WM_COMMAND */

        default:
            break;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
// Function: HandleCreateIdentity
//
// Purpose:  Extracts the friendly name from the dialog and
//           creates a new identity.
//
// Returns:  HRESULT
//
HRESULT HandleCreateIdentity(HWND hDlg)
{
    HRESULT hr = S_OK;
    WCHAR   wzName[MAX_USERNAME];

    UINT cch = GetDlgItemText(hDlg, IDC_EDT_FRIENDLYNAME, wzName, celems(wzName));
    if (0 == cch)
    {
        hr = E_FAIL;
        DisplayHrError(L"Please type a name for the identity.", hr);
    }

    if (SUCCEEDED(hr))
    {
        PCWSTR pwzClassifier = L"GroupChatMember";
        PWSTR  pwzIdentity = NULL;
        hr = PeerIdentityCreate(pwzClassifier, wzName, 0, &pwzIdentity);
        if (FAILED(hr))
        {
            DisplayHrError(L"Failed to create identity", hr);
        }
        PeerFreeData(pwzIdentity);
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//  D E L E T E   I D E N T I T Y

//-----------------------------------------------------------------------------
// Function: DeleteIdentityProc
//
// Purpose:  DialogProc to delete an identity
//
// Returns:  Returns TRUE if it processed the message, and FALSE if it did not.
//
LRESULT CALLBACK DeleteIdentityProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PEER_NAME_PAIR **ppIdentities = NULL;

    //Unreferenced parameter
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:
            (void)RefreshIdentityCombo(GetDlgItem(hDlg, IDC_CB_IDENTITY), FALSE, &ppIdentities);
            return TRUE;

        case WM_DESTROY:
            if (ppIdentities != NULL)
            {
                PeerFreeData(ppIdentities);
                ppIdentities = NULL;
            }

            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    if (SUCCEEDED(HandleDeleteIdentity(hDlg)))
                        EndDialog(hDlg, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;
            }
            break; /* WM_COMMAND */

        default:
            break;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
// Function: HandleDeleteIdentity
//
// Purpose:  Extracts the selected identity from the dialog
//           and deletes it.
//
// Returns:  HRESULT
//
HRESULT HandleDeleteIdentity(HWND hDlg)
{
    HRESULT hr = S_OK;
    PCWSTR  pwzIdentity = GetSelectedIdentity(hDlg);

    hr = DeleteIdentity(pwzIdentity);

    return hr;
}

//-----------------------------------------------------------------------------
// Function: DeleteIdentity
//
// Purpose:  Deletes an identity
//
// Parameters:
//      pwzIdentity [in] : The peer identity to delete
//
// Returns:  HRESULT
//
HRESULT DeleteIdentity(PCWSTR pwzIdentity)
{
    HRESULT hr = S_OK;

    if ((NULL == pwzIdentity) || (0 == *pwzIdentity))
    {
        hr = E_INVALIDARG;
        DisplayHrError(L"Please select an identity.", hr);
    }

    if (SUCCEEDED(hr))
    {
        CleanupGroup( );

        hr = PeerIdentityDelete(pwzIdentity);
        
        if (FAILED(hr))
        {
            DisplayHrError(L"Failed to delete identity.", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        SetStatus(L"Deleted identity");
    }
    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//  J O I N   G R O U P

//-----------------------------------------------------------------------------
// Function: JoinGroupProc
//
// Purpose:  DialogProc to join some existing group with an invitation
//
// Returns:  Returns TRUE if it processed the message, and FALSE if it did not.
//
LRESULT CALLBACK JoinGroupProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PEER_NAME_PAIR **ppIdentities = NULL;
    BOOL fPass = FALSE;

    //Unreferenced parameters
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:
            EnableWindow(GetDlgItem(hDlg, IDC_EDT_PASSWORD), FALSE);
            RefreshIdentityCombo(GetDlgItem(hDlg, IDC_CB_IDENTITY), TRUE, &ppIdentities);

            return TRUE;

        case WM_DESTROY:
            if (ppIdentities != NULL)
            {
                PeerFreeData(ppIdentities);
                ppIdentities = NULL;
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDOK:
                    if (SUCCEEDED(HandleJoinGroup(hDlg)))
                    {
                        SetStatus(L"Joined group");
                        EndDialog(hDlg, IDOK);
                    }
                    break;

                case IDC_BTN_BROWSE:
                    BrowseHelper(hDlg, IDC_EDT_LOCATION, L"Group Invitation", c_wzFileExtInv, TRUE);
                    break;
                case IDC_CHECK_PASSWORD:
                    fPass = (SendDlgItemMessage(hDlg, IDC_CHECK_PASSWORD, BM_GETCHECK, BST_CHECKED, 0) == BST_CHECKED);
                    EnableWindow(GetDlgItem(hDlg, IDC_EDT_PASSWORD), fPass);
                    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_PASSWORD), fPass);
                    break;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
// Function: HandleJoinGroup
//
// Purpose:  Extracts the information from the dialog and calls
//           JoinGroup to do the actual work.
//
// Returns:  HRESULT
//
HRESULT HandleJoinGroup(HWND hDlg)
{
    HRESULT hr = S_OK;
    WCHAR   wzInvitation[MAX_PATH];
    WCHAR   wzPassword[MAX_PASSWORD] = {0};
    PCWSTR  pwzIdentity = GetSelectedIdentity(hDlg);
    
    if (SendDlgItemMessage(hDlg, IDC_CHECK_PASSWORD, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        GetDlgItemText(hDlg, IDC_EDT_PASSWORD, wzPassword, celems(wzPassword));
        g_dwAuth = PEER_GROUP_PASSWORD_AUTHENTICATION;
    }
    else
    {
        g_dwAuth = PEER_GROUP_GMC_AUTHENTICATION;
    }

    GetDlgItemText(hDlg, IDC_EDT_LOCATION, wzInvitation, celems(wzInvitation));
    
    hr = JoinGroup(pwzIdentity, wzInvitation, wzPassword);

    return hr;
}

//-----------------------------------------------------------------------------
// Function: JoinGroup
//
// Purpose:  Uses the invitation to join a group with a specific identity.
//           Displays a message if there was an error.
//
// Parameters:
//  pwzIdentity [in] : Path to identity file (not used w/password based groups)
//  pwzFileName [in] : Path to the group invitation
//  pwzPassword [in] : Password of group (if applicable)
//
// Returns:  HRESULT
//
HRESULT JoinGroup(PCWSTR pwzIdentity, PCWSTR pwzFileName, PWSTR pwzPassword)
{
    HRESULT     hr = S_OK;
    WCHAR       wzInvitation[MAX_INVITATION] = {0};
    FILE        *file = NULL;
    errno_t     err;

    err = _wfopen_s(&file, pwzFileName, L"rb");
    if (err !=  0)
    {
        hr = E_FAIL;
        DisplayHrError(L"Error opening group invitation file", hr);
        return hr;
    }
    else
    {
        fread(wzInvitation, sizeof(WCHAR), MAX_INVITATION, file);
        if (ferror(file))
        {
            hr = E_FAIL;
            DisplayHrError(L"File read error occurred.", hr);
        }
        fclose(file);

        if (g_dwAuth == PEER_GROUP_GMC_AUTHENTICATION)
        {
            // NULL parameter indicates that the cloud name will be selected automatically 
            hr = PeerGroupJoin(pwzIdentity, wzInvitation, NULL, &g_hGroup);
            
            // In case of failure, try using a local cloud name
            if (FAILED(hr))
            {
                WCHAR wzCloudName[MAX_CLOUD_NAME] = {0};
                hr = GetLocalCloudName(celems(wzCloudName), wzCloudName);
                if (FAILED(hr))
                {
                    DisplayHrError(L"Could not find local cloud name.", hr);
                }
                else
                {
                    hr = PeerGroupJoin(pwzIdentity, wzInvitation, wzCloudName, &g_hGroup);
                }
            }
        }
        else
        {
            WCHAR wzCloudName[MAX_CLOUD_NAME] = {0};

            // NULL parameter indicates that the cloud name will be selected automatically 
            hr = PeerGroupPasswordJoin(pwzIdentity, wzInvitation,  pwzPassword, 
                                       NULL, &g_hGroup);
            // In case of failure, try using a local cloud name
            if (FAILED(hr))
            {
                hr = GetLocalCloudName(celems(wzCloudName), wzCloudName);
                if (FAILED(hr))
                {
                    
                    DisplayHrError(L"Could not find local cloud name.", hr);
                }
                else
                {
                    hr = PeerGroupPasswordJoin(pwzIdentity, wzInvitation,  pwzPassword, 
                                       wzCloudName, &g_hGroup);
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = PrepareToChat( );
    }
    else
    {
        DisplayHrError(L"Failed to join group.", hr);
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//  D E L E T E   G R O U P

//-----------------------------------------------------------------------------
// Function: DeleteGroupProc
//
// Purpose:  DialogProc to delete a group
//
// Returns:  Returns TRUE if it processed the message, and FALSE if it did not.
//
LRESULT CALLBACK DeleteGroupProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PEER_NAME_PAIR **ppIdentities = NULL;
    static PEER_NAME_PAIR **ppGroups = NULL;

    //Unreferenced parameters
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:
            if (SUCCEEDED(RefreshIdentityCombo(GetDlgItem(hDlg, IDC_CB_IDENTITY), TRUE, &ppIdentities)))
            {
                PCWSTR pwzIdentity = GetSelectedIdentity(hDlg);
                RefreshGroupCombo(GetDlgItem(hDlg, IDC_CB_GROUP), pwzIdentity, &ppGroups);
            }

            return TRUE;

        case WM_DESTROY:
            if (ppGroups != NULL)
            {
                PeerFreeData(ppGroups);
                ppGroups = NULL;
            }

            if (ppIdentities != NULL)
            {
                PeerFreeData(ppIdentities);
                ppIdentities = NULL;
            }

            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    if (SUCCEEDED(HandleDeleteGroup(hDlg)))
                    {
                        SetStatus(L"Group Deleted");
                        EndDialog(hDlg, IDOK);

                    }
                    break;

                case IDC_CB_IDENTITY:

                    // When identities change, refresh the group list.
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        PCWSTR pwzIdentity = GetSelectedIdentity(hDlg);

                        if (ppGroups != NULL)
                        {
                            PeerFreeData(ppGroups);
                            ppGroups = NULL;
                        }
                        RefreshGroupCombo(GetDlgItem(hDlg, IDC_CB_GROUP), pwzIdentity, &ppGroups);
                    }
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                default:
                    break;
            }
            break; /* WM_COMMAND */

        default:
            break;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
// Function: HandleDeleteGroup
//
// Purpose:  Extracts the information from the dialog and calls
//           DeleteGroup to do the actual work.
//
// Returns:  HRESULT
//
HRESULT HandleDeleteGroup(HWND hDlg)
{
    HRESULT hr = S_OK;
    PCWSTR  pwzGroup = GetSelectedGroup(hDlg);
    PCWSTR  pwzIdentity = GetSelectedIdentity(hDlg);

    hr = DeleteGroup(pwzGroup, pwzIdentity);

    return hr;
}

//-----------------------------------------------------------------------------
// Function: DeleteGroup
//
// Purpose:  Deletes a group
//
// Parameters:
//      pwzName     [in] : Friendly name of group
//      pwzIdentity [in] : The peer identity of the group creator
//
// Returns:  HRESULT
//
HRESULT DeleteGroup(PCWSTR pwzName, PCWSTR pwzIdentity)
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        CleanupGroup( );

        hr = PeerGroupDelete(pwzIdentity, pwzName);
        
        if (FAILED(hr))
        {
            DisplayHrError(L"Failed to delete group.", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        ProcessStatusChanged(0);
        SetStatus(L"Deleted group");
    }
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//  S A V E   I D E N T I T Y   I N F O R M A T I O N

//-----------------------------------------------------------------------------
// Function: SaveIdentityInfoProc
//
// Purpose:  DialogProc to save the information for a local identity to a file.
//
// Returns:  Returns TRUE if it processed the message, and FALSE if it did not.
//
LRESULT CALLBACK SaveIdentityInfoProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PEER_NAME_PAIR **ppIdentities = NULL;

    //Unreferenced parameter
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:
            RefreshIdentityCombo(GetDlgItem(hDlg, IDC_CB_IDENTITY), FALSE, &ppIdentities);
            return TRUE;

        case WM_DESTROY:
            if (ppIdentities != NULL)
            {
                PeerFreeData(ppIdentities);
                ppIdentities = NULL;
            }
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDOK:
                    if (SUCCEEDED(HandleSaveIdentityInfo(hDlg)))
                    {
                        EndDialog(hDlg, IDOK);
                    }
                    break;

                case IDC_BTN_NEW_IDENTITY:
                    if (IDOK == DialogBox(g_hInst, MAKEINTRESOURCE(IDD_NEWIDENTITY), hDlg, NewIdentityProc))
                    {
                        if (ppIdentities != NULL)
                        {
                            PeerFreeData(ppIdentities);
                            ppIdentities = NULL;
                        }
                        RefreshIdentityCombo(GetDlgItem(hDlg, IDC_CB_IDENTITY), FALSE, &ppIdentities);
                    }
                    break;

                case IDC_BTN_BROWSE:
                    BrowseHelper(hDlg, IDC_EDT_LOCATION, L"Identity Information", c_wzFileExtIdt, FALSE);
                    break;
            }
            break; /* WM_COMMAND */

        default:
            break;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
// Function: HandleSaveIdentityInfo
//
// Purpose:  Extracts the information from the dialog and calls
//           SaveIdentityInfo to do the actual work.
//
// Returns:  HRESULT
//
HRESULT HandleSaveIdentityInfo(HWND hDlg)
{
    HRESULT hr = S_OK;
    WCHAR   wzFile[MAX_PATH];
    PCWSTR  pwzIdentity = GetSelectedIdentity(hDlg);

    if ((NULL == pwzIdentity) || (0 == *pwzIdentity))
    {
        hr = E_FAIL;
        DisplayHrError(L"Please select an identity.", hr);
    }

    if (SUCCEEDED(hr))
    {
        LRESULT cch = SendDlgItemMessage(hDlg, IDC_EDT_LOCATION, WM_GETTEXT, celems(wzFile), (LPARAM) wzFile);
        if (0 == cch)
        {
            hr = E_FAIL;
            DisplayHrError(L"Please enter a filename where the identity will be saved.", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = SaveIdentityInfo(pwzIdentity, wzFile);
        if (FAILED(hr))
        {
            DisplayHrError(L"Failed to save identity information.", hr);
        }
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function: SaveIdentityInfo
//
// Purpose:  Saves the the information for an identity to a file.
//           Displays a message if there was an error.
//
// Returns:  HRESULT
//
HRESULT SaveIdentityInfo(PCWSTR pwzIdentity, PCWSTR pwzFile)
{
    PWSTR pwzXML = NULL;
    HRESULT hr = PeerIdentityGetXML(pwzIdentity, &pwzXML);

    if (FAILED(hr))
    {
        DisplayHrError(L"Unable to retreive the XML data for the identity.", hr);
    }
    else
    {
        FILE *pFile = NULL;
        errno_t err = 0;

        err = _wfopen_s(&pFile, pwzFile, L"wb");
        if (err != 0)
        {
            hr = E_FAIL;
            DisplayHrError(L"Please choose a valid path", hr);
        }
        else
        {
            if (fputws(pwzXML, pFile) == WEOF)
            {
                hr = E_FAIL;
                DisplayHrError(L"End of file error.", hr);
            }
            fclose(pFile);
        }

        PeerFreeData(pwzXML);
    }

    return hr;
}





///////////////////////////////////////////////////////////////////////////////
//  C R E A T E   I N V I T A T I O N

//-----------------------------------------------------------------------------
// Function: CreateInvitationProc
//
// Purpose:  DialogProc to create and save an invitation to a file.
//
// Returns:  Returns TRUE if it processed the message, and FALSE if it did not.
//
LRESULT CALLBACK CreateInvitationProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    //Unreferenced parameter
    lParam;
    
    switch (message)
    {
        case WM_INITDIALOG:
            if (g_dwAuth == PEER_GROUP_PASSWORD_AUTHENTICATION)
            {
                SendDlgItemMessage(hDlg, IDC_EDT_IDENT_LOCATION, WM_SETTEXT, 0, (LPARAM) L"Password-based group");
                EnableWindow(GetDlgItem(hDlg, IDC_EDT_IDENT_LOCATION), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_STATIC_IDENT), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_BTN_IDENT_BROWSE), FALSE);
            }
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_BTN_IDENT_BROWSE:
                    BrowseHelper(hDlg, IDC_EDT_IDENT_LOCATION, L"Identity Information", c_wzFileExtIdt, TRUE);
                    break;

                case IDC_BTN_INV_BROWSE:
                    BrowseHelper(hDlg, IDC_EDT_INV_LOCATION, L"Group Invitation", c_wzFileExtInv, FALSE);
                    break;

                case IDOK:
                    if (SUCCEEDED(HandleCreateInvitation(hDlg)))
                    {
                        EndDialog(hDlg, IDOK);
                    }
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;
                default:
                    break;
            }
            break; /* WM_COMMAND */

        default:
            break;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
// Function: HandleCreateInvitation
//
// Purpose:  Extracts the information from the dialog and calls CreateInvitation
//           to do the actual work.
//
// Returns:  HRESULT
//
HRESULT HandleCreateInvitation(HWND hDlg)
{
    HRESULT hr = S_OK;

    WCHAR wzIdentityInfo[MAX_PATH] = {0};
    WCHAR wzInvitation[MAX_PATH] = {0};

    SendDlgItemMessage(hDlg, IDC_EDT_INV_LOCATION, WM_GETTEXT,
            celems(wzInvitation), (LPARAM) wzInvitation);

    if (g_dwAuth == PEER_GROUP_GMC_AUTHENTICATION)
    {
        SendDlgItemMessage(hDlg, IDC_EDT_IDENT_LOCATION, WM_GETTEXT,
            celems(wzIdentityInfo), (LPARAM) wzIdentityInfo);
    }        

    hr = CreateInvitation(wzIdentityInfo, wzInvitation);
    return hr;
}

//-----------------------------------------------------------------------------
// Function: CreateInvitation
//
// Purpose:  Creates an invitation file for an identity.
//           Displays a message if there was an error.
//
// Parameters:
//     wzIdentityInfoPath [in] : Path to identity information file
//                              (not used in password based invitations)
//     wzInvitationPath   [in] : Path to invitation file
//
// Returns:  HRESULT
//
HRESULT CreateInvitation(PCWSTR wzIdentityInfoPath, PCWSTR wzInvitationPath)
{
    HRESULT hr = S_OK;
    WCHAR wzIdentityInfo[MAX_INVITATION] = {0};
    PWSTR pwzInvitation = NULL;

    // Do not try to open an identity file if we are creating an invitation
    // for a password based group
    if (g_dwAuth == PEER_GROUP_GMC_AUTHENTICATION)
    {
        FILE*    pFile = NULL;
        errno_t  err  = 0;
        
        err = _wfopen_s(&pFile, wzIdentityInfoPath, L"rb");
        if (err != 0)
        {
            hr = E_FAIL;
            DisplayHrError(L"Please choose a valid path to the identity information file.", hr);
        }
        else
        {
            fread(wzIdentityInfo, sizeof(WCHAR), MAX_INVITATION, pFile);
            if (ferror(pFile))
            {
                hr = E_FAIL;
                DisplayHrError(L"File read error occurred.", hr);
            }
            fclose(pFile);
        }
    }

    if (SUCCEEDED(hr))
    {
        ULONGLONG ulExpire; // adjust time using this structure
        GetSystemTimeAsFileTime((FILETIME *)&ulExpire);

        // 15 days in 100 nanoseconds resolution
        ulExpire += ((ULONGLONG) (60 * 60 * 24 * 15)) * ((ULONGLONG)1000*1000*10);

        if (g_dwAuth == PEER_GROUP_GMC_AUTHENTICATION)
        {
            hr = PeerGroupCreateInvitation(g_hGroup, wzIdentityInfo, (FILETIME*)&ulExpire, 1, 
                                        (PEER_ROLE_ID*) &PEER_GROUP_ROLE_MEMBER, &pwzInvitation);
        }
        else
        {
            hr = PeerGroupCreatePasswordInvitation(g_hGroup, &pwzInvitation);
        }
        if (FAILED(hr))
        {
            DisplayHrError(L"Failed to create the invitation.", hr);
        }
    }

    // Save invitation to file
    if (SUCCEEDED(hr))
    {
        FILE* pFile = NULL;
        errno_t err = 0;
        
        err = _wfopen_s(&pFile, wzInvitationPath, L"wb");
        if (err != 0)
        {
            hr = E_FAIL;
            DisplayHrError(L"Please choose a valid path to the invitation file.", hr);
        }
        else
        {
            if (fputws(pwzInvitation, pFile) == WEOF)
            {
                hr = E_FAIL;
                DisplayHrError(L"End of file error.", hr);
            }
            fclose(pFile);
        }
    }

    PeerFreeData(pwzInvitation);
    return hr;
}



//-----------------------------------------------------------------------------
// Function: BrowseHelper
//
// Purpose:  Use the common dialog to get/set a path.
//
// Returns:  nothing
//
void BrowseHelper(HWND hDlg, int idEditbox, PCWSTR pwzFileType, PCWSTR pwzFileExtension, BOOL fOpen)
{
    OPENFILENAME ofn = {0};
    WCHAR wzPath[MAX_PATH] = {0};
    WCHAR wzFilter[512] = {0};
    BOOL fSuccess = FALSE;

    StringCbPrintf(wzFilter, sizeof(wzFilter), L"%s (*.%s)%c*.%s", pwzFileType, pwzFileExtension, L'\0', pwzFileExtension);

    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFilter = wzFilter;
    ofn.lpstrFile = wzPath;
    ofn.lpstrDefExt = pwzFileExtension;
    ofn.nMaxFile = celems(wzPath);

    if (fOpen)
    {
        fSuccess = GetOpenFileName(&ofn);
    }
    else
    {
        fSuccess = GetSaveFileName(&ofn);
    }

    if (fSuccess)
    {
        SetDlgItemText(hDlg, idEditbox, wzPath);
    }
}

//-----------------------------------------------------------------------------
// Function: RegisterForEvents
//
// Purpose:  Registers the EventCallback function so it will be called for only
//           those events that are specified.
//
// Returns:  HRESULT
//
HRESULT RegisterForEvents(void)
{
    HRESULT hr = S_OK;
    PEER_GROUP_EVENT_REGISTRATION regs[] = {
        { PEER_GROUP_EVENT_RECORD_CHANGED, &RECORD_TYPE_CHAT_MESSAGE },
        { PEER_GROUP_EVENT_MEMBER_CHANGED, 0 },
        { PEER_GROUP_EVENT_STATUS_CHANGED, 0 },
        { PEER_GROUP_EVENT_DIRECT_CONNECTION, &DATA_TYPE_WHISPER_MESSAGE },
        { PEER_GROUP_EVENT_INCOMING_DATA, 0 },
    };

    g_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_hEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        hr = PeerGroupRegisterEvent(g_hGroup, g_hEvent, celems(regs), regs,  &g_hPeerEvent);
    }

    if (SUCCEEDED(hr))
    {
        if (!RegisterWaitForSingleObject(&g_hWait, g_hEvent, EventCallback, NULL, INFINITE, WT_EXECUTEDEFAULT))
        {
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function: ProcessRecordChanged
//
// Purpose:  Processes the PEER_GROUP_EVENT_RECORD_CHANGED event.
//
// Returns:  nothing
//
void ProcessRecordChanged(PEER_EVENT_RECORD_CHANGE_DATA * pData)
{
    switch (pData->changeType)
    {
        case PEER_RECORD_ADDED:
            if (IsEqualGUID(&pData->recordType, &RECORD_TYPE_CHAT_MESSAGE))
            {
                PEER_RECORD * pRecord = {0};
                HRESULT hr = PeerGroupGetRecord(g_hGroup, &pData->recordId, &pRecord);
                if (SUCCEEDED(hr))
                {
                    DisplayChatMessage(pRecord->pwzCreatorId, (PCWSTR) pRecord->data.pbData);
                    PeerFreeData(pRecord);
                }
            }
            break;

        case PEER_RECORD_UPDATED:
        case PEER_RECORD_DELETED:
        case PEER_RECORD_EXPIRED:
            break;

        default:
            break;
    }
}


//-----------------------------------------------------------------------------
// Function: ProcessStatusChanged
//
// Purpose:  Processes the PEER_GROUP_EVENT_STATUS_CHANGED event.
//
// Returns:  nothing
//
void ProcessStatusChanged(DWORD dwStatus)
{
    HRESULT hr = S_OK;
    PWSTR pwzStatus = L"";
    PEER_GROUP_PROPERTIES * pProperties = NULL;
    WCHAR wzChatTitle[MAX_GROUPNAME + 20] = {0};

    StringCbPrintf(wzChatTitle, sizeof(wzChatTitle), L"Offline");

    if (dwStatus & PEER_GROUP_STATUS_HAS_CONNECTIONS)
    {
        pwzStatus = L"connected";
        hr = PeerGroupGetProperties(g_hGroup, &pProperties);
        if (SUCCEEDED(hr))
            StringCbPrintf(wzChatTitle, sizeof(wzChatTitle), L"Chatting in %s ", pProperties->pwzFriendlyName);

    }
    else if (dwStatus & PEER_GROUP_STATUS_LISTENING)
    {
        pwzStatus = L"listening";
        hr = PeerGroupGetProperties(g_hGroup, &pProperties);
        if (SUCCEEDED(hr))
            StringCbPrintf(wzChatTitle, sizeof(wzChatTitle), L"Waiting to chat in %s ", pProperties->pwzFriendlyName);
    }

    SetWindowText(g_hwndChatLabel, wzChatTitle);
    PeerFreeData(pProperties);

    SendMessage(g_hwndStatus, SB_SETTEXT, SB_PART_STATUS, (LPARAM) pwzStatus);

    UpdateParticipantList( );
}


//-----------------------------------------------------------------------------
// Function: PrepareToChat
//
// Purpose:  Does the initial hookup of the group required after a successful
//           create, open or join.
//
// Returns:  HRESULT
//
HRESULT PrepareToChat(void)
{
    HRESULT hr = RegisterForEvents( );
    DWORD dwStatus;

    if (FAILED(hr))
    {
        DisplayHrError(L"Unable to register for events.", hr);
    }
    if (SUCCEEDED(hr))
    {
        hr = PeerGroupConnect(g_hGroup);
        if (FAILED(hr))
        {
            DisplayHrError(L"Unable to connect to the group.", hr);
        }
    }
    if (SUCCEEDED(hr))
    {
        EnableMenuItem(GetMenu(g_hwndMain), IDM_CREATEINVITATION, MF_ENABLED);
        hr = PeerGroupGetStatus(g_hGroup, &dwStatus);
        if (SUCCEEDED(hr))
        {
            ProcessStatusChanged(dwStatus);
        }
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function: FindParticipant
//
// Purpose:  Find a participant of the group chat in the list based on their identity.
//
// Returns:  the index of position in the list or -1 if not found.
//
int FindParticipant(PCWSTR pwzIdentity)
{
    int iItem = ListBox_GetCount(g_hwndMembers);
    while ((--iItem) >= 0)
    {
        PCWSTR pwzData = (PCWSTR) ListBox_GetItemData(g_hwndMembers, iItem);
        if (pwzData != NULL)
        {
            if (0 == wcscmp(pwzData, pwzIdentity))
            {
                return iItem;
            }
        }
    }

    return -1;
}

//-----------------------------------------------------------------------------
// Function: AddParticipantName
//
// Purpose:  Adds a participant name to the list.
//           Allocates a copy of the identity string to store in the data area
//           of the list item.
//
// Returns:  nothing
//
void AddParticipantName(PCWSTR pwzIdentity, PCWSTR pwzUserName)
{
    int iItem;
    int cch;
    PWSTR pwzData = NULL;

    iItem = ListBox_AddString(g_hwndMembers,  pwzUserName);
    if (iItem < 0)
    {
        DisplayError(L"Unable to add participant name");
    }
    else
    {
        cch = (int) wcslen(pwzIdentity);
        pwzData = malloc((cch+1) * sizeof(WCHAR));
        if (NULL != pwzData)
        {
            StringCchCopy(pwzData, cch + 1, pwzIdentity);
        }

        ListBox_SetItemData(g_hwndMembers, iItem, pwzData);
    }
}


//-----------------------------------------------------------------------------
// Function: AddParticipant
//
// Purpose:  Adds a participant name to the member list.
//           Allocates a copy of the identity string to store in the data area
//           of the list item.
//
// Returns:  nothing
//
void AddParticipant(PCWSTR pwzIdentity)
{
    WCHAR  wzUserName[MAX_USERNAME];
    GetFriendlyNameForIdentity(pwzIdentity, wzUserName, celems(wzUserName));
    AddParticipantName(pwzIdentity, wzUserName);
    DisplayChatMessage(pwzIdentity, L"has joined the group");
}


//-----------------------------------------------------------------------------
// Function: DeleteParticipant
//
// Purpose:  Deletes the participant and associated data from the member list.
//
// Returns:  nothing
//
void DeleteParticipant(int iItem)
{
    PWSTR pwzData = (PWSTR) ListBox_GetItemData(g_hwndMembers, iItem);
    free(pwzData);
    ListBox_DeleteString(g_hwndMembers, iItem);
}

//-----------------------------------------------------------------------------
// Function: RemoveParticipant
//
// Purpose:  Removes a participant from the member list based on their identity.
//
// Returns:  nothing
//
void RemoveParticipant(PCWSTR pwzIdentity)
{
    int iItem = FindParticipant(pwzIdentity);
    if (iItem < 0)
    {
        DisplayError(L"Unable to find participant");
    }
    else
    {
        DeleteParticipant(iItem);
        DisplayChatMessage(pwzIdentity, L"has left the group");
    }
}


//-----------------------------------------------------------------------------
// Function: ClearParticipantList
//
// Purpose:  Clear the list of partipants.
//
// Returns:  nothing
//
void ClearParticipantList(void)
{
    while (0 != ListBox_GetCount(g_hwndMembers))
    {
        DeleteParticipant(0);
    }
}

//-----------------------------------------------------------------------------
// Function: UpdateParticipantList
//
// Purpose:  Update the list of partipants.
//
// Returns:  nothing
//
void UpdateParticipantList(void)
{
    HRESULT          hr = S_OK;
    HPEERENUM        hPeerEnum = NULL;
    PEER_MEMBER   ** ppMember = NULL;

    ClearParticipantList( );
    if (NULL == g_hGroup)
    {
        return;
    }

    // Retreive only the members currently present in the group.
    hr = PeerGroupEnumMembers(g_hGroup, PEER_MEMBER_PRESENT, NULL, &hPeerEnum);
    if (SUCCEEDED(hr))
    {
        while (SUCCEEDED(hr))
        {
            ULONG cItem = 1;
            hr = PeerGetNextItem(hPeerEnum, &cItem, (PVOID **) &ppMember);
            if (SUCCEEDED(hr))
            {
                if (0 == cItem)
                {
                    PeerFreeData(ppMember);
                    break;
                }
            }

            if (SUCCEEDED(hr))
            {
                if (0 != ((*ppMember)->dwFlags & PEER_MEMBER_PRESENT))
                {
                    AddParticipantName((*ppMember)->pwzIdentity, (*ppMember)->pCredentialInfo->pwzFriendlyName);
                }
                PeerFreeData(ppMember);
            }
        }

        PeerEndEnumeration(hPeerEnum);
    }
}

//-----------------------------------------------------------------------------
// Function: ProcessMemberChanged
//
// Purpose:  Processes the PEER_GROUP_EVENT_MEMBER_CHANGED event.
//
// Returns:  nothing
//
void ProcessMemberChanged(PEER_EVENT_MEMBER_CHANGE_DATA * pData)
{
    switch (pData->changeType)
    {
        case PEER_MEMBER_JOINED:
            break;

        case PEER_MEMBER_LEFT:
            break;

        case PEER_MEMBER_CONNECTED:
            // This check must be made in case PEER_MEMBER_UPDATED is fired first
            if (FindParticipant(pData->pwzIdentity) == -1)
            {
                AddParticipant(pData->pwzIdentity);
            }
            break;

        case PEER_MEMBER_DISCONNECTED:
            RemoveParticipant(pData->pwzIdentity);
            break;

        case PEER_MEMBER_UPDATED:
            if (FindParticipant(pData->pwzIdentity) != -1)
            {
                RemoveParticipant(pData->pwzIdentity);
            }
            AddParticipant(pData->pwzIdentity);
            break;

        default:
            break;
    }
}

//-----------------------------------------------------------------------------
// Function: EventCallback
//
// Purpose:  Handle events raised by the grouping infrastructure.
//
// Returns:  nothing
//
VOID CALLBACK EventCallback(PVOID lpParam, BOOLEAN reason)
{
    //Unreferenced parameters
    lpParam;
    reason;

    for (;;)
    {
        PEER_GROUP_EVENT_DATA *pEventData = NULL;
        HRESULT hr = PeerGroupGetEventData(g_hPeerEvent, &pEventData);
        if (FAILED(hr) || (PEER_S_NO_EVENT_DATA == hr) || (NULL == pEventData))
        {
            break;
        }

        switch (pEventData->eventType)
        {
            case PEER_GROUP_EVENT_RECORD_CHANGED:
                ProcessRecordChanged(&pEventData->recordChangeData);
                break;

            case PEER_GROUP_EVENT_STATUS_CHANGED:
                ProcessStatusChanged(pEventData->dwStatus);
                break;

            case PEER_GROUP_EVENT_MEMBER_CHANGED:
                ProcessMemberChanged(&pEventData->memberChangeData);
                break;

            case PEER_GROUP_EVENT_DIRECT_CONNECTION:
                break;

            case PEER_GROUP_EVENT_INCOMING_DATA:
                ProcessIncomingData(pEventData);
                break;

            default:
                break;
        }

        PeerFreeData(pEventData);
    }
}

//-----------------------------------------------------------------------------
// Function: RefreshGroupCombo
//
// Purpose:  Given an identity name (in pwzIdentity), fills the combo box (specified by hwnd) with
//           all the groups accessible by the specified identity.  The ItemData of each entry
//           in the combobox points to the groupid.  For these pointers to remain valid after
//           the call, the PEER_NAME_PAIR array is returned, and must be freed via PeerFreeData( )
//           by the calling function OpenGroupProc( ).
//
// Returns:  HRESULT
//
HRESULT RefreshGroupCombo(HWND hwndCtrl, PCWSTR pwzIdentity, PEER_NAME_PAIR ***pppNamePairs)
{
    ULONG cGroups = 0;
    HPEERENUM hPeerEnum = NULL;
    HRESULT hr = S_OK;

    *pppNamePairs = NULL;
    SendMessage(hwndCtrl, CB_RESETCONTENT, 0, 0);

    hr = PeerEnumGroups(pwzIdentity, &hPeerEnum);
    if (SUCCEEDED(hr))
    {
        hr = PeerGetItemCount(hPeerEnum, &cGroups);

        if (SUCCEEDED(hr))
        {
            hr = PeerGetNextItem(hPeerEnum, &cGroups, (VOID***) pppNamePairs);
        }

        if (SUCCEEDED(hr))
        {
            ULONG i;
            for (i = 0; i < cGroups; i++)
            {
                int iItem = ComboBox_AddString(hwndCtrl, (*pppNamePairs)[i]->pwzFriendlyName);
                ComboBox_SetItemData(hwndCtrl, iItem, (*pppNamePairs)[i]->pwzPeerName);
            }
            if (cGroups > 0)
            {
                ComboBox_SetCurSel(hwndCtrl, 0);
            }
        }
        PeerEndEnumeration(hPeerEnum);
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function: RefreshIdentityCombo
//
// Purpose:  Fills the specified combo box w/ all the available identities.  The combo box will show
//           the friendly names for the identities, and the ItemData will point to the PeerNames.  For
//           these pointers to remain valid the PEER_NAME_PAIR array is returned w/ the function and
//           must be freed via PeerFreeData( ) by the calling function.
//
// Returns:  HRESULT
//
HRESULT RefreshIdentityCombo(HWND hwndCtrl, BOOL bAddNullIdentity, PEER_NAME_PAIR ***pppNamePairs)
{
    ULONG cIdentities = 0;
    HPEERENUM hPeerEnum = NULL;
    HRESULT hr = S_OK;

    *pppNamePairs = NULL;

    ComboBox_ResetContent(hwndCtrl);

    hr = PeerEnumIdentities(&hPeerEnum);
    if (SUCCEEDED(hr))
    {
        hr = PeerGetItemCount(hPeerEnum, &cIdentities);

        if (SUCCEEDED(hr))
        {
            hr = PeerGetNextItem(hPeerEnum, &cIdentities, (VOID***) pppNamePairs);
        }

        if (SUCCEEDED(hr))
        {
            ULONG i;
            for (i = 0; i < cIdentities; i++)
            {
                int iIdentity = ComboBox_AddString(hwndCtrl, (*pppNamePairs)[i]->pwzFriendlyName);
                ComboBox_SetItemData(hwndCtrl, iIdentity, (*pppNamePairs)[i]->pwzPeerName);
            }
            if (cIdentities > 0)
            {
                ComboBox_SetCurSel(hwndCtrl, 0);
            }
            if (bAddNullIdentity)
            {
                ComboBox_AddString(hwndCtrl, L"NULL Identity");
                
                if (cIdentities == 0)
                {
                    ComboBox_SetCurSel(hwndCtrl, 0);
                }
            }
        }

        PeerEndEnumeration(hPeerEnum);
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Function: DisplayChatMessage
//
// Purpose:  Display a chat message for an identity.
//
// Returns:  nothing
//
void DisplayChatMessage(PCWSTR pwzIdentity, PCWSTR pwzMsg)
{
    WCHAR wzName[MAX_USERNAME];
    WCHAR wzMessage[MAX_CHAT_MESSAGE + MAX_USERNAME + 10];

    // Retrieve the friendly name for the identity
    GetFriendlyNameForIdentity(pwzIdentity, wzName, celems(wzName));

    // Format the message
    StringCbPrintf(wzMessage, sizeof(wzMessage), L"[%s]: %s\r\n", wzName, pwzMsg);

    DisplayMsg(wzMessage);
}

//-----------------------------------------------------------------------------
// Function: DisplayReceivedWhisper
//
// Purpose:  Display a whispered message for an identity.
//
// Returns:  nothing
//
void DisplayReceivedWhisper(PCWSTR pwzIdentity, PCWSTR pwzMsg)
{
    WCHAR wzName[MAX_USERNAME];
    WCHAR wzMsg[MAX_CHAT_MESSAGE + MAX_USERNAME + 20];

    // Format the incoming message with the originator's name
    GetFriendlyNameForIdentity(pwzIdentity, wzName, celems(wzName));

    // Format the message
    StringCbPrintf(wzMsg, sizeof(wzMsg), L"<Whisper from %s>: %s\r\n", wzName, pwzMsg);

    DisplayMsg(wzMsg);
}

//-----------------------------------------------------------------------------
// Function: DisplaySentWhisper
//
// Purpose:  Display a whispered message that was sent.
//
// Returns:  nothing
//
void DisplaySentWhisper(PCWSTR pwzMsg)
{
    WCHAR wzMsg[MAX_CHAT_MESSAGE + MAX_USERNAME + 20];

    // Format the originator's message
    StringCbPrintf(wzMsg, sizeof(wzMsg), L"<Whisper to %s>: %s\r\n", g_wzDCName, pwzMsg);
    DisplayMsg(wzMsg);
}


//-----------------------------------------------------------------------------
// Function: DisplayMsg
//
// Purpose:  Display a message in the window.
//
// Returns:  nothing
//
void DisplayMsg(PCWSTR pwzMsg)
{
    SendMessage(g_hwndMsg, EM_SETSEL, 0x7fffffff, (LPARAM) -1);
    SendMessage(g_hwndMsg, EM_REPLACESEL, 0, (LPARAM) pwzMsg);
    SendMessage(g_hwndMsg, EM_SCROLLCARET, 0, 0);
}


//-----------------------------------------------------------------------------
// Function: AddChatRecord
//
// Purpose:  This adds a new chat message record to the group.
//
// Returns:  HRESULT
//
HRESULT AddChatRecord(PCWSTR pwzMessage)
{
    HRESULT     hr = S_OK;
    PEER_RECORD record = {0};
    GUID        idRecord;
    ULONGLONG   ulExpire;
    ULONG cch = (ULONG) wcslen(pwzMessage);

    GetSystemTimeAsFileTime((FILETIME *) &ulExpire);

    // calculate 2 minute expiration time in 100 nanosecond resolution
    ulExpire += ((ULONGLONG) 60 * 2) * ((ULONGLONG)1000*1000*10);

    // Set up the record
    record.dwSize = sizeof(record);
    record.data.cbData = (cch+1) * sizeof(WCHAR);
    record.data.pbData = (PBYTE) pwzMessage;
    memcpy(&record.ftExpiration, &ulExpire, sizeof(ulExpire));

    PeerGroupUniversalTimeToPeerTime(g_hGroup, &record.ftExpiration, &record.ftExpiration);

    // Set the record type GUID
    record.type = RECORD_TYPE_CHAT_MESSAGE;

    // Add the record to the database
    hr = PeerGroupAddRecord(g_hGroup, &record, &idRecord);
    if (FAILED(hr))
    {
        DisplayHrError(L"Failed to add a chat record to the group.", hr);
    }

    return hr;
}


//-----------------------------------------------------------------------------
// Function: CleanupGroup
//
// Purpose:  Clean up all the global variables associated with this group.
//
// Returns:  nothing
//
void CleanupGroup(void)
{
    if (g_hPeerEvent != NULL)
    {
        PeerGroupUnregisterEvent(g_hPeerEvent);
        g_hPeerEvent = NULL;
    }

    if (g_hEvent != NULL)
    {
        CloseHandle(g_hEvent);
        g_hEvent = NULL;
    }

    if (g_hWait != NULL)
    {
        UnregisterWaitEx(g_hWait, INVALID_HANDLE_VALUE);
        g_hWait = NULL;
    }

    if (g_hGroup != NULL)
    {
        PeerGroupClose(g_hGroup);
        g_hGroup = NULL;
    }
}

void CmdCloseGroup(void)
{
    CleanupGroup( );
    ProcessStatusChanged(0);
    SetStatus(L"Closed group");
}




///////////////////////////////////////////////////////////////////////////////
//  D I R E C T    C O N N E C T I O N    R O U T I N E S


//-----------------------------------------------------------------------------
// Function: ProcessIncomingData
//
// Purpose:  Processes the PEER_GROUP_EVENT_INCOMING_DATA event.
//
// Returns:  nothing
//
void ProcessIncomingData(PEER_GROUP_EVENT_DATA  * pEventData)
{
    HRESULT hr = S_OK;
    HPEERENUM hPeerEnum;
    PEER_CONNECTION_INFO ** ppConnectionInfo = NULL;
    ULONG ulCount = 0;

    // Get a list of all the active direct connections
    hr = PeerGroupEnumConnections(g_hGroup, PEER_CONNECTION_DIRECT, &hPeerEnum);
    if (SUCCEEDED(hr))
    {
        hr = PeerGetItemCount(hPeerEnum, &ulCount);
        if (SUCCEEDED(hr))
        {
            hr = PeerGetNextItem(hPeerEnum, &ulCount, (PVOID**) &ppConnectionInfo);
        }

        if (SUCCEEDED(hr))
        {
            ULONG i;
            for (i=0; i < ulCount; i++)
            {
                PEER_CONNECTION_INFO * pConnectionInfo = ppConnectionInfo[i];
                if (pEventData->incomingData.ullConnectionId == pConnectionInfo->ullConnectionId)
                {
                    // assume pbData is a null terminated string
                    DisplayReceivedWhisper(pConnectionInfo->pwzPeerId, (PCWSTR) pEventData->incomingData.data.pbData);
                    break;
                }
            }
            PeerFreeData(ppConnectionInfo);
        }
        hr = PeerEndEnumeration(hPeerEnum);
    }

    hr = PeerGroupCloseDirectConnection(g_hGroup, pEventData->incomingData.ullConnectionId);
}


//-----------------------------------------------------------------------------
// Function: SetupDirectConnection
//
// Purpose:  Setup the DirectConnection for the Whisper.
//
// Returns:  HRESULT
//
HRESULT SetupDirectConnection(PCWSTR pwzIdentity)
{
    HRESULT hr = S_OK;

    HPEERENUM hPeerEnum;
    ULONG cItem = 1;
    PEER_MEMBER ** ppMember = NULL;

    GetFriendlyNameForIdentity(pwzIdentity, g_wzDCName, celems(g_wzDCName));

    // Determine the identity of the peer
    hr = PeerGroupEnumMembers(g_hGroup, PEER_MEMBER_PRESENT, pwzIdentity, &hPeerEnum);
    if (SUCCEEDED(hr))
    {
        // Get the PEER_MEMBER info for this peer to later obtain its pAddresses
        hr = PeerGetNextItem(hPeerEnum, &cItem, (PVOID**) &ppMember);
        PeerEndEnumeration(hPeerEnum);
    }

    if (SUCCEEDED(hr))
    {
        hr = PeerGroupOpenDirectConnection(g_hGroup, pwzIdentity, (*ppMember)->pAddresses, &g_ullConnectionId);

        if (FAILED(hr))
        {
            if (PEER_E_CONNECT_SELF == hr)
            {
               DisplayHrError(L"\nCan't whisper to yourself.\n\nPEER_E_CONNECT_SELF", hr);
            }
            else if (E_INVALIDARG == hr)
            {
               DisplayHrError(L"Can’t open direct connection.", hr);
            }
        }
        PeerFreeData(ppMember);
    }
    return hr;
}


//-----------------------------------------------------------------------------
// Function: WhisperMessageProc
//
// Purpose:  DialogProc to send a Whisper
//
// Returns:  Returns TRUE if it processed the message, and FALSE if it did not.
//
LRESULT CALLBACK WhisperMessageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    //Unreferenced parameters
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:
            SendDlgItemMessage(hDlg, IDC_EDT_MESSAGE, EM_SETLIMITTEXT, MAX_CHAT_MESSAGE - 1, 0);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDSEND:
                    if (SUCCEEDED(HandleWhisperMessage(hDlg)))
                    {
                        EndDialog(hDlg, IDOK);
                    }
                    break;

                case IDCANCEL:
                    PeerGroupCloseDirectConnection(g_hGroup, g_ullConnectionId);
                    EndDialog(hDlg, IDCANCEL);
                    break;
            }
            break;

        default:
            break; /* WM_COMMAND */
    }

    return FALSE;
}


//-----------------------------------------------------------------------------
// Function: HandleWhisperMessage
//
// Purpose:  Extracts the message from the dialog and
//           readies a whisper to be sent via PeerGroupOpenDirectConnection.
//
// Returns:  HRESULT
//
HRESULT HandleWhisperMessage(HWND hDlg)
{
    HRESULT hr = S_OK;
    WCHAR    wzMessage[MAX_CHAT_MESSAGE];

    UINT cch = GetDlgItemText(hDlg, IDC_EDT_MESSAGE, wzMessage, celems(wzMessage));
    if (0 == cch)
    {
        DisplayHrError(L"Please type a msg for the Whisper.", hr);
        return E_FAIL;
    }

    // Send the message using the current direct connection
    hr = PeerGroupSendData(g_hGroup, g_ullConnectionId, &DATA_TYPE_WHISPER_MESSAGE,
        sizeof(WCHAR) * (cch+1), wzMessage);

    if (FAILED(hr))
    {
        DisplayHrError(L"Failed to send data across direct connection.", hr);
    }
    else
    {
        DisplaySentWhisper(wzMessage);
    }

    return hr;
}




///////////////////////////////////////////////////////////////////////////////
//  U T I L I T Y    R O U T I N E S

//-----------------------------------------------------------------------------
// Function: GetSelectedIdentity
//
// Purpose:  Get the currently selected identity.
//
// Returns:  A pointer to the identity string.
//
PCWSTR GetSelectedIdentity(HWND hDlg)
{
    PCWSTR pwzIdentity = NULL;
    HWND hwndCtrl = GetDlgItem(hDlg, IDC_CB_IDENTITY);
    int iItem =  ComboBox_GetCurSel(hwndCtrl);

    if (iItem >= 0)
    {
        pwzIdentity = (PCWSTR) ComboBox_GetItemData(hwndCtrl, iItem);
    }
    return pwzIdentity;
}

//-------------------------------------------------------------------------
// Function: GetLocalCloudName
//
// Purpose:  Retrieve first available local cloud name
//
// Arguments:
//   cchCloudNameSize[in]: number of characters in pwzCloudName 
//                         (usually MAX_CLOUD_NAME)
//   pwzCloudName [out]  : location to which cloud name will be copied
//
// Returns:  HRESULT
//
HRESULT GetLocalCloudName(ULONG cchCloudName, __out_ecount(cchCloudName) PWSTR pwzCloudName)
{
    HRESULT         hr = S_OK;
    PNRPCLOUDINFO   CloudInfo = {0};
    BLOB            blPnrpData = {0};
    WSAQUERYSET     querySet = {0};
    WSAQUERYSET*    pResults = NULL;
    HANDLE          hLookup = NULL;
    INT             iErr = 0;
    DWORD           dwErr = 0;
    DWORD           dwResultSize = 0;

    // Fill out information for WSA query
    CloudInfo.dwSize = sizeof(CloudInfo);
    CloudInfo.Cloud.Scope = PNRP_LINK_LOCAL_SCOPE;
    
    blPnrpData.cbSize = sizeof(CloudInfo);
    blPnrpData.pBlobData = (LPBYTE)&CloudInfo;

    querySet.dwSize = sizeof(querySet);
    querySet.dwNameSpace = NS_PNRPCLOUD;
    querySet.lpServiceClassId = (LPGUID)&SVCID_PNRPCLOUD;
    querySet.lpBlob = &blPnrpData;

    iErr = WSALookupServiceBegin(&querySet, LUP_RETURN_NAME, 
            &hLookup);

    if (iErr != 0)
    {
        return iErr;
    } 
    else
    {
        WSAQUERYSET     tempResultSet = {0};
        dwResultSize = sizeof(tempResultSet);

        // Get size of results
        iErr = WSALookupServiceNext(hLookup, 0, &dwResultSize, &tempResultSet);

        if (iErr != 0)
        {   
            dwErr = WSAGetLastError();
        
            if (dwErr == WSAEFAULT)
            {
                // allocate space for results
                pResults = (WSAQUERYSET*) malloc(dwResultSize);
                if (pResults == NULL)
                {
                    hr = HRESULT_FROM_WIN32(WSAGetLastError());
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dwErr);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        // retrieve the local cloud information
        iErr = WSALookupServiceNext(hLookup, 0, &dwResultSize, pResults);
        if (iErr != 0)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }
    
    // Copy the cloud name (if applicable) and scope ID
    if (SUCCEEDED(hr))
    {
        hr = StringCchCopy(pwzCloudName, cchCloudName, pResults->lpszServiceInstanceName);                
        if (FAILED(hr))
        {
            DisplayHrError(L"Failed to copy cloud name", hr);
        }
    }
      
    if (hLookup != NULL)
    {
        (void) WSALookupServiceEnd(hLookup);
    }

    free(pResults);
    return hr;
}




//-----------------------------------------------------------------------------
// Function: GetSelectedGroup
//
// Purpose:  Get the currently selected group.
//
// Returns:  A pointer to the group PeerName.
//
PCWSTR GetSelectedGroup(HWND hDlg)
{
    PCWSTR pwzGroup = NULL;
    HWND hwndCtrl = GetDlgItem(hDlg, IDC_CB_GROUP);
    int iItem =  ComboBox_GetCurSel(hwndCtrl);
    if (iItem >= 0)
    {
        pwzGroup = (PCWSTR) ComboBox_GetItemData(hwndCtrl, iItem);
    }
    return pwzGroup;
}


//-----------------------------------------------------------------------------
// Function: GetSelectedChatMember
//
// Purpose:  Get the currently selected member from the members list.
//
// Returns:  A pointer to the identity string.
//
PCWSTR GetSelectedChatMember( )
{
    PCWSTR pwzIdentity = NULL;
    int    iItem = ListBox_GetCurSel(g_hwndMembers);
    if (iItem >= 0)
    {
        pwzIdentity = (PWSTR) ListBox_GetItemData(g_hwndMembers, iItem);
    }
    return pwzIdentity;
}


//-----------------------------------------------------------------------------
// Function: SetStatus
//
// Purpose:  Set the text of the status bar.
//
// Returns:  nothing
//
void SetStatus(PCWSTR pwzStatus)
{
    SendMessage(g_hwndStatus, SB_SETTEXT, SB_PART_MESSAGE, (LPARAM) pwzStatus);
}


//-----------------------------------------------------------------------------
// Function: GetFriendlyNameForIdentity
//
// Purpose:  Retrieve the friendly name for an identity
//
// Returns:  The number of characters copied to the buffer
//           (not including the terminating null.)
//
int GetFriendlyNameForIdentity(
              PCWSTR pwzIdentity, // The identity to find
        __out PWSTR pwzName,      // The buffer for the friendly name
              int cchMax)         // The size of the buffer (WCHARs)
{
    HRESULT          hr = S_OK;
    HPEERENUM        hPeerEnum = NULL;
    PEER_MEMBER   ** ppMember = NULL;

    // Always provide a default friendly name
    StringCchCopy(pwzName, cchMax, L"?");

    hr = PeerGroupEnumMembers(g_hGroup, 0, pwzIdentity, &hPeerEnum);
    if (SUCCEEDED(hr))
    {
        ULONG cItem = 1;
        hr = PeerGetNextItem(hPeerEnum, &cItem, (PVOID **) &ppMember);
        if (SUCCEEDED(hr) && (cItem > 0) && (NULL != ppMember))
        {
            StringCchCopy(pwzName, cchMax, (*ppMember)->pCredentialInfo->pwzFriendlyName);
        }
        PeerFreeData(ppMember);
        PeerEndEnumeration(hPeerEnum);
    }

    return (int) wcslen(pwzName);
}

//-----------------------------------------------------------------------------
// Function: DisplayError
//
// Purpose:  Display an error message.
//
// Returns:  nothing
//
void DisplayError(PCWSTR pwzMsg)
{
    WCHAR wzTitle[MAX_LOADSTRING];
    LoadString(g_hInst, IDS_APP_ERROR, wzTitle, MAX_LOADSTRING);
    MessageBox(GetWindow(g_hwndMain, GW_ENABLEDPOPUP), pwzMsg, wzTitle, MB_OK | MB_ICONWARNING);
}

//-----------------------------------------------------------------------------
// Function: GetErrorMsg
//
// Purpose:  Generate an error string from a HRESULT error code.
//
// Returns:  HRESULT
//
HRESULT GetErrorMsg(HRESULT hrError, ULONG cchMsg, __out_ecount(cchMsg) PWSTR pwzMsg)
{
    DWORD cch = 0;

    if (cchMsg == 0)
    {
        return E_FAIL;
    }

    if (HRESULT_FACILITY(hrError) == FACILITY_P2P)
    {
        HMODULE hResDll = GetModuleHandle(L"p2p.dll");

        if (NULL != hResDll)
        {
            cch = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                                hResDll,
                                HRESULTTOWIN32(hrError),
                                0,
                                pwzMsg,
                                cchMsg,
                                NULL);
            FreeLibrary(hResDll);
        }
    }
    else
    {
        cch = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            HRESULTTOWIN32(hrError),
                            0,
                            pwzMsg,
                            cchMsg,
                            NULL);
    }

    if (cch == 0)
        return E_FAIL;
    else
        return S_OK;
}  

//-----------------------------------------------------------------------------
// Function: MsgErrHr
//
// Purpose:  Display a message for a function with an error code.
//
// Returns:  nothing
//
void MsgErrHr(PCWSTR pwzText, HRESULT hrErr, PCSTR pszFunction)
{
    WCHAR wzError[512];
    WCHAR wzMsg[512];
    const WCHAR *pwzMsg;

    HRESULT hr = GetErrorMsg(hrErr, celems(wzMsg), wzMsg);

    if (FAILED(hr))
        pwzMsg = L"Unknown";
    else
        pwzMsg = wzMsg;

    StringCchPrintf(wzError, celems(wzError), L"%s\r\n\r\nFunction: %S\r\nHRESULT=0x%08X (%s)", 
                    pwzText, pszFunction, hrErr, pwzMsg);

    MessageBox(GetWindow(g_hwndMain, GW_ENABLEDPOPUP), wzError, L"Graph Chat Error", MB_OK | MB_ICONWARNING);
}
