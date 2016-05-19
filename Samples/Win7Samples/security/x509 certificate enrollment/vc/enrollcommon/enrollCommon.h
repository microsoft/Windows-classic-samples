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
#include <wincrypt.h>

#define wcBOM               (WCHAR) 0xfeff
#define wcBOMBIGENDIAN      (WCHAR) 0xfffe

#define DECF_FORCEOVERWRITE             0x00000100
#define DECF_WRITEUNICODE               0x00000200  

#define _JumpIfError(hr, label, pszMessage) \
    { \
    if (S_OK != (hr)) \
    { \
        wprintf(L"Error in %S: 0x%x\n", pszMessage, hr); \
        goto label; \
    } \
    }

#define _JumpError(hr, label, pszMessage) \
    { \
    wprintf(L"Error in %S: 0x%x\n", pszMessage, hr); \
    goto label; \
    }

#define _PrintIfError(hr, pszMessage) \
    { \
    if (S_OK != (hr)) \
    { \
        wprintf(L"Error in %S: 0x%x\n", pszMessage, hr); \
    } \
    }

#define _PrintError(hr, pszMessage) \
    { \
        wprintf(L"Error in %S: 0x%x\n", pszMessage, hr); \
    }


BOOL
convertWszToSz(
    __deref_out PSTR *ppsz,
    __in_ecount(cwc) WCHAR const *pwc,
    __in LONG cwc);

BOOL
convertSzToWsz(
    __deref_out PWSTR *ppwsz,
    __in_ecount(cch) CHAR const *pch,
    __in LONG cch);

BOOL
convertSzToBstr(
    __deref_out BSTR *pbstr,
    __in_ecount(cch) CHAR const *pch,
    __in LONG cch);

BOOL
convertWszToBstr(
    __deref_out BSTR *pbstr,
    __in_ecount(cb) WCHAR const *pwc,
    __in LONG cb);

HRESULT
checkEnrollStatus(
    __in IX509Enrollment* pEnroll);

HRESULT
findCertByKeyUsage(
    __in BYTE usageFlags,
    __deref_out CERT_CONTEXT const **ppCert);

HRESULT
findCertByEKU(
    __in CHAR const *pszObjId,
    __deref_out CERT_CONTEXT const **ppCert);

HRESULT
findCertByTemplate(
    __in_opt PCWSTR pwszNameTemplate,
    __deref_out CERT_CONTEXT const **ppCert);

HRESULT
enrollCertByTemplate(
    __in PCWSTR pwszTemplateName);

HRESULT
verifyCertContext(
    __in CERT_CONTEXT const *pCert,
    __in_opt PSTR pszEKU);
   
HRESULT
decConvertFromUnicode(
    __deref_inout_ecount(*pcb) BYTE **ppb,
    __inout DWORD *pcb);


HRESULT
DecodeFileW(
    __in TCHAR const *pszfn,
    __out BYTE **ppbOut,
    __out DWORD *pcbOut,
    __in DWORD Flags);


HRESULT
EncodeToFileW(
    __in TCHAR const *pszfn,
    __in_bcount(cbIn) BYTE const *pbIn,
    __in DWORD cbIn,
    __in DWORD Flags);


HRESULT
findOIDFromTemplateName(
    __in PCWSTR pwszTemplateName,
    __deref_out PSTR *ppszTemplateOID);