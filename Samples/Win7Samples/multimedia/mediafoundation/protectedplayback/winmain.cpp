//////////////////////////////////////////////////////////////////////////
//
// winmain.cpp : Defines the entry point for the application.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

// Note: Most of the interesting code is in Player.cpp

#include "ProtectedPlayback.h"
#include <assert.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE   hInst;                              // current instance
TCHAR       szTitle[MAX_LOADSTRING];            // The title bar text
TCHAR       szWindowClass[MAX_LOADSTRING];      // the main window class name

BOOL        g_bRepaintClient = TRUE;            // Repaint the application client area?

CPlayer     *g_pPlayer = NULL;                  // Global player object. 

// Note: After WM_CREATE is processed, g_pPlayer remains valid until the
// window is destroyed.


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
HRESULT             InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    OpenUrlDialogProc(HWND, UINT, WPARAM, LPARAM);
void                NotifyError(HWND hwnd, const WCHAR *sErrorMessage, HRESULT hr);
void                UpdateUI(HWND hwnd, PlayerState state);

// Message handlers
LRESULT             OnCreateWindow(HWND hwnd);
void                OnFileOpen(HWND hwnd);
void                OnOpenURL(HWND hwnd);
void                OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr);
void                OnPaint(HWND hwnd);
void                OnKeyPress(WPARAM key);

void                OnContentEnablerMessage();
void                OnWebBrowserClosed();



// OpenUrlDialogInfo: Contains data passed to the "Open URL" dialog proc.
struct OpenUrlDialogInfo
{
    WCHAR *pszURL;
    DWORD cch;
};

/////////////////////////////////////////////////////////////////////////
//  Name: _tWinMain
//  Description:  Entry point to the application.
/////////////////////////////////////////////////////////////////////////

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE /* hPrevInstance */,
                     LPTSTR    /* lpCmdLine */,
                     int       nCmdShow)
{
    MSG msg;
    HACCEL hAccelTable;

    ZeroMemory(&msg, sizeof(msg));

    // Initialize COM
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        NotifyError(NULL, L"CoUninitialize failed.", hr);
        return FALSE;
    }

    TRACE_INIT();

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MFPLAYBACK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);


    // Perform application initialization.
    if (SUCCEEDED(hr))
    {
        hr = InitInstance (hInstance, nCmdShow);
    }


    // Main message loop.
    if (SUCCEEDED(hr))
    {
        hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MFPLAYBACK));

        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    // Clean up.
    
    if (g_pPlayer)
    {
        g_pPlayer->Shutdown();
        SAFE_RELEASE(g_pPlayer);
    }

    // Uninitialize COM.
    CoUninitialize();
 
    TRACE_CLOSE();

    return (int) msg.wParam;
}



/////////////////////////////////////////////////////////////////////////
//  Name: MyRegisterClass
//  Description:  Registers the window class.
/////////////////////////////////////////////////////////////////////////

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MFPLAYBACK));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_MFPLAYBACK);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}



////////////////////////////////////////////////////////////////////////
//  Name: InitInstance
//  Description: Saves the instance handle and creates the main window.
////////////////////////////////////////////////////////////////////////

HRESULT InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hwnd;

   hInst = hInstance; // Store instance handle in our global variable

   // Create the window.


   hwnd = CreateWindow(
      szWindowClass, szTitle, 
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hwnd)
   {
      return __HRESULT_FROM_WIN32(GetLastError());
   }

   ShowWindow(hwnd, nCmdShow);
   UpdateWindow(hwnd);

   return S_OK;
}


/////////////////////////////////////////////////////////////////////////
//  Name: WndProc
//  Description: Message handler for the main window.
/////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        return OnCreateWindow(hwnd);

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            DestroyWindow(hwnd);
            break;
        case ID_FILE_OPENFILE:
            OnFileOpen(hwnd);
            break;
        case ID_FILE_OPENURL:
            OnOpenURL(hwnd);
            break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;

    case WM_PAINT:
        OnPaint(hwnd);
        break;

    case WM_SIZE:
        g_pPlayer->ResizeVideo(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_ERASEBKGND:
        // Suppress window erasing, to reduce flickering while the video is playing.
        return 1;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_CHAR:
        OnKeyPress(wParam);
        break;

    case WM_SETCURSOR:
        return 1;

    case WM_APP_PLAYER_EVENT:
        OnPlayerEvent(hwnd, wParam);
        break;

    case WM_APP_CONTENT_ENABLER:
        OnContentEnablerMessage();
        break;

    case WM_APP_BROWSER_DONE:
        OnWebBrowserClosed();
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}



/////////////////////////////////////////////////////////////////////////
//  Name: OnCreateWindow
//  Description: Handles the WM_CREATE message.
//
//  hwnd: Handle to the video clipping window. (For this sample, the
//        video window is just the main application window.)
/////////////////////////////////////////////////////////////////////////

LRESULT OnCreateWindow(HWND hwnd)
{
    // Initialize the player object.
    HRESULT hr = CPlayer::CreateInstance(hwnd, hwnd, &g_pPlayer); 

    if (SUCCEEDED(hr))
    {
        UpdateUI(hwnd, Ready);
        return 0;   // Success.
    }
    else
    {
        NotifyError(NULL, L"Could not initialize the player object.", hr);
        return -1;  // Destroy the window
    }
}


/////////////////////////////////////////////////////////////////////////
//  Name: OnFileOpen
//  Description: Open an audio/video file.
/////////////////////////////////////////////////////////////////////////

void OnFileOpen(HWND hwnd)
{
    HRESULT hr = S_OK;

    // Show the File Open dialog.
    WCHAR path[MAX_PATH];
    path[0] = L'\0';

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Windows Media\0*.wmv;*.wma;*.asf\0MP3\0*.mp3\0All files\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.hInstance = hInst;

    if (GetOpenFileName(&ofn))
    {
        // Open the file with the playback object.
        hr = g_pPlayer->OpenURL(ofn.lpstrFile);

        if (SUCCEEDED(hr))
        {
            UpdateUI(hwnd, OpenPending);
        }
        else
        {
            NotifyError(hwnd, L"Could not open the file.", hr);
            UpdateUI(hwnd, Ready);
        }
    }

}

/////////////////////////////////////////////////////////////////////////
//  Name: OnOpenURL
//  Description: Opens a media file from a URL.
/////////////////////////////////////////////////////////////////////////

void OnOpenURL(HWND hwnd)
{
    HRESULT hr = S_OK;

    // Pass in an OpenUrlDialogInfo structure to the dialog. The dialog 
    // fills in this structure with the URL. The dialog proc allocates
    // the memory for the string. 

    OpenUrlDialogInfo url;
    ZeroMemory(&url, sizeof(&url));

    // Show the Open URL dialog.
    if (IDOK == DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_OPENURL), hwnd, OpenUrlDialogProc, (LPARAM)&url))
    {
        // Open the file with the playback object.
        hr = g_pPlayer->OpenURL(url.pszURL);

        if (SUCCEEDED(hr))
        {
            UpdateUI(hwnd, OpenPending);
        }
        else
        {
            NotifyError(hwnd, L"Could not open this URL.", hr);
            UpdateUI(hwnd, Ready);
        }
    }

    // The app must release the string.
    CoTaskMemFree(url.pszURL);
}


/////////////////////////////////////////////////////////////////////////
//  Name: OnPaint
//  Description: Handles WM_PAINT messages.
/////////////////////////////////////////////////////////////////////////

void OnPaint(HWND hwnd)
{
    if (g_bRepaintClient)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // The video is not playing, so we must paint the application window.
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, (HBRUSH) COLOR_WINDOW);

        EndPaint(hwnd, &ps);
    }
    else
    {
        // Video is playing. Ask the player to repaint.
        g_pPlayer->Repaint();
    }
}

void OnKeyPress(WPARAM key)
{
    switch (key)
    {
    // Space key toggles between running and paused
    case VK_SPACE:
        if (g_pPlayer->GetState() == Started)
        {
            g_pPlayer->Pause();
        }
        else if (g_pPlayer->GetState() == Paused)
        {
            g_pPlayer->Play();
        }
        break;
    }
}


/////////////////////////////////////////////////////////////////////////
//  Name: OnContentEnablerMessage
//  Description: Handles requests from the content protection manager.
/////////////////////////////////////////////////////////////////////////

void OnContentEnablerMessage()
{
    ContentProtectionManager *pManager = NULL;

    HRESULT hr = g_pPlayer->GetContentProtectionManager(&pManager);

    if (SUCCEEDED(hr))
    {
        EnablerState state = pManager->GetState();
        HRESULT hrStatus = pManager->GetStatus();   // Status of the last action.


        // EnablerState is defined for this application; it is not a standard
        // Media Foundation enum. It specifies what action the 
        // ContentProtectionManager helper object is requesting.

        switch (state)
        {
        case Enabler_Ready:
            // Start the enable action.
            hr = pManager->DoEnable();
            break;

        case Enabler_SilentInProgress:
            // We are currently in the middle of silent enable.

            // If the status code is NS_E_DRM_LICENSE_NOTACQUIRED,
            // we need to try non-silent enable.
            if (hrStatus == NS_E_DRM_LICENSE_NOTACQUIRED)
            {
                TRACE((L"Silent enabler failed, attempting non-silent.\n"));
                hr = pManager->DoEnable(ForceNonSilent); // Try non-silent this time;
            }
            else
            {
                // Complete the operation. If it succeeded, the content will play.
                // If it failed, the pipeline will queue an event with an error code.
                pManager->CompleteEnable();
            }
            break;
        
        case Enabler_NonSilentInProgress:
            // We are currently in the middle of non-silent enable. 
            // Either we succeeded or an error occurred. Either way, complete
            // the operation.
            pManager->CompleteEnable();
            break;

        case Enabler_Complete:
            // Nothing to do.
            break;

        default:
            TRACE((L"Unknown EnablerState value! (%d)\n", state));
            assert(false);
            break;
        }

        // If a previous call to DoEnable() failed, complete the operation
        // so that the pipeline will get the correct failure code.
        if (FAILED(hr))
        {
            pManager->CompleteEnable();
        }
    }

    SAFE_RELEASE(pManager);
}

/////////////////////////////////////////////////////////////////////////
//  Name: OnWebBrowserClosed
//  Description: Called when the user closes the browser window.
/////////////////////////////////////////////////////////////////////////

void OnWebBrowserClosed()
{
    ContentProtectionManager *pManager = NULL;

    HRESULT hr = g_pPlayer->GetContentProtectionManager(&pManager);

    if (SUCCEEDED(hr))
    {
        EnablerState state = pManager->GetState();

        if (state != Enabler_Complete)
        {
            TRACE((L"User closed the browser window before we got the licence. Cancel.\n"));
            pManager->CancelEnable();
        }
    }

    SAFE_RELEASE(pManager);
}


void OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr)
{
    HRESULT hr = S_OK;

    hr = g_pPlayer->HandleEvent(pUnkPtr);

    if (FAILED(hr))
    {
        NotifyError(hwnd, L"An error occurred.", hr);
    }
    UpdateUI(hwnd, g_pPlayer->GetState());
}



/////////////////////////////////////////////////////////////////////////
//  Name: UpdateUI
//  Description: Enables or disables controls, based on the player state.
/////////////////////////////////////////////////////////////////////////

void UpdateUI(HWND hwnd, PlayerState state)
{
    BOOL bWaiting = FALSE;
    BOOL bPlayback = FALSE;

    switch (state)
    {
    case OpenPending:
        bWaiting = TRUE;
        break;

    case Started:
        bPlayback = TRUE;
        break;

    case Paused:
        bPlayback = TRUE;
        break;
    }

    HMENU hMenu = GetMenu(hwnd);
    UINT  uEnable = MF_BYCOMMAND | (bWaiting ? MF_GRAYED : MF_ENABLED);

    EnableMenuItem(hMenu, ID_FILE_OPENFILE, uEnable);
    EnableMenuItem(hMenu, ID_FILE_OPENURL, uEnable);

    HCURSOR hCursor = LoadCursor(NULL, MAKEINTRESOURCE(bWaiting ? IDC_WAIT : IDC_ARROW));
    SetCursor(hCursor);

    if (bPlayback && g_pPlayer->HasVideo())
    {
        g_bRepaintClient = FALSE;
    }
    else
    {
        g_bRepaintClient = TRUE;
    }
}

/////////////////////////////////////////////////////////////////////////
//  Name: NotifyError
//  Description: Show a message box with an error message.
//
//  sErrorMessage: NULL-terminated string containing the error message.
//  hrErr: HRESULT from the error.
/////////////////////////////////////////////////////////////////////////

void NotifyError(HWND hwnd, const WCHAR *sErrorMessage, HRESULT hrErr)
{
    const size_t MESSAGE_LEN = 512;
    WCHAR message[MESSAGE_LEN];

    HRESULT hr = StringCchPrintf (message, MESSAGE_LEN, L"%s (HRESULT = 0x%X)", sErrorMessage, hrErr);
    if (SUCCEEDED(hr))
    {
        MessageBox(hwnd, message, NULL, MB_OK | MB_ICONERROR);
    }
}






/////////////////////////////////////////////////////////////////////////
//  Name: OpenUrlDialogProc
//  Description: Message handler for the "Open URL" window.
/////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK OpenUrlDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static OpenUrlDialogInfo *pUrl = NULL;

    BOOL result = FALSE;

    switch (message)
    {
    case WM_INITDIALOG:
        // The app sends a pointer to an OpenUrlDialogInfo structure as the lParam. 
        // We use this structure to store the URL.
        pUrl = (OpenUrlDialogInfo*)lParam;
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (pUrl)
            {
                // Get the URL from the edit box in the dialog.
                // This function allocates memory. The app must call CoTaskMemAlloc.
                HRESULT hr = AllocGetWindowText(GetDlgItem(hDlg, IDC_EDIT_URL), &pUrl->pszURL, &pUrl->cch);
                if (SUCCEEDED(hr))
                {
                    result = TRUE;
                }
            }
            EndDialog(hDlg, result ? IDOK : IDABORT);
            break;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(IDCANCEL));
            break;
        }
        return (INT_PTR)FALSE;
    }
    return (INT_PTR)FALSE;
}
