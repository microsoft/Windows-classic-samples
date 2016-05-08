//////////////////////////////////////////////////////////////////////////
//
// winmain.cpp. Application entry-point.
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
#include <mfidl.h>
#include <mfreadwrite.h>
#include <assert.h>
#include <strsafe.h>
#include <shlwapi.h>
#include <Dbt.h>
#include <ks.h>
#include <ksmedia.h>

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


#include "capture.h"
#include "resource.h"

// Include the v6 common controls in the manifest
#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")

enum FileContainer
{
    FileContainer_MP4 = IDC_CAPTURE_MP4,
    FileContainer_WMV = IDC_CAPTURE_WMV
};
    
DeviceList  g_devices;
CCapture    *g_pCapture = NULL;
HDEVNOTIFY  g_hdevnotify = NULL;

const UINT32 TARGET_BIT_RATE = 240 * 1000;

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void    OnInitDialog(HWND hDlg); 
void    OnCloseDialog();

void    UpdateUI(HWND hDlg);
void    StopCapture(HWND hDlg);
void    StartCapture(HWND hDlg);
void    OnSelectEncodingType(HWND hDlg, FileContainer file);

HRESULT GetSelectedDevice(HWND hDlg, IMFActivate **ppActivate);
HRESULT UpdateDeviceList(HWND hDlg);
void    OnDeviceChange(HWND hwnd, WPARAM reason, DEV_BROADCAST_HDR *pHdr);

void    NotifyError(HWND hwnd, const WCHAR *sErrorMessage, HRESULT hrErr);
void    EnableDialogControl(HWND hDlg, int nIDDlgItem, BOOL bEnable); 


INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, INT /*nCmdShow*/)
{
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    INT_PTR ret = DialogBox(
        hInstance, 
        MAKEINTRESOURCE(IDD_DIALOG1), 
        NULL, 
        DialogProc 
        );

    if (ret == 0 || ret == -1)
    {
        MessageBox( NULL, L"Could not create dialog", L"Error", MB_OK | MB_ICONERROR );
    }

    return 0;
}




//-----------------------------------------------------------------------------
// Dialog procedure
//-----------------------------------------------------------------------------

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        OnInitDialog(hDlg);
        break;

    case WM_DEVICECHANGE:
        OnDeviceChange(hDlg, wParam, (PDEV_BROADCAST_HDR)lParam);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CAPTURE_MP4:   // Fall through
        case IDC_CAPTURE_WMV:
            OnSelectEncodingType(hDlg, (FileContainer)(LOWORD(wParam)));
            return TRUE;

        case IDC_CAPTURE:
            if (g_pCapture && g_pCapture->IsCapturing())
            {
                StopCapture(hDlg);
            }
            else
            {
                StartCapture(hDlg);
            }
            return TRUE;

        case IDCANCEL:
            OnCloseDialog();
            ::EndDialog(hDlg, IDCANCEL);
            return TRUE;

        }
        break;
    }
    return FALSE;
}

//-----------------------------------------------------------------------------
// OnInitDialog
// Handler for WM_INITDIALOG message.
//-----------------------------------------------------------------------------

void OnInitDialog(HWND hDlg)
{
    HRESULT hr = S_OK;

    HWND hEdit = GetDlgItem(hDlg, IDC_OUTPUT_FILE);

    SetWindowText(hEdit, TEXT("capture.mp4"));

    CheckRadioButton(hDlg, IDC_CAPTURE_MP4, IDC_CAPTURE_WMV, IDC_CAPTURE_MP4); 

    // Initialize the COM library
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    // Initialize Media Foundation
    if (SUCCEEDED(hr))
    {
        hr = MFStartup(MF_VERSION);
    }

    // Register for device notifications
    if (SUCCEEDED(hr))
    {
        DEV_BROADCAST_DEVICEINTERFACE di = { 0 };

        di.dbcc_size = sizeof(di);
        di.dbcc_devicetype  = DBT_DEVTYP_DEVICEINTERFACE;
        di.dbcc_classguid  = KSCATEGORY_CAPTURE; 

        g_hdevnotify = RegisterDeviceNotification(
            hDlg,
            &di,
            DEVICE_NOTIFY_WINDOW_HANDLE
            );

        if (g_hdevnotify == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Enumerate the video capture devices.
    if (SUCCEEDED(hr))
    {
        hr = UpdateDeviceList(hDlg);
    }

    if (SUCCEEDED(hr))
    {
        UpdateUI(hDlg);

        if (g_devices.Count() == 0)
        {
            ::MessageBox(
                hDlg, 
                TEXT("Could not find any video capture devices."),
                TEXT("MFCaptureToFile"),
                MB_OK
                );
        }
    }
    else    
    {
        OnCloseDialog();
        ::EndDialog(hDlg, 0);
    }
}



//-----------------------------------------------------------------------------
// OnCloseDialog
// 
// Frees resources before closing the dialog.
//-----------------------------------------------------------------------------

void OnCloseDialog()
{
    if (g_pCapture)
    {
        g_pCapture->EndCaptureSession();
    }

    SafeRelease(&g_pCapture);

    g_devices.Clear();

    if (g_hdevnotify)
    {
        UnregisterDeviceNotification(g_hdevnotify);
    }

    MFShutdown();
    CoUninitialize();
}


//-----------------------------------------------------------------------------
// StartCapture
// 
// Starts video capture.
//-----------------------------------------------------------------------------

void StartCapture(HWND hDlg)
{
    EncodingParameters params;

    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CAPTURE_WMV))
    {
        params.subtype = MFVideoFormat_WMV3;
    }
    else
    {
        params.subtype = MFVideoFormat_H264;
    }

    params.bitrate = TARGET_BIT_RATE;

    HRESULT hr = S_OK;
    WCHAR   pszFile[MAX_PATH] = { 0 };
    HWND    hEdit = GetDlgItem(hDlg, IDC_OUTPUT_FILE);

    IMFActivate *pActivate = NULL;

    // Get the name of the target file.

    if (0 == GetWindowText(hEdit, pszFile, MAX_PATH))
    {
       hr = HRESULT_FROM_WIN32(GetLastError());
    }

    // Create the media source for the capture device.

    if (SUCCEEDED(hr))
    {
        hr = GetSelectedDevice(hDlg, &pActivate);
    }

    // Start capturing.

    if (SUCCEEDED(hr))
    {
        hr = CCapture::CreateInstance(hDlg, &g_pCapture);
    }

    if (SUCCEEDED(hr))
    {
        hr = g_pCapture->StartCapture(pActivate, pszFile, params);
    }

    if (SUCCEEDED(hr))
    {
        UpdateUI(hDlg);
    }


    SafeRelease(&pActivate);

    if (FAILED(hr))
    {
        NotifyError(hDlg, L"Error starting capture.", hr);
    }
}


//-----------------------------------------------------------------------------
// StopCapture
// 
// Stops video capture.
//-----------------------------------------------------------------------------

void StopCapture(HWND hDlg)
{
    HRESULT hr = S_OK;
    
    hr = g_pCapture->EndCaptureSession();

    SafeRelease(&g_pCapture);

    UpdateDeviceList(hDlg);

    // NOTE: Updating the device list releases the existing IMFActivate 
    // pointers. This ensures that the current instance of the video capture 
    // source is released.

    UpdateUI(hDlg);

    if (FAILED(hr))
    {
        NotifyError(hDlg, L"Error stopping capture. File might be corrupt.", hr);
    }
}



//-----------------------------------------------------------------------------
// CreateSelectedDevice
// 
// Create a media source for the video capture device selected by the user.
//-----------------------------------------------------------------------------

HRESULT GetSelectedDevice(HWND hDlg, IMFActivate **ppActivate)
{
    HWND hDeviceList = GetDlgItem(hDlg, IDC_DEVICE_LIST);

    // First get the index of the selected item in the combo box.
    int iListIndex = ComboBox_GetCurSel(hDeviceList);

    if (iListIndex == CB_ERR)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Now find the index of the device within the device list.
    //
    // This index is stored as item data in the combo box, so that 
    // the order of the combo box items does not need to match the
    // order of the device list.

    LRESULT iDeviceIndex = ComboBox_GetItemData(hDeviceList, iListIndex);

    if (iDeviceIndex == CB_ERR)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Now create the media source.

    return g_devices.GetDevice((UINT32)iDeviceIndex, ppActivate);
}


//-----------------------------------------------------------------------------
// UpdateDeviceList
// 
// Enumerates the video capture devices and populates the list of device
// names in the dialog UI.
//-----------------------------------------------------------------------------

HRESULT UpdateDeviceList(HWND hDlg)
{
    HRESULT hr = S_OK;

    WCHAR *szFriendlyName = NULL;

    HWND hCombobox = GetDlgItem(hDlg, IDC_DEVICE_LIST);

    ComboBox_ResetContent( hCombobox );

    g_devices.Clear();

    hr = g_devices.EnumerateDevices();

    if (FAILED(hr)) { goto done; }

    for (UINT32 iDevice = 0; iDevice < g_devices.Count(); iDevice++)
    {
        // Get the friendly name of the device.

        hr = g_devices.GetDeviceName(iDevice, &szFriendlyName);

        if (FAILED(hr)) { goto done; }

        // Add the string to the combo-box. This message returns the index in the list.

        int iListIndex = ComboBox_AddString(hCombobox, szFriendlyName);
        if (iListIndex == CB_ERR || iListIndex == CB_ERRSPACE)
        {
            hr = E_FAIL;
            goto done;
        }

        // The list might be sorted, so the list index is not always the same as the
        // array index. Therefore, set the array index as item data.

        int result = ComboBox_SetItemData(hCombobox, iListIndex, iDevice);

        if (result == CB_ERR)
        {
            hr = E_FAIL;
            goto done;
        }

        CoTaskMemFree(szFriendlyName);
        szFriendlyName = NULL;
    }

    if (g_devices.Count() > 0)
    {
        // Select the first item.
        ComboBox_SetCurSel(hCombobox, 0);
    }

done:
    return hr;
}


//-----------------------------------------------------------------------------
// OnSelectEncodingType
// 
// Called when the user toggles between file-format types.
//-----------------------------------------------------------------------------

void OnSelectEncodingType(HWND hDlg, FileContainer file)
{
    WCHAR pszFile[MAX_PATH] = { 0 };

    HWND hEdit = GetDlgItem(hDlg, IDC_OUTPUT_FILE);

    GetWindowText(hEdit, pszFile, MAX_PATH);

    switch (file)
    {
    case FileContainer_MP4:

        PathRenameExtension(pszFile, L".mp4");
        break;

    case FileContainer_WMV:
        PathRenameExtension(pszFile, L".wmv");
        break;

    default:
        assert(false);
        break;
    }

    SetWindowText(hEdit, pszFile);
}


//-----------------------------------------------------------------------------
// UpdateUI
// 
// Updates the dialog UI for the current state.
//-----------------------------------------------------------------------------

void UpdateUI(HWND hDlg)
{
    BOOL bEnable = (g_devices.Count() > 0);     // Are there any capture devices?
    BOOL bCapturing = (g_pCapture != NULL);     // Is video capture in progress now?

    HWND hButton = GetDlgItem(hDlg, IDC_CAPTURE);

    if (bCapturing)
    {
        SetWindowText(hButton, L"Stop Capture");
    }
    else
    {
        SetWindowText(hButton, L"Start Capture");
    }

    EnableDialogControl(hDlg, IDC_CAPTURE, bCapturing || bEnable);

    EnableDialogControl(hDlg, IDC_DEVICE_LIST, !bCapturing && bEnable);

    // The following cannot be changed while capture is in progress,
    // but are OK to change when there are no capture devices.

    EnableDialogControl(hDlg, IDC_CAPTURE_MP4, !bCapturing);
    EnableDialogControl(hDlg, IDC_CAPTURE_WMV, !bCapturing);
    EnableDialogControl(hDlg, IDC_OUTPUT_FILE, !bCapturing);
}


//-----------------------------------------------------------------------------
// OnDeviceChange
// 
// Handles WM_DEVICECHANGE messages.
//-----------------------------------------------------------------------------

void OnDeviceChange(HWND hDlg, WPARAM reason, DEV_BROADCAST_HDR *pHdr)
{
    if (reason == DBT_DEVNODES_CHANGED || reason == DBT_DEVICEARRIVAL)
    {
        // Check for added/removed devices, regardless of whether
        // the application is capturing video at this time.

        UpdateDeviceList(hDlg);
        UpdateUI(hDlg);
    }

    // Now check if the current video capture device was lost.

    if (pHdr == NULL)
    {
        return;
    }
    if (pHdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
    {
        return;
    }

    HRESULT hr = S_OK;
    BOOL bDeviceLost = FALSE;

    if (g_pCapture && g_pCapture->IsCapturing())
    {
        hr = g_pCapture->CheckDeviceLost(pHdr, &bDeviceLost);

        if (FAILED(hr) || bDeviceLost)
        {
            StopCapture(hDlg);

            MessageBox(hDlg, L"The capture device was removed or lost.", L"Lost Device", MB_OK);
        }
    }
}


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


void EnableDialogControl(HWND hDlg, int nIDDlgItem, BOOL bEnable) 
{ 
    HWND hwnd = GetDlgItem(hDlg, nIDDlgItem);

    if (!bEnable &&  hwnd == GetFocus())
    {
        // When disabling a control that has focus, set the 
        // focus to the next control.

        ::SendMessage(GetParent(hwnd), WM_NEXTDLGCTL, 0, FALSE);
    }
    EnableWindow(hwnd, bEnable); 
}



