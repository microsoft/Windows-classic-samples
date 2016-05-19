// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "PasswordSILO.h"
#include "EhStorEnumerator2.h"

TCHAR g_szData[256] = {0};
CPasswordSiloInformation g_siloInformation;

class CChangePasswordData
{
public:
    CChangePasswordData()
    {
        memset(szOldPassword, 0, _countof(szOldPassword));
        memset(szNewPassword, 0, _countof(szNewPassword));
        memset(szSID, 0, _countof(szSID));
        memset(szPasswordHint, 0, _countof(szPasswordHint));
        nPasswordIndicator = PASSWD_INDICATOR_ADMIN;
    }

    TCHAR szOldPassword[256];
    TCHAR szNewPassword[256];
    TCHAR szSID[256];
    TCHAR szPasswordHint[256];
    PASSWD_INDICATOR nPasswordIndicator;
};

CChangePasswordData g_ChangePwdData;

void OnQueryPwdDialogInit(HWND hwndDlg)
{
    CPortableDeviceImp device;
    int nSelectedDeviceIDX = -1;
    TCHAR szDevicePNPID[MAX_PATH] = {0};
    HRESULT hr = S_OK;

    nSelectedDeviceIDX = GetSelectedDevice(GetParent(hwndDlg), szDevicePNPID, _countof(szDevicePNPID));
    if (nSelectedDeviceIDX < 0) {
        return;
    }

    EXEC_CHECKHR(device.OpenDevice(szDevicePNPID));
    EXEC_CHECKHR(device.PasswordQueryInformation(g_siloInformation));

    if (FAILED(hr))
    {
        DisplayError(hwndDlg, _T("OnPasswordQueryinformation(): Error"), hr);
    }

    if (SUCCEEDED(hr))
    {
        SetDlgItemText(hwndDlg, IDC_FRIENDLY_NAME, g_siloInformation.get_SiloName());
        SetDlgItemText(hwndDlg, IDC_ADMIN_HINT, g_siloInformation.get_AdminHint());

        if (g_siloInformation.SiloInfo.UserCreated)
        {
            SetDlgItemText(hwndDlg, IDC_USER_NAME, g_siloInformation.get_UserName());
            SetDlgItemText(hwndDlg, IDC_USER_HINT, g_siloInformation.get_UserHint());
        }
        else
        {
            SetDlgItemText(hwndDlg, IDC_USER_NAME, _T("{No User Created}"));
        }

        switch (g_siloInformation.dwAuthnState)
        {
        case 0x00:
            SetDlgItemText(hwndDlg, IDC_AUTHN_STATE, _T("Not Authorized"));
            break;
        case 0x01:
            SetDlgItemText(hwndDlg, IDC_AUTHN_STATE, _T("Authorized"));
            break;
        case 0x02:
            SetDlgItemText(hwndDlg, IDC_AUTHN_STATE, _T("Not Provisioned"));
            break;
        default:
            SetDlgItemText(hwndDlg, IDC_AUTHN_STATE, _T("Undefined state"));
            break;
        }
    }
}

INT_PTR CALLBACK PwdDefDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
            case IDCANCEL:
            case IDOK:
                EndDialog(hwndDlg, LOWORD(wParam));
                return TRUE;
        }
        break;
    }

    return 0;
}

INT_PTR CALLBACK PwdQueryDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        OnQueryPwdDialogInit(hwndDlg);
        break;
    default:
        return PwdDefDialogProc(hwndDlg, uMsg, wParam, lParam);
    }

    return 0;
}

INT_PTR CALLBACK PwdITMSDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        EnableWindow(GetDlgItem(hwndDlg, IDC_DEVICE_SID), g_siloInformation.SiloInfo.SecurityIDAvailable);
        break;
    case WM_DESTROY:
        GetDlgItemText(hwndDlg, IDC_DEVICE_SID, g_szData, _countof(g_szData));
        break;
    default:
        return PwdDefDialogProc(hwndDlg, uMsg, wParam, lParam);
    }

    return 0;
}

void OnPasswordQueryInformation(HWND hwndDlg)
{
    DialogBox(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_PWDSILO_INFO), hwndDlg, PwdQueryDialogProc);
}

void OnPasswordInittomanufacturerstate(HWND hwndDlg)
{
    HRESULT hr = S_OK;
    CPortableDeviceImp device;
    int nSelectedDeviceIDX = -1;
    TCHAR szDevicePNPID[MAX_PATH] = {0};

    nSelectedDeviceIDX = GetSelectedDevice(hwndDlg, szDevicePNPID, _countof(szDevicePNPID));
    if (nSelectedDeviceIDX < 0)
    {
        return;
    }

    // Open device
    EXEC_CHECKHR(device.OpenDevice(szDevicePNPID));

    // Query device information
    EXEC_CHECKHR(device.PasswordQueryInformation(g_siloInformation));

    if (SUCCEEDED(hr))
    {
        memset(g_szData, 0, sizeof(g_szData));

        if (DialogBox(GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDD_PWD_ITMS), hwndDlg, PwdITMSDialogProc) == IDOK)
        {
            // Send command
            EXEC_CHECKHR(device.PasswordInitializeToManufacturerState(g_szData));
        }
    }

    if (FAILED(hr))
    {
        DisplayError(hwndDlg, _T("OnPasswordInittomanufacturerstate(): Error"), hr);
    }
}

INT_PTR CALLBACK PwdPasswordSetDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        CheckDlgButton(hwndDlg, IDC_INDICATOR_ADMIN, BST_CHECKED);
        EnableWindow(GetDlgItem(hwndDlg, IDC_INDICATOR_USER), g_siloInformation.SiloInfo.UserCreated);
        EnableWindow(GetDlgItem(hwndDlg, IDC_DEVICE_SID), g_siloInformation.SiloInfo.SecurityIDAvailable);
        EnableWindow(GetDlgItem(hwndDlg, IDC_ODL_PASSWORD), (g_siloInformation.dwAuthnState != 0x02));
        SetDlgItemText(hwndDlg, IDC_PASSWORD_HINT, g_siloInformation.get_AdminHint());
        break;
    case WM_DESTROY:
        GetDlgItemText(hwndDlg, IDC_ODL_PASSWORD, g_ChangePwdData.szOldPassword, _countof(g_ChangePwdData.szOldPassword));
        GetDlgItemText(hwndDlg, IDC_NEW_PASSWORD, g_ChangePwdData.szNewPassword, _countof(g_ChangePwdData.szNewPassword));
        GetDlgItemText(hwndDlg, IDC_DEVICE_SID, g_ChangePwdData.szSID, _countof(g_ChangePwdData.szSID));
        GetDlgItemText(hwndDlg, IDC_PASSWORD_HINT, g_ChangePwdData.szPasswordHint, _countof(g_ChangePwdData.szPasswordHint));
        if (IsDlgButtonChecked(hwndDlg, IDC_INDICATOR_USER) == BST_CHECKED) {
            g_ChangePwdData.nPasswordIndicator = PASSWD_INDICATOR_USER;
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
            case IDC_INDICATOR_ADMIN:
                SetDlgItemText(hwndDlg, IDC_PASSWORD_HINT, g_siloInformation.get_AdminHint());
                break;
            case IDC_INDICATOR_USER:
                SetDlgItemText(hwndDlg, IDC_PASSWORD_HINT, g_siloInformation.get_UserHint());
                break;
            default:
                return PwdDefDialogProc(hwndDlg, uMsg, wParam, lParam);
        }
        break;
    default:
        return PwdDefDialogProc(hwndDlg, uMsg, wParam, lParam);
    }

    return 0;
}

void OnPasswordSet(HWND hwndDlg)
{
    HRESULT hr = S_OK;
    CPortableDeviceImp device;
    int nSelectedDeviceIDX = -1;
    TCHAR szDevicePNPID[MAX_PATH] = {0};

    nSelectedDeviceIDX = GetSelectedDevice(hwndDlg, szDevicePNPID, _countof(szDevicePNPID));
    if (nSelectedDeviceIDX < 0)
    {
        return;
    }

    // Open device
    EXEC_CHECKHR(device.OpenDevice(szDevicePNPID));

    // Query device information
    EXEC_CHECKHR(device.PasswordQueryInformation(g_siloInformation));

    if (SUCCEEDED(hr))
    {
        g_ChangePwdData = CChangePasswordData();
        if (DialogBox(GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDD_SET_PASSWORD), hwndDlg, PwdPasswordSetDialogProc) == IDOK)
        {
            // transmit command to the silo
            EXEC_CHECKHR(device.PasswordChangePassword(
                    g_ChangePwdData.nPasswordIndicator, 
                    g_ChangePwdData.szOldPassword,
                    g_ChangePwdData.szNewPassword,
                    g_ChangePwdData.szPasswordHint,
                    g_ChangePwdData.szSID));
        }
    }

    if (FAILED(hr))
    {
        DisplayError(hwndDlg, _T("OnPasswordSet(): Error"), hr);
    }
}
