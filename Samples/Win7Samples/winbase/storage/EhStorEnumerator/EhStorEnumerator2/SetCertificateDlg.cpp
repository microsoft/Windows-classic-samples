// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "StdAfx.h"
#include "SetCertificateDlg.h"
#include "Resource.h"
#include "PortableDeviceManagerImp.h"

CCertificate g_Certificate;     // incoming data
CCertProperties g_newCertProps; // outcoming data

// check the specified certificate index exists on device
BOOL CertificateExists(ULONG nIndex)
{
    ULONG n = 0;

    for (n = 0; n < g_DeviceCertData.m_nCertificatesCount; n ++)
    {
        if (g_DeviceCertData.m_parCertificates[n].nIndex == nIndex)
        {
            return TRUE;
        }
    }

    return FALSE;
}

void AddComboStringWithData(HWND hwndDlg, INT nID, LPCTSTR szData, DWORD_PTR nData)
{
    int nItemIndex = -1;
    HWND hControl = GetDlgItem(hwndDlg, nID);

    if (hControl)
    {
        nItemIndex = ComboBox_AddString(hControl, szData);
        if (nItemIndex >= 0)
        {
            ComboBox_SetItemData(hControl, nItemIndex, nData);
        }
    }
}

void OnSetCertDialogInit(HWND hwndDlg)
{
    CPortableDeviceImp device;
    HRESULT hr = S_OK;
    ULONG nStoredCertCount = 0;
    ULONG nMaxCertCount = 0;
    ULONG nCertIndex = 0;

    EXEC_CHECKHR(device.OpenDevice(g_DeviceCertData.m_szDevicePNPID));
    EXEC_CHECKHR(device.CertGetCertificatesCount(nStoredCertCount, nMaxCertCount));

    AddComboStringWithData(hwndDlg, IDC_CERT_SIGNER_INDEX, _T("None"), -1);

    if (SUCCEEDED(hr))
    {
        for (nCertIndex = 0; nCertIndex < nMaxCertCount; nCertIndex ++)
        {
            TCHAR szBuffer[20] = {0};

            StringCbPrintf(szBuffer, _countof(szBuffer), _T("%d"), nCertIndex);
            // if certificate with that index exists on device we may use it as signer
            // if not - we have available slot to add new
            if (CertificateExists(nCertIndex)) {
                AddComboStringWithData(hwndDlg, IDC_CERT_SIGNER_INDEX, szBuffer, nCertIndex);
            }
            else {
                AddComboStringWithData(hwndDlg, IDC_CERT_INDEX, szBuffer, nCertIndex);
            }
        }
    }

    AddComboStringWithData(hwndDlg, IDC_VALIDATION_POLICY, _T("None"), CERTVP_NONE);
    AddComboStringWithData(hwndDlg, IDC_VALIDATION_POLICY, _T("Basic"), CERTVP_BASIC);
    AddComboStringWithData(hwndDlg, IDC_VALIDATION_POLICY, _T("Extended"), CERTVP_EXTENDED);

    AddComboStringWithData(hwndDlg, IDC_CERT_TYPE, _T("Provisioning Certificate (PCp)"), CERTTYPE_PCP);
    AddComboStringWithData(hwndDlg, IDC_CERT_TYPE, _T("Auth. Silo Certificate (ASCh)"), CERTTYPE_ASCH);
    AddComboStringWithData(hwndDlg, IDC_CERT_TYPE, _T("Host Certificate (HCh)"), CERTTYPE_HCH);
    AddComboStringWithData(hwndDlg, IDC_CERT_TYPE, _T("Signer Certificate (SCh)"), CERTTYPE_SCH);

    SetDlgItemText(hwndDlg, IDC_DEVICE_ID, g_DeviceCertData.m_szDevicePNPID);
    SetDlgItemText(hwndDlg, IDC_CERT_SUBJECT, g_Certificate.get_Subject());
}

BOOL GetComboData(HWND hwndDlg, INT nID, DWORD &dwVal)
{
    HWND hControl = GetDlgItem(hwndDlg, nID);
    int nCurSet = -1;
    BOOL bResult = FALSE;

    if (hControl)
    {
        nCurSet = ComboBox_GetCurSel(hControl);
        if (nCurSet >= 0)
        {
            dwVal = (DWORD)ComboBox_GetItemData(hControl, nCurSet);
            bResult = TRUE;
        }
    }

    return bResult;
}

void OnSetCertDialogDestroy(HWND hwndDlg)
{
    DWORD dwData = 0;
    g_newCertProps = CCertProperties();

    if (GetComboData(hwndDlg, IDC_CERT_TYPE, dwData)) {
        g_newCertProps.nCertType = (CERTIFICATE_TYPES)dwData;
    }

    if (GetComboData(hwndDlg, IDC_VALIDATION_POLICY, dwData)) {
        g_newCertProps.nValidationPolicy = (CERTIFICATE_VALIDATION_POLICIES)dwData;
    }

    if (GetComboData(hwndDlg, IDC_CERT_SIGNER_INDEX, dwData)) {
        g_newCertProps.nSignerCertIndex = dwData;
    }

    if (GetComboData(hwndDlg, IDC_CERT_INDEX, dwData)) {
        g_newCertProps.nIndex = dwData;
    }
}

INT_PTR CALLBACK CertSetDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        OnSetCertDialogInit(hwndDlg);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
            case IDCANCEL:
            case IDOK:
                EndDialog(hwndDlg, LOWORD(wParam));
                return TRUE;
        }
        break;
    case WM_DESTROY:
        OnSetCertDialogDestroy(hwndDlg);
        break;
    }

    return 0;
}

// "Add to Device" button handler
void OnBnClickedAddToDevice(HWND hwndDlg)
{
    if (GetSelectedCertificate(hwndDlg, g_Certificate) >= 0)
    {
        if (DialogBox(GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDD_SET_CERTIFICATE), hwndDlg, CertSetDialogProc) == IDOK)
        {
            BOOL bProcessAdding = TRUE;
            DWORD nCertEncodedDataSize = 0;

            g_Certificate.get_EncodedData(nCertEncodedDataSize);
            g_newCertProps.set_CertificateData(g_Certificate.get_EncodedData(nCertEncodedDataSize), nCertEncodedDataSize);

            // add certificate...
            if ((g_newCertProps.nCertType == CERTTYPE_PCP) && (g_newCertProps.nIndex != 1))
            {
                bProcessAdding = (MessageBox(hwndDlg,
                    _T("Warning!\nPCp certificate should be placed to slot 1 only.\nAre you sure?"),
                    _T("Confirm..."),
                    MB_YESNO | MB_ICONWARNING) == IDYES);
            }

            if ((g_newCertProps.nCertType != CERTTYPE_PCP) && (g_newCertProps.nIndex == 1))
            {
                bProcessAdding = (MessageBox(hwndDlg,
                    _T("Warning!\nSlot 1 reserved for PCp certificate.\nAre you sure?"),
                    _T("Confirm..."),
                    MB_YESNO | MB_ICONWARNING) == IDYES);
            }
            
            if (bProcessAdding)
            {
                CPortableDeviceImp device;
                HRESULT hr = S_OK;
                TCHAR szMessageBuf[512];

                EXEC_CHECKHR(device.OpenDevice(g_DeviceCertData.m_szDevicePNPID));
                EXEC_CHECKHR(device.CertSetCertificate(g_newCertProps.nIndex, g_newCertProps));

                if (FAILED(hr))
                {
                    LPTSTR errBuf = NULL;

                    if (FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&errBuf, 0, NULL) == 0)
                    {
                        // failed, check GetLastError()...
                    }
                    if (errBuf)
                    {
                        StringCbPrintf(szMessageBuf, _countof(szMessageBuf),
                            _T("Certificate set error:\nhr = 0x%08X\n%s"), hr, errBuf);
                        LocalFree(errBuf);
                    }
                    else {
                        StringCbPrintf(szMessageBuf, _countof(szMessageBuf), _T("Certificate set error:\nhr = 0x%08X"), hr);
                    }
                }
                else
                {
                    StringCbPrintf(szMessageBuf, _countof(szMessageBuf), _T("Certificate has been set."));
                }

                MessageBox(hwndDlg,
                    szMessageBuf, _T("Result..."),
                    MB_OK | (FAILED(hr) ? MB_ICONERROR : MB_ICONINFORMATION));
            }
        }
    }
}
