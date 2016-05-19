// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

typedef enum
{
    SYSTEM_STORE_CA,
    SYSTEM_STORE_MY,
    SYSTEM_STORE_ROOT,
    SYSTEM_STORE_SPC,
} SYSTEM_STORE_NAMES;

// implements enumerated data for certificate in store
class CCertificate
{
public:
    CCertificate()
    {
        dwVersion       = 0;
        pCertEncoded    = NULL;
        szSubject       = NULL;
        szIssuer        = NULL;
    }

    virtual ~CCertificate()
    {
        Clear();
    }

    CCertificate(const CCertificate &src)
    {
        dwVersion       = 0;
        pCertEncoded    = NULL;
        szSubject       = NULL;
        szIssuer        = NULL;

        operator =(src);
    }

    void Clear()
    {
        if (pCertEncoded)
        {
            delete[] pCertEncoded;
            pCertEncoded = NULL;
        }

        if (szSubject)
        {
            free(szSubject);
            szSubject = NULL;
        }

        if (szIssuer)
        {
            free(szIssuer);
            szIssuer = NULL;
        }
    }

    CCertificate &operator = (const CCertificate &src)
    {
        Clear();

        dwVersion   = src.dwVersion;
        if (src.szSubject) {
            szSubject  = _tcsdup(src.szSubject);
        }
        if (src.szIssuer) {
            szIssuer   = _tcsdup(src.szIssuer);
        }

        if (src.pCertEncoded && src.nEncodedLength)
        {
            nEncodedLength = src.nEncodedLength;
            pCertEncoded = new BYTE[nEncodedLength];
            RtlCopyMemory(pCertEncoded, src.pCertEncoded, nEncodedLength);
        }

        return *this;
    }

    CCertificate &operator = (const CCertificate *src)
    {
        if (src)
        {
            operator =(*src);
        }
        return *this;
    }

    LPCTSTR get_Subject() { return szSubject; }
    void set_Subject(LPCTSTR s)
    {
        if (szSubject) {
            free(szSubject);
        }
        szSubject = _tcsdup(s);
    }

    LPCTSTR get_Issuer() { return szIssuer; }
    void set_Issuer(LPCTSTR s)
    {
        if (szIssuer) {
            free(szIssuer);
        }
        szIssuer = _tcsdup(s);
    }
    
    DWORD get_Version() { return dwVersion; }
    void set_Version(DWORD v) { dwVersion = v; }

    void set_EncodedData(const PBYTE pData, DWORD nSize)
    {
        nEncodedLength = nSize;
        pCertEncoded = new BYTE[nEncodedLength];
        RtlCopyMemory(pCertEncoded, pData, nEncodedLength);
    }

    const PBYTE get_EncodedData(DWORD &nDataCnt)
    {
        nDataCnt = nEncodedLength;
        return pCertEncoded;
    }
protected:
    DWORD       dwVersion;      // The version number of a certificate
    LPTSTR      szSubject;      // Subject of the certificate
    LPTSTR      szIssuer;       // Certificate issuer
    PBYTE       pCertEncoded;   // Encoded certificate
    DWORD       nEncodedLength; // Encoded certificate length
};

class CLocalCertStoreImp
{
public:
    CLocalCertStoreImp(void);
    virtual ~CLocalCertStoreImp(void);

    HRESULT OpenSystemStore(SYSTEM_STORE_NAMES nStore);
    HRESULT GetCertificatesList(CCertificate *&parCertificates, DWORD &nCertificatesCnt);
    HRESULT AddCertificate(CCertificate &certificate);
private:
    // Decodes the name stored in CERT_NAME_BLOB type
    HRESULT GetDecodedCertName(PCERT_NAME_BLOB pCertName, DWORD dwStrType, LPTSTR szCertName, ULONG nBufferMax);
private:
    HCERTSTORE m_hCertStoreHandle;
};
