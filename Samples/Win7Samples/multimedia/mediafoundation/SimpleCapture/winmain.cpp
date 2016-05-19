//////////////////////////////////////////////////////////////////////////
//
// Media Foundation video capture sample.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
////////////////////////////////////////////////////////////////////////// 

#include <windows.h>
#include <windowsx.h>
#include <mfapi.h>
#include <mfplay.h>
#include <strsafe.h>
#include <Dbt.h>
#include <ks.h>
#include <ksmedia.h>

#include "resource.h"
#include "preview.h"

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


// Include the v6 common controls in the manifest
#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")



//-------------------------------------------------------------------
// ChooseDeviceParam struct
//
// Contains an array of IMFActivate pointers. Each pointer represents
// a video capture device. This struct is passed to the dialog where
// the user selects a device.
//-------------------------------------------------------------------

struct ChooseDeviceParam
{
    IMFActivate **ppDevices;
    UINT32      count;
    UINT32      selection;
};


BOOL    InitializeApp();
BOOL    InitializeWindow(HWND *pHwnd);
void    CleanUp();
INT     MessageLoop(HWND hwnd);
void    ShowErrorMessage(HWND hwnd, PCWSTR format, HRESULT hr);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Window message handlers
BOOL    OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void    OnClose(HWND hwnd);
void    OnPaint(HWND hwnd);
void    OnSize(HWND hwnd, UINT state, int cx, int cy);
void    OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void    OnDeviceLost(HWND hwnd, DEV_BROADCAST_HDR *pHdr);

// Command handlers
void    OnChooseDevice(HWND hwnd);

// Constants 
const WCHAR CLASS_NAME[]  = L"SimpleCapture Window Class";
const WCHAR WINDOW_NAME[] = L"SimpleCapture Sample Application";

// Global variables
CPreview    *g_pPreview = NULL;
HDEVNOTIFY  g_hdevnotify = NULL;

//-------------------------------------------------------------------
// WinMain: Application entry point
//-------------------------------------------------------------------

INT WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,INT)
{
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    HWND hwnd = 0;

    if (InitializeApp() && InitializeWindow(&hwnd))
    {
        MessageLoop(hwnd);
    }

    CleanUp();

    return 0;
}

//-------------------------------------------------------------------
// WindowProc: Window procedure
//-------------------------------------------------------------------

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE,  OnCreate);
        HANDLE_MSG(hwnd, WM_CLOSE,   OnClose);
        HANDLE_MSG(hwnd, WM_PAINT,   OnPaint);
        HANDLE_MSG(hwnd, WM_SIZE,    OnSize);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

    case WM_DEVICECHANGE:
        OnDeviceLost(hwnd, (PDEV_BROADCAST_HDR)lParam);
        return TRUE;

    case WM_ERASEBKGND:
        return 1;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}


//-------------------------------------------------------------------
// InitializeApp: One-time initialization
//-------------------------------------------------------------------

BOOL InitializeApp()
{
    HRESULT hr = S_OK;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    return (SUCCEEDED(hr));
}


//-------------------------------------------------------------------
// CleanUp: Frees resources before the application exits
//-------------------------------------------------------------------

void CleanUp()
{
    if (g_pPreview)
    {
        g_pPreview->CloseDevice();
        SafeRelease(&g_pPreview);
    }

    CoUninitialize();
}

//-------------------------------------------------------------------
// InitializeWindow: Creates the application window.
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
// MessageLoop: Message loop for the main application window.
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
// OnCreate: Handler for WM_CREATE
//-------------------------------------------------------------------

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT /*lpCreateStruct*/)
{
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
        ShowErrorMessage(
            hwnd, 
            L"RegisterDeviceNotification failed.", 
            HRESULT_FROM_WIN32(GetLastError())
            );
        return FALSE;
    }

    return TRUE;
}



//-------------------------------------------------------------------
// OnClose: Handler for WM_CLOSE 
//-------------------------------------------------------------------

void OnClose(HWND /*hwnd*/)
{
    if (g_hdevnotify)
    {
        UnregisterDeviceNotification(g_hdevnotify);
    }

    PostQuitMessage(0);
}


//-------------------------------------------------------------------
// OnPaint: Handler for WM_PAINT
//-------------------------------------------------------------------

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = 0;

    hdc = BeginPaint(hwnd, &ps);

    if (hdc)
    {
        if (g_pPreview && g_pPreview->HasVideo())
        {
            g_pPreview->UpdateVideo();
        }
        else
        {
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_APPWORKSPACE+1));
        }
    }
    EndPaint(hwnd, &ps);
}


//-------------------------------------------------------------------
// OnSize: Handler for WM_SIZE
//-------------------------------------------------------------------

void OnSize(HWND /*hwnd*/, UINT state, int /*cx*/, int /*cy*/)
{
    if (state == SIZE_RESTORED)
    {
        if (g_pPreview)
        {
            // Resize the video to cover the entire client area.
            g_pPreview->UpdateVideo();
        }
    }
}


//-------------------------------------------------------------------
// OnCommand: Handler for WM_COMMAND
//-------------------------------------------------------------------

void OnCommand(HWND hwnd, int id, HWND /*hwndCtl*/, UINT /*codeNotify*/)
{
    switch (id)
    {
        case ID_FILE_CHOOSEDEVICE:
            OnChooseDevice(hwnd);
            break;
    }
}

//-------------------------------------------------------------------
// OnChooseDevice
// 
// Displays a dialog for the user to select a capture device.
//-------------------------------------------------------------------

void OnChooseDevice(HWND hwnd)
{
    HRESULT hr = S_OK;
    ChooseDeviceParam param = { 0 };

    IMFAttributes *pAttributes = NULL;

    // Release the previous instance of the preview object, if any.
    if (g_pPreview)
    {
        g_pPreview->CloseDevice();
        SafeRelease(&g_pPreview);
    }

    // Create a new instance of the preview object.
    hr = CPreview::CreateInstance(hwnd, &g_pPreview);

    // Create an attribute store to specify the enumeration parameters.

    if (SUCCEEDED(hr))
    {
        hr = MFCreateAttributes(&pAttributes, 1);
    }

    // Ask for source type = video capture devices

    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, 
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
            );
    }
    
    // Enumerate devices.

    if (SUCCEEDED(hr))
    {
        hr = MFEnumDeviceSources(pAttributes, &param.ppDevices, &param.count);
    }

    if (SUCCEEDED(hr))
    {
        // Ask the user to select one.
        INT_PTR result = DialogBoxParam(
            GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDD_CHOOSE_DEVICE),
            hwnd,
            DlgProc,
            (LPARAM)&param
            );

        if ((result == IDOK) && (param.selection != (UINT32)-1))
        {
            UINT iDevice = param.selection;

            if (iDevice >= param.count)
            {
                hr = E_UNEXPECTED;
            }
			else
            {
                // Give this source to the CPreview object for preview.
                hr = g_pPreview->SetDevice(param.ppDevices[iDevice]);
            }
        }
    }

    SafeRelease(&pAttributes);

    for (DWORD i = 0; i < param.count; i++)
    {
        SafeRelease(&param.ppDevices[i]);
    }
    CoTaskMemFree(param.ppDevices);

    if (FAILED(hr))
    {
        ShowErrorMessage(hwnd, L"Cannot create the video capture device", hr);
    }
}

void OnDeviceLost(HWND hwnd, DEV_BROADCAST_HDR *pHdr)
{
    if (g_pPreview == NULL || pHdr == NULL)
    {
        return;
    }

    HRESULT hr = S_OK;
    BOOL bDeviceLost = FALSE;

    hr = g_pPreview->CheckDeviceLost(pHdr, &bDeviceLost);

    if (FAILED(hr) || bDeviceLost)
    {
        g_pPreview->CloseDevice();

        MessageBox(hwnd, L"Lost the capture device.", WINDOW_NAME, MB_OK);
    }
}


/////////////////////////////////////////////////////////////////////

// Dialog functions

HRESULT OnInitDialog(HWND hwnd, ChooseDeviceParam *pParam);
HRESULT OnOK(HWND hwnd, ChooseDeviceParam *pParam);

//-------------------------------------------------------------------
// DlgProc: Window procedure for the dialog.
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
// OnInitDialog: Handler for WM_INITDIALOG
//-------------------------------------------------------------------

HRESULT OnInitDialog(HWND hwnd, ChooseDeviceParam *pParam)
{
    HRESULT hr = S_OK;

    HWND hList = GetDlgItem(hwnd, IDC_DEVICE_LIST);

    // Display a list of the devices.

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
    else
    {
        // Select the first device in the list.
        ListBox_SetCurSel(hList, 0);
    }

    return hr;
}


//-------------------------------------------------------------------
// OnOK: Handler for the OK button
//-------------------------------------------------------------------

HRESULT OnOK(HWND hwnd, ChooseDeviceParam *pParam)
{
    HWND hList = GetDlgItem(hwnd, IDC_DEVICE_LIST);

    // Get the current selection and return it to the application.
    int sel = ListBox_GetCurSel(hList);

    if (sel != LB_ERR)
    {
        pParam->selection = (UINT32)ListBox_GetItemData(hList, sel);
    }

    return S_OK;
}


//-------------------------------------------------------------------
// ShowErrorMessage
//
// Displays an error message.
//-------------------------------------------------------------------

void ShowErrorMessage(HWND hwnd, PCWSTR format, HRESULT hrErr)
{
    HRESULT hr = S_OK;
    WCHAR msg[MAX_PATH];

    hr = StringCbPrintf(msg, sizeof(msg), L"%s (hr=0x%X)", format, hrErr);

    if (SUCCEEDED(hr))
    {
        MessageBox(hwnd, msg, L"Error", MB_ICONERROR);
    }
}

