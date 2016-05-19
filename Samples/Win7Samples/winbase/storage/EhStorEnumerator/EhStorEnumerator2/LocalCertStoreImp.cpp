// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "StdAfx.h"
#include "LocalCertStoreImp.h"

CLocalCertStoreImp::CLocalCertStoreImp(void)
{
    m_hCertStoreHandle = NULL;
}

CLocalCertStoreImp::~CLocalCertStoreImp(void)
{
    if (m_hCertStoreHandle)
    {
        CertCloseStore(m_hCertStoreHandle, 0);
        m_hCertStoreHandle = NULL;
    }
}

HRESULT CLocalCertStoreImp::OpenSystemStore(SYSTEM_STORE_NAMES nStore)
{
    LPTSTR szSubsystemProtocol;

    switch (nStore)
    {
    case SYSTEM_STORE_CA:
        szSubsystemProtocol = _T("CA");
        break;
    case SYSTEM_STORE_MY:
        szSubsystemProtocol = _T("MY");
        break;
    case SYSTEM_STORE_ROOT:
        szSubsystemProtocol = _T("ROOT");
        break;
    case SYSTEM_STORE_SPC:
        szSubsystemProtocol = _T("SPC");
        break;
    default:
        return FALSE;
    }

    m_hCertStoreHandle = ::CertOpenSystemStore(NULL, szSubsystemProtocol);

    return (m_hCertStoreHandle) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT CLocalCertStoreImp::GetDecodedCertName(PCERT_NAME_BLOB pCertName, DWORD dwStrType, LPTSTR szCertName, ULONG nBufferMax)
{
    DWORD dwDataLength = 0;
    PWCHAR pString = NULL;
    HRESULT hr = S_OK;

    if (pCertName == NULL)
    {
        return E_INVALIDARG;
    }

    // find the length first
    dwDataLength = ::CertNameToStr(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, pCertName, dwStrType, NULL, 0);
    if (dwDataLength == 0)
    {
        return E_FAIL;
    }

    pString = new WCHAR[dwDataLength];
    if (pString == NULL)
    {
        return E_OUTOFMEMORY;
    }

    CertNameToStr(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, pCertName, dwStrType, pString, dwDataLength);
    StringCbCopy(szCertName, nBufferMax, pString);

    // perform cleanup
    delete[] pString;
    pString = NULL;

    return hr;
}

HRESULT CLocalCertStoreImp::GetCertificatesList(CCertificate *&parCertificates, DWORD &nCertificatesCnt)
{
    HRESULT hr = S_OK;
    PCCERT_CONTEXT pCertContext = NULL;
    DWORD nCertIndex = 0;

    // cert store must be opened first...
    if (m_hCertStoreHandle == NULL)
    {
        return E_UNEXPECTED;
    }

    // pass 1, get certificates count
    nCertificatesCnt = 0; 
    while(pCertContext = ::CertEnumCertificatesInStore(m_hCertStoreHandle, pCertContext))
    {
        nCertificatesCnt ++;
    }

    pCertContext = NULL;
    parCertificates = new CCertificate[nCertificatesCnt];

    while(pCertContext = ::CertEnumCertificatesInStore(m_hCertStoreHandle, pCertContext))
    {
        TCHAR szStringBuf[256];
        CCertificate &certificate = parCertificates[nCertIndex++];

        hr = GetDecodedCertName(
                &pCertContext->pCertInfo->Issuer,
                CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG | CERT_NAME_STR_NO_QUOTING_FLAG,
                szStringBuf, _countof(szStringBuf));
        if (SUCCEEDED(hr)) {
            certificate.set_Issuer(szStringBuf);
        }

        hr = GetDecodedCertName(
                &pCertContext->pCertInfo->Subject,
                CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG | CERT_NAME_STR_NO_QUOTING_FLAG,
                szStringBuf, _countof(szStringBuf));
        if (SUCCEEDED(hr))
        {
            certificate.set_Subject(szStringBuf);
            // copy encoded certificate blob to byte array
            certificate.set_EncodedData(pCertContext->pbCertEncoded, pCertContext->cbCertEncoded);
            certificate.set_Version(pCertContext->pCertInfo->dwVersion);
        }
    }

    return hr;
}

HRESULT CLocalCertStoreImp::AddCertificate(CCertificate &certificate)
{
    HRESULT hr = S_OK;
    DWORD nCertEncodedSize = 0;

    // cert store must be opened first...
    if (m_hCertStoreHandle == NULL)
    {
        return E_UNEXPECTED;
    }

    if (CertAddEncodedCertificateToStore(
        m_hCertStoreHandle, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        certificate.get_EncodedData(nCertEncodedSize), nCertEncodedSize,
        CERT_STORE_ADD_NEW, NULL) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    
    return hr;
}

