// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "PortableDeviceManagerImp.h"
#include "LocalCertStoreImp.h"

class CDeviceCertData
{
public:
    CDeviceCertData()
    {
        m_parCertificates = NULL;
        memset(m_szDevicePNPID, 0, sizeof(m_szDevicePNPID));
        m_nCertificatesCount = 0;
    }
    ~CDeviceCertData()
    {
        EmptyCertList();
    }

    void EmptyCertList()
    {
        if (m_parCertificates)
        {
            delete[] m_parCertificates;
            m_parCertificates = NULL;
        }
        m_nCertificatesCount = 0;
    }

    TCHAR m_szDevicePNPID[MAX_PATH];
    CCertProperties *m_parCertificates;
    ULONG m_nCertificatesCount;
};


void OnCertificateQueryinformation(HWND hwndDlg);
void OnCertificateHostauthentication(HWND hwndDlg);
void OnCertificateDeviceauthentication(HWND hwndDlg);
void OnCertificateUnauthentication(HWND hwndDlg);
void OnCertificateCertificates(HWND hwndDlg);
void OnCertificateInittomanufacturerstate(HWND hwndDlg);
int GetSelectedCertificate(HWND hwndDlg, CCertificate &certificate);
