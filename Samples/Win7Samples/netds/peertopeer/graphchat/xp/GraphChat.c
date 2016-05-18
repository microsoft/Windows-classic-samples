/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    GraphChat.c

Abstract:

    This C file includes sample code for a chat application built
    with the Peer-to-Peer Graphing API.

Feedback:
    If you have any questions or feedback, please contact us using 
    any of the mechanisms below:

    Email: peerfb@microsoft.com 
    Newsgroup: Microsoft.public.win32.programmer.networks 
    Website: http://www.microsoft.com/p2p 

--********************************************************************/

#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4306)   // conversion to object of greater size

#include "GraphChat.h"
#include "pnrp.h"

// Global Variables:
//
HINSTANCE   g_hInst = NULL;             // The application instance
HWND        g_hwndMain    = NULL;       // The main window
HWND        g_hwndText    = NULL;       // The text to send for the user
HWND        g_hwndMsg     = NULL;       // The message area
HWND        g_hwndMembers = NULL;       // The list view of active members
HWND        g_hwndChatLabel = NULL;     // The static text of message area
HWND        g_hwndMemberLabel = NULL;   // The static text of active members
HWND        g_hwndSend    = NULL;       // The "Send" button
HWND        g_hwndStatus  = NULL;       // The status bar window
BOOL        g_fSynchronized = FALSE;    // True if this graph is synchonized
HGRAPH      g_hGraph = NULL;            // The graph object handle
HPEEREVENT  g_hPeerEvent = NULL;        // The one PeerEvent handle
HANDLE      g_hEvent = NULL;            // Handle signaled by Graphing when we have an event
HANDLE      g_hWait = NULL;             // Handle from RegisterWaitForSingleObject
WCHAR       g_wzGraphId[MAX_PEERNAME];  // Unique identifier for the graph
WCHAR       g_wzPeerId[MAX_PEERNAME];   // Unique identifier for this peer
PWSTR       g_pwzIdentity = NULL;       // Identity string
BOOL        g_fPnrpRegistered = FALSE;  // True if we have registered in PNRP
ULONGLONG   g_ullConnection = 0;        // Stores DirectConnectionID
WCHAR       g_wzWsprId[MAX_PEERNAME];   // Unique identifier for Whisper peer
PWSTR       g_pwzWsprMsg = NULL;        // Whisper Msg to send to the peer
BOOL        g_fGlobalScope = TRUE;      // True if the graph scope is global

// The unique identifier for chat messages
GUID CHAT_MESSAGE_RECORD_TYPE =
    {0x4d5b2f11, 0x6522, 0x433b, {0x84, 0xef, 0xa2, 0x98, 0xe6, 0x7, 0x57, 0xb0}};

// The unique identifier for Whisper (Private chat) messages
GUID WHISPER_MESSAGE_TYPE =
    {0x4d5b2f11, 0x6522, 0x433b, {0x84, 0xef, 0xa2, 0x98, 0xe6, 0x7, 0xbb, 0xbb}};


// Size of controls
const int c_dxBorder  = 5;
const int c_dyBorder  = 5;
const int c_dxMembers = 140;
const int c_dyStatic  = 20;
const int c_dyText    = 78;
const int c_dxButton  = 70;
const int c_dyButton  = 40;

const int c_dxWindow  = 640;
const int c_dyWindow  = 350;


// Local functions
HRESULT InitSystem(int nCmdShow);
BOOL ProcessSpecialKeys(MSG * pMsg);
void CmdDisconnect();


//-----------------------------------------------------------------------------
// Function: WinMain
//
// Purpose:  This is the main entry point for the application.
//
// Returns:  0
//
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, int nCmdShow)
{
    BOOL bAcclKeySet = TRUE;
    g_hInst = hInstance;

    //Unreferenced parameter
    hPrevInstance;
    lpCmdLine;

    SystemParametersInfo(SPI_SETKEYBOARDCUES, 0, &bAcclKeySet, 0);
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
        MessageBox(NULL, L"One or more GraphChat Components could not be initialized, make sure Peer-To-Peer is installed and enabled", L"Graph Chat Error", MB_OK | MB_ICONWARNING);
    }


    // Cleanup
    CleanupGraph();
    DeleteIdentity();
    WSACleanup();
    PeerGraphShutdown();

    return 0;
}



//-----------------------------------------------------------------------------
// Function: InitUI
//
// Purpose:  Initialize the User Interface for the main window.
//
// Returns:  HRESULT
//
HRESULT InitUI(void)
{
    HRESULT hr = S_OK;
    WNDCLASSEX wcex;
    WCHAR szTitle[MAX_LOADSTRING];
    WCHAR szWindowClass[MAX_LOADSTRING];

    InitCommonControls();

    // The title bar text
    LoadString(g_hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

    // the main window class name
    LoadString(g_hInst, IDS_APP_CLASS, szWindowClass, MAX_LOADSTRING);

    // Register the window class
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize           = sizeof(wcex);
    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc      = (WNDPROC)MainProc;
    wcex.hInstance        = g_hInst;
    wcex.hIcon            = LoadIcon(g_hInst, (LPCTSTR)IDI_APP);
    wcex.hIconSm          = wcex.hIcon;
    wcex.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_BTNFACE+1);
    wcex.lpszMenuName     = (LPCTSTR)IDC_MENU;
    wcex.lpszClassName    = szWindowClass;
    RegisterClassEx(&wcex);

    // Create the main window
    CreateWindowEx(0,szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, c_dxWindow, c_dyWindow,
        NULL, NULL, g_hInst, NULL);
    
    // g_hwndMain is set by WM_CREATE
    if (g_hwndMain == NULL)
        hr = E_FAIL;
    
    return hr;
}


//-----------------------------------------------------------------------------
// Function: InitSystem
//
// Purpose:  Initialize the main system (Peer-to-Peer, windows, controls, etc.)
//
// Returns:  HRESULT
//
HRESULT InitSystem(int nCmdShow)
{
    HRESULT hr = S_OK;
    WORD wVersionRequested = MAKEWORD(2,2);
    WSADATA wsaData;

    // Initialize WinSock (required to use PNRP)
    DWORD nRetVal = WSAStartup(wVersionRequested,&wsaData);
    if (nRetVal != 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());;
    }

    // Initialize Peer-to-Peer
    if (SUCCEEDED(hr))
    {
        PEER_VERSION_DATA peerVersion;
        hr = PeerGraphStartup(PEER_GRAPH_VERSION, &peerVersion);
    }

    // Create an identity for PNRP to use
    if (SUCCEEDED(hr))
    {
        hr = CreateIdentity();
    }

    if (SUCCEEDED(hr))
    {
        hr = InitUI();
    }

    if (SUCCEEDED(hr))
    {
        ShowWindow(g_hwndMain, nCmdShow);
        UpdateWindow(g_hwndMain);
    }

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

    if (NULL == g_hGraph)
        return;

    if (0 == GetWindowText(g_hwndText, wzMessage, celems(wzMessage)))
        return;

    if (FAILED(AddChatRecord(wzMessage)))
        return;

    // Clear the text box and prepare for the next line
    SetWindowText(g_hwndText, L"");
    SetFocus(g_hwndText);
}

//-----------------------------------------------------------------------------
// Function: ProcessSpecialKeys
//
// Purpose:  Conrol Tab key order and enter key.
//
// Returns:  Returns TRUE if we don't want default msg processing to occur.
//
BOOL ProcessSpecialKeys(__in MSG * pMsg)
{
    switch (pMsg->message)
    {
        case WM_CHAR:
            switch (pMsg->wParam)
            {
                case VK_TAB:
                {
                    BOOL fShift = 0 > GetKeyState(VK_SHIFT);
                    HWND hwnd = GetFocus();
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
                }
                case VK_RETURN:
                    ProcessSendButton();
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
            break;
        
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
void ProcessResizing(__in LPRECT pRect)
{
    const int dxMin = 300; // Minimum width of the main window
    const int dyMin = 200; // Minimum height of the main window

    if ((pRect->right - pRect->left) < dxMin)
    {
        pRect->right = pRect->left + dxMin;
    }

    if ((pRect->bottom - pRect->top) < dyMin)
    {
        pRect->bottom = pRect->top + dyMin;
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

    HDWP hDWP = BeginDeferWindowPos(7); // 7 windows will be repositioned

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

    hDWP = DeferWindowPos(hDWP, g_hwndMemberLabel, NULL,
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

    int  rgStatusBarWidths[1+SB_PART_MESSAGE] = {
        80,   // SB_PART_LISTENING
        180,  // SB_PART_SYNCHRONIZED
        260,  // SB_PART_CONNECTED
        510,  // SB_PART_ADDRESS
        -1};  // SB_PART_MESSAGE


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
    SendMessage(g_hwndText, EM_SETLIMITTEXT, (MAX_CHAT_MESSAGE - 1), 0);
    SetWindowFont(g_hwndText, hFont, FALSE);

    // Create the static text "Chat Members"
    g_hwndMemberLabel = CreateWindowEx(
        0, WC_STATIC, L"Chat Members",
        WS_CHILD | WS_VISIBLE | ES_READONLY | SS_CENTER,
        0, c_dyBorder, c_dxMembers, c_dyStatic,
        g_hwndMain, (HMENU) IDC_STATIC_MEMBERS, NULL, NULL);
    SetWindowFont(g_hwndMemberLabel, hFont, FALSE);

    // Create the listbox of active members
    g_hwndMembers = CreateWindowEx(
        0, WC_LISTBOX, L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_VSCROLL |
        LBS_SORT | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        0, c_dyBorder, c_dxMembers, 0,
        g_hwndMain, (HMENU) IDC_MEMBERS, NULL, NULL);
    SetWindowFont(g_hwndMembers, (HFONT) GetStockObject(DEFAULT_GUI_FONT), FALSE);

    // Create the static text for current Chat Status
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
    g_hwndStatus = CreateWindowEx(0,STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0,
        g_hwndMain, (HMENU) IDC_STATUS, NULL, NULL);
    SendMessage(
        g_hwndStatus, 
        (UINT)SB_SETPARTS, 
        celems(rgStatusBarWidths), 
        (LPARAM) &rgStatusBarWidths[0]
        );
}


//-----------------------------------------------------------------------------
// Function: MainProc
//
// Purpose:  Processes messages for the main window.
//
// Returns:  LRESULT (depends on message)
//
LRESULT CALLBACK MainProc(
                    HWND hWnd, 
                    UINT message, 
                    WPARAM wParam, 
                    LPARAM lParam
                    )
{
    switch (message) 
    {
        case WM_CREATE:
            g_hwndMain = hWnd;
            AddControls();
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

                case IDM_NEWGRAPH:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_NEWGRAPH), g_hwndMain, NewGraphProc);
                    break;

                case IDM_OPENGRAPH:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_OPENGRAPH), g_hwndMain, OpenGraphProc);
                    break;

                case IDC_MEMBERS:
                    if(HIWORD(wParam) == LBN_DBLCLK) //double click
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_WHISPER), g_hwndMain, WhisperProc);
                    break;

                case IDM_DISCONNECT:
                    CmdDisconnect();
                    break;

                case IDC_SEND:
                    ProcessSendButton();
                    break;

                default:
                    break;
            }

            break;
        
        default:
            break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}



//-----------------------------------------------------------------------------
// Function: EnableDisconnectMenu
//
// Purpose:  Enable (or disable) the "Disconnect" menu.
//
// Returns:  nothing
//
void EnableDisconnectMenu()
{
    BOOL fEnable = (g_hGraph != NULL);
    EnableMenuItem(GetMenu(g_hwndMain), IDM_DISCONNECT,
        fEnable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED));
}

//-----------------------------------------------------------------------------
// Function: CmdDisconnect
//
// Purpose:  Disconnect from the current graph.
//
// Returns:  nothing
//
void CmdDisconnect()
{
    CleanupGraph();
    SetStatusPart(SB_PART_ADDRESS, L"");
    ProcessStatusChangeEvent(0);
    SetStatus(L"Disconnected");
}

//-----------------------------------------------------------------------------
// Function: AboutProc
//
// Purpose:  DialogProc for the About box
//
// Returns:  Returns TRUE if we don't want default msg processing to occur.
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
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, 0);
            }
            break;
        default:
            break;
    }
    return FALSE;
}

//-----------------------------------------------------------------------------
// Function: NewGraphProc
//
// Purpose:  DialogProc to create a new graph
//
// Returns:  Returns TRUE if we don't want default msg processing to occur.
//
LRESULT CALLBACK NewGraphProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT CreatorIdLen;
    LRESULT GraphNameLen;

    //Unreferenced parameter
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:
            
            // Use global cloud by default (check global scope radio button)
            SendDlgItemMessage(hDlg, IDC_RADIO_GLOBAL_SCOPE, BM_SETCHECK, BST_CHECKED, 0);
            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    if (SUCCEEDED(HandleNewGraph(hDlg)))
                    {
                        SetStatus(L"graph created");
                    }
                    EndDialog(hDlg, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;
                case IDC_EDT_GRAPHNAME:
                case IDC_EDT_CREATORID:
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        GraphNameLen = SendDlgItemMessage(hDlg, IDC_EDT_GRAPHNAME, WM_GETTEXTLENGTH, 0, 0);
                        CreatorIdLen = SendDlgItemMessage(hDlg, IDC_EDT_CREATORID, WM_GETTEXTLENGTH, 0, 0);
                        EnableWindow(GetDlgItem(hDlg, IDOK), GraphNameLen > 0 && CreatorIdLen > 0);
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
// Function: OpenGraphProc
//
// Purpose:  DialogProc to open an existing graph
//
// Returns:  Returns TRUE if we don't want default msg processing to occur.
//
LRESULT CALLBACK OpenGraphProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT GraphIdLen;
    LRESULT PeerIdLen;

    //Unreferenced parameter
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:

            // Use global scope by default
            SendDlgItemMessage(hDlg, IDC_RADIO_GLOBAL_SCOPE, BM_SETCHECK, BST_CHECKED, 0);
            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
            
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    if (SUCCEEDED(HandleOpenGraph(hDlg)))
                    {
                        SetStatus(L"graph opened");
                    }
                    EndDialog(hDlg, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;
                case IDC_EDT_GRAPHID:
                case IDC_EDT_PEERID:
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        GraphIdLen = SendDlgItemMessage(hDlg, IDC_EDT_GRAPHID, WM_GETTEXTLENGTH, 0, 0);
                        PeerIdLen = SendDlgItemMessage(hDlg, IDC_EDT_PEERID, WM_GETTEXTLENGTH, 0, 0);
                        EnableWindow(GetDlgItem(hDlg, IDOK), GraphIdLen > 0 && PeerIdLen > 0);
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
// Function: WhisperProc
//
// Purpose:  DialogProc to do private messages in a graph
//
// Returns:  Returns TRUE if we don't want default msg processing to occur.
//
LRESULT CALLBACK WhisperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT WhisperLen;
    HRESULT hr = S_OK;

    //Unreferenced parameter
    lParam;

    switch (message)
    {
        case WM_INITDIALOG:
            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
            SetFocus(GetDlgItem(hDlg, IDC_EDT_WHISPER));
            SendMessage(GetDlgItem(hDlg, IDC_EDT_WHISPER), EM_SETLIMITTEXT, MAX_CHAT_MESSAGE, 0);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    hr = HandleDirectConnection(hDlg);
                    if (SUCCEEDED(hr))
                    {
                        SetStatus(L"Direct msg sent");
                    }
                    else if (PEER_E_CONNECT_SELF == hr)
                    {
                        DisplayHrError(L"Cannot Whisper to Self", hr);
                    }
                    else
                    {
                        DisplayHrError(L"Direct Communication Failed", hr);
                    }
                    EndDialog(hDlg, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDC_EDT_WHISPER:
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        WhisperLen = SendDlgItemMessage(hDlg, IDC_EDT_WHISPER, WM_GETTEXTLENGTH, 0, 0);
                        EnableWindow(GetDlgItem(hDlg, IDOK), WhisperLen > 0);
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
// Function: HandleNewGraph
//
// Purpose:  Extracts the information from the dialog and calls
//           CreateGraph to do the actual work.
//
// Returns:  HRESULT
//
HRESULT HandleNewGraph(__in HWND hDlg)
{
    HRESULT hr = S_OK;
    PEER_ADDRESS addr;
    WCHAR wzCreatorId[MAX_PEERNAME];
    WCHAR wzGraphName[MAX_PEERNAME];
    
    g_fGlobalScope = (SendDlgItemMessage(hDlg, IDC_RADIO_GLOBAL_SCOPE, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SendDlgItemMessage(hDlg, IDC_EDT_GRAPHNAME, WM_GETTEXT, celems(wzGraphName), (LPARAM) wzGraphName);    
    
    // Check for preexisting graph with this name
    hr = DiscoverAddress(wzGraphName, &addr); 
    if (hr == HRESULT_FROM_WIN32(WSA_E_NO_MORE)) 
    {
        // means no other graph with this name was found.
        // Hence we can go ahead and register one.
        SendDlgItemMessage(hDlg, IDC_EDT_CREATORID, WM_GETTEXT, celems(wzCreatorId), (LPARAM) wzCreatorId);
        hr = CreateGraph(wzCreatorId, wzGraphName);

    }
    else if (SUCCEEDED(hr))
    {
        DisplayHrError(L"GraphName already in use, please choose another name", hr);
    }
    else
    {
        DisplayHrError(L"Unable to Create Graph", hr);
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function: HandleOpenGraph
//
// Purpose:  Extracts the information from the dialog and calls
//           OpenGraph to do the actual work.
//
// Returns:  HRESULT
//
HRESULT HandleOpenGraph(__in HWND hDlg)
{
    HRESULT hr = S_OK;
    WCHAR wzGraphId[MAX_PEERNAME];
    WCHAR wzPeerId[MAX_PEERNAME];

    SendDlgItemMessage(hDlg, IDC_EDT_GRAPHID, WM_GETTEXT, celems(wzGraphId), (LPARAM) wzGraphId);
    SendDlgItemMessage(hDlg, IDC_EDT_PEERID, WM_GETTEXT, celems(wzPeerId), (LPARAM) wzPeerId);
    g_fGlobalScope = (SendDlgItemMessage(hDlg, IDC_RADIO_GLOBAL_SCOPE, BM_GETCHECK, 0, 0) == BST_CHECKED);
    
    hr = OpenGraph(wzPeerId, wzGraphId);
    if(FAILED(hr))
    {
        DisplayHrError(L"Unable to Open Graph", hr);
    }
    return hr;
}


//-----------------------------------------------------------------------------
// Function: HandleDirectConnection
//
// Purpose:  Extracts the information from the dialog and calls
//           OpenDirectConnection to do the actual work.
//
// Returns:  HRESULT
//
HRESULT HandleDirectConnection(__in HWND hDlg)
{
    LRESULT WhisperLen = 0;
    
    int iItem = ListBox_GetCurSel(g_hwndMembers);
    if (iItem < 0)
        return E_FAIL;
 
    ListBox_GetText(g_hwndMembers, iItem, g_wzWsprId);
    WhisperLen = SendDlgItemMessage(hDlg, IDC_EDT_WHISPER, WM_GETTEXTLENGTH, 0, 0);
    g_pwzWsprMsg = (WCHAR*)malloc( sizeof(WCHAR) * (WhisperLen+1) );
    if (g_pwzWsprMsg == NULL)
    {
        return E_OUTOFMEMORY;
    }
    SendDlgItemMessage(hDlg, IDC_EDT_WHISPER, WM_GETTEXT, WhisperLen+1, (LPARAM) g_pwzWsprMsg);
    return OpenDirectConnection();
}
    
//------------------------------------------------------------------
// Function: OpenDirectConnection
//
// Purpose:  Opens Direct Connection to the peer
//
// Returns:  HRESULT
//
HRESULT OpenDirectConnection()
{
    HRESULT hr = S_OK;
    HPEERENUM hPeerEnum;
    PEER_NODE_INFO ** ppNodeInfo = NULL;
    ULONG cItem = 0;
    WCHAR wzMsg[MAX_PATH];
    
    // Get a handle to an enumeration of peers corresponding to g_wzWsprID
    hr = PeerGraphEnumNodes(g_hGraph, g_wzWsprId, &hPeerEnum);
    if (SUCCEEDED(hr))
    {
        // Search for only one match
        cItem = 1; 

        // Extract the first data item from the enumeration 
        hr = PeerGraphGetNextItem(hPeerEnum, &cItem, &ppNodeInfo);
        PeerGraphEndEnumeration(hPeerEnum);
    }

    if (SUCCEEDED(hr))
    {
        if ( (cItem == 0) || (NULL == ppNodeInfo) )
        {
            hr = E_FAIL;
        }
        else if ((*ppNodeInfo)->cAddresses == 0)
        {
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
        // The direct connection uses the first address from the destination
        // node's PEER_NODE_INFO struct (pAddresses[0])
        hr = PeerGraphOpenDirectConnection(g_hGraph, g_wzWsprId,
            &(*ppNodeInfo)->pAddresses[0], &g_ullConnection);
        
        if (SUCCEEDED(hr))
        {
            StringCbPrintf(wzMsg, sizeof(wzMsg), L"ID: %08X", g_ullConnection);
            SetStatusPart(SB_PART_MESSAGE, wzMsg);
        }
    }

    if (NULL != ppNodeInfo)
    {
        PeerGraphFreeData(ppNodeInfo);
        ppNodeInfo = NULL;
    }

    return hr;
}    

//-----------------------------------------------------------------------------
// Function: CreateIdentity
//
// Purpose:  Create the identity that this node will use when interacting with
//           PNRP
//
// Returns:  HRESULT
//
HRESULT CreateIdentity()
{
    return PeerIdentityCreate(
        L"GraphChat",
        L"GraphChatMember",
        (HCRYPTPROV)NULL,
        &g_pwzIdentity);
}

//-----------------------------------------------------------------------------
// Function: DeleteIdentity
//
// Purpose:  Delete the identity created by CreateIdentity
//
// Returns:  HRESULT
//
HRESULT DeleteIdentity()
{
    HRESULT hr = S_OK;

    if (g_pwzIdentity)
    {
        hr = PeerIdentityDelete(g_pwzIdentity);
        g_pwzIdentity = NULL;
    }

    return hr;
}

//------------------------------------------------------------------
// Function: CreateGraph
//
// Parameters:
// ------------
// PCWSTR wzCreatorID - Name for graph creator
// PCWSTR wzGraphID   - Name of graph to be created
//
// Purpose:  Creates a new graph
//
// Returns:  HRESULT
//
HRESULT CreateGraph(PCWSTR wzCreatorId, PCWSTR wzGraphId)
{
    HRESULT hr = S_OK;
    PEER_GRAPH_PROPERTIES props = {0};
    WCHAR wzMessage[MAX_CHAT_MESSAGE] = {0};
    DWORD dwLocalScopeID;      // Identifies specific local connection (if app)

    // Release any previous Graph resources
    CleanupGraph();
    // Delete old database if present.
    PeerGraphDelete(wzGraphId, wzCreatorId, L"SampleChatGraph");

    props.dwSize = sizeof(props);
    props.pwzFriendlyName = L"Sample Graph";
    props.pwzGraphId = (PWSTR) wzGraphId;
    props.pwzCreatorId = (PWSTR) wzCreatorId;
    props.cPresenceMax = (ULONG) -1;    // publish presence for everyone.
    
    // Set scope to match user input - global or local
    if (g_fGlobalScope)
    {
        props.dwScope = (DWORD) PEER_GRAPH_SCOPE_GLOBAL;
    }
    else
    {
        props.dwScope = (DWORD) PEER_GRAPH_SCOPE_LINKLOCAL;
    }

    // save graphid for Registration, once we are synchronized and prepared to listen.
    StringCbCopy(g_wzGraphId, sizeof(g_wzGraphId), wzGraphId);

    // save peerid for use when registering w/ PNRP
    StringCbCopy(g_wzPeerId, sizeof(g_wzPeerId), wzCreatorId);

    hr = PeerGraphCreate(&props, L"SampleChatGraph", NULL, &g_hGraph);

    if (SUCCEEDED(hr))
    {
        StringCchPrintf(wzMessage, celems(wzMessage), L"New graph created.  GraphName = %s", wzGraphId);
        DisplaySysMsg(wzMessage);
        hr = PrepareToChat(wzGraphId, FALSE);
    }
    else
    {
        DisplayHrError(L"Failed to create a new Graph", hr);
    }
    
    // A brand new graph is always in sync, and ready to listen
    if (SUCCEEDED(hr))
    {
        g_fSynchronized = TRUE;

        if (g_fGlobalScope)
        {
            hr = PeerGraphListen(g_hGraph, PEER_GRAPH_SCOPE_GLOBAL, 0, GRAPHING_PORT);
        }
        else
        {
            // Just Retrieve the Scope ID, don't need the cloud name for now.
            hr = GetLocalCloudInfo(0, NULL, &dwLocalScopeID);
           
            if (SUCCEEDED(hr))
            {
                hr = PeerGraphListen(g_hGraph, PEER_GRAPH_SCOPE_LINKLOCAL, dwLocalScopeID, GRAPHING_PORT);
            }
            else
            {
                DisplayHrError(L"Error attempting to retrieve local scope ID", hr);
            }

        }

        if (SUCCEEDED(hr))
        {
            UpdateParticipant(wzCreatorId, TRUE);
            hr = RegisterAddress(g_wzGraphId);
            if (FAILED(hr))
                DisplayHrError(L"Error attempting to Register Graph ID.",hr);
        }
        else
        {
            DisplayHrError(L"Failed call to PeerGraphListen", hr);
        }
    }

    return hr;
}

//------------------------------------------------------------------
// Function: OpenGraph
//
// Purpose:  Opens an existing graph
//
// Returns:  HRESULT
//
HRESULT OpenGraph(PCWSTR wzPeerId, PCWSTR wzGraphId)
{
    HRESULT hr = S_OK;
    HRESULT hrOpen = S_OK;

    // Release any previous Graph resources
    CleanupGraph();

    // save graphid for Registration, once we are synchronized and prepared to listen.
    StringCbCopy(g_wzGraphId, sizeof(g_wzGraphId), wzGraphId);

    // save peerid for use when registering w/ PNRP
    StringCbCopy(g_wzPeerId, sizeof(g_wzPeerId), wzPeerId);

    hrOpen = PeerGraphOpen(wzGraphId, wzPeerId, L"SampleChatGraph", NULL, 0, NULL, &g_hGraph);

    if (SUCCEEDED(hrOpen))
    {
        hr = PrepareToChat(wzGraphId, TRUE);
        if (SUCCEEDED(hr))
        {    
            UpdateParticipant(wzPeerId, TRUE);
        }
        
        if (hrOpen == PEER_S_GRAPH_DATA_CREATED)
        {
            if (hr == HRESULT_FROM_WIN32(WSA_E_NO_MORE))
            {
                MessageBox(GetWindow(g_hwndMain, GW_ENABLEDPOPUP), L"This is the first time the specified peer has tried to open the specified graph, on this machine.\r\nNobody could be found to connect to.  Please try again later.", L"Graph Chat Info", MB_OK | MB_ICONWARNING);
            }
            else if(FAILED(hr))
            {
                DisplayHrError(L"Node could not connect to graph", hr);
            }
        }
    }
    else
    {
        hr = hrOpen;
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function: PrepareToChat
//
// Purpose:  Does the initial hookup of the graph that is required after a
//           successful create or open
//
// Returns:  HRESULT
//
HRESULT PrepareToChat(PCWSTR wzGraphId, __in BOOL fConnect)
{
    PEER_ADDRESS addr;
    HRESULT hr = RegisterForEvents();

    if (SUCCEEDED(hr) && fConnect)
    {
        hr = DiscoverAddress(wzGraphId, &addr);
       
        if (SUCCEEDED(hr))
        {
            hr = PeerGraphConnect(g_hGraph, NULL, &addr, NULL);
        }
        if (FAILED(hr))
        {
            DisplayHrError(L"Couldn't find/connect to Graph", hr);
        }           
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Function: RegisterForEvents
//
// Purpose:  Registers the EventCallback function so it will be called for only
//           those events that are specified.
//
// Returns:  HRESULT
//
HRESULT RegisterForEvents()
{
    HRESULT hr = S_OK;
    PEER_GRAPH_EVENT_REGISTRATION regs[] = {
        { PEER_GRAPH_EVENT_RECORD_CHANGED, &CHAT_MESSAGE_RECORD_TYPE },
        { PEER_GRAPH_EVENT_NODE_CHANGED,   0 },
        { PEER_GRAPH_EVENT_STATUS_CHANGED, 0 },
        { PEER_GRAPH_EVENT_DIRECT_CONNECTION, 0 },
        { PEER_GRAPH_EVENT_INCOMING_DATA, &WHISPER_MESSAGE_TYPE },
        { PEER_GRAPH_EVENT_CONNECTION_REQUIRED, 0 },
    };

    g_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_hEvent == NULL)
    {
        DisplayHrError(L"CreateEvent call failed", E_OUTOFMEMORY);
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        hr = PeerGraphRegisterEvent(
                 g_hGraph, 
                 g_hEvent, 
                 celems(regs), 
                 regs,  
                 &g_hPeerEvent
                 );
        if (FAILED(hr))
        {
            DisplayHrError(L"PeerGraphRegisterEvents failed", hr);
            CloseHandle(g_hEvent);
            g_hEvent = NULL;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (!RegisterWaitForSingleObject(&g_hWait, g_hEvent, EventCallback, NULL, INFINITE, WT_EXECUTEDEFAULT))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DisplayHrError(L"Could not setup event callback", hr);
            if (g_hPeerEvent != NULL)
            {
                PeerGraphUnregisterEvent(g_hPeerEvent);
                g_hPeerEvent = NULL;
            }
            CloseHandle(g_hEvent);
            g_hEvent = NULL;
        }
    }

    return hr;
}


//-----------------------------------------------------------------------
// Function: EventCallback
//
// Purpose:  Handle events raised by the graphing infrastructure
//
// Returns:  nothing
//
VOID CALLBACK EventCallback(PVOID lpParam, BOOLEAN reason)
{
    HRESULT hr= S_OK;
    PEER_GRAPH_EVENT_DATA *pEventData = NULL;

    //Unreferenced parameter
    lpParam;
    reason;

    for (;;)
    {
        hr = PeerGraphGetEventData(g_hPeerEvent, &pEventData);

        if (FAILED(hr) || (hr == PEER_S_NO_EVENT_DATA) || (NULL == pEventData) )
        {
            break;
        }

        switch (pEventData->eventType)
        {
            case PEER_GRAPH_EVENT_RECORD_CHANGED:
                ProcessRecordChangeEvent(pEventData);
                break;

            case PEER_GRAPH_EVENT_STATUS_CHANGED:
                ProcessStatusChangeEvent(pEventData->dwStatus);
                break;

            case PEER_GRAPH_EVENT_NODE_CHANGED:
                ProcessNodeChangeEvent(pEventData);
                break;
                
            case PEER_GRAPH_EVENT_DIRECT_CONNECTION:
                ProcessDirectConnectionEvent(pEventData);
                break;
        
            case PEER_GRAPH_EVENT_INCOMING_DATA:
                ProcessIncomingDataEvent(pEventData);
                break;

            case PEER_GRAPH_EVENT_CONNECTION_REQUIRED:
                ProcessConnectionRequiredEvent(pEventData);
                break;
            
            default:
                break;
        }

        PeerGraphFreeData(pEventData);
    }
}

//-------------------------------------------------------------------------
// Function: ProcessNodeChangeEvent
//
// Purpose:  Processes the PEER_GRAPH_EVENT_NODE_CHANGED event.
//
// Returns:  void
//
void ProcessNodeChangeEvent(__in PEER_GRAPH_EVENT_DATA *pEventData)
{
    HRESULT hr = S_OK;

    if (pEventData->nodeChangeData.changeType == PEER_NODE_CHANGE_CONNECTED) 
    {
         UpdateParticipant(pEventData->nodeChangeData.pwzPeerId, TRUE);
    }
    else if (pEventData->nodeChangeData.changeType == 
                PEER_NODE_CHANGE_DISCONNECTED
                )
    {
        UpdateParticipant(pEventData->nodeChangeData.pwzPeerId, FALSE);
    }
    else if (
       (pEventData->nodeChangeData.changeType == PEER_NODE_CHANGE_UPDATED) &&
       (pEventData->nodeChangeData.ullNodeId == 0)
       )
    {
       // PEER_GRAPH_EVENT_NODE_CHANGED event is generated when a
       // node's attribute changes or when its IPv6 address changes.
       // If we have received this event in the local node, we
       // assume in this sample that it is because of an address
       // change as we don't do anything to change the node attributes.
       // When we get an address change re-register with PNRP.

       SetStatus(L"Address Change Occured.");

       hr = RegisterAddress(g_wzGraphId);
       if (FAILED(hr))
       {
          DisplayHrError(L"Error attempting to Register Graph ID.",hr);
       }
    }
}

//-------------------------------------------------------------------------
// Function: ProcessStatusChangeEvent
//
// Purpose:  Processes the PEER_GRAPH_EVENT_STATUS_CHANGED event.
//
// Returns:  void
//
void ProcessStatusChangeEvent(__in DWORD dwStatus)
{
    PWSTR pwzMsg = L"";
    PEER_GRAPH_PROPERTIES * pGraphProperties = NULL;
    WCHAR wzChatTitle[MAX_PEERNAME + 30] = L"Offline";
    HRESULT hr = S_OK;
    DWORD dwLocalScopeID;

    hr = PeerGraphGetProperties(g_hGraph, &pGraphProperties);
    pwzMsg = L"";
    if (SUCCEEDED(hr))
    {
        if (pGraphProperties != NULL)
        {
            // Check if listening
            if (dwStatus & PEER_GRAPH_STATUS_LISTENING)
            {
                pwzMsg = L"Listening";
                StringCbPrintf(wzChatTitle, sizeof(wzChatTitle), L"Waiting to chat in %s", 
                               pGraphProperties->pwzGraphId);
            }
        }
    }
    SetStatusPart(SB_PART_LISTENING, pwzMsg);

    pwzMsg = L"";
    if (SUCCEEDED(hr))
    {
        if (pGraphProperties != NULL)
        {
            // Check if has connections
            if (dwStatus & PEER_GRAPH_STATUS_HAS_CONNECTIONS)
            {
                pwzMsg = L"Connected";
                StringCbPrintf(wzChatTitle, sizeof(wzChatTitle), L"Chatting in %s", 
                               pGraphProperties->pwzGraphId);
            }
        }
    }
    
    PeerGraphFreeData(pGraphProperties);
    
    SetStatusPart(SB_PART_CONNECTED, pwzMsg);

    // Check if database is synchronized
    pwzMsg = L"";
    if (dwStatus & PEER_GRAPH_STATUS_SYNCHRONIZED)
    {
        if (!g_fSynchronized)
        {
            g_fSynchronized = TRUE;

            // If we aren't already listening, do so now.
            if (!(dwStatus & PEER_GRAPH_STATUS_LISTENING))
            {
                if (g_fGlobalScope)
                {
                    hr = PeerGraphListen(g_hGraph, PEER_GRAPH_SCOPE_GLOBAL, 0, GRAPHING_PORT);
                }
                else
                {
                    hr = GetLocalCloudInfo(0, NULL, &dwLocalScopeID);
                    if (SUCCEEDED(hr))
                    {
                        hr = PeerGraphListen(g_hGraph, PEER_GRAPH_SCOPE_LINKLOCAL, dwLocalScopeID, GRAPHING_PORT);
                    }
                    else
                    {
                        DisplayHrError(L"Error retrieving local scope ID", hr);
                    }
                }
                if (SUCCEEDED(hr))
                {
                    hr = RegisterAddress(g_wzGraphId);
                    if (FAILED(hr))
                        DisplayHrError(L"Error attempting to Register Graph ID.",hr);
                }
                else
                {
                    DisplayHrError(L"Error attempting to listen", hr);
                }
            }
        }
        pwzMsg = L"Synchronized";
    }
    SetStatusPart(SB_PART_SYNCHRONIZED, pwzMsg);
    
    SetWindowText(g_hwndChatLabel, wzChatTitle);    

    EnableDisconnectMenu();
}

//-------------------------------------------------------------------------
// Function: ProcessRecordChangeEvent
//
// Purpose:  Processes the PEER_GRAPH_EVENT_RECORD_CHANGED event.
//
// Returns:  nothing
//
void ProcessRecordChangeEvent(__in PEER_GRAPH_EVENT_DATA *pEventData)
{
    if (pEventData->recordChangeData.changeType == PEER_RECORD_ADDED)
    {
        if (IsEqualGUID(&pEventData->recordChangeData.recordType, &CHAT_MESSAGE_RECORD_TYPE))
        {
            PEER_RECORD *pRecord = NULL;
            HRESULT hr = PeerGraphGetRecord(g_hGraph, &pEventData->recordChangeData.recordId, &pRecord);
            if (SUCCEEDED(hr))
            {
                DisplayMessage(pRecord->pwzCreatorId, (PCWSTR) pRecord->data.pbData);
                PeerGraphFreeData(pRecord);
            }
            else
            {
                DisplayHrError(L"Error adding chat record", hr);
            }
        }
    }
}

//-------------------------------------------------------------------------
// Function: ProcessDirectConnectionEvent
//
// Purpose:  Processes the PEER_GRAPH_EVENT_DIRECT_CONNECTION event.
//
// Returns:  void
//
void ProcessDirectConnectionEvent(__in PEER_GRAPH_EVENT_DATA *pEventData)
{
    switch (pEventData->connectionChangeData.status)
    {
        case PEER_CONNECTED:
            SetStatusPart(SB_PART_MESSAGE, L"DC Connected");
            if (g_ullConnection == pEventData->connectionChangeData.ullConnectionId)
                SendDirectData();
            break;

        case PEER_CONNECTION_FAILED:
            g_ullConnection = 0;
            SetStatusPart(SB_PART_MESSAGE, L"DC Failed");
            break;

        case PEER_DISCONNECTED:
            g_ullConnection = 0;
            SetStatusPart(SB_PART_MESSAGE, L"DC Disconnected");
            break;
    
        default:
            break;
    }
}

//-------------------------------------------------------------------------
// Function: ProcessIncomingDataEvent
//
// Purpose:  Processes the PEER_GRAPH_EVENT_INCOMING_DATA event.
//
// Returns:  void
//
void ProcessIncomingDataEvent(__in PEER_GRAPH_EVENT_DATA *pEventData)
{
    WCHAR wzSentFrom[MAX_PEERNAME + 20] = {0};
    HPEERENUM hPeerEnum = NULL;
    HRESULT hr;
    ULONG cItem;
    
    g_ullConnection = (ULONGLONG)pEventData->incomingData.ullConnectionId;
    hr = PeerGraphEnumConnections(g_hGraph, PEER_CONNECTION_DIRECT, &hPeerEnum);
    if (SUCCEEDED(hr))
    {
        PEER_CONNECTION_INFO ** ppConnections = NULL;
        hr = PeerGraphGetItemCount(hPeerEnum, &cItem);

        if (SUCCEEDED(hr))
        {
            hr = PeerGraphGetNextItem(hPeerEnum, &cItem, (PVOID**) &ppConnections);
        }

        if (SUCCEEDED(hr))
        {
            ULONG i;
            for (i=0; i < cItem; i++)
            {
                PEER_CONNECTION_INFO * pConnectionInfo = ppConnections[i];
                if (g_ullConnection == pConnectionInfo->ullConnectionId)
                {
                    StringCbPrintf(wzSentFrom, sizeof(wzSentFrom), L"Whisper from %s", 
                                   pConnectionInfo->pwzPeerId);
                    DisplayMessage(wzSentFrom, (PCWSTR) pEventData->incomingData.data.pbData);
                    break;
                }
            }
            PeerGraphFreeData(ppConnections);
        }

        PeerGraphEndEnumeration(hPeerEnum);
    }
    
    PeerGraphCloseDirectConnection(g_hGraph, g_ullConnection);
    g_ullConnection = 0;
}

//----------------------------------------------------------------------------
// Function: ProcessConnectionRequiredEvent
//
// Purpose : Process the PEER_GRAPH_EVENT_CONNECTION_REQUIRED event. 
//
// Returns: void 
//
void ProcessConnectionRequiredEvent(__in PEER_GRAPH_EVENT_DATA *pEventData)
{
    HRESULT hr = S_OK;
    PEER_ADDRESS addr;

    //Unreferenced parameter
    pEventData;

    hr = DiscoverAddress(g_wzGraphId, &addr);
    // We could find an address then connect to it. 
    if (SUCCEEDED(hr))
    {
        hr = PeerGraphConnect(g_hGraph, NULL, &addr, NULL);
    }
}

//------------------------------------------------------------------
// Function: SendDirectData
//
// Purpose:  Called by ProcessDirectConnectionEvent 
//           to send message to peer.
//
// Returns:  nothing
//
void SendDirectData()
{
    ULONG   cb = 0;
    HRESULT hr = S_OK;
    WCHAR   wzSentTo[MAX_PEERNAME + 20];

    cb = (ULONG)(sizeof(WCHAR) * (wcslen(g_pwzWsprMsg)+1));
    hr = PeerGraphSendData(g_hGraph, g_ullConnection, &WHISPER_MESSAGE_TYPE, cb, g_pwzWsprMsg);
    if (SUCCEEDED(hr))
    {
        StringCbPrintf(wzSentTo, sizeof(wzSentTo), L"Whisper to %s", g_wzWsprId);
        DisplayMessage(wzSentTo, g_pwzWsprMsg);
    }
    else
    {
        DisplayHrError(L"unable to send data",hr);
    }

    if (g_pwzWsprMsg)
    {
        free(g_pwzWsprMsg);
        g_pwzWsprMsg = NULL;
    }    
}

//-------------------------------------------------------------------------
// Function: DisplayMessage
//
// Purpose: Utility function to display a message in the chat area
//
// Returns:  nothing
//
void DisplayMessage(PCWSTR pwzName, PCWSTR pwzMsg)
{
    WCHAR wzMessage[MAX_CHAT_MESSAGE + MAX_PEERNAME + 6] = {0};

    StringCchPrintf(wzMessage, celems(wzMessage), L"[%s]: %s\r\n", pwzName, pwzMsg);

    SendDlgItemMessage(g_hwndMain, IDC_MESSAGES, EM_SETSEL, 0x7fffffff, (LPARAM) -1);
    SendDlgItemMessage(g_hwndMain, IDC_MESSAGES, EM_REPLACESEL, 0, (LPARAM) wzMessage);
    SendDlgItemMessage(g_hwndMain, IDC_MESSAGES, EM_SCROLLCARET, 0, 0);
};

//-------------------------------------------------------------------------
// Function: ClearParticipantList
//
// Purpose: Small wrapper used to reset contents of participant list
//
// Returns:  nothing
//
void ClearParticipantList(void)
{
    ListBox_ResetContent(GetDlgItem(g_hwndMain, IDC_MEMBERS));
}


//-------------------------------------------------------------------------
// Function: UpdateParticipant
//
// Purpose: Adds or removes a person from the participant combo box
//
// Returns:  nothing
//
void UpdateParticipant(PCWSTR wzPerson, __in BOOL fAdd)
{
    LRESULT index;
    WCHAR wzMsg[MAX_PEERNAME + 30] = {0};

    if (fAdd)
    {
        SendDlgItemMessage(g_hwndMain, IDC_MEMBERS, LB_ADDSTRING, 0, (LPARAM) wzPerson);
        StringCbPrintf(wzMsg, sizeof(wzMsg), L"<%s> has joined the Chat", wzPerson);
        DisplaySysMsg(wzMsg);
    }
    else
    {
        index = SendDlgItemMessage(g_hwndMain, IDC_MEMBERS, LB_FINDSTRINGEXACT, (WPARAM) -1, (LPARAM) wzPerson);
        if (index != LB_ERR)
        {
            SendDlgItemMessage(g_hwndMain, IDC_MEMBERS, LB_DELETESTRING, index, 0);
            StringCbPrintf(wzMsg, sizeof(wzMsg), L"<%s> has left the Chat", wzPerson);
            DisplaySysMsg(wzMsg);
        }
    }
};

//-----------------------------------------------------------------------------
// Function: AddChatRecord
//
// Purpose:  This adds a new chat message record to the graph.
//
// Returns:  HRESULT
//
HRESULT AddChatRecord(PCWSTR pwzMessage)
{
    HRESULT     hr = S_OK;
    PEER_RECORD record = {0};
    GUID        idRecord;
    FILETIME    ftExpire;

    // Calculate the expiration time
    GetSystemTimeAsFileTime(&ftExpire);

    hr = PeerGraphUniversalTimeToPeerTime(g_hGraph, &ftExpire, &ftExpire);
    if (SUCCEEDED(hr))
    {
        *((ULONGLONG UNALIGNED*)&ftExpire) += 600000000;  // Now + 60 seconds

        // Set up the record
        record.dwSize = sizeof(record);
        record.data.cbData = ( ((ULONG) wcslen(pwzMessage)) +1) * sizeof(WCHAR);
        record.data.pbData = (PBYTE) pwzMessage;
        record.ftExpiration = ftExpire;

        // Set the record type GUID
        record.type = CHAT_MESSAGE_RECORD_TYPE;

        // Add the record to the database
        hr = PeerGraphAddRecord(g_hGraph, &record, &idRecord);
        if (FAILED(hr))
        {
            DisplayHrError(L"Failed to add a chat record to the graph.", hr);
        }
    }
    else
    {
        DisplayHrError(L"Failed to add a chat record to the graph.", hr);
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function: CleanupGraph
//
// Purpose: Cleans up all the global variables associated w/ this Graph.
//
// Returns:  nothing
//
void CleanupGraph()
{
    if (g_fPnrpRegistered)
    {
        UnregisterAddress(g_wzGraphId);
    }

    if (g_hPeerEvent != NULL)
    {
        PeerGraphUnregisterEvent(g_hPeerEvent);
        g_hPeerEvent = NULL;
    }

    if (g_hWait != NULL)
    {
        UnregisterWaitEx(g_hWait, INVALID_HANDLE_VALUE);
        g_hWait = NULL;
    }

    if (g_hEvent != NULL)
    {
        CloseHandle(g_hEvent);
        g_hEvent = NULL;
    }

    if (g_hGraph != NULL)
    {
        PeerGraphClose(g_hGraph);
        g_hGraph = NULL;
    }

    if (g_pwzWsprMsg)
    {
        free(g_pwzWsprMsg);
        g_pwzWsprMsg = NULL;
    }    

    ClearParticipantList();

    g_fSynchronized = FALSE;
}

//-------------------------------------------------------------------------
// Function: FormatAddress
//
// Purpose: Simple wrapper function converting an IP address into a string
// 
// Returns:  nothing
//
void FormatAddress(__in SOCKADDR_IN6 * pSockAddr, __out PWSTR pwzBuff, int cchMax)
{
    DWORD cb = sizeof(struct sockaddr_in6);
    int err = WSAAddressToString((LPSOCKADDR)pSockAddr, cb, NULL, pwzBuff, (LPDWORD) &cchMax);
    if (0 != err)
    {
        err = WSAGetLastError();
        StringCchPrintf(pwzBuff, cchMax, L"err=%08X", err);
    }
}


//-------------------------------------------------------------------------
// Function: DiscoverAddress
//
// Purpose:  Given a graphID, uses PnrpResolve to find address(es)
// 
// Returns:  HRESULT
//
HRESULT DiscoverAddress(PCWSTR wzGraphId, __out PEER_ADDRESS *pPeerAddress)
{
    HRESULT hr = S_OK;
    HCURSOR hCursor;
    PWSTR pwzUnsecuredName = NULL;
    WCHAR wzCloudName[MAX_CLOUD_NAME];
    DWORD dwLocalScopeID;

    if(wzGraphId == NULL)
    {
        hr = E_INVALIDARG;
        return hr;
    }
    
    // generate the unsecured PeerName [0.id]
    hr = PeerCreatePeerName(NULL,wzGraphId,&pwzUnsecuredName);
    if(FAILED(hr))
    {
        DisplayHrError(L"PeerName could not be created", hr);
        return hr;
    }
    
    ZeroMemory(pPeerAddress, sizeof(PEER_ADDRESS));
    pPeerAddress->dwSize = sizeof(PEER_ADDRESS);

    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (g_fGlobalScope)
    {
        hr = PnrpResolve(pwzUnsecuredName, NULL, &pPeerAddress->sin6);
    } 
    else
    {
        hr = GetLocalCloudInfo(celems(wzCloudName), wzCloudName, &dwLocalScopeID);
        if (SUCCEEDED(hr))
        {
            hr = PnrpResolve(pwzUnsecuredName, wzCloudName, &pPeerAddress->sin6);  
        }
        else
        {
            DisplayHrError(L"Local Cloud Info could not be retrieved.", hr);
        }
    }

    SetCursor(hCursor);
    PeerFreeData(pwzUnsecuredName);
    return hr;
}

//-------------------------------------------------------------------------
// Function: RegisterAddress
//
// Purpose:  Registers the current node's address with PNRP
// 
// Returns:  HRESULT
//
HRESULT RegisterAddress(PCWSTR wzGraphId)
{
    HRESULT hr = S_OK;
    PEER_NODE_INFO *pNodeInfo = NULL;
    PWSTR pwzUnsecuredName = NULL;
    WCHAR wzCloudName[MAX_CLOUD_NAME];
    DWORD dwLocalScopeID;
    WCHAR wzTemp[512];
    SOCKADDR_IN6 * pAddress; 

    // generate the unsecured PeerName [0.id]
    if(wzGraphId != NULL)
    {
        hr = PeerCreatePeerName(NULL,wzGraphId,&pwzUnsecuredName);
    }

    if(FAILED(hr))
    {
        DisplayHrError(L"PeerName could not be created", hr);
        return hr;
    }

    hr = PeerGraphGetNodeInfo(g_hGraph, 0, &pNodeInfo);
    if (SUCCEEDED(hr) && pNodeInfo->cAddresses >= 1)
    {
        if (g_fGlobalScope)
        {
            hr = PnrpRegister(g_pwzIdentity, pwzUnsecuredName, NULL, pNodeInfo);
        }
        else
        {
            hr = GetLocalCloudInfo(celems(wzCloudName), wzCloudName, &dwLocalScopeID);            
            if (SUCCEEDED(hr))
            {
                hr = PnrpRegister(g_pwzIdentity, pwzUnsecuredName, wzCloudName, pNodeInfo);
            }
        }

        if (SUCCEEDED(hr))
        {
            pAddress = &pNodeInfo->pAddresses->sin6;
            FormatAddress(pAddress, wzTemp, celems(wzTemp));
            SetStatusPart(SB_PART_ADDRESS, wzTemp);
            g_fPnrpRegistered = TRUE;
        }
        else
        {
            DisplayHrError(L"Failed to register node in PNRP", hr);
        }

        PeerGraphFreeData(pNodeInfo);
    }
    else
    {
        DisplayHrError(L"Error getting local node info", hr);
    }
    PeerFreeData(pwzUnsecuredName);
    return hr;
}

//-------------------------------------------------------------------------
// Function: UnregisterAddress
//
// Purpose:  Unregisters the current node's address with PNRP
// 
// Returns:  HRESULT
//
HRESULT UnregisterAddress(PCWSTR wzGraphId)
{
    HRESULT hr = S_OK;
    PWSTR pwzUnsecuredName = NULL;
    WCHAR wzCloudName[MAX_CLOUD_NAME];
    DWORD dwLocalScopeID;

    // generate the unsecured PeerName [0.id]
    if(wzGraphId != NULL)
        PeerCreatePeerName(NULL,wzGraphId,&pwzUnsecuredName);

    if (g_fGlobalScope)
    {
        hr = PnrpUnregister(g_pwzIdentity, pwzUnsecuredName, NULL);
    }
    else
    {
        hr = GetLocalCloudInfo(celems(wzCloudName), wzCloudName, &dwLocalScopeID);
        if (SUCCEEDED(hr))
        {
             hr = PnrpUnregister(g_pwzIdentity, pwzUnsecuredName, wzCloudName);
        }
    }
    if (FAILED(hr))
    {
        DisplayHrError(L"Failed to unregister node in PNRP", hr);
    }
    else
        g_fPnrpRegistered = FALSE;
    
    PeerFreeData(pwzUnsecuredName);
    return hr;
}

//-----------------------------------------------------------------------------
// Function: MsgErrHr
//
// Purpose:  Display a message for a function with an error code.
//
// Returns:  nothing
//
void MsgErrHr(PCWSTR pwzMsg, __in HRESULT hr, PCSTR pszFunction)
{
    WCHAR wzError[512];

    StringCchPrintf(wzError, celems(wzError), L"%s\r\n\r\nFunction: %S\r\nHRESULT=0x%08X", 
                    pwzMsg, pszFunction, hr);

    MessageBox(GetWindow(g_hwndMain, GW_ENABLEDPOPUP), wzError, L"Graph Chat Error", MB_OK | MB_ICONWARNING);
}

//-----------------------------------------------------------------------------
// Function: SetStatusPart
//
// Purpose:  Set the text of the status bar.
//
// Returns:  nothing
//
void SetStatusPart(int sbPart, PCWSTR pwzStatus)
{
    SendMessage(g_hwndStatus, SB_SETTEXT, sbPart, (LPARAM) pwzStatus);
}

