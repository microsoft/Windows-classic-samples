// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "EhStorEnumerator2.h"
#include "PasswordSILO.h"
#include "CertificateSILO.h"

CPortableDeviceManagerImp g_DevManager;

void RefreshList(HWND hwndDlg)
{
    HRESULT hr = S_OK;
    CPortableDeviceDesc *parDevices = NULL;
    DWORD nDeviceCount = 0;
    DWORD nDeviceIndex = 0;

    hr = g_DevManager.EnumerateDevices(parDevices, nDeviceCount);
    if (FAILED(hr))
    {
        // display error message
        return;
    }

    ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_DEVLIST));

    for (nDeviceIndex = 0; nDeviceIndex < nDeviceCount; nDeviceIndex ++)
    {
        int nItemIndex;
        LVITEM item = {LVIF_TEXT, nDeviceIndex, 0, 0, 0, (LPWSTR)parDevices[nDeviceIndex].get_Description()};

        nItemIndex = ListView_InsertItem(GetDlgItem(hwndDlg, IDC_DEVLIST), &item);
        ListView_SetItemText(GetDlgItem(hwndDlg, IDC_DEVLIST), nItemIndex, 1, (LPWSTR)parDevices[nDeviceIndex].get_Manufacturer());
        ListView_SetItemText(GetDlgItem(hwndDlg, IDC_DEVLIST), nItemIndex, 2, (LPWSTR)parDevices[nDeviceIndex].get_PnpID());
    }

    ListView_SetColumnWidth(GetDlgItem(hwndDlg, IDC_DEVLIST), 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(GetDlgItem(hwndDlg, IDC_DEVLIST), 1, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(GetDlgItem(hwndDlg, IDC_DEVLIST), 2, LVSCW_AUTOSIZE_USEHEADER);

    delete[] parDevices;
}

void OnInitDialog(HWND hwndDlg)
{
    HRESULT hr = S_OK;

    ListView_SetExtendedListViewStyle(GetDlgItem(hwndDlg, IDC_DEVLIST), LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// Init portable devices manager
    hr = g_DevManager.InitManager();
    if (FAILED(hr))
    {
        MessageBox(hwndDlg, _T("Unable to initialize portable device manager"), _T("ERROR"), MB_OK | MB_ICONERROR);
        EndDialog(hwndDlg, IDOK);
        return;
    }

    LVCOLUMN col1 = {LVCF_FMT | LVCF_WIDTH | LVCF_TEXT, LVCFMT_LEFT, 100, _T("Description")};
    ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_DEVLIST), 0, &col1);

    LVCOLUMN col2 = {LVCF_FMT | LVCF_WIDTH | LVCF_TEXT, LVCFMT_LEFT, 100, _T("Manufacturer")};
    ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_DEVLIST), 1, &col2);

    LVCOLUMN col3 = {LVCF_FMT | LVCF_WIDTH | LVCF_TEXT, LVCFMT_LEFT, 100, _T("PNP ID")};
    ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_DEVLIST), 2, &col3);

    RefreshList(hwndDlg);
}

INT_PTR MainDialogOnCommand(HWND hwndDlg, WORD wControlID, WORD wNotifyCode, HWND hControlWnd)
{
    switch (wControlID)
    {
    case IDC_REFRESH:
        return TRUE;
    case IDCANCEL:
        EndDialog(hwndDlg, wControlID);
        return TRUE;
    case ID_PASSWORD_QUERYINFORMATION:
        OnPasswordQueryInformation(hwndDlg);
        return TRUE;
    case ID_PASSWORD_SET:
        OnPasswordSet(hwndDlg);
        return TRUE;
    case ID_PASSWORD_INITTOMANUFACTURERSTATE:
        OnPasswordInittomanufacturerstate(hwndDlg);
        return TRUE;
    case ID_CERTIFICATE_QUERYINFORMATION:
        OnCertificateQueryinformation(hwndDlg);
        return TRUE;
    case ID_CERTIFICATE_HOSTAUTHENTICATION:
        OnCertificateHostauthentication(hwndDlg);
        return TRUE;
    case ID_CERTIFICATE_DEVICEAUTHENTICATION:
        OnCertificateDeviceauthentication(hwndDlg);
        return TRUE;
    case ID_CERTIFICATE_UNAUTHENTICATION:
        OnCertificateUnauthentication(hwndDlg);
        return TRUE;
    case ID_CERTIFICATE_CERTIFICATES:
        OnCertificateCertificates(hwndDlg);
        return TRUE;
    case ID_CERTIFICATE_INITTOMANUFACTURERSTATE:
        OnCertificateInittomanufacturerstate(hwndDlg);
        return TRUE;
    default:
        return FALSE;
    }
}

void OnDevListRightClick(HWND hwndDlg, LPNMITEMACTIVATE pNMItemActivate)
{
    HMENU hmenu = NULL;
    HMENU hmenuTrackPopup = NULL;
    TCHAR szStringBuffer[256];

    ClientToScreen(hwndDlg, &pNMItemActivate->ptAction);

    hmenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_POPUPMENU)); 
    if (hmenu == NULL) {
        return;
    }

    ListView_GetItemText(GetDlgItem(hwndDlg, IDC_DEVLIST), pNMItemActivate->iItem, 0, szStringBuffer, _countof(szStringBuffer));
    // are we on 1667 based device ???
    if (lstrcmp(szStringBuffer, _T("Microsoft WPD Enhanced Storage Password Driver")) == 0)
    {
        // password silo device
        hmenuTrackPopup = GetSubMenu(hmenu, 0); 
    }
    else
    if (lstrcmp(szStringBuffer, _T("Microsoft WPD Enhanced Storage Certificate Driver")) == 0)
    {
        // certificate silo device
        hmenuTrackPopup = GetSubMenu(hmenu, 1); 
    }
    else
    {
        // not a 1667 device, give up
        ::DestroyMenu(hmenu);
        return;
    }

    ::TrackPopupMenu(hmenuTrackPopup, TPM_LEFTBUTTON, pNMItemActivate->ptAction.x, pNMItemActivate->ptAction.y, 0, hwndDlg, NULL);

    ::DestroyMenu(hmenu);
}

BOOL MainDialogOnNotify(HWND hwndDlg, int idCtrl, LPNMHDR pnmh)
{
    switch(idCtrl)
    {
    case IDC_DEVLIST:
        switch(pnmh->code)
        {
        case NM_RCLICK:
            OnDevListRightClick(hwndDlg, (LPNMITEMACTIVATE)pnmh);
            return TRUE;
        default:
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

INT_PTR CALLBACK MainDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_COMMAND:
        return MainDialogOnCommand(hwndDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_NOTIFY:
        return MainDialogOnNotify(hwndDlg, (int)wParam, (LPNMHDR)lParam);
    case WM_INITDIALOG:
        OnInitDialog(hwndDlg);
        break;
    }

    return 0;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    // create main dialog
    DialogBox(hInstance,  MAKEINTRESOURCE(IDD_EHSTORENUMERATOR_DIALOG), GetDesktopWindow(), MainDialogProc);

	return 0;
}

// get selected device index and PNP id
int GetSelectedDevice(HWND hwndDlg, LPTSTR szPNPID, LONG nBufferSize)
{
    int nSelectedDeviceIDX = -1;
    HWND hListCtrl = GetDlgItem(hwndDlg, IDC_DEVLIST);

    if (szPNPID == NULL || nBufferSize < 1 || hListCtrl == NULL) {
        return -1;
    }

    if (ListView_GetSelectedCount(hListCtrl) != 1)
    {
        // no selected item
        szPNPID[0] = 0;
        return -1;
    }

    nSelectedDeviceIDX = ListView_GetNextItem(hListCtrl, -1 , LVNI_SELECTED);
    if (nSelectedDeviceIDX >= 0) {
        ListView_GetItemText(hListCtrl, nSelectedDeviceIDX, 2, szPNPID, nBufferSize);
    }

    return nSelectedDeviceIDX;
}

void DisplayError(HWND hwndDlg, LPCTSTR szMessage, HRESULT hrError)
{
    TCHAR szMessageBuf[MAX_PATH];

    StringCbPrintf(szMessageBuf, _countof(szMessageBuf), _T("%s\n0x%08X"), szMessage, hrError);
    MessageBox(hwndDlg, szMessageBuf, _T(""),
        MB_OK | (FAILED(hrError) ? MB_ICONERROR : MB_ICONINFORMATION));
}
