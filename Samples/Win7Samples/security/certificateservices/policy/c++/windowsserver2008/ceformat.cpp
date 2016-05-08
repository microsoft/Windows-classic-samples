//+-------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//  File:       ceformat.cpp
//
//  Contents:   helper functions
//
//--------------------------------------------------------------------------

#include "pch.cpp"
#pragma hdrstop

#include "celib.h"
#include <assert.h>
#include <wininet.h>



HRESULT
ceDupString(
    __in WCHAR const *pwszIn,
    __deref_out WCHAR **ppwszOut)
{
    size_t cb;
    HRESULT hr;

    cb = (wcslen(pwszIn) + 1) * sizeof(WCHAR);
    *ppwszOut = (WCHAR *) LocalAlloc(LMEM_FIXED, cb);
    if (NULL == *ppwszOut)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(*ppwszOut, pwszIn, cb);
    hr = S_OK;

error:
    return(hr);
}


#define cwcCNMAX 	64		// 64 chars max for CN
#define cwcCHOPHASHMAX	(1 + 5)		// "-%05hu" decimal USHORT hash digits
#define cwcCHOPBASE 	(cwcCNMAX - (cwcCHOPHASHMAX + cwcSUFFIXMAX))

HRESULT
ceSanitizedNameToDSName(
    __in WCHAR const *pwszSanitizedName,
    __deref_out WCHAR **ppwszNameOut)
{
    HRESULT hr;
    size_t cwc;
    size_t cwcCopy;
    WCHAR wszDSName[cwcCHOPBASE + cwcCHOPHASHMAX + 1];

    *ppwszNameOut = NULL;

    cwc = wcslen(pwszSanitizedName);
    cwcCopy = cwc;
    if (cwcCHOPBASE < cwcCopy)
    {
	cwcCopy = cwcCHOPBASE;
    }
    CopyMemory(wszDSName, pwszSanitizedName, cwcCopy * sizeof(WCHAR));
    wszDSName[cwcCopy] = L'\0';

    if (cwcCHOPBASE < cwc)
    {
        // Hash the rest of the name into a USHORT
        USHORT usHash = 0;
	size_t i;
	WCHAR *pwsz;

	// Truncate an incomplete sanitized Unicode character

	pwsz = wcsrchr(wszDSName, L'!');
	if (NULL != pwsz && wcslen(pwsz) < 5)
	{
	    cwcCopy -= wcslen(pwsz);
	    *pwsz = L'\0';
	}

        for (i = cwcCopy; i < cwc; i++)
        {
            USHORT usLowBit = (0x8000 & usHash)? 1 : 0;

	    usHash = ((usHash << 1) | usLowBit) + pwszSanitizedName[i];
        }
	hr = StringCchPrintf(&wszDSName[cwcCopy], sizeof(wszDSName)/sizeof(WCHAR) - cwcCopy, L"-%05hu", usHash);
	_JumpIfError(hr, error, "StringCchPrintf");
	assert(wcslen(wszDSName) < ARRAYSIZE(wszDSName));
    }

    hr = ceDupString(wszDSName, ppwszNameOut);
    _JumpIfError(hr, error, "ceDupString");

error:
    return(hr);
}


HRESULT
ceInternetCanonicalizeUrl(
    __in WCHAR const *pwszIn,
    __deref_out WCHAR **ppwszOut)
{
    HRESULT hr;
    WCHAR *pwsz = NULL;

    assert(NULL != pwszIn);

    if (0 == _wcsnicmp(L"file:", pwszIn, 5))
    {
	hr = ceDupString(pwszIn, &pwsz);
        _JumpIfError(hr, error, "ceDupString");
    }
    else
    {
	// Calculate required buffer size by passing a very small buffer
	// The call will fail, and tell us how big the buffer should be.

	WCHAR wszPlaceHolder[1];
	DWORD cwc = ARRAYSIZE(wszPlaceHolder);
	BOOL bResult;

	bResult = InternetCanonicalizeUrl(
				    pwszIn,		// lpszUrl
				    wszPlaceHolder,	// lpszBuffer
				    &cwc,		// lpdwBufferLength
				    0);		// dwFlags
	assert(!bResult);	// This will always fail

	hr = ceHLastError();
	if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
	{
	    // unexpected error

	    _JumpError(hr, error, "InternetCanonicalizeUrl");
	}

	// NOTE: InternetCanonicalizeUrl counts characters, not bytes as doc'd
	// cwc includes trailing L'0'

	pwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
	if (NULL == pwsz)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	// canonicalize
	if (!InternetCanonicalizeUrl(
				pwszIn,	// lpszUrl
				pwsz,	// lpszBuffer
				&cwc,	// lpdwBufferLength
				0))		// dwFlags
	{
	    hr = ceHLastError();
	    _JumpError(hr, error, "InternetCanonicalizeUrl");
	}
    }
    *ppwszOut = pwsz;
    pwsz = NULL;
    hr = S_OK;

error:
    if (NULL != pwsz)
    {
        LocalFree(pwsz);
    }
    return(hr);
}


// ceFormatCertsrvStringArray FormatMessage arguments:
//
// %1 -- Machine full DNS name: pwszServerName_p1_2;
//
// %2 -- Machine short name: first DNS component of pwszServerName_p1_2
//
// %3 -- Sanitized CA name: pwszSanitizedName_p3_7
//
// %4 -- Cert Filename Suffix:
//       if 0 == iCert_p4 && MAXDWORD == iCertTarget_p4: L""
//       else if MAXDWORD != iCertTarget_p4 L"(%u-%u)"
//       else L"(%u)"
//
// %5 -- DS DN path to Domain root: pwszDomainDN_p5
//
// %6 -- DS DN path to Configuration container: pwszConfigDN_p6
//
// %7 -- Sanitized CA name, truncated and hash suffix added if too long:
//	 pwszSanitizedName_p3_7
//
// %8 -- CRL Filename/Key Name Suffix: L"" if 0 == iCRL_p8; else L"(%u)"
//
// %9 -- CRL Filename Suffix: L"" if !fDeltaCRL_p9; else L"+"
//
// %10 -- DS CRL attribute: L"" if !fDSAttrib_p10_11; depends on fDeltaCRL_p9
//
// %11 -- DS CA Cert attribute: L"" if !fDSAttrib_p10_11
//
// %12 -- DS user cert attribute
//
// %13 -- DS KRA cert attribute
//
// %14 -- DS cross cert pair attribute

#ifndef wszDSSEARCHBASECRLATTRIBUTE
#define wszDSSEARCHBASECRLATTRIBUTE L"?certificateRevocationList?base?objectclass=cRLDistributionPoint"
#endif

#ifndef wszDSSEARCHDELTACRLATTRIBUTE
#define wszDSSEARCHDELTACRLATTRIBUTE L"?deltaRevocationList?base?objectclass=cRLDistributionPoint"
#endif

#ifndef wszDSSEARCHCACERTATTRIBUTE
#define wszDSSEARCHCACERTATTRIBUTE L"?cACertificate?base?objectclass=certificationAuthority"
#endif

#ifndef wszDSSEARCHUSERCERTATTRIBUTE
#define wszDSSEARCHUSERCERTATTRIBUTE L"?userCertificate?base?objectClass=*"
#endif

#ifndef wszDSSEARCHKRACERTATTRIBUTE
#define wszDSSEARCHKRACERTATTRIBUTE L"?userCertificate?one?objectClass=msPKI-PrivateKeyRecoveryAgent"
#endif

#ifndef wszDSSEARCHCROSSCERTPAIRATTRIBUTE
#define wszDSSEARCHCROSSCERTPAIRATTRIBUTE L"?crossCertificatePair?one?objectClass=certificationAuthority"
#endif


HRESULT
ceFormatCertsrvStringArray(
    IN BOOL    fURL,
    __in LPCWSTR pwszServerName_p1_2,
    __in LPCWSTR pwszSanitizedName_p3_7,
    IN DWORD   iCert_p4,
    IN DWORD   iCertTarget_p4,
    __in LPCWSTR pwszDomainDN_p5,
    __in LPCWSTR pwszConfigDN_p6,
    IN DWORD   iCRL_p8,
    IN BOOL    fDeltaCRL_p9,
    IN BOOL    fDSAttrib_p10_11,
    IN DWORD   cStrings,
    __in_ecount(cStrings) LPCWSTR *apwszStringsIn,
    __out LPWSTR *apwszStringsOut)
{
    HRESULT hr = S_OK;
    LPCWSTR apwszInsertionArray[100];  // 100 'cause this is the max number of insertion numbers allowed by FormatMessage
    LPWSTR    pwszCurrent = NULL;
    BSTR      strShortMachineName = NULL;
    DWORD     i;
    WCHAR *pwszSanitizedDSName = NULL;
    WCHAR wszCertSuffix[2 * cwcFILENAMESUFFIXMAX];
    WCHAR wszCRLSuffix[cwcFILENAMESUFFIXMAX];
    WCHAR wszDeltaCRLSuffix[cwcFILENAMESUFFIXMAX];
    WCHAR const *pwszT;


    ZeroMemory(apwszStringsOut, cStrings * sizeof(apwszStringsOut[0]));
    ZeroMemory(apwszInsertionArray, sizeof(apwszInsertionArray));

    // Format the template into a real name
    // Initialize the insertion string array.

    //+================================================
    // Machine DNS name (%1)

    assert(L'1' == wszFCSAPARM_SERVERDNSNAME[1]);
    apwszInsertionArray[1 - 1] = pwszServerName_p1_2;

    //+================================================
    // Short Machine Name (%2)

    assert(L'2' == wszFCSAPARM_SERVERSHORTNAME[1]);
    strShortMachineName = SysAllocString(pwszServerName_p1_2);
    if (strShortMachineName == NULL)
    {
        hr = E_OUTOFMEMORY;
        _JumpIfError(hr, error, "SysAllocString");
    }

    pwszCurrent = wcschr(strShortMachineName, L'.');
    if (NULL != pwszCurrent)
    {
        *pwszCurrent = 0;
    }
    apwszInsertionArray[2 - 1] = strShortMachineName;

    //+================================================
    // sanitized name (%3)

    assert(L'3' == wszFCSAPARM_SANITIZEDCANAME[1]);
    apwszInsertionArray[3 - 1] = pwszSanitizedName_p3_7;

    //+================================================
    // Cert filename suffix (%4) | (%4-%4)

    assert(L'4' == wszFCSAPARM_CERTFILENAMESUFFIX[1]);
    wszCertSuffix[0] = L'\0';
    if (0 != iCert_p4 || MAXDWORD != iCertTarget_p4)
    {
        StringCbPrintf(
	    wszCertSuffix,
	    sizeof(wszCertSuffix),
	    MAXDWORD != iCertTarget_p4? L"(%u-%u)" : L"(%u)",
	    iCert_p4,
	    iCertTarget_p4);
    }
    apwszInsertionArray[4 - 1] = wszCertSuffix;

    //+================================================
    // Domain DN (%5)

    if (NULL == pwszDomainDN_p5 || L'\0' == *pwszDomainDN_p5)
    {
	pwszDomainDN_p5 = L"DC=UnavailableDomainDN";
    }
    assert(L'5' == wszFCSAPARM_DOMAINDN[1]);
    apwszInsertionArray[5 - 1] = pwszDomainDN_p5;

    //+================================================
    // Config DN (%6)

    if (NULL == pwszConfigDN_p6 || L'\0' == *pwszConfigDN_p6)
    {
	pwszConfigDN_p6 = L"DC=UnavailableConfigDN";
    }
    assert(L'6' == wszFCSAPARM_CONFIGDN[1]);
    apwszInsertionArray[6 - 1] = pwszConfigDN_p6;

    // Don't pass pwszSanitizedName_p3_7 to SysAllocStringLen with the extended
    // length to avoid faulting past end of pwszSanitizedName_p3_7.

    //+================================================
    // Sanitized Short Name (%7)

    assert(L'7' == wszFCSAPARM_SANITIZEDCANAMEHASH[1]);
    hr = ceSanitizedNameToDSName(pwszSanitizedName_p3_7, &pwszSanitizedDSName);
    _JumpIfError(hr, error, "ceSanitizedNameToDSName");

    apwszInsertionArray[7 - 1] = pwszSanitizedDSName;

    //+================================================
    // CRL filename suffix (%8)

    assert(L'8' == wszFCSAPARM_CRLFILENAMESUFFIX[1]);
    wszCRLSuffix[0] = L'\0';
    if (0 != iCRL_p8)
    {
        StringCbPrintf(wszCRLSuffix, sizeof(wszCRLSuffix), L"(%u)", iCRL_p8);
    }
    apwszInsertionArray[8 - 1] = wszCRLSuffix;

    //+================================================
    // Delta CRL filename suffix (%9)

    assert(L'9' == wszFCSAPARM_CRLDELTAFILENAMESUFFIX[1]);
    wszDeltaCRLSuffix[0] = L'\0';
    if (fDeltaCRL_p9)
    {
        StringCbCopy(wszDeltaCRLSuffix, sizeof(wszDeltaCRLSuffix), L"+");
    }
    apwszInsertionArray[9 - 1] = wszDeltaCRLSuffix;

    //+================================================
    // CRL attribute (%10)

    assert(L'1' == wszFCSAPARM_DSCRLATTRIBUTE[1]);
    assert(L'0' == wszFCSAPARM_DSCRLATTRIBUTE[2]);
    pwszT = L"";
    if (fDSAttrib_p10_11)
    {
	pwszT = fDeltaCRL_p9?
		    wszDSSEARCHDELTACRLATTRIBUTE :
		    wszDSSEARCHBASECRLATTRIBUTE;
    }
    apwszInsertionArray[10 - 1] = pwszT;

    //+================================================
    // CA cert attribute (%11)

    assert(L'1' == wszFCSAPARM_DSCACERTATTRIBUTE[1]);
    assert(L'1' == wszFCSAPARM_DSCACERTATTRIBUTE[2]);
    pwszT = L"";
    if (fDSAttrib_p10_11)
    {
	pwszT = wszDSSEARCHCACERTATTRIBUTE;
    }
    apwszInsertionArray[11 - 1] = pwszT;

    //+================================================
    // User cert attribute (%12)

    assert(L'1' == wszFCSAPARM_DSUSERCERTATTRIBUTE[1]);
    assert(L'2' == wszFCSAPARM_DSUSERCERTATTRIBUTE[2]);
    pwszT = L"";
    if (fDSAttrib_p10_11)
    {
	pwszT = wszDSSEARCHUSERCERTATTRIBUTE;
    }
    apwszInsertionArray[12 - 1] = pwszT;

    //+================================================
    // KRA cert attribute (%13)

    assert(L'1' == wszFCSAPARM_DSKRACERTATTRIBUTE[1]);
    assert(L'3' == wszFCSAPARM_DSKRACERTATTRIBUTE[2]);
    pwszT = L"";
    if (fDSAttrib_p10_11)
    {
	pwszT = wszDSSEARCHKRACERTATTRIBUTE;
    }
    apwszInsertionArray[13 - 1] = pwszT;

    //+================================================
    // Cross cert pair attribute (%14)

    assert(L'1' == wszFCSAPARM_DSCROSSCERTPAIRATTRIBUTE[1]);
    assert(L'4' == wszFCSAPARM_DSCROSSCERTPAIRATTRIBUTE[2]);
    pwszT = L"";
    if (fDSAttrib_p10_11)
    {
	pwszT = wszDSSEARCHCROSSCERTPAIRATTRIBUTE;
    }
    apwszInsertionArray[14 - 1] = pwszT;

    //+================================================
    // Now format the strings...

    for (i = 0; i < cStrings; i++)
    {
        if (0 == FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			    FORMAT_MESSAGE_FROM_STRING |
			    FORMAT_MESSAGE_ARGUMENT_ARRAY,
			(VOID *) apwszStringsIn[i],
			0,              // dwMessageID
			0,              // dwLanguageID
			(LPWSTR) &apwszStringsOut[i],
			(DWORD)wcslen(apwszStringsIn[i]),
			(va_list *) apwszInsertionArray))
        {
            hr = ceHLastError();
	    _JumpError(hr, error, "FormatMessage");
        }
	if (fURL)
	{
	    WCHAR *pwsz;

	    hr = ceInternetCanonicalizeUrl(apwszStringsOut[i], &pwsz);
	    _JumpIfError(hr, error, "ceInternetCanonicalizeUrl");

	    LocalFree(apwszStringsOut[i]);
	    apwszStringsOut[i] = pwsz;
	}
    }

error:
    if (S_OK != hr)
    {
	for (i = 0; i < cStrings; i++)
	{
	    if (NULL != apwszStringsOut[i])
	    {
		LocalFree(apwszStringsOut[i]);
		apwszStringsOut[i] = NULL;
	    }
	}
    }
    SysFreeString(strShortMachineName);
    if (NULL != pwszSanitizedDSName)
    {
        LocalFree(pwszSanitizedDSName);
    }
    return (hr);
}
