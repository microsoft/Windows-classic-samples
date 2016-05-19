// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "CertificateSILO.h"
#include "EhStorEnumerator2.h"
#include "SetCertificateDlg.h"

CDeviceCertData g_DeviceCertData;

INT_PTR CALLBACK CertDefDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

void OnQueryCertDialogInit(HWND hwndDlg)
{
    HRESULT hr = S_OK;
    CPortableDeviceImp device;
    int nSelectedDeviceIDX = -1;
    TCHAR szDevicePNPID[MAX_PATH] = {0};
    ULONG nStoredCertCount = 0;
    ULONG nMaxCertCount = 0;
    ULONG nState = 0;
    TCHAR szStringBuffer[256];
    LPTSTR *ppSiloCapablities = NULL;
    ULONG n, nCapCnt = 0;

    nSelectedDeviceIDX = GetSelectedDevice(GetParent(hwndDlg), szDevicePNPID, _countof(szDevicePNPID));
    if (nSelectedDeviceIDX < 0)
    {
        return;
    }

    // Open device
    EXEC_CHECKHR(device.OpenDevice(szDevicePNPID));

    // execute commands to query info
    EXEC_CHECKHR(device.CertGetSiloFriendlyName(szStringBuffer, _countof(szStringBuffer)));
    if (SUCCEEDED(hr)) {
        SetDlgItemText(hwndDlg, IDC_FRIENDLY_NAME, szStringBuffer);
    }

    if (SUCCEEDED(hr))
    {
        hr = device.CertGetSiloGUID(szStringBuffer, _countof(szStringBuffer));
        if (hr = CRYPT_E_NO_TRUSTED_SIGNER)
        {
            SetDlgItemText(hwndDlg, IDC_SILO_GUID, _T("Cannot retrieve, device not trusted"));
            hr = S_OK;
        }
        else
        {
            SetDlgItemText(hwndDlg, IDC_SILO_GUID, szStringBuffer);
        }
    }

    EXEC_CHECKHR(device.CertGetCertificatesCount(nStoredCertCount, nMaxCertCount));
    if (SUCCEEDED(hr)) {
        StringCchPrintf(szStringBuffer, _countof(szStringBuffer), _T("Stored: %d, Max: %d"), nStoredCertCount, nMaxCertCount);
        SetDlgItemText(hwndDlg, IDC_CERT_COUNT, szStringBuffer);
    }
    else
    {
        SetDlgItemText(hwndDlg, IDC_CERT_COUNT, _T("{Unable to retrieve}"));
        hr = S_OK;
    }

    EXEC_CHECKHR(device.CertGetState(nState, szStringBuffer, _countof(szStringBuffer)));
    if (SUCCEEDED(hr)) {
        SetDlgItemText(hwndDlg, IDC_AUTHN_STATE, szStringBuffer);
    }

    EXEC_CHECKHR(device.CertGetSiloCapablity(CAPTYPE_ASSYM_KEYS, ppSiloCapablities, nCapCnt));
    if (SUCCEEDED(hr) && ppSiloCapablities)
    {
        for (n = 0; n < nCapCnt; n ++)
        {
            if (ppSiloCapablities[n])
            {
                ListBox_AddString(GetDlgItem(hwndDlg, IDC_ASYMM_KEY), ppSiloCapablities[n]);
                free(ppSiloCapablities[n]);
            }
        }
        free(ppSiloCapablities);
        ppSiloCapablities = NULL;
    }

    EXEC_CHECKHR(device.CertGetSiloCapablity(CAPTYPE_HASH_ALGS, ppSiloCapablities, nCapCnt));
    if (SUCCEEDED(hr) && ppSiloCapablities)
    {
        for (n = 0; n < nCapCnt; n ++)
        {
            if (ppSiloCapablities[n])
            {
                ListBox_AddString(GetDlgItem(hwndDlg, IDC_HASH_ALGS), ppSiloCapablities[n]);
                free(ppSiloCapablities[n]);
            }
        }
        free(ppSiloCapablities);
        ppSiloCapablities = NULL;
    }

    EXEC_CHECKHR(device.CertGetSiloCapablity(CAPTYPE_SIGNING_ALGS, ppSiloCapablities, nCapCnt));
    if (SUCCEEDED(hr) && ppSiloCapablities)
    {
        for (n = 0; n < nCapCnt; n ++)
        {
            if (ppSiloCapablities[n])
            {
                ListBox_AddString(GetDlgItem(hwndDlg, IDC_SIGNING_ALGS), ppSiloCapablities[n]);
                free(ppSiloCapablities[n]);
            }
        }
        free(ppSiloCapablities);
        ppSiloCapablities = NULL;
    }
}

INT_PTR CALLBACK CertQueryDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        OnQueryCertDialogInit(hwndDlg);
        break;
    default:
        return CertDefDialogProc(hwndDlg, uMsg, wParam, lParam);
    }

    return 0;
}

void OnCertificateQueryinformation(HWND hwndDlg)
{
    DialogBox(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_CERT_SILO_INFO), hwndDlg, CertQueryDialogProc);
}

void OnCertificateHostauthentication(HWND hwndDlg)
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
    EXEC_CHECKHR(device.CertHostAuthentication());

    DisplayError(hwndDlg, _T("OnCertificateHostauthentication(): Returned code:"), hr);
}

void OnCertificateDeviceauthentication(HWND hwndDlg)
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
    EXEC_CHECKHR(device.CertDeviceAuthentication());

    DisplayError(hwndDlg, _T("OnCertificateDeviceauthentication(): Returned code:"), hr);
}

void OnCertificateUnauthentication(HWND hwndDlg)
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
    EXEC_CHECKHR(device.CertUnAuthentication());

    DisplayError(hwndDlg, _T("OnCertificateUnauthentication(): Returned code:"), hr);
}

void OnCertificateInittomanufacturerstate(HWND hwndDlg)
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
    EXEC_CHECKHR(device.CertInitializeToManufacturedState());

    DisplayError(hwndDlg, _T("OnCertificateInittomanufacturerstate(): Returned code:"), hr);
}

// delete all columns from the list view
void DeleteAllColumns(HWND hwndDlg)
{
    HWND hCertList = GetDlgItem(hwndDlg, IDC_CERT_LIST);
    HWND hHeader = hCertList ? ListView_GetHeader(hCertList) : NULL;
    int nColumnsCnt = hHeader ? Header_GetItemCount(hHeader) : 4;

    for (; nColumnsCnt >= 0; nColumnsCnt--)
        ListView_DeleteColumn(hCertList, nColumnsCnt);
}

// fill the list of certificates from device
void FillDeviceList(HWND hwndDlg)
{
    CPortableDeviceImp device;
    HRESULT hr = S_OK;
    HWND hCertList = GetDlgItem(hwndDlg, IDC_CERT_LIST);
    ULONG nMaxCertCount = 0;
    ULONG nArrayIndex = 0;
    ULONG nNextCertIndex = 0;

    DeleteAllColumns(hwndDlg);
    ListView_DeleteAllItems(hCertList);

    LVCOLUMN col1 = {LVCF_FMT | LVCF_WIDTH | LVCF_TEXT, LVCFMT_LEFT, 100, _T("Index")};
    ListView_InsertColumn(hCertList, 0, &col1);

    LVCOLUMN col2 = {LVCF_FMT | LVCF_WIDTH | LVCF_TEXT, LVCFMT_LEFT, 100, _T("Type")};
    ListView_InsertColumn(hCertList, 1, &col2);

    LVCOLUMN col3 = {LVCF_FMT | LVCF_WIDTH | LVCF_TEXT, LVCFMT_LEFT, 100, _T("V. Policy")};
    ListView_InsertColumn(hCertList, 2, &col3);

    LVCOLUMN col4 = {LVCF_FMT | LVCF_WIDTH | LVCF_TEXT, LVCFMT_LEFT, 100, _T("Signer")};
    ListView_InsertColumn(hCertList, 3, &col4);

    EXEC_CHECKHR(device.OpenDevice(g_DeviceCertData.m_szDevicePNPID));

    g_DeviceCertData.EmptyCertList();

    // create array for certificates
    EXEC_CHECKHR(device.CertGetCertificatesCount(g_DeviceCertData.m_nCertificatesCount, nMaxCertCount));
    if (FAILED(hr) || (g_DeviceCertData.m_nCertificatesCount == 0))
    {
        return;
    }

    g_DeviceCertData.m_parCertificates = new CCertProperties[g_DeviceCertData.m_nCertificatesCount];

    // walk trough certificate list on device. At least index '0' certificate should exists
    nNextCertIndex = 0;
    do
    {
        CCertProperties &certProperties = g_DeviceCertData.m_parCertificates[nArrayIndex++];

        EXEC_CHECKHR(device.CertGetCertificate(nNextCertIndex, certProperties));
        if (SUCCEEDED(hr))
        {
            int nItemIdx;
            TCHAR szStringBuffer[256];
            TCHAR szFormatString[256];

            nNextCertIndex = certProperties.nNextCertIndex;

            // add certificate to list view
            StringCbPrintf(szFormatString, _countof(szFormatString), _T("%d"), certProperties.nIndex);
            LVITEM item = {LVIF_TEXT, 0, 0, 0, 0, (LPWSTR)szFormatString};
            nItemIdx = ListView_InsertItem(hCertList, &item);
            
            if (certProperties.get_CertType(szStringBuffer, _countof(szStringBuffer))) {
                ListView_SetItemText(hCertList, nItemIdx, 1, szStringBuffer);
            }
            
            if (certProperties.get_ValidationPolicy(szStringBuffer, _countof(szStringBuffer))) {
                ListView_SetItemText(hCertList, nItemIdx, 2, szStringBuffer);
            }

            StringCbPrintf(szFormatString, _countof(szFormatString), _T("%d"), certProperties.nSignerCertIndex);
            ListView_SetItemText(hCertList, nItemIdx, 3, szFormatString);
        }
    }
    while (SUCCEEDED(hr) && (nNextCertIndex > 0));

    ListView_SetColumnWidth(hCertList, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hCertList, 1, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hCertList, 2, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hCertList, 3, LVSCW_AUTOSIZE_USEHEADER);
}

void FillStoreList(HWND hwndDlg, SYSTEM_STORE_NAMES store)
{
    HRESULT hr = S_OK;
    CLocalCertStoreImp localStore;
    CCertificate *parCertificates = NULL;
    DWORD nCertificatesCnt = 0;
    DWORD nIndex;
    HWND hCertList = GetDlgItem(hwndDlg, IDC_CERT_LIST);

    DeleteAllColumns(hwndDlg);
    ListView_DeleteAllItems(hCertList);

    LVCOLUMN col1 = {LVCF_FMT | LVCF_WIDTH | LVCF_TEXT, LVCFMT_LEFT, 100, _T("Subject")};
    ListView_InsertColumn(hCertList, 0, &col1);

    LVCOLUMN col2 = {LVCF_FMT | LVCF_WIDTH | LVCF_TEXT, LVCFMT_LEFT, 100, _T("Version")};
    ListView_InsertColumn(hCertList, 1, &col2);

    LVCOLUMN col3 = {LVCF_FMT | LVCF_WIDTH | LVCF_TEXT, LVCFMT_LEFT, 100, _T("Issuer")};
    ListView_InsertColumn(hCertList, 2, &col3);

    EXEC_CHECKHR(localStore.OpenSystemStore(store));
    EXEC_CHECKHR(localStore.GetCertificatesList(parCertificates, nCertificatesCnt));

    if (SUCCEEDED(hr))
    {
        for (nIndex = 0; nIndex < nCertificatesCnt; nIndex ++)
        {
            int nItemIndex = -1;
            TCHAR szFormat[256] = {0};
            CCertificate &cert = parCertificates[nIndex];

            LVITEM item = {LVIF_TEXT, 0, 0, 0, 0, (LPWSTR)cert.get_Subject()};
            nItemIndex = ListView_InsertItem(hCertList, &item);

            StringCbPrintf(szFormat, _countof(szFormat), _T("%d"), cert.get_Version());
            ListView_SetItemText(hCertList, nItemIndex, 1, szFormat);
            ListView_SetItemText(hCertList, nItemIndex, 2, (LPWSTR)cert.get_Issuer());

            // chain certificate to the list control item
            RtlZeroMemory(&item, sizeof(LVITEM));

            item.iItem = nItemIndex;
            item.mask = LVIF_PARAM;
            item.lParam = reinterpret_cast<DWORD_PTR>(new CCertificate(cert));
            ListView_SetItem(hCertList, &item);
            // ListView_SetItemData(hCertList, nItemIndex, reinterpret_cast<DWORD_PTR>(new CCertificate(cert)));
        }

        delete[] parCertificates;
    }

    ListView_SetColumnWidth(hCertList, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hCertList, 1, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hCertList, 2, LVSCW_AUTOSIZE_USEHEADER);
}

int GetSelectedCertificate(HWND hwndDlg, ULONG &nIndex)
{
    int nSelectedIDX = -1;
    HWND hCertList = GetDlgItem(hwndDlg, IDC_CERT_LIST);
    TCHAR szBuffer[50] = {0};

    if (ListView_GetSelectedCount(hCertList) != 1)
    {
        // no selected item
        return -1;
    }

    nSelectedIDX = ListView_GetNextItem(hCertList, -1 , LVNI_SELECTED);
    ListView_GetItemText(hCertList, nSelectedIDX, 0, szBuffer, _countof(szBuffer));
    nIndex = _ttol(szBuffer);

    return nSelectedIDX;
}

int GetSelectedCertificate(HWND hwndDlg, CCertificate &certificate)
{
    int nSelectedIDX = -1;
    HWND hCertList = GetDlgItem(hwndDlg, IDC_CERT_LIST);
    LVITEM item = {0};

    if (ListView_GetSelectedCount(hCertList) != 1)
    {
        // no selected item
        return -1;
    }

    nSelectedIDX = ListView_GetNextItem(hCertList, -1 , LVNI_SELECTED);

    item.iItem = nSelectedIDX;
    item.mask = LVIF_PARAM;
    if (ListView_GetItem(hCertList, &item))
    {
        certificate = reinterpret_cast<CCertificate*>(item.lParam);
    }

    return nSelectedIDX;
}

void CheckEnableButtons(HWND hwndDlg)
{
    ULONG certNum;
    int nTabSel = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_CERTTAB));

    // enable IDC_ADD_TO_DEVICE button only if tab > 0 (not device) selected and certificate selected
    EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_TO_DEVICE), (nTabSel > 0) && (GetSelectedCertificate(hwndDlg, certNum) >= 0));
    // enable IDC_DELETE button only if tab == 0 (device) selected and certificate selected
    EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), (nTabSel == 0) && (GetSelectedCertificate(hwndDlg, certNum) >= 0));
}

void OnCertDialogInit(HWND hwndDlg)
{
    TCITEM item = {0};

    item.mask = TCIF_TEXT; item.pszText = _T("Device");
    TabCtrl_InsertItem(GetDlgItem(hwndDlg, IDC_CERTTAB), 0, &item);

    item.mask = TCIF_TEXT; item.pszText = _T("Store CA");
    TabCtrl_InsertItem(GetDlgItem(hwndDlg, IDC_CERTTAB), 1, &item);

    item.mask = TCIF_TEXT; item.pszText = _T("Store MY");
    TabCtrl_InsertItem(GetDlgItem(hwndDlg, IDC_CERTTAB), 2, &item);

    item.mask = TCIF_TEXT; item.pszText = _T("Store ROOT");
    TabCtrl_InsertItem(GetDlgItem(hwndDlg, IDC_CERTTAB), 3, &item);

    item.mask = TCIF_TEXT; item.pszText = _T("Store SPC");
    TabCtrl_InsertItem(GetDlgItem(hwndDlg, IDC_CERTTAB), 4, &item);

    ListView_SetExtendedListViewStyle(GetDlgItem(hwndDlg, IDC_CERT_LIST), LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    FillDeviceList(hwndDlg);

    CheckEnableButtons(hwndDlg);
}

void OnTabSelChange(HWND hwndDlg, LPNMHDR pnmh)
{
    int nTabSel = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_CERTTAB));

    switch(nTabSel)
    {
    case 0:
        FillDeviceList(hwndDlg);
        break;
    case 1:
        FillStoreList(hwndDlg, SYSTEM_STORE_CA);
        break;
    case 2:
        FillStoreList(hwndDlg, SYSTEM_STORE_MY);
        break;
    case 3:
        FillStoreList(hwndDlg, SYSTEM_STORE_ROOT);
        break;
    case 4:
        FillStoreList(hwndDlg, SYSTEM_STORE_SPC);
        break;
    }

    CheckEnableButtons(hwndDlg);
}

void OnCertListItemChanged(HWND hwndDlg, LPNMLISTVIEW pnmv)
{
    CheckEnableButtons(hwndDlg);
}

void OnCertListDeleteItem(HWND hwndDlg, LPNMLISTVIEW pnmv)
{
    LVITEM item = {0};

    // if we have any data atached to the item we need to delete it
    item.iItem = pnmv->iItem;
    item.mask = LVIF_PARAM;
    if ((pnmv->iItem >= 0) && ListView_GetItem(GetDlgItem(hwndDlg, IDC_CERT_LIST), &item))
    {
        CCertificate *pCertificate = reinterpret_cast<CCertificate*>(item.lParam);
        if (pCertificate)
        {
            delete pCertificate;
        }
    }
}

// "Delete" button handler
void OnBnClickedDelete(HWND hwndDlg)
{
    ULONG nCertIndex;
    CPortableDeviceImp device;
    HRESULT hr = S_OK;
    TCHAR szMessage[512];

    if (GetSelectedCertificate(hwndDlg, nCertIndex) < 0) {
        return;
    }

    EXEC_CHECKHR(device.OpenDevice(g_DeviceCertData.m_szDevicePNPID));
    EXEC_CHECKHR(device.CertRemoveCertificate(nCertIndex));

    if (FAILED(hr))
    {
        LPTSTR errBuf = NULL;

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errBuf, 0, NULL);
        if (errBuf)
        {
            StringCbPrintf(szMessage, _countof(szMessage),
                _T("Certificate delete error:\nhr = 0x%08X\n%s"), hr, errBuf);
            LocalFree(errBuf);
        }
        else
        {
            StringCbPrintf(szMessage, _countof(szMessage),
                _T("Certificate delete error:\nhr = 0x%08X"), hr);
        }
    }
    else
    {
        StringCbPrintf(szMessage, _countof(szMessage),
            _T("Certificate has been deleted."));
    }

    MessageBox(hwndDlg, szMessage, _T("Delete Certificate"), MB_OK | (FAILED(hr) ? MB_ICONERROR : MB_ICONINFORMATION));

    FillDeviceList(hwndDlg);
}

BOOL CertificatesDialogOnNotify(HWND hwndDlg, int idCtrl, LPNMHDR pnmh)
{
    switch(idCtrl)
    {
    case IDC_CERTTAB:   // tab control
        switch(pnmh->code)
        {
        case TCN_SELCHANGE:
            OnTabSelChange(hwndDlg, pnmh);
            break;
        }
        break;
    case IDC_CERT_LIST: // list view
        switch(pnmh->code)
        {
        case LVN_ITEMCHANGED:
            OnCertListItemChanged(hwndDlg, (LPNMLISTVIEW)pnmh);
            break;
        case LVN_DELETEITEM:
            OnCertListDeleteItem(hwndDlg, (LPNMLISTVIEW)pnmh);
            break;
        }
        break;
    }

    return FALSE;
}

INT_PTR CALLBACK CertificatesDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        OnCertDialogInit(hwndDlg);
        break;
    case WM_NOTIFY:
        return CertificatesDialogOnNotify(hwndDlg, (UINT)wParam, (LPNMHDR)lParam);
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
            case IDC_ADD_TO_DEVICE:
                OnBnClickedAddToDevice(hwndDlg);
                break;
            case IDC_DELETE:
                OnBnClickedDelete(hwndDlg);
                break;
            default:
                return CertDefDialogProc(hwndDlg, uMsg, wParam, lParam);
        }
        break;
    default:
        return CertDefDialogProc(hwndDlg, uMsg, wParam, lParam);
    }

    return 0;
}

void OnCertificateCertificates(HWND hwndDlg)
{
    int nSelectedDeviceIDX = -1;

    nSelectedDeviceIDX = GetSelectedDevice(hwndDlg, g_DeviceCertData.m_szDevicePNPID, _countof(g_DeviceCertData.m_szDevicePNPID));
    if (nSelectedDeviceIDX < 0)
    {
        return;
    }

    DialogBox(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_CERTIFICATES), hwndDlg, CertificatesDialogProc);
}
