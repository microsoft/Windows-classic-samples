//---------------------------------------------------------------------
//  This file is part of the Microsoft .NET Framework SDK Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
// 
//This source code is intended only as a supplement to Microsoft
//Development Tools and/or on-line documentation.  See these other
//materials for detailed information regarding Microsoft code samples.
// 
//THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//---------------------------------------------------------------------


#include <stdio.h>
#include <certenroll.h>
#include <certcli.h>
#include <wincrypt.h>
#include "enrollCommon.h"



// Converts WCHAR * to CHAR *
// Returns true is succeed, false otherwise
BOOL
convertWszToSz(
    __deref_out PSTR *ppsz,
    __in_ecount(cwc) WCHAR const *pwc,
    __in LONG cwc)
{
    HRESULT hr;
    LONG cch = 0;

    *ppsz = NULL;
    for (;;)
    {
        cch = WideCharToMultiByte(
            GetACP(),
            0,          // dwFlags
            pwc,
            cwc,        // cchWideChar, -1 => null terminated
            *ppsz,
            cch,
            NULL,
            NULL);
        if (0 >= cch && 
            (0 != cch || (0 != cwc && (MAXLONG != cwc || L'\0' != *pwc))))
        {
            hr = GetLastError();
            _PrintError(hr, "WideCharToMultiByte");

            if (NULL != *ppsz)
            {
                LocalFree(*ppsz);
                *ppsz = NULL;
            }
            break;
        }
        if (NULL != *ppsz)
        {
            (*ppsz)[cch] = '\0';
            hr = S_OK;
            break;
        }
        *ppsz = (CHAR *) LocalAlloc(LMEM_FIXED, cch + 1);
        if (NULL == *ppsz)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    }
    if (S_OK != hr)
    {
        SetLastError(hr);
    }
    return(S_OK == hr);
}

// Converts CHAR * to WCHAR *
// Returns true is succeed, false otherwise
BOOL
convertSzToWsz(
    __deref_out PWSTR *ppwsz,
    __in_ecount(cch) CHAR const *pch,
    __in LONG cch)
{
    HRESULT hr;
    LONG cwc = 0;

    *ppwsz = NULL;
    for (;;)
    {
        cwc = MultiByteToWideChar(GetACP(), 0, pch, cch, *ppwsz, cwc);
        if (0 >= cwc)
        {
            hr = GetLastError();
            _PrintError(hr, "MultiByteToWideChar");

            if (NULL != *ppwsz)
            {
                LocalFree(*ppwsz);
                *ppwsz = NULL;
            }
            break;
        }
        if (NULL != *ppwsz)
        {
            (*ppwsz)[cwc] = L'\0';
            hr = S_OK;
            break;
        }
        *ppwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
        if (NULL == *ppwsz)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    }
    if (S_OK != hr)
    {
        SetLastError(hr);
    }
    return(S_OK == hr);
}

// Converts CHAR * to BSTR
// Returns true is succeed, false otherwise
BOOL
convertSzToBstr(
    __deref_out BSTR *pbstr,
    __in_ecount(cch) CHAR const *pch,
    __in LONG cch)
{
    HRESULT hr;
    BSTR bstr = NULL;
    LONG cwc = 0;

    if (-1 == cch)
    {
        cch = (LONG)strlen(pch);
    }

    for (;;)
    {
        cwc = MultiByteToWideChar(GetACP(), 0, pch, cch, bstr, cwc);
        if (0 >= cwc)
        {
            hr = GetLastError();
            _PrintError(hr, "MultiByteToWideChar");
            SysFreeString(bstr);
            break;
        }
        if (NULL != bstr)
        {
            bstr[cwc] = L'\0';
            SysFreeString(*pbstr);
            *pbstr = bstr;
            hr = S_OK;
            break;
        }
        bstr = SysAllocStringLen(NULL, cwc);
        if (NULL == bstr)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    }
    if (S_OK != hr)
    {
        SetLastError(hr);
    }
    return(S_OK == hr);
}

// Converts WCHAR * to BSTR
// Returns true is succeed, false otherwise
BOOL
convertWszToBstr(
    __deref_out BSTR *pbstr,
    __in_ecount(cb) WCHAR const *pwc,
    __in LONG cb)
{
    HRESULT hr;
    BSTR bstr;

    bstr = NULL;
    if (NULL != pwc)
    {
        if (-1 == cb)
        {
            cb = (LONG)(wcslen(pwc) * sizeof(WCHAR));
        }
        bstr = SysAllocStringByteLen((char const *) pwc, cb);
        if (NULL == bstr)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "SysAllocStringByteLen");
        }
    }
    SysFreeString(*pbstr);
    *pbstr = bstr;
    hr = S_OK;

error:
    if (S_OK != hr)
    {
        SetLastError(hr);
    }
    return(S_OK == hr);
}


// Check enrollment status
// Return S_OK if enrollment succeeds
HRESULT
checkEnrollStatus(
    __in IX509Enrollment* pEnroll)
{
    HRESULT hr = S_OK;
    HRESULT hEnrollError = S_OK;
    IX509EnrollmentStatus* pStatus = NULL;
    EnrollmentEnrollStatus EnrollStatus;
    BSTR strText = NULL;
    BSTR strErrorText = NULL;

    // Get IX509EnrollmentStatus
    hr = pEnroll->get_Status(&pStatus);
    _JumpIfError(hr, error, "get_Status"); 

    // Get enrollment status
    hr = pStatus->get_Status(&EnrollStatus);
    _JumpIfError(hr, error, "get_Status"); 

    // Get enrollment HRESULT
    hr = pStatus->get_Error(&hEnrollError);
    _JumpIfError(hr, error, "get_Error");

    // Get enrollment error text
    hr = pStatus->get_ErrorText(&strErrorText);
    _JumpIfError(hr, error, "get_ErrorText"); 

    // Get enrollment info text
    hr = pStatus->get_Text(&strText);
    _JumpIfError(hr, error, "get_Text"); 

    if (Enrolled != EnrollStatus) // Not enrolled
    {
        if (EnrollPended != EnrollStatus) // Enroll failed
        {
            wprintf(L"Request failed: %ws -- %ws\n", strErrorText, strText);
            hr = hEnrollError;
            _JumpError(hr, error, "EnrollError");
        }

        // Enroll pending
        wprintf(L"Request is pending: %ws -- %ws\n", strErrorText, strText);
        hr = E_FAIL; // Return failure even if the status is pending
    }
    else // Enrolled
        wprintf(L"Cert Issued: %ws -- %ws\n", strErrorText, strText);
error:
    SysFreeString(strText);
    SysFreeString(strErrorText);
    if (NULL != pStatus) pStatus->Release();
    return hr;
}

// Find a cert in user my store that has key usage containing usageFlags
// Return S_OK is succeeds
HRESULT
findCertByKeyUsage(
    __in BYTE usageFlags,
    __deref_out CERT_CONTEXT const **ppCert)
{
    HRESULT hr;
    HCERTSTORE hStore = NULL;
    CERT_CONTEXT const *pCert = NULL;
    BYTE KeyUsage;
    BOOL fMatch = FALSE;

    // Open user MY store
    hStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            X509_ASN_ENCODING,
            NULL,
            CERT_SYSTEM_STORE_CURRENT_USER,
            L"MY"  );
    
    if (NULL == hStore)
    {
        hr = GetLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

    for (;;)
    {
        // Enumerate cert in MY store
        pCert = CertEnumCertificatesInStore(hStore, pCert);
        if (NULL == pCert)
        {
            break;
        }

        // Get key usage
        if (!CertGetIntendedKeyUsage(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                pCert->pCertInfo,
                &KeyUsage,
                1))
        {
            break;
        }

        // Check if key usage matches       
        if ((KeyUsage & usageFlags) == usageFlags)
        {
            fMatch = TRUE;
            break;
        }
    }

    if (fMatch)
    {
        *ppCert = pCert;
        hr = S_OK;
    }
    else
        hr = CRYPT_E_NOT_FOUND;

error:
    if (NULL != hStore)
    {
        CertCloseStore(hStore, 0);
    }
    return hr;   

}

// Find a cert in user my store that has EKU containing pszObjId
// Return S_OK is succeeds
HRESULT
findCertByEKU(
    __in CHAR const *pszObjId,
    __deref_out CERT_CONTEXT const **ppCert)
{
    HRESULT hr;
    HCERTSTORE hStore = NULL;
    CERT_CONTEXT const *pCert = NULL;
    CERT_ENHKEY_USAGE *pKeyUsage = NULL;
    DWORD cbKeyUsage = 0;
    DWORD i;
    BOOL fMatch = FALSE;

    // Open user MY store
    hStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            X509_ASN_ENCODING,
            NULL,
            CERT_SYSTEM_STORE_CURRENT_USER,
            L"MY"  );
    
    if (NULL == hStore)
    {
        hr = GetLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

    for (;;)
    {
        // Enumerate cert in MY store
        pCert = CertEnumCertificatesInStore(hStore, pCert);
        if (NULL == pCert)
        {
            break;
        }

        for (;;)
        {
            // Get enhanced key usage OIDs
            if (!CertGetEnhancedKeyUsage(
                pCert,
                CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                pKeyUsage,
                &cbKeyUsage))
            {
                break;
            }
            if (NULL != pKeyUsage)
            {
                break;  // EKU extension fetched; break out of while loop
            }
            pKeyUsage = (CERT_ENHKEY_USAGE *)LocalAlloc
                    (LMEM_FIXED, cbKeyUsage);
            if (NULL == pKeyUsage)
            {
                hr = E_OUTOFMEMORY;
                _JumpError(hr, error, "out of memory");
            }
        }
        
        if (NULL != pKeyUsage)
        {
            if (0 == pKeyUsage->cUsageIdentifier)
            {
                hr = GetLastError();    // set by CertGetEnhancedKeyUsage
                if (S_OK != hr)
                {
                    fMatch = TRUE;
                }
            }
            else
            {
                for (i = 0; i < pKeyUsage->cUsageIdentifier; i++)
                {   
                    if (0 == strcmp(
                        pKeyUsage->rgpszUsageIdentifier[i],
                        pszObjId))
                    {
                        fMatch = TRUE;  // found matching EKU OID
                        break;
                    }
                }
            }
            LocalFree(pKeyUsage);
            pKeyUsage = NULL;
        }

        if (fMatch)
            break;
    }

    if (fMatch)
    {
        *ppCert = pCert;
        hr = S_OK;
    }
    else
        hr = CRYPT_E_NOT_FOUND;

error:
    if (NULL != pKeyUsage)
    {
        LocalFree(pKeyUsage);
    }
     if (NULL != hStore)
    {
        CertCloseStore(hStore, 0);
    }
    return hr;   

}

// Find a cert in user my store that has a string template
// name matches pwszNameTemplate or OID template name 
// matches pszObjIdTemplate
// Return S_OK is succeeds
HRESULT
findCertByTemplate(
    __in_opt PCWSTR pwszNameTemplate,
    __deref_out CERT_CONTEXT const **ppCert)
{
    HRESULT hr;
    HCERTSTORE hStore = NULL;
    CERT_CONTEXT const *pCert = NULL;
    BOOL fMatch = FALSE;
    CERT_EXTENSION *pExt;
    DWORD cExtension;
    CERT_EXTENSION const *rgExtension;
    WCHAR *pwszCertTypeNameV1 = NULL;
    WCHAR *pwszCertTypeObjId = NULL;
    CERT_TEMPLATE_EXT *pTemplate = NULL;
    CERT_NAME_VALUE *pName = NULL;
    PSTR pszObjIdTemplate = NULL;
    __bound DWORD cb;

    if (NULL == pwszNameTemplate) 
    {
        hr = CRYPT_E_NOT_FOUND;
        _JumpError(hr, error, "findOIDFromTemplateName");
    }

    hr = findOIDFromTemplateName(pwszNameTemplate, &pszObjIdTemplate);
    _JumpIfError(hr, error, "findOIDFromTemplateName");

    // Open user MY store
    hStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            X509_ASN_ENCODING,
            NULL,
            CERT_SYSTEM_STORE_CURRENT_USER,
            L"MY"  );
    
    if (NULL == hStore)
    {
        hr = GetLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

    for (;;)
    {
        // Enumerate cert in MY store
        pCert = CertEnumCertificatesInStore(hStore, pCert);
        if (NULL == pCert)
        {
            break;
        }
        
        cExtension = pCert->pCertInfo->cExtension;
        rgExtension = pCert->pCertInfo->rgExtension;

        // Look for the V1 cert type extension first

        if (NULL != pwszNameTemplate)
        {
            pExt = CertFindExtension(
                szOID_ENROLL_CERTTYPE_EXTENSION,
                cExtension,
                const_cast<CERT_EXTENSION *>(rgExtension));
            
            if (NULL != pExt)
            {
                if (!CryptDecodeObjectEx(
                        X509_ASN_ENCODING,
                        X509_UNICODE_ANY_STRING,
                        pExt->Value.pbData,
                        pExt->Value.cbData,
                        CRYPT_DECODE_ALLOC_FLAG,       // dwFlags
                        NULL,
                        (VOID **) &pName,
                        &cb))

                {
                    
                    hr = GetLastError();
                    _JumpError(hr, error, "CryptDecodeObjectEx");
                }
                
                cb = (pName->Value.cbData + 1) * sizeof(WCHAR);
                pwszCertTypeNameV1 = (WCHAR *) LocalAlloc(LMEM_FIXED, cb);
                if (NULL == pwszCertTypeNameV1)
                {
                    hr = E_OUTOFMEMORY;
                    _JumpError(hr, error, "LocalAlloc");
                }
                CopyMemory(pwszCertTypeNameV1, pName->Value.pbData, cb);
            
                if (0 == _wcsicmp(pwszNameTemplate, pwszCertTypeNameV1))
                {
                    fMatch = TRUE;
                    break;
                }
            }
        }

        if (NULL != pszObjIdTemplate && !fMatch)
        {
            pExt = CertFindExtension(
                szOID_CERTIFICATE_TEMPLATE,
                cExtension,
                const_cast<CERT_EXTENSION *>(rgExtension));
        
            if (NULL != pExt)
            {
                if (!CryptDecodeObjectEx(
                        X509_ASN_ENCODING,
                        X509_CERTIFICATE_TEMPLATE,
                        pExt->Value.pbData,
                        pExt->Value.cbData,
                        CRYPT_DECODE_ALLOC_FLAG,       // dwFlags
                        NULL,
                        (VOID **) &pTemplate,
                        &cb))

                {
                    
                    hr = GetLastError();
                    _JumpError(hr, error, "CryptDecodeObjectEx");
                }

                if (0 == strcmp(pszObjIdTemplate, pTemplate->pszObjId))
                {
                    fMatch = TRUE;
                    break;
                }
                if (!convertSzToWsz(
                        &pwszCertTypeObjId, 
                        pTemplate->pszObjId, 
                        -1))
                {
                    hr = E_OUTOFMEMORY;
                    _JumpError(hr, error, "convertSzToWsz");
                }
                if (0 == _wcsicmp(pwszNameTemplate, pwszCertTypeObjId))
                {
                    fMatch = TRUE;
                    break;
                }
            }
        }
        if (NULL != pwszCertTypeNameV1)
        {
            LocalFree(pwszCertTypeNameV1);
            pwszCertTypeNameV1 = NULL;

        }
        if (NULL != pwszCertTypeObjId)
        {
            LocalFree(pwszCertTypeObjId);
            pwszCertTypeObjId = NULL;

        }
        if (NULL != pTemplate)
        {
            LocalFree(pTemplate);
            pTemplate = NULL;
        }


    }

    if (fMatch)
    {
        *ppCert = pCert;
        hr = S_OK;
    }
    else
        hr = CRYPT_E_NOT_FOUND;

error:
    if (NULL != pwszCertTypeNameV1)
    {
        LocalFree(pwszCertTypeNameV1);
    }
    if (NULL != pwszCertTypeObjId)
    {
        LocalFree(pwszCertTypeObjId);
    }
    if (NULL != pTemplate)
    {
        LocalFree(pTemplate);
    }
    if (NULL != hStore)
    {
        CertCloseStore(hStore, 0);
    }
    return hr;
}

// Verify the certificate chain and EKU based on pCert
// Returns S_OK if verified
HRESULT
verifyCertContext(
    __in CERT_CONTEXT const *pCert,
    __in_opt PSTR pszEKU)
   
{
    HRESULT hr = S_OK;
    CERT_CHAIN_PARA ChainParams;
    CERT_CHAIN_POLICY_PARA ChainPolicy;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_CONTEXT const *pChainContext = NULL;

    ZeroMemory(&ChainParams, sizeof(ChainParams));
    ChainParams.cbSize = sizeof(ChainParams);

    if ( NULL != pszEKU)
    {
        ChainParams.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
        ChainParams.RequestedUsage.Usage.cUsageIdentifier = 1;
        ChainParams.RequestedUsage.Usage.rgpszUsageIdentifier = 
            const_cast<char **>(&pszEKU);
    }

    // Get the chain and verify the cert:
    if (!CertGetCertificateChain(
                            NULL,   // hChainEngine
                            pCert,      // pCertContext
                            NULL, // pTime
                            NULL,   // hAdditionalStore
                            &ChainParams,   // pChainPara
                            0,      // dwFlags
                            NULL,       // pvReserved
                            &pChainContext))    // ppChainContext
    {
        hr = GetLastError();
        _JumpError(hr, error, "CertGetCertificateChain");
    }
 
    ZeroMemory(&ChainPolicy, sizeof(ChainPolicy));
    ChainPolicy.cbSize = sizeof(ChainPolicy);
    ChainPolicy.dwFlags = CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG;

    ZeroMemory(&PolicyStatus, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);
    PolicyStatus.lChainIndex = -1;
    PolicyStatus.lElementIndex = -1;
    
    if (!CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_BASE,
            pChainContext,
            &ChainPolicy,
            &PolicyStatus))
    {
        hr = GetLastError();
        _JumpError(hr, error, "CertVerifyCertificateChainPolicy");
    }

    if (S_OK != PolicyStatus.dwError)
    {
        hr = PolicyStatus.dwError;
        _JumpError(hr, error, "PolicyStatus.dwError");
    }

error:
    if (NULL != pChainContext) CertFreeCertificateChain(pChainContext);
    return(hr);
}


// Enroll cert by template name
// Returns S_OK if succeeds
HRESULT
enrollCertByTemplate(
    __in PCWSTR pwszTemplateName)
{
    HRESULT hr = S_OK;
    IX509Enrollment* pEnroll = NULL; 
    BSTR strTemplateName = NULL;
    
    // Allocate BSTR for template name
    strTemplateName = SysAllocString(pwszTemplateName);
    if (NULL == strTemplateName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Create IX509Enrollment
    hr = CoCreateInstance(
            __uuidof(CX509Enrollment),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509Enrollment),
            (void **) &pEnroll);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initiate IX509Enrollment
    hr = pEnroll->InitializeFromTemplateName(
            ContextUser,        
            strTemplateName); 
    _JumpIfError(hr, error, "InitializeFromTemplateName");

    // Enroll
    hr = pEnroll->Enroll();
    _JumpIfError(hr, error, "Enroll"); 

    // Check enrollment status
    hr = checkEnrollStatus(pEnroll);
    _JumpIfError(hr, error, "checkEnrollStatus"); 

error:
    SysFreeString(strTemplateName);
    if (NULL != pEnroll) pEnroll->Release();
    return hr;
}

// Converts unicode string or ansi string to ansi string
// Returns S_OK is succeeds
HRESULT
decConvertFromUnicode(
    __deref_inout_ecount(*pcb) BYTE **ppb,
    __inout DWORD *pcb)
{
    HRESULT hr;

    if (2 * sizeof(WCHAR) <= *pcb && 0 == ((sizeof(WCHAR) - 1) & *pcb))
    {
        WCHAR *pwcIn = (WCHAR *) *ppb;
        DWORD cwcIn = *pcb;
        bool fUnicode = true;
    
        if (wcBOM == pwcIn[0] || wcBOMBIGENDIAN == pwcIn[0])
        {
            if (wcBOMBIGENDIAN == pwcIn[0])
            {
                _swab((char *) *ppb, (char *) *ppb, *pcb);
            }
            pwcIn++;
            cwcIn--;
        }
        else
        {
            for (DWORD i = 0; i < cwcIn; i++)
            {
                WCHAR wc = pwcIn[i];

                switch (wc)
                {
                    case L'\t':
                    case L'\r':
                    case L'\n':
                    continue;
                }
                if (L' ' > wc || L'~' < wc)
                {
                    fUnicode = false;
                    break;
                }
            }
        }
        if (fUnicode)
        {
            PSTR pszT;
        
            if (!convertWszToSz(&pszT, pwcIn, cwcIn))
            {
                hr = E_OUTOFMEMORY;
                _JumpError(hr, error, "convertWszToSz");
            }
            LocalFree(*ppb);
            *ppb = (BYTE *) pszT;
            *pcb = (DWORD)strlen(pszT);
        }
    }
    hr = S_OK;

error:
    return hr;
}

// Read request or certificate file
// Accepts binary, base64 or unicode base64 format
// Returns S_OK if succeeds
HRESULT
DecodeFileW(
    __in TCHAR const *pszfn,
    __out BYTE **ppbOut,
    __out DWORD *pcbOut,
    __in DWORD Flags)
{
    HRESULT hr;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BYTE *pbIn = NULL;
    BYTE *pbOut = NULL;
    DWORD cbIn;
    DWORD cbRead;
    DWORD cbOut;

    hFile = CreateFile(
            pszfn,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = GetLastError();
        _JumpError(hr, error, "CreateFile");
    }
    if (FILE_TYPE_DISK != GetFileType(hFile))
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        _JumpError(hr, error, "GetFileType");
    }

    cbIn = GetFileSize(hFile, NULL);
    if (INVALID_FILE_SIZE == cbIn || 0 == cbIn)
    {
        if (0 == cbIn) 
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        } 
        else 
        {
            hr = GetLastError();
        }
        _JumpError(hr, error, "GetFileSize");
    }

    pbIn = (BYTE *) LocalAlloc(LMEM_FIXED, cbIn);
    if (NULL == pbIn)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    if (!ReadFile(hFile, pbIn, cbIn, &cbRead, NULL))
    {
        hr = GetLastError();
        _JumpError(hr, error, "ReadFile");
    }

    if (cbRead != cbIn)
    {
        hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
        _JumpError(hr, error, "ReadFile(cbRead)");
    }

    if (CRYPT_STRING_BINARY == Flags)
    {
        pbOut = (BYTE *) pbIn;
        cbOut = cbIn;
        pbIn = NULL;
    }
    else
    {
        hr = decConvertFromUnicode(&pbIn, &cbIn);
        _JumpIfError(hr, error, "decConvertUnicode");

        // Decode file contents.

        if (!CryptStringToBinaryA(
                        (PCSTR) pbIn,
                        cbIn,
                        Flags,
                        pbOut,
                        &cbOut,
                        NULL,
                        NULL))
        {
            hr = GetLastError();
            _JumpError(hr, error, "CryptStringToBinaryA");
        }

        pbOut = (BYTE *) LocalAlloc(LMEM_FIXED, cbOut);
        if (NULL == pbOut)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        if (!CryptStringToBinaryA(
                        (PCSTR) pbIn,
                        cbIn,
                        Flags,
                        pbOut,
                        &cbOut,
                        NULL,
                        NULL))
        {
            hr = GetLastError();
            _JumpError(hr, error, "CryptStringToBinaryA");
        }

    }
    *pcbOut = cbOut;
    *ppbOut = pbOut;
    pbOut = NULL;
    hr = S_OK;

error:
    if (INVALID_HANDLE_VALUE != hFile) CloseHandle(hFile);
    if (NULL != pbIn) LocalFree(pbIn);
    if (S_OK != hr && NULL != pbOut) LocalFree(pbOut);
    return(hr);
}

// Save request or certificate to file
// Save to binary, base64 or unicode base64 format
// Returns S_OK if succeeds
HRESULT
EncodeToFileW(
    __in TCHAR const *pszfn,
    __in_bcount(cbIn) BYTE const *pbIn,
    __in DWORD cbIn,
    __in DWORD Flags)
{
    HRESULT hr;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD cbWritten;
    DWORD cbOut;
    CHAR *pchOut = NULL;
    DWORD cch;
    WCHAR *pwcOut = NULL;
    DWORD cwc;
    WCHAR *pwcT = NULL;
    BYTE const *pbOut;
    bool fForceOverWrite = 0 != (DECF_FORCEOVERWRITE & Flags);
    bool fUnicode = 0 != (DECF_WRITEUNICODE & Flags);

    Flags &= ~(DECF_FORCEOVERWRITE | DECF_WRITEUNICODE);

    if (CRYPT_STRING_BINARY == (CR_OUT_ENCODEMASK & Flags))
    {
        if (CRYPT_STRING_BINARY != Flags)
        {
            hr = E_INVALIDARG;
            _JumpError(hr, error, "Flags");
        }
        pbOut = pbIn;
        cbOut = cbIn;
    }
    else if (fUnicode)
    {
        if (!CryptBinaryToStringW(pbIn, cbIn, Flags, pwcOut, &cwc))
        {
            hr = GetLastError();
            _JumpError(hr, error, "CryptBinaryToStringW");
        }

        pwcOut = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
        if (NULL == pwcOut)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
        if (!CryptBinaryToStringW(pbIn, cbIn, Flags, pwcOut, &cwc))
        {
            hr = GetLastError();
            _JumpError(hr, error, "CryptBinaryToStringW");
        }
    
        // move the string (overwrite the terminating L'\0') to make room
        // for wcBOM.

        cwc = (DWORD)wcslen(pwcOut);


        for (pwcT = &pwcOut[cwc - 1]; pwcT >= pwcOut; pwcT--)
        {
            pwcT[1] = pwcT[0];
        }
        pwcT[1] = wcBOM;

        cbOut = (1 + cwc) * sizeof(WCHAR);
        pbOut = (BYTE const *) pwcOut;
    }
    else
    {
        if (!CryptBinaryToStringA(pbIn, cbIn, Flags, pchOut, &cch))
        {
            hr = GetLastError();
            _JumpError(hr, error, "CryptBinaryToStringA");
        }

        pchOut = (char *) LocalAlloc(LMEM_FIXED, cch * sizeof(char));
        if (NULL == pchOut)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        if (!CryptBinaryToStringA(pbIn, cbIn, Flags, pchOut, &cch))
        {
            hr = GetLastError();
            _JumpError(hr, error, "CryptBinaryToStringA");
        }

        cbOut = (DWORD)strlen(pchOut);
        pbOut = (BYTE const *) pchOut;
    }

    // Write encoded certificate to file

    hFile = CreateFile(
            pszfn,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_NEW,
            0,
            NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = GetLastError();
        if (fForceOverWrite && 
            HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) == HRESULT_FROM_WIN32(hr))
        {
            hFile = CreateFile(
                pszfn,
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                0,
                NULL);
        }
        if (INVALID_HANDLE_VALUE == hFile)
        {
            hr = GetLastError();
            _JumpError(hr, error, "CreateFile");
        }
    }
    if (FILE_TYPE_DISK != GetFileType(hFile))
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        _JumpError(hr, error, "GetFileType");
    }

    if (!WriteFile(hFile, pbOut, cbOut, &cbWritten, NULL))
    {
        hr = GetLastError();
        _JumpError(hr, error, "WriteFile");
    }
    if (cbWritten != cbOut)
    {
        hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
        _JumpError(hr, error, "WriteFile(cbWritten)");
    }
    hr = S_OK;

error:
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }
    if (NULL != pchOut)
    {
        LocalFree(pchOut);
    }
    if (NULL != pwcOut)
    {
        LocalFree(pwcOut);
    }

    return(hr);
}

// Find template oid from template name
// Returns S_OK if succeeds
HRESULT
findOIDFromTemplateName(
    __in PCWSTR pwszTemplateName,
    __deref_out PSTR *ppszTemplateOID)
{

    HRESULT hr = S_OK;
    IX509CertificateRequestPkcs10* pPkcs10 = NULL;
    IObjectId* pObjectId = NULL;
    BSTR strTemplateName = NULL;
    BSTR strTemplateOID = NULL;
    
    // Create IX509CertificateRequestPkcs10
    hr = CoCreateInstance(
            __uuidof(CX509CertificateRequestPkcs10),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509CertificateRequestPkcs10),
            (void **) &pPkcs10);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Allocate BSTR for template name
    strTemplateName = SysAllocString(pwszTemplateName);
    if (NULL == strTemplateName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Initialize IX509CertificateRequestPkcs10
    hr = pPkcs10->InitializeFromTemplateName(
            ContextUser,
            strTemplateName);
    _JumpIfError(hr, error, "InitializeFromTemplateName");

    // Initialize IX509CertificateRequestPkcs10
    hr = pPkcs10->get_TemplateObjectId(&pObjectId);
    _JumpIfError(hr, error, "get_TemplateObjectId");

    hr = pObjectId->get_Value(&strTemplateOID);
    _JumpIfError(hr, error, "get_Value");

    if (!convertWszToSz(ppszTemplateOID, (PCWSTR)strTemplateOID, -1))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "convertWszToSz");
    }

error:
    SysFreeString(strTemplateName);
    SysFreeString(strTemplateOID);
    if (NULL != pPkcs10) pPkcs10->Release();
    if (NULL != pObjectId) pObjectId->Release();
    return hr;

}