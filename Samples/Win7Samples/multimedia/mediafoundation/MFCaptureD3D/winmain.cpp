//////////////////////////////////////////////////////////////////////////
//
// winmain.cpp : Application entry-point
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "MFCaptureD3D.h"
#include "resource.h"

// Include the v6 common controls in the manifest
#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")


//
// ChooseDeviceParam structure
//
// Holds an array of IMFActivate pointers that represent video
// capture devices.
//

struct ChooseDeviceParam
{
    IMFActivate **ppDevices;    // Array of IMFActivate pointers.
    UINT32      count;          // Number of elements in the array.
    UINT32      selection;      // Selected device, by array index.
};



BOOL    InitializeApplication();
BOOL    InitializeWindow(HWND *pHwnd);
void    CleanUp();
INT     MessageLoop(HWND hwnd);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void    ShowErrorMessage(PCWSTR format, HRESULT hr);

// Window message handlers
BOOL    OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void    OnClose(HWND hwnd);
void    OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void    OnSize(HWND hwnd, UINT state, int cx, int cy);
void    OnDeviceChange(HWND hwnd, DEV_BROADCAST_HDR *pHdr);

// Command handlers
void    OnChooseDevice(HWND hwnd, BOOL bPrompt);


// Constants 
const WCHAR CLASS_NAME[]  = L"MFCapture Window Class";
const WCHAR WINDOW_NAME[] = L"MFCapture Sample Application";


// Global variables

CPreview    *g_pPreview = NULL;
HDEVNOTIFY  g_hdevnotify = NULL;


//-------------------------------------------------------------------
// WinMain
//
// Application entry-point. 
//-------------------------------------------------------------------

INT WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,INT)
{
    HWND hwnd = 0;

    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (InitializeApplication() && InitializeWindow(&hwnd))
    {
        MessageLoop(hwnd);
    }

    CleanUp();

    return 0;
}


//-------------------------------------------------------------------
//  WindowProc
//
//  Window procedure.
//-------------------------------------------------------------------

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_CLOSE,  OnClose);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_SIZE,    OnSize);

    case WM_APP_PREVIEW_ERROR:
        ShowErrorMessage(L"Error", (HRESULT)wParam);
        break;

    case WM_DEVICECHANGE:
        OnDeviceChange(hwnd, (PDEV_BROADCAST_HDR)lParam);
        break;

    case WM_ERASEBKGND:
        return 1;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


//-------------------------------------------------------------------
// InitializeApplication
//
// Initializes the application.
//-------------------------------------------------------------------

BOOL InitializeApplication()
{
    HRESULT hr = S_OK;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        hr = MFStartup(MF_VERSION);
    }

    return (SUCCEEDED(hr));
}

//-------------------------------------------------------------------
// CleanUp
//
// Releases resources.
//-------------------------------------------------------------------

void CleanUp()
{
    if (g_hdevnotify)
    {
        UnregisterDeviceNotification(g_hdevnotify);
    }

    if (g_pPreview)
    {
        g_pPreview->CloseDevice();
    }

    SafeRelease(&g_pPreview);

    MFShutdown();
    CoUninitialize();
}


//-------------------------------------------------------------------
// InitializeWindow
//
// Creates the application window.
//-------------------------------------------------------------------

BOOL InitializeWindow(HWND *pHwnd)
{
    WNDCLASS wc = {0};

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);

    if (!RegisterClass(&wc))
    {
        return FALSE;
    }

    HWND hwnd = CreateWindow(
        CLASS_NAME,
        WINDOW_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
        );

    if (!hwnd)
    {
        return FALSE;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    *pHwnd = hwnd;

    return TRUE;
}


//-------------------------------------------------------------------
// MessageLoop 
//
// Implements the window message loop.
//-------------------------------------------------------------------

INT MessageLoop(HWND hwnd)
{
    MSG msg = {0};

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(hwnd);

    return INT(msg.wParam);
}


//-------------------------------------------------------------------
// OnCreate
//
// Handles the WM_CREATE message.
//-------------------------------------------------------------------

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT)
{
    HRESULT hr = S_OK;

    // Register this window to get device notification messages.

    DEV_BROADCAST_DEVICEINTERFACE di = { 0 };
    di.dbcc_size = sizeof(di);
    di.dbcc_devicetype  = DBT_DEVTYP_DEVICEINTERFACE;
    di.dbcc_classguid  = KSCATEGORY_CAPTURE; 

    g_hdevnotify = RegisterDeviceNotification(
        hwnd,
        &di,
        DEVICE_NOTIFY_WINDOW_HANDLE
        );

    if (g_hdevnotify == NULL)
    {
        ShowErrorMessage(L"RegisterDeviceNotification failed.", HRESULT_FROM_WIN32(GetLastError()));
        return FALSE;
    }

    // Create the object that manages video preview. 
    hr = CPreview::CreateInstance(hwnd, hwnd, &g_pPreview);

    if (FAILED(hr))
    {
        ShowErrorMessage(L"CPreview::CreateInstance failed.", hr);
        return FALSE;
    }

    // Select the first available device (if any).
    OnChooseDevice(hwnd, FALSE);

    return TRUE;
}



//-------------------------------------------------------------------
// OnClose
//
// Handles WM_CLOSE messages.
//-------------------------------------------------------------------

void OnClose(HWND /*hwnd*/)
{
    PostQuitMessage(0);
}



//-------------------------------------------------------------------
// OnSize
//
// Handles WM_SIZE messages.
//-------------------------------------------------------------------

void OnSize(HWND hwnd, UINT /*state */, int cx, int cy)
{
    if (g_pPreview)
    {
        g_pPreview->ResizeVideo((WORD)cx, (WORD)cy);

        InvalidateRect(hwnd, NULL, FALSE);
    }
}


//-------------------------------------------------------------------
// OnCommand 
//
// Handles WM_COMMAND messages
//-------------------------------------------------------------------

void OnCommand(HWND hwnd, int id, HWND /*hwndCtl*/, UINT /*codeNotify*/)
{
    switch (id)
    {
        case ID_FILE_CHOOSEDEVICE:
            OnChooseDevice(hwnd, TRUE);
            break;
    }
}

//-------------------------------------------------------------------
//  OnChooseDevice
//
//  Select a video capture device.
//
//  hwnd:    A handle to the application window.
/// bPrompt: If TRUE, prompt to user to select the device. Otherwise,
//           select the first device in the list.
//-------------------------------------------------------------------

void OnChooseDevice(HWND hwnd, BOOL bPrompt)
{
    HRESULT hr = S_OK;
    ChooseDeviceParam param = { 0 };

    UINT iDevice = 0;   // Index into the array of devices
    BOOL bCancel = FALSE;

    IMFAttributes *pAttributes = NULL;

    // Initialize an attribute store to specify enumeration parameters.

    hr = MFCreateAttributes(&pAttributes, 1);
    
    if (FAILED(hr)) { goto done; }

    // Ask for source type = video capture devices.
    
    hr = pAttributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, 
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
        );

    if (FAILED(hr)) { goto done; }
    
    // Enumerate devices.
    hr = MFEnumDeviceSources(pAttributes, &param.ppDevices, &param.count);

    if (FAILED(hr)) { goto done; }

    // NOTE: param.count might be zero.

    if (bPrompt)
    {
        // Ask the user to select a device.

        INT_PTR result = DialogBoxParam(
            GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDD_CHOOSE_DEVICE),
            hwnd,
            DlgProc,
            (LPARAM)&param
            );

        if (result == IDOK)
        {
            iDevice = param.selection;
        }
        else
        {
            bCancel = TRUE; // User cancelled
        }
    }

    if (!bCancel && (param.count > 0))
    {
        // Give this source to the CPlayer object for preview.
        hr = g_pPreview->SetDevice( param.ppDevices[iDevice] );
    }

done:

    SafeRelease(&pAttributes);

    for (DWORD i = 0; i < param.count; i++)
    {
        SafeRelease(&param.ppDevices[i]);
    }
    CoTaskMemFree(param.ppDevices);

    if (FAILED(hr))
    {
        ShowErrorMessage(L"Cannot create a video capture device", hr);
    }
}


//-------------------------------------------------------------------
//  OnDeviceChange
//
//  Handles WM_DEVICECHANGE messages.
//-------------------------------------------------------------------

void OnDeviceChange(HWND hwnd, DEV_BROADCAST_HDR *pHdr)
{
    if (g_pPreview == NULL || pHdr == NULL)
    {
        return;
    }

    HRESULT hr = S_OK;
    BOOL bDeviceLost = FALSE;

    // Check if the current device was lost.

    hr = g_pPreview->CheckDeviceLost(pHdr, &bDeviceLost);

    if (FAILED(hr) || bDeviceLost)
    {
        g_pPreview->CloseDevice();

        MessageBox(hwnd, L"Lost the capture device.", WINDOW_NAME, MB_OK);
    }
}


/////////////////////////////////////////////////////////////////////

// Dialog functions

void    OnInitDialog(HWND hwnd, ChooseDeviceParam *pParam);
HRESULT OnOK(HWND hwnd, ChooseDeviceParam *pParam);

//-------------------------------------------------------------------
//  DlgProc
//
//  Dialog procedure for the "Select Device" dialog.
//-------------------------------------------------------------------

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static ChooseDeviceParam *pParam = NULL;

    switch (msg)
    {
    case WM_INITDIALOG:
        pParam = (ChooseDeviceParam*)lParam;
        OnInitDialog(hwnd, pParam);
        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            OnOK(hwnd, pParam);
            EndDialog(hwnd, LOWORD(wParam));
            return TRUE;

        case IDCANCEL:
            EndDialog(hwnd, LOWORD(wParam));
            return TRUE;
        }
        break;
    }

    return FALSE;
}



//-------------------------------------------------------------------
//  OnInitDialog
//
//  Handles the WM_INITDIALOG message.
//-------------------------------------------------------------------

void OnInitDialog(HWND hwnd, ChooseDeviceParam *pParam)
{
    HRESULT hr = S_OK;

    // Populate the list with the friendly names of the devices.

    HWND hList = GetDlgItem(hwnd, IDC_DEVICE_LIST);

    for (DWORD i = 0; i < pParam->count; i++)
    {
        WCHAR *szFriendlyName = NULL;

        hr = pParam->ppDevices[i]->GetAllocatedString(
            MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, 
            &szFriendlyName, 
            NULL
            );

        if (FAILED(hr)) 
        { 
            break; 
        }


        int index = ListBox_AddString(hList, szFriendlyName);

        ListBox_SetItemData(hList, index, i);

        CoTaskMemFree(szFriendlyName);
    }

    // Assume no selection for now.
    pParam->selection = (UINT32)-1;

    if (pParam->count == 0)
    {
        // If there are no devices, disable the "OK" button.
        EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
    }
}


HRESULT OnOK(HWND hwnd, ChooseDeviceParam *pParam)
{
    HWND hList = GetDlgItem(hwnd, IDC_DEVICE_LIST);

    int sel = ListBox_GetCurSel(hList);

    if (sel != LB_ERR)
    {
        pParam->selection = (UINT32)ListBox_GetItemData(hList, sel);
    }

    return S_OK;
}


void ShowErrorMessage(PCWSTR format, HRESULT hrErr)
{
    HRESULT hr = S_OK;
    WCHAR msg[MAX_PATH];

    hr = StringCbPrintf(msg, sizeof(msg), L"%s (hr=0x%X)", format, hrErr);

    if (SUCCEEDED(hr))
    {
        MessageBox(NULL, msg, L"Error", MB_ICONERROR);
    }
    else
    {
        DebugBreak();
    }
}
