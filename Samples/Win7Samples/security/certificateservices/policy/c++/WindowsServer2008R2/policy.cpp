//+--------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// File:        policy.cpp
//
// Contents:    Cert Server Policy Module implementation
//
//---------------------------------------------------------------------------

#include "pch.cpp"
#pragma hdrstop

#include <assert.h>
#include <winhttp.h>
#include "celib.h"
#pragma warning(push)
#pragma warning(disable : 4996) // to disable SDK warning from using deprecated APIs with ATL 7.0 and greater
#include "policy.h"
#include "module.h"
#pragma warning(pop)

BOOL fDebug = DBG_CERTSRV;

#ifndef DBG_CERTSRV
#error -- DBG_CERTSRV not defined!
#endif


// worker
HRESULT
polGetServerCallbackInterface(
    OUT ICertServerPolicy **ppServer,
    IN LONG Context)
{
    HRESULT hr;

    if (NULL == ppServer)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "Policy:polGetServerCallbackInterface");
    }

    hr = CoCreateInstance(
                    CLSID_CCertServerPolicy,
                    NULL,               // pUnkOuter
                    CLSCTX_INPROC_SERVER,
                    IID_ICertServerPolicy,
                    (VOID **) ppServer);
    _JumpIfError(hr, error, "Policy:CoCreateInstance");

    if (NULL == *ppServer)
    {
        hr = E_UNEXPECTED;
	_JumpError(hr, error, "Policy:CoCreateInstance");
    }

    // only set context if nonzero
    if (0 != Context)
    {
        hr = (*ppServer)->SetContext(Context);
        _JumpIfError(hr, error, "Policy:SetContext");
    }

error:
    return hr;
}


HRESULT
polGetProperty(
    IN ICertServerPolicy *pServer,
    IN BOOL fRequest,
    IN WCHAR const *pwszPropertyName,
    IN DWORD PropType,
    OUT VARIANT *pvarOut)
{
    HRESULT hr;
    BSTR strName = NULL;

    VariantInit(pvarOut);
    strName = SysAllocString(pwszPropertyName);
    if (IsNullBStr(strName))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    if (fRequest)
    {
	hr = pServer->GetRequestProperty(strName, PropType, pvarOut);
	_JumpIfErrorStr2(
		    hr,
		    error,
		    "Policy:GetRequestProperty",
		    pwszPropertyName,
		    CERTSRV_E_PROPERTY_EMPTY);
    }
    else
    {
	hr = pServer->GetCertificateProperty(strName, PropType, pvarOut);
	_JumpIfErrorStr2(
		    hr,
		    error,
		    "Policy:GetCertificateProperty",
		    pwszPropertyName,
		    CERTSRV_E_PROPERTY_EMPTY);
    }

error:
    SysFreeString(strName);
    return(hr);
}


HRESULT
polGetStringProperty(
    IN ICertServerPolicy *pServer,
    IN BOOL fRequest,
    IN WCHAR const *pwszPropertyName,
    OUT BSTR *pstrOut)
{
    HRESULT hr;
    VARIANT var;

    VariantInit(&var);
    SysFreeString(*pstrOut);
    *pstrOut = NULL;

    hr = polGetProperty(
		    pServer,
		    fRequest,
		    pwszPropertyName,
		    PROPTYPE_STRING,
		    &var);
    _JumpIfError2(
	    hr,
	    error,
	    "Policy:polGetProperty",
	    CERTSRV_E_PROPERTY_EMPTY);

    if (VT_BSTR != var.vt || IsNullOrEmptyBStr(var.bstrVal))
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpError(hr, error, "Policy:polGetProperty");
    }
    *pstrOut = var.bstrVal;
    var.vt = VT_EMPTY;
    hr = S_OK;

error:
    VariantClear(&var);
    return(hr);
}


HRESULT
polGetLongProperty(
    IN ICertServerPolicy *pServer,
    IN BOOL fRequest,
    IN WCHAR const *pwszPropertyName,
    OUT LONG *plOut)
{
    HRESULT hr;
    VARIANT var;

    VariantInit(&var);
    hr = polGetProperty(
		    pServer,
		    fRequest,
		    pwszPropertyName,
		    PROPTYPE_LONG,
		    &var);
    _JumpIfError2(hr, error, "Policy:polGetProperty", CERTSRV_E_PROPERTY_EMPTY);

    if (VT_I4 != var.vt)
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpError(hr, error, "Policy:polGetProperty");
    }
    *plOut = var.lVal;
    hr = S_OK;

error:
    VariantClear(&var);
    return(hr);
}


HRESULT
polGetRequestStringProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT BSTR *pstrOut)
{
    HRESULT hr;

    hr = polGetStringProperty(pServer, TRUE, pwszPropertyName, pstrOut);
    _JumpIfError2(hr, error, "polGetStringProperty", CERTSRV_E_PROPERTY_EMPTY);

error:
    return(hr);
}


HRESULT
polGetCertificateStringProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT BSTR *pstrOut)
{
    HRESULT hr;

    hr = polGetStringProperty(pServer, FALSE, pwszPropertyName, pstrOut);
    _JumpIfError2(hr, error, "polGetStringProperty", CERTSRV_E_PROPERTY_EMPTY);

error:
    return(hr);
}


HRESULT
polGetRequestLongProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT LONG *plOut)
{
    HRESULT hr;

    hr = polGetLongProperty(pServer, TRUE, pwszPropertyName, plOut);
    _JumpIfError2(hr, error, "polGetLongProperty", CERTSRV_E_PROPERTY_EMPTY);

error:
    return(hr);
}


HRESULT
polGetCertificateLongProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT LONG *plOut)
{
    HRESULT hr;

    hr = polGetLongProperty(pServer, FALSE, pwszPropertyName, plOut);
    _JumpIfError2(hr, error, "polGetLongProperty", CERTSRV_E_PROPERTY_EMPTY);

error:
    return(hr);
}


HRESULT
polGetRequestAttribute(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszAttributeName,
    OUT BSTR *pstrOut)
{
    HRESULT hr;
    BSTR strName = NULL;

    strName = SysAllocString(pwszAttributeName);
    if (IsNullBStr(strName))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    hr = pServer->GetRequestAttribute(strName, pstrOut);
    _JumpIfErrorStr2(
		hr,
		error,
		"Policy:GetRequestAttribute",
		pwszAttributeName,
		CERTSRV_E_PROPERTY_EMPTY);

error:
    SysFreeString(strName);
    return(hr);
}


HRESULT
polGetCertificateExtension(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszExtensionName,
    IN DWORD dwPropType,
    IN OUT VARIANT *pvarOut)
{
    HRESULT hr;
    BSTR strName = NULL;

    strName = SysAllocString(pwszExtensionName);
    if (IsNullBStr(strName))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    hr = pServer->GetCertificateExtension(strName, dwPropType, pvarOut);
    _JumpIfErrorStr2(
		hr,
		error,
		"Policy:GetCertificateExtension",
		pwszExtensionName,
		CERTSRV_E_PROPERTY_EMPTY);

error:
    SysFreeString(strName);
    return(hr);
}


HRESULT
polSetCertificateExtension(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszExtensionName,
    IN DWORD dwPropType,
    IN DWORD dwExtFlags,
    IN VARIANT const *pvarIn)
{
    HRESULT hr;
    BSTR strName = NULL;

    strName = SysAllocString(pwszExtensionName);
    if (IsNullBStr(strName))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    hr = pServer->SetCertificateExtension(
				    strName,
				    dwPropType,
				    dwExtFlags,
				    pvarIn);
    _JumpIfErrorStr(
		hr,
		error,
		"Policy:SetCertificateExtension",
		pwszExtensionName);

error:
    SysFreeString(strName);
    return(hr);
}


HRESULT
polSetCertificateStringProperty(
    __in ICertServerPolicy *pServer,
    __in LPCWSTR pwszPropName,
    __in_opt BSTR strPropertyValue)

{
    HRESULT hr;
    BSTR strName = NULL;
    VARIANT varValue;

    strName = SysAllocString(pwszPropName);
    if (IsNullBStr(strName))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    V_VT(&varValue) = VT_BSTR;
    V_BSTR(&varValue) = strPropertyValue;
    hr = pServer->SetCertificateProperty(
				    strName,
				    PROPTYPE_STRING,
                                    &varValue);

    _JumpIfErrorStr(
		hr,
		error,
		"Policy:SetCertificateProperty",
		strName);

error:
    SysFreeString(strName);
    return(hr);
}


HRESULT
polSetCertificateLongProperty(
    __in ICertServerPolicy *pServer,
    __in LPCWSTR pwszPropName,
    __in LONG lVal)

{
    HRESULT hr;
    BSTR strName = NULL;
    VARIANT varValue;

    strName = SysAllocString(pwszPropName);
    if (IsNullBStr(strName))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    V_VT(&varValue) = VT_I4;
    V_I4(&varValue) = lVal;
    hr = pServer->SetCertificateProperty(
				    strName,
				    PROPTYPE_LONG,
                                    &varValue);

    _JumpIfErrorStr(
		hr,
		error,
		"Policy:SetCertificateProperty",
		strName);

error:
    SysFreeString(strName);
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertPolicySample::~CCertPolicySample -- destructor
//
// free memory associated with this instance
//+--------------------------------------------------------------------------

CCertPolicySample::~CCertPolicySample()
{
    _Cleanup();

}


VOID
CCertPolicySample::_FreeStringArray(
    __inout DWORD *pcString,
    __inout LPWSTR **papwsz)
{
    LPWSTR *apwsz = *papwsz;
    DWORD i;

    if (NULL != apwsz)
    {
        for (i = *pcString; i-- > 0; )
        {
            if (NULL != apwsz[i])
            {
                DBGPRINT((fDebug, "_FreeStringArray[%u]: '%ws'\n", i, apwsz[i]));
                LocalFree(apwsz[i]);
            }
        }
        LocalFree(apwsz);
        *papwsz = NULL;
    }
    *pcString = 0;
}

VOID
CCertPolicySample::_FreeStringArray(
    __inout DWORD *pcString,
    __inout LPSTR **papsz)
{
    LPSTR *apsz = *papsz;
    DWORD i;

    if (NULL != apsz)
    {
        for (i = *pcString; i-- > 0; )
        {
            if (NULL != apsz[i])
            {
                DBGPRINT((fDebug, "_FreeStringArray[%u]: '%s'\n", i, apsz[i]));
                LocalFree(apsz[i]);
            }
        }
        LocalFree(apsz);
        *papsz = NULL;
    }
    *pcString = 0;
}




//+--------------------------------------------------------------------------
// CCertPolicySample::_Cleanup -- free memory associated with this instance
//
// free memory associated with this instance
//+--------------------------------------------------------------------------

VOID
CCertPolicySample::_Cleanup()
{

    SysFreeString(m_strDescription);
    m_strDescription = NULL;

    // RevocationExtension variables:

    if (NULL != m_wszASPRevocationURL)
    {
        LocalFree(m_wszASPRevocationURL);
    	m_wszASPRevocationURL = NULL;
    }


    _FreeStringArray(&m_cEnableRequestExtensions, &m_apwszEnableRequestExtensions);
    _FreeStringArray(&m_cEnableEnrolleeRequestExtensions, &m_apwszEnableEnrolleeRequestExtensions);
    _FreeStringArray(&m_cDisableExtensions, &m_apwszDisableExtensions);
    _FreeStringArray(&m_cEKUOIDsForVolatileRequests, &m_apszEKUOIDsForVolatileRequests);


    SysFreeString(m_strCAName);
    m_strCAName = NULL;

    SysFreeString(m_strCASanitizedName);
    m_strCASanitizedName = NULL;

    SysFreeString(m_strCASanitizedDSName);
    m_strCASanitizedDSName = NULL;

    SysFreeString(m_strRegStorageLoc);
    m_strRegStorageLoc = NULL;
    if (NULL != m_pCert)
    {
        CertFreeCertificateContext(m_pCert);
        m_pCert = NULL;
    }
    SysFreeString(m_strMachineDNSName);
    m_strMachineDNSName=NULL;

}


HRESULT
CCertPolicySample::_ReadRegistryString(
    IN HKEY hkey,
    IN BOOL fURL,
    __in WCHAR const *pwszRegName,
    __in WCHAR const *pwszSuffix,
    __deref_out LPWSTR *ppwszOut)
{
    HRESULT hr;
    WCHAR *pwszRegValue = NULL;
    DWORD cbValue;
    DWORD dwType;
    size_t cblen;

    *ppwszOut = NULL;
    hr = RegQueryValueEx(
		    hkey,
		    pwszRegName,
		    NULL,           // lpdwReserved
		    &dwType,
		    NULL,
		    &cbValue);
    _JumpIfErrorStr2(
		hr,
		error,
		"Policy:RegQueryValueEx",
		pwszRegName,
		ERROR_FILE_NOT_FOUND);

    if (REG_SZ != dwType && REG_MULTI_SZ != dwType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        _JumpErrorStr(hr, error, "Policy:RegQueryValueEx TYPE", pwszRegName);
    }
    if (NULL != pwszSuffix)
    {
	size_t cbValueT = wcslen(pwszSuffix) * sizeof(WCHAR);
	if (CENCODEMAX < cbValueT)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
	    _JumpIfError(hr, error, "pwszSuffix");
	}
	cbValue += (DWORD) cbValueT;
    }
    pwszRegValue = (WCHAR *) LocalAlloc(LMEM_FIXED, cbValue + sizeof(WCHAR));
    if (NULL == pwszRegValue)
    {
        hr = E_OUTOFMEMORY;
        _JumpErrorStr(hr, error, "Policy:LocalAlloc", pwszRegName);
    }
    cblen = cbValue + sizeof(WCHAR);

    hr = RegQueryValueEx(
		    hkey,
		    pwszRegName,
		    NULL,           // lpdwReserved
		    &dwType,
		    (BYTE *) pwszRegValue,
		    &cbValue);
    _JumpIfErrorStr(hr, error, "Policy:RegQueryValueEx", pwszRegName);

    // Handle malformed registry values cleanly:

    pwszRegValue[cbValue / sizeof(WCHAR)] = L'\0';
    if (NULL != pwszSuffix)
    {
	StringCbCat(pwszRegValue, cblen, pwszSuffix);
    }

    hr = ceFormatCertsrvStringArray(
			fURL,			// fURL
			m_strMachineDNSName, 	// pwszServerName_p1_2
			m_strCASanitizedName,	// pwszSanitizedName_p3_7
			m_iCert,		// iCert_p4
			MAXDWORD,		// iCertTarget_p4
			L"",		// pwszDomainDN_p5
			L"",		// pwszConfigDN_p6
			m_iCRL,			// iCRL_p8
			FALSE,			// fDeltaCRL_p9
			TRUE,			// fDSAttrib_p10_11
			1,			// cStrings
			(LPCWSTR *) &pwszRegValue, // apwszStringsIn
			ppwszOut);		// apwszStringsOut
    _JumpIfError(hr, error, "Policy:ceFormatCertsrvStringArray");

error:
    if (NULL != pwszRegValue)
    {
        LocalFree(pwszRegValue);
    }
    return(ceHError(hr));	// Reg routines return Win32 error codes
}


#if DBG_CERTSRV

VOID
CCertPolicySample::_DumpStringArray(
    __in PCSTR pszType,
    __in DWORD count,
    __in_ecount(count) LPWSTR const *apwsz)
{
    DWORD i;
    WCHAR const *pwszName;

    for (i = 0; i < count; i++)
    {
	pwszName = L"";
	if (iswdigit(apwsz[i][0]))
	{
	    pwszName = ceGetOIDName(
                                     apwsz[i]); // Static: do not free!
	}
	DBGPRINT((
		fDebug,
		"%hs[%u]: %ws%hs%ws\n",
		pszType,
		i,
		apwsz[i],
		L'\0' != *pwszName? " -- " : "",
		pwszName));
    }
}
#endif // DBG_CERTSRV


HRESULT
CCertPolicySample::_SetSystemStringProp(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszName,
    OPTIONAL IN WCHAR const *pwszValue)
{
    HRESULT hr;
    BSTR strName = NULL;
    VARIANT varValue;

    varValue.vt = VT_NULL;
    varValue.bstrVal = NULL;

    if (!ceConvertWszToBstr(&strName, pwszName, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:ceConvertWszToBstr");
    }

    if (NULL != pwszValue)
    {
        if (!ceConvertWszToBstr(&varValue.bstrVal, pwszValue, -1))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:ceConvertWszToBstr");
	}
	varValue.vt = VT_BSTR;
    }

    hr = pServer->SetCertificateProperty(strName, PROPTYPE_STRING, &varValue);
    _JumpIfError(hr, error, "Policy:SetCertificateProperty");

error:
    VariantClear(&varValue);
    SysFreeString(strName);
    return(hr);
}


HRESULT
CCertPolicySample::_AddStringArray(
    __in CSPCZZWSTR pwszzValue,
    IN BOOL fURL,
    __inout DWORD *pcStrings,
    __deref_inout_ecount(*pcStrings) LPWSTR **papwszRegValues)
{
    HRESULT hr;
    DWORD cString = 0;
    WCHAR const *pwsz;
    LPCWSTR *awszFormatStrings = NULL;
    LPWSTR *awszOutputStrings = NULL;

    // Count the number of strings we're adding
    for (pwsz = pwszzValue; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
        cString++;
    }
    if (0 == cString)		// no strings
    {
	hr = S_OK;
        goto error;
    }
    awszFormatStrings = (LPCWSTR *) LocalAlloc(
			    LMEM_FIXED | LMEM_ZEROINIT,
			    cString * sizeof(LPWSTR));
    if (NULL == awszFormatStrings)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:LocalAlloc");
    }

    cString = 0;
    for (pwsz = pwszzValue; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
        // Skip strings that start with a an unescaped minus sign.
        // Strings with an escaped minus sign (2 minus signs) are not skipped.

        if (L'-' == *pwsz)
        {
	    pwsz++;
	    if (L'-' != *pwsz)
	    {
                continue;
	    }
        }
        awszFormatStrings[cString++] = pwsz;
    }

    // if no strings to add, don't modify
    if (cString > 0)
    {
        awszOutputStrings = (LPWSTR *) LocalAlloc(
			        LMEM_FIXED | LMEM_ZEROINIT,
			        (cString + *pcStrings) * sizeof(LPWSTR));
        if (NULL == awszOutputStrings)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "Policy:LocalAlloc");
        }

        if (0 != *pcStrings)
        {
            assert(NULL != *papwszRegValues);
            CopyMemory(
		awszOutputStrings,
		*papwszRegValues,
		*pcStrings * sizeof(LPWSTR));
        }

        hr = ceFormatCertsrvStringArray(
		fURL,				// fURL
		m_strMachineDNSName,		// pwszServerName_p1_2
		m_strCASanitizedName,		// pwszSanitizedName_p3_7
		m_iCert,			// iCert_p4
		MAXDWORD,			// iCertTarget_p4
		L"",			// pwszDomainDN_p5
		L"",			// pwszConfigDN_p6
		m_iCRL,				// iCRL_p8
		FALSE,				// fDeltaCRL_p9
		TRUE,				// fDSAttrib_p10_11
		cString,			// cStrings
		awszFormatStrings,		// apwszStringsIn
		awszOutputStrings + (*pcStrings)); // apwszStringsOut
	_JumpIfError(hr, error, "Policy:ceFormatCertsrvStringArray");

        *pcStrings = (*pcStrings) + cString;
        if (*papwszRegValues)
        {
            LocalFree(*papwszRegValues);
        }
        *papwszRegValues = awszOutputStrings;
        awszOutputStrings = NULL;
    }
    hr = S_OK;

error:
    if (NULL != awszOutputStrings)
    {
        LocalFree(awszOutputStrings);
    }
    if (NULL != awszFormatStrings)
    {
        LocalFree(awszFormatStrings);
    }
    return(hr);
}

HRESULT
CCertPolicySample::_ReadRegistryMultiString(
    __in HKEY hkey,
    __in LPCWSTR pwszRegName,
    __deref_out PZPWSTR  ppwszzRegValue)
{
    HRESULT hr;
    WCHAR *pwszzValue = NULL;
    DWORD cbValue;
    DWORD dwType;


    hr = RegQueryValueEx(
	        hkey,
	        pwszRegName,
	        NULL,           // lpdwReserved
	        &dwType,
	        NULL,
	        &cbValue);
    hr = HRESULT_FROM_WIN32(hr);
    _JumpIfError(hr, error, "Policy:RegQueryValueEx");

    if (REG_SZ != dwType && REG_MULTI_SZ != dwType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        _JumpError(hr, error, "Policy:RegQueryValueEx TYPE");
    }

     // Handle malformed registry values cleanly by adding two WCHAR L'\0's
     // allocate space for 3 WCHARs to allow for unaligned (odd) cbValue;

    pwszzValue = (WCHAR *) LocalAlloc(
			        LMEM_FIXED,
			        cbValue + 3 * sizeof(WCHAR));
    if (NULL == pwszzValue)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:LocalAlloc");
    }
    hr = RegQueryValueEx(
	        hkey,
	        pwszRegName,
	        NULL,           // lpdwReserved
	        &dwType,
	        (BYTE *) pwszzValue,
	        &cbValue);
    hr = HRESULT_FROM_WIN32(hr);
    _JumpIfError(hr, error, "Policy:RegQueryValueEx");

    // Handle malformed registry values cleanly:

    pwszzValue[cbValue / sizeof(WCHAR)] = L'\0';
    pwszzValue[cbValue / sizeof(WCHAR) + 1] = L'\0';

    *ppwszzRegValue = pwszzValue;
    pwszzValue = NULL;

    hr = S_OK;

error:
    if (NULL != pwszzValue)
    {
        LocalFree(pwszzValue);
    }
    return(hr);
}


HRESULT
CCertPolicySample::_ReadRegistryStringArray(
    IN HKEY hkey,
    IN BOOL fURL,
    IN DWORD dwFlags,
    IN DWORD cRegNames,
    __in DWORD *aFlags,
    __in_ecount(cRegNames) LPCWSTR const *apwszRegNames,
    __inout DWORD *pcStrings,
    __deref_inout_ecount(*pcStrings) LPWSTR **papwszRegValues)
{
    HRESULT hr;
    DWORD i;
    WCHAR *pwszzValue = NULL;

    for (i = 0; i < cRegNames; i++)
    {
        if (0 == (dwFlags & aFlags[i]))
        {
	    continue;
        }
        if (NULL != pwszzValue)
        {
	    LocalFree(pwszzValue);
	    pwszzValue = NULL;
        }
 
        hr = _ReadRegistryMultiString(
                                                       hkey,
                                                       apwszRegNames[i],
                                                       &pwszzValue);
        if (S_OK != hr)
        {

            _PrintErrorStr2(
                                       hr,
                                       "_ReadRegistryMultiStringValue",
                                       apwszRegNames[i],
                                       HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
            continue;
        }

        hr = _AddStringArray(
			pwszzValue,
			fURL,
			pcStrings,
			papwszRegValues);
        _JumpIfErrorStr(hr, error, "_AddStringArray", apwszRegNames[i]);
    }
    hr = S_OK;

error:
    if (NULL != pwszzValue)
    {
        LocalFree(pwszzValue);
    }
    return(hr);
}




//+--------------------------------------------------------------------------
// CCertPolicySample::_InitRevocationExtension
//
//+--------------------------------------------------------------------------

VOID
CCertPolicySample::_InitRevocationExtension(
    IN HKEY hkey)
{
    HRESULT hr;
    DWORD dwType;
    DWORD cb;

    cb = sizeof(m_dwRevocationFlags);
    hr = RegQueryValueEx(
                hkey,
                wszREGREVOCATIONTYPE,
                NULL,           // lpdwReserved
                &dwType,
                (BYTE *) &m_dwRevocationFlags,
                &cb);
    if (S_OK != hr ||
	REG_DWORD != dwType ||
	sizeof(m_dwRevocationFlags) != cb)
    {
	m_dwRevocationFlags = 0;
        goto error;
    }
    DBGPRINT((fDebug, "Revocation Flags = %x\n", m_dwRevocationFlags));

    // clean up from previous call

    if (NULL != m_wszASPRevocationURL)
    {
	LocalFree(m_wszASPRevocationURL);
	m_wszASPRevocationURL = NULL;
    }

    if (REVEXT_ASPENABLE & m_dwRevocationFlags)
    {
        hr = _ReadRegistryString(
			    hkey,
			    TRUE,			// fURL
			    wszREGREVOCATIONURL,	// pwszRegName
			    L"?",			// pwszSuffix
			    &m_wszASPRevocationURL);	// pstrRegValue
        _JumpIfErrorStr(hr, error, "_ReadRegistryString", wszREGREVOCATIONURL);
        _DumpStringArray("ASP", 1, &m_wszASPRevocationURL);
    }

error:
    ;
}


//+--------------------------------------------------------------------------
// CCertPolicySample::_InitRequestExtensionList
//
//+--------------------------------------------------------------------------

VOID
CCertPolicySample::_InitRequestExtensionList(
    IN HKEY hkey)
{
    HRESULT hr;
    DWORD adwFlags[] = {
	EDITF_REQUESTEXTENSIONLIST,
    };
    WCHAR *apwszRegNames[] = {
	wszREGENABLEREQUESTEXTENSIONLIST,
    };
    WCHAR *apwszRegNamesEnrollee[] = {
	wszREGENABLEENROLLEEREQUESTEXTENSIONLIST,
    };

    assert(ARRAYSIZE(adwFlags) == ARRAYSIZE(apwszRegNames));
    assert(ARRAYSIZE(adwFlags) == ARRAYSIZE(apwszRegNamesEnrollee));

    // clean up from previous call

    if (NULL != m_apwszEnableRequestExtensions)
    {
        _FreeStringArray(
		    &m_cEnableRequestExtensions,
		    &m_apwszEnableRequestExtensions);
    }
    if (NULL != m_apwszEnableEnrolleeRequestExtensions)
    {
        _FreeStringArray(
		    &m_cEnableEnrolleeRequestExtensions,
		    &m_apwszEnableEnrolleeRequestExtensions);
    }

    hr = _ReadRegistryStringArray(
			hkey,
			FALSE,			// fURL
			m_dwEditFlags,
			ARRAYSIZE(adwFlags),
			adwFlags,
			apwszRegNames,
			&m_cEnableRequestExtensions,
			&m_apwszEnableRequestExtensions);
    _JumpIfError(hr, error, "_ReadRegistryStringArray");

    _DumpStringArray(
		"Request",
		m_cEnableRequestExtensions,
		m_apwszEnableRequestExtensions);

    hr = _ReadRegistryStringArray(
			hkey,
			FALSE,			// fURL
			m_dwEditFlags,
			ARRAYSIZE(adwFlags),
			adwFlags,
			apwszRegNamesEnrollee,
			&m_cEnableEnrolleeRequestExtensions,
			&m_apwszEnableEnrolleeRequestExtensions);
    _JumpIfError(hr, error, "_ReadRegistryStringArray");

    _DumpStringArray(
		"EnrolleeRequest",
		m_cEnableEnrolleeRequestExtensions,
		m_apwszEnableEnrolleeRequestExtensions);

error:
    ;
}

//+--------------------------------------------------------------------------
// CCertPolicySample::_InitVolatileEKUExtensionList
//
//+--------------------------------------------------------------------------

HRESULT
CCertPolicySample::_InitEKUExtensionListForVolatileMode(
    IN HKEY hkey
)
{
    HRESULT hr;
    DWORD i;
    LPWSTR pwszzEKUList = NULL;

    // clean up from previous call

    if (NULL != m_apszEKUOIDsForVolatileRequests)
    {
        _FreeStringArray(
		    &m_cEKUOIDsForVolatileRequests,
		    &m_apszEKUOIDsForVolatileRequests);
    }

    hr = _ReadRegistryMultiString(
			hkey,
			wszREGEKUOIDSFORVOLATILEREQUESTS,
			&pwszzEKUList);
    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
        hr = S_OK;
        goto error;
    }
    _JumpIfError(hr, error, "_ReadRegistryMultiString");

    // Verify each value is an OID and convert them to ansi
    hr = ceVerifyAndConvertWszToSzObjIds(
            pwszzEKUList,
            &m_cEKUOIDsForVolatileRequests,
            &m_apszEKUOIDsForVolatileRequests);
    if (S_OK != hr)
    {
            _PrintError(hr, "ceVerifyObjId");	   
            goto error;
    }

    m_fMatchAnyEKU = FALSE;
    for (i = 0; i < m_cEKUOIDsForVolatileRequests; ++i)
    {
        if (0 == _strcmpi(
                                   m_apszEKUOIDsForVolatileRequests[i],
                                   szOID_ANY_ENHANCED_KEY_USAGE))
        {
             m_fMatchAnyEKU = TRUE;
        }
    }

    hr = S_OK;
error:
    if (NULL != pwszzEKUList)
   {
       LocalFree(pwszzEKUList);
   }
    return hr;
}



//+--------------------------------------------------------------------------
// CCertPolicySample::_InitDisableExtensionList
//
//+--------------------------------------------------------------------------

VOID
CCertPolicySample::_InitDisableExtensionList(
    IN HKEY hkey)
{
    HRESULT hr;
    DWORD adwFlags[] = {
	EDITF_DISABLEEXTENSIONLIST,
    };
    WCHAR *apwszRegNames[] = {
	wszREGDISABLEEXTENSIONLIST,
    };

    assert(ARRAYSIZE(adwFlags) == ARRAYSIZE(apwszRegNames));

    // clean up from previous call

    if (NULL != m_apwszDisableExtensions)
    {
        _FreeStringArray(&m_cDisableExtensions, &m_apwszDisableExtensions);
    }

    hr = _ReadRegistryStringArray(
			hkey,
			FALSE,			// fURL
			m_dwEditFlags,
			ARRAYSIZE(adwFlags),
			adwFlags,
			apwszRegNames,
			&m_cDisableExtensions,
			&m_apwszDisableExtensions);
    _JumpIfError(hr, error, "_ReadRegistryStringArray");

    _DumpStringArray(
		"Disable",
		m_cDisableExtensions,
		m_apwszDisableExtensions);

error:
    ;
}




//+--------------------------------------------------------------------------
// CCertPolicySample::Initialize
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

STDMETHODIMP
CCertPolicySample::Initialize(
    /* [in] */ BSTR const strConfig)
{
    HRESULT hr;
    HKEY hkey = NULL;
    DWORD dwType;
    DWORD dwSize;
    ICertServerPolicy *pServer = NULL;
    BOOL fUpgraded;
    BSTR bstrDescription = NULL;

    CERT_RDN_ATTR rdnAttr = { szOID_COMMON_NAME, CERT_RDN_ANY_TYPE, };

    rdnAttr.Value.pbData = NULL;

    DBGPRINT((fDebug, "Policy:Initialize:\n"));

    hr = S_OK;

	_Cleanup();

	m_strCAName = SysAllocString(strConfig);
	if (IsNullBStr(m_strCAName))
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, leave_block, "CCertPolicySample::SysAllocString");
	}

	// force loading the description from resources

	hr = GetDescription(&bstrDescription);
	_LeaveIfError(hr, leave_block, "CCertPolicySample::GetDescription");

	// get server callbacks

	hr = polGetServerCallbackInterface(&pServer, 0);
	_LeaveIfError(hr, leave_block, "Policy:polGetServerCallbackInterface");

	hr = ReqInitialize(pServer);
	_JumpIfError(hr, error, "ReqInitialize");


	// get storage location
	hr = polGetCertificateStringProperty(
				    pServer,
				    wszPROPMODULEREGLOC,
				    &m_strRegStorageLoc);
	_LeaveIfErrorStr(
		    hr,
            leave_block,
		    "Policy:polGetCertificateStringProperty",
		    wszPROPMODULEREGLOC);

	// get CA type

	hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPCATYPE,
				    (LONG *) &m_CAType);
	_LeaveIfErrorStr(
		    hr,
                leave_block,
		    "Policy:polGetCertificateLongProperty",
		    wszPROPCATYPE);

       // get the Volatile Mode Database property

	hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPVOLATILEMODE,
				    (LONG *) &m_fVolatileMode);
	if (S_OK != hr)
	{
	    m_fVolatileMode= FALSE;
	    _PrintErrorStr(
		    hr,
		    "Policy:polGetCertificateLongProperty",
		    wszPROPVOLATILEMODE);
	}


	// get sanitized name

	hr = polGetCertificateStringProperty(
				    pServer,
				    wszPROPSANITIZEDCANAME,
				    &m_strCASanitizedName);
	_LeaveIfErrorStr(
		    hr,
            leave_block,
		    "Policy:polGetCertificateStringProperty",
		    wszPROPSANITIZEDCANAME);

	// get sanitized name

	hr = polGetCertificateStringProperty(
				    pServer,
				    wszPROPSANITIZEDSHORTNAME,
				    &m_strCASanitizedDSName);
	_LeaveIfErrorStr(
		    hr,
            leave_block,
		    "Policy:polGetCertificateStringProperty",
		    wszPROPSANITIZEDSHORTNAME);

	hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPSERVERUPGRADED,
				    (LONG *) &fUpgraded);
	if (S_OK != hr)
	{
	    fUpgraded = FALSE;
	    _PrintErrorStr(
		    hr,
		    "Policy:polGetCertificateLongProperty",
		    wszPROPSERVERUPGRADED);
	}

	hr = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		m_strRegStorageLoc,
		0,              // dwReserved
		fUpgraded?
		    KEY_ALL_ACCESS :
		    (KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_QUERY_VALUE),
		&hkey);
	_LeaveIfErrorStr(
		    hr,
            leave_block,
		    "Policy:Initialize:RegOpenKeyEx",
		    m_strRegStorageLoc);

	// Ignore error codes.

	dwSize = sizeof(m_dwDispositionFlags);
	hr = RegQueryValueEx(
			hkey,
			wszREGREQUESTDISPOSITION,
			0,
			&dwType,
			(BYTE *) &m_dwDispositionFlags,
			&dwSize);
	if (S_OK != hr || REG_DWORD != dwType)
	{
	    m_dwDispositionFlags = REQDISP_PENDINGFIRST | REQDISP_ISSUE;
	}
	DBGPRINT((
	    fDebug,
	    "Disposition Flags = %x\n",
	    m_dwDispositionFlags));

	dwSize = sizeof(m_dwEditFlags);
	hr = RegQueryValueEx(
			hkey,
			wszREGEDITFLAGS,
			0,
			&dwType,
			(BYTE *) &m_dwEditFlags,
			&dwSize);
	if (S_OK != hr || REG_DWORD != dwType)
	{
	    m_dwEditFlags =
		    EDITF_DEFAULT_STANDALONE;
	}
	if (fUpgraded)
	{
	    DBGPRINT((
		fDebug,
		"Initialize: setting EDITF_SERVERUPGRADED\n"));

	    m_dwEditFlags |= EDITF_SERVERUPGRADED;
	    dwSize = sizeof(m_dwEditFlags);
	    hr = RegSetValueEx(
			    hkey,
			    wszREGEDITFLAGS,
			    0,
			    REG_DWORD,
			    (BYTE *) &m_dwEditFlags,
			    dwSize);
	    _PrintIfError(hr, "Policy:RegSetValueEx");
	}
	DBGPRINT((fDebug, "Edit Flags = %x\n", m_dwEditFlags));

	dwSize = sizeof(m_CAPathLength);
	hr = RegQueryValueEx(
			hkey,
			wszREGCAPATHLENGTH,
			0,
			&dwType,
			(BYTE *) &m_CAPathLength,
			&dwSize);
	if (S_OK != hr || REG_DWORD != dwType)
	{
	    m_CAPathLength = CAPATHLENGTH_INFINITE;
	}
	DBGPRINT((fDebug, "CAPathLength = %x\n", m_CAPathLength));


	// Initialize the insertion string array.
	// Machine DNS name (%1)

	hr = polGetCertificateStringProperty(
			    pServer,
			    wszPROPMACHINEDNSNAME,
			    &m_strMachineDNSName);
	_LeaveIfErrorStr(
		    hr,
            leave_block,
		    "Policy:polGetCertificateStringProperty",
		    wszPROPMACHINEDNSNAME);

	hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPCERTCOUNT,
				    (LONG *) &m_iCert);
	_LeaveIfErrorStr(
		    hr,
            leave_block,
		    "Policy:polGetCertificateLongProperty",
		    wszPROPCERTCOUNT);

	if (0 == m_iCert)	// no CA certs?
	{
	    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	    _LeaveIfErrorStr(
			hr,
            leave_block,
			"Policy:polGetCertificateLongProperty",
			wszPROPCERTCOUNT);
	}
	m_iCert--;

	hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPCRLINDEX,
				    (LONG *) &m_iCRL);
	_LeaveIfErrorStr(
		    hr,
            leave_block,
		    "Policy:polGetCertificateLongProperty",
		    wszPROPCRLINDEX);

	_InitRevocationExtension(hkey);
	_InitRequestExtensionList(hkey);
	_InitDisableExtensionList(hkey);
       {
           if (m_fVolatileMode)
           {
               hr = _InitEKUExtensionListForVolatileMode(
                                                                                 hkey
                                                                                );
               _LeaveIfError(hr, error, "_InitEKUExtensionListForVolatileMode");
           }
       }

	hr = S_OK;
leave_block:

error:
    SysFreeString(bstrDescription);
    if (NULL != hkey)
    {
	RegCloseKey(hkey);
    }
    if (NULL != pServer)
    {
	pServer->Release();
    }
    return(ceHError(hr));	// Reg routines return Win32 error codes
}



DWORD
polFindObjIdInList(
    IN WCHAR const *pwsz,
    IN DWORD count,
    IN WCHAR const * const *ppwsz)
{
    DWORD i;

    for (i = 0; i < count; i++)
    {
	if (NULL == pwsz || NULL == ppwsz[i])
	{
	    i = count;
	    break;
	}
	if (0 == wcscmp(pwsz, ppwsz[i]))
	{
	    break;
	}
    }
    return(i < count? i : MAXDWORD);
}


HRESULT
CCertPolicySample::_EnumerateExtensions(
    IN ICertServerPolicy *pServer,
    IN LONG bNewRequest,
    IN BOOL fFirstPass,
    IN BOOL fEnableEnrolleeExtensions,
    IN DWORD cCriticalExtensions,
    OPTIONAL IN WCHAR const * const *apwszCriticalExtensions)
{
    HRESULT hr;
    HRESULT hr2;
    BSTR strName = NULL;
    LONG ExtFlags;
    VARIANT varValue;
    BOOL fClose = FALSE;
    BOOL fEnable;
    BOOL fDisable;
    BOOL fCritical;

    VariantInit(&varValue);

    hr = pServer->EnumerateExtensionsSetup(0);
    _JumpIfError(hr, error, "Policy:EnumerateExtensionsSetup");

    fClose = TRUE;
    while (TRUE)
    {
        hr = pServer->EnumerateExtensions(&strName);
        if (S_FALSE == hr)
        {
            hr = S_OK;
            break;
        }
        _JumpIfError(hr, error, "Policy:EnumerateExtensions");

        hr = pServer->GetCertificateExtension(
                                        strName,
                                        PROPTYPE_BINARY,
                                        &varValue);
        _JumpIfError(hr, error, "Policy:GetCertificateExtension");

        hr = pServer->GetCertificateExtensionFlags(&ExtFlags);
	_JumpIfError(hr, error, "Policy:GetCertificateExtensionFlags");

	fEnable = FALSE;
	fDisable = FALSE;
	fCritical = FALSE;

        if (fFirstPass)
        {
            if (bNewRequest && (EXTENSION_DISABLE_FLAG & ExtFlags))
            {
                switch (EXTENSION_ORIGIN_MASK & ExtFlags)
                {
                    case EXTENSION_ORIGIN_REQUEST:
                    case EXTENSION_ORIGIN_RENEWALCERT:
                    case EXTENSION_ORIGIN_PKCS7:
                    case EXTENSION_ORIGIN_CMC:
                    if ((EDITF_ENABLEREQUESTEXTENSIONS & m_dwEditFlags) ||
			MAXDWORD != polFindObjIdInList(
				    strName,
				    m_cEnableRequestExtensions,
				    m_apwszEnableRequestExtensions) ||
			(fEnableEnrolleeExtensions &&
			 MAXDWORD != polFindObjIdInList(
				    strName,
				    m_cEnableEnrolleeRequestExtensions,
				    m_apwszEnableEnrolleeRequestExtensions)))
                    {
			ExtFlags &= ~EXTENSION_DISABLE_FLAG;
			fEnable = TRUE;
                    }
                    break;
                }
            }
        }
        else
        {
            if (0 == (EXTENSION_DISABLE_FLAG & ExtFlags) &&
		MAXDWORD != polFindObjIdInList(
				    strName,
				    m_cDisableExtensions,
				    m_apwszDisableExtensions))
            {
                ExtFlags |= EXTENSION_DISABLE_FLAG;
                fDisable = TRUE;
            }
            if (0 == (EXTENSION_CRITICAL_FLAG & ExtFlags) &&
		MAXDWORD != polFindObjIdInList(
				    strName,
				    cCriticalExtensions,
				    apwszCriticalExtensions))
            {
                ExtFlags |= EXTENSION_CRITICAL_FLAG;
                fCritical = TRUE;
            }
        }

        if (fDisable || fEnable)
        {
            hr = pServer->SetCertificateExtension(
			            strName,
			            PROPTYPE_BINARY,
			            ExtFlags,
			            &varValue);
            _JumpIfError(hr, error, "Policy:SetCertificateExtension");
        }

        if (fFirstPass || fDisable || fEnable)
        {
	    DBGPRINT((
		fDebug,
                "Policy:EnumerateExtensions(%ws, Flags=%x, %x bytes)%hs%hs\n",
                strName,
                ExtFlags,
                SysStringByteLen(varValue.bstrVal),
		fDisable? " DISABLING" : (fEnable? " ENABLING" : ""),
		fCritical? " +CRITICAL" : ""));
        }
	SysFreeString(strName);
	strName = NULL;

        VariantClear(&varValue);
    }

error:
    if (fClose)
    {
        hr2 = pServer->EnumerateExtensionsClose();
        if (S_OK != hr2)
        {
            if (S_OK == hr)
            {
                hr = hr2;
            }
	    _PrintError(hr2, "Policy:EnumerateExtensionsClose");
        }
    }
    SysFreeString(strName);
    VariantClear(&varValue);
    return(hr);
}


HRESULT
EnumerateAttributes(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    HRESULT hr2;
    BSTR strName = NULL;
    BOOL fClose = FALSE;
    BSTR strValue = NULL;

    hr = pServer->EnumerateAttributesSetup(0);
    _JumpIfError(hr, error, "Policy:EnumerateAttributesSetup");

    fClose = TRUE;
    while (TRUE)
    {
        hr = pServer->EnumerateAttributes(&strName);
	if (S_FALSE == hr)
	{
	    hr = S_OK;
	    break;
	}
	_JumpIfError(hr, error, "Policy:EnumerateAttributes");

        hr = pServer->GetRequestAttribute(strName, &strValue);
	_JumpIfError(hr, error, "Policy:GetRequestAttribute");

	DBGPRINT((
		fDebug,
                "Policy:EnumerateAttributes(%ws = %ws)\n",
                strName,
                strValue));
	SysFreeString(strName);
	strName = NULL;

	SysFreeString(strValue);
	strValue = NULL;
    }

error:
    if (fClose)
    {
        hr2 = pServer->EnumerateAttributesClose();
        if (S_OK != hr2)
        {
	    _PrintError(hr2, "Policy:EnumerateAttributesClose");
            if (S_OK == hr)
            {
                hr = hr2;
            }
        }
    }
    SysFreeString(strName);
    SysFreeString(strValue);
    return(hr);
}


HRESULT
GetRequestId(
    IN ICertServerPolicy *pServer,
    OUT LONG *plRequestId)
{
    HRESULT hr;

    hr = polGetRequestLongProperty(pServer, wszPROPREQUESTREQUESTID, plRequestId);
    _JumpIfError(hr, error, "Policy:polGetRequestLongProperty");

    DBGPRINT((
	fDebug,
	"Policy:GetRequestId(%ws = %u)\n",
	wszPROPREQUESTREQUESTID,
	*plRequestId));

error:
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertPolicySample::_AddRevocationExtension
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

HRESULT
CCertPolicySample::_AddRevocationExtension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr = S_OK;
    BSTR strASPExtension = NULL;
    VARIANT varExtension;

    if (NULL != m_wszASPRevocationURL)
    {
	strASPExtension = SysAllocString(m_wszASPRevocationURL);
	if (IsNullBStr(strASPExtension))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
	}

	varExtension.vt = VT_BSTR;
	varExtension.bstrVal = strASPExtension;
	hr = polSetCertificateExtension(
				pServer,
				TEXT(szOID_NETSCAPE_REVOCATION_URL),
				PROPTYPE_STRING,
				0,
				&varExtension);
	_JumpIfErrorStr(hr, error, "Policy:polSetCertificateExtension", L"ASP");
    }

error:
    SysFreeString(strASPExtension);
    return(hr);
}


#define HIGHBIT(bitno)	(1 << (7 - (bitno)))	// bit counted from high end

#define SSLBIT_CLIENT	((BYTE) HIGHBIT(0))	// certified for client auth
#define SSLBIT_SERVER	((BYTE) HIGHBIT(1))	// certified for server auth
#define SSLBIT_SMIME	((BYTE) HIGHBIT(2))	// certified for S/MIME
#define SSLBIT_SIGN	((BYTE) HIGHBIT(3))	// certified for signing

#define SSLBIT_RESERVED	((BYTE) HIGHBIT(4))	// reserved for future use

#define SSLBIT_CASSL	((BYTE) HIGHBIT(5))	// CA for SSL auth certs
#define SSLBIT_CASMIME	((BYTE) HIGHBIT(6))	// CA for S/MIME certs
#define SSLBIT_CASIGN	((BYTE) HIGHBIT(7))	// CA for signing certs

#define NSCERTTYPE_CLIENT  ((BYTE) SSLBIT_CLIENT)
#define NSCERTTYPE_SERVER  ((BYTE) (SSLBIT_SERVER | SSLBIT_CLIENT))
#define NSCERTTYPE_SMIME   ((BYTE) SSLBIT_SMIME)
#define NSCERTTYPE_CA	   ((BYTE) (SSLBIT_CASSL | SSLBIT_CASMIME | SSLBIT_CASIGN))

//+--------------------------------------------------------------------------
// CCertPolicySample::_AddOldCertTypeExtension
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

HRESULT
CCertPolicySample::_AddOldCertTypeExtension(
    IN ICertServerPolicy *pServer,
    IN BOOL fCA)
{
    HRESULT hr = S_OK;
    ICertEncodeBitString *pBitString = NULL;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    BSTR strBitString = NULL;
    BSTR strCertType = NULL;
    CERT_BASIC_CONSTRAINTS2_INFO Constraints;
    VARIANT varConstraints;
    DWORD cb;

    VariantInit(&varConstraints);

    if (EDITF_ADDOLDCERTTYPE & m_dwEditFlags)
    {
	BYTE CertType;

	if (!fCA)
	{
	    hr = polGetCertificateExtension(
				    pServer,
				    TEXT(szOID_BASIC_CONSTRAINTS2),
				    PROPTYPE_BINARY,
				    &varConstraints);
	    if (S_OK == hr)
	    {
		cb = sizeof(Constraints);
		if (!CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_BASIC_CONSTRAINTS2,
				    (BYTE const *) varConstraints.bstrVal,
				    SysStringByteLen(varConstraints.bstrVal),
				    0,
				    &Constraints,
				    &cb))
		{
		    hr = ceHLastError();
		    _JumpError(hr, error, "Policy:CryptDecodeObject");
		}
		fCA = Constraints.fCA;
	    }
	}

	hr = CoCreateInstance(
			CLSID_CCertEncodeBitString,
			NULL,               // pUnkOuter
			CLSCTX_INPROC_SERVER,
			IID_ICertEncodeBitString,
			(VOID **) &pBitString);
	_JumpIfError(hr, error, "Policy:CoCreateInstance");

	CertType = NSCERTTYPE_CLIENT;	// Default to client auth. cert
	if (fCA)
	{
	    CertType = NSCERTTYPE_CA;
	}
	else
	{
	    hr = polGetRequestAttribute(pServer, wszPROPCERTTYPE, &strCertType);
	    if (S_OK == hr)
	    {
		if (0 == celstrcmpiL(strCertType, L"server"))
		{
		    CertType = NSCERTTYPE_SERVER;
		}
	    }
	}

        if (!ceConvertWszToBstr(
		    &strBitString,
		    (WCHAR const *) &CertType,
		    sizeof(CertType)))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:ceConvertWszToBstr");
	}

	hr = pBitString->Encode(
			    sizeof(CertType) * 8,
			    strBitString,
			    &strExtension);
	_JumpIfError(hr, error, "Policy:BitString:Encode");

        varExtension.vt = VT_BSTR;
	varExtension.bstrVal = strExtension;
	hr = polSetCertificateExtension(
				pServer,
				TEXT(szOID_NETSCAPE_CERT_TYPE),
				PROPTYPE_BINARY,
				0,
				&varExtension);
	_JumpIfError(hr, error, "Policy:polSetCertificateExtension");
    }

error:
    VariantClear(&varConstraints);
    SysFreeString(strExtension);
    SysFreeString(strBitString);
    SysFreeString(strCertType);
    if (NULL != pBitString)
    {
        pBitString->Release();
    }
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertPolicySample::_AddAuthorityKeyId
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

HRESULT
CCertPolicySample::_AddAuthorityKeyId(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr = S_OK;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    VARIANT varExtensionT;
    PCERT_AUTHORITY_KEY_ID2_INFO pInfo = NULL;
    DWORD cbInfo = 0;
    LONG ExtFlags = 0;

    VariantInit(&varExtension);

    // Optimization

    if ((EDITF_ENABLEAKIKEYID |
	 EDITF_ENABLEAKIISSUERNAME |
	 EDITF_ENABLEAKIISSUERSERIAL) ==
	((EDITF_ENABLEAKIKEYID |
	  EDITF_ENABLEAKIISSUERNAME |
	  EDITF_ENABLEAKIISSUERSERIAL |
	  EDITF_ENABLEAKICRITICAL) & m_dwEditFlags))
    {
        goto error;
    }

    hr = polGetCertificateExtension(
			    pServer,
			    TEXT(szOID_AUTHORITY_KEY_IDENTIFIER2),
			    PROPTYPE_BINARY,
			    &varExtension);
    _JumpIfError(hr, error, "Policy:polGetCertificateExtension");

    hr = pServer->GetCertificateExtensionFlags(&ExtFlags);
    _JumpIfError(hr, error, "Policy:GetCertificateExtensionFlags");

    if (VT_BSTR != varExtension.vt)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "Policy:GetCertificateExtension");
    }

    cbInfo = 0;
    if (!ceDecodeObject(
		    X509_ASN_ENCODING,
                    X509_AUTHORITY_KEY_ID2,
                    (BYTE *) varExtension.bstrVal,
                    SysStringByteLen(varExtension.bstrVal),
		    FALSE,
                    (VOID **) &pInfo,
                    &cbInfo))
    {
	hr = ceHLastError();
	_JumpIfError(hr, error, "Policy:ceDecodeObject");
    }

    // Make Any Modifications Here

    if (0 == (EDITF_ENABLEAKIKEYID & m_dwEditFlags))
    {
        pInfo->KeyId.cbData = 0;
        pInfo->KeyId.pbData = NULL;
    }
    if (0 == (EDITF_ENABLEAKIISSUERNAME & m_dwEditFlags))
    {
        pInfo->AuthorityCertIssuer.cAltEntry = 0;
        pInfo->AuthorityCertIssuer.rgAltEntry = NULL;
    }
    if (0 == (EDITF_ENABLEAKIISSUERSERIAL & m_dwEditFlags))
    {
        pInfo->AuthorityCertSerialNumber.cbData = 0;
        pInfo->AuthorityCertSerialNumber.pbData = NULL;
    }
    if (EDITF_ENABLEAKICRITICAL & m_dwEditFlags)
    {
	ExtFlags |= EXTENSION_CRITICAL_FLAG;
    }
    if (0 ==
	((EDITF_ENABLEAKIKEYID |
	  EDITF_ENABLEAKIISSUERNAME |
	  EDITF_ENABLEAKIISSUERSERIAL) & m_dwEditFlags))
    {
	ExtFlags |= EXTENSION_DISABLE_FLAG;
    }

    if (!ceEncodeObject(
		    X509_ASN_ENCODING,
		    X509_AUTHORITY_KEY_ID2,
		    pInfo,
		    0,
		    FALSE,
		    &pbEncoded,
		    &cbEncoded))
    {
	hr = ceHLastError();
	_JumpError(hr, error, "Policy:ceEncodeObject");
    }
    if (!ceConvertWszToBstr(
			&strExtension,
			(WCHAR const *) pbEncoded,
			cbEncoded))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:ceConvertWszToBstr");
    }

    varExtensionT.vt = VT_BSTR;
    varExtensionT.bstrVal = strExtension;
    hr = polSetCertificateExtension(
			    pServer,
			    TEXT(szOID_AUTHORITY_KEY_IDENTIFIER2),
			    PROPTYPE_BINARY,
			    ExtFlags,
			    &varExtensionT);
    _JumpIfError(hr, error, "Policy:polSetCertificateExtension(AuthorityKeyId2)");

error:
    VariantClear(&varExtension);
    if (NULL != pInfo)
    {
	LocalFree(pInfo);
    }
    if (NULL != pbEncoded)
    {
	LocalFree(pbEncoded);
    }
    SysFreeString(strExtension);
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertPolicy::_AddDefaultKeyUsageExtension
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

HRESULT
CCertPolicySample::_AddDefaultKeyUsageExtension(
    IN ICertServerPolicy *pServer,
    IN BOOL fCA)
{
    HRESULT hr;
    BSTR strName = NULL;
    ICertEncodeBitString *pBitString = NULL;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    BSTR strBitString = NULL;
    CERT_BASIC_CONSTRAINTS2_INFO Constraints;
    VARIANT varConstraints;
    VARIANT varKeyUsage;
    CRYPT_BIT_BLOB *pKeyUsage = NULL;
    DWORD cb;
    BYTE abKeyUsage[1];
    BYTE *pbKeyUsage;
    DWORD cbKeyUsage;

    VariantInit(&varConstraints);
    VariantInit(&varKeyUsage);

    if (EDITF_ADDOLDKEYUSAGE & m_dwEditFlags)
    {
	BOOL fModified = FALSE;

	if (!fCA)
	{
	    hr = polGetCertificateExtension(
				    pServer,
				    TEXT(szOID_BASIC_CONSTRAINTS2),
				    PROPTYPE_BINARY,
				    &varConstraints);
	    if (S_OK == hr)
	    {
		cb = sizeof(Constraints);
		if (!CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_BASIC_CONSTRAINTS2,
				    (BYTE const *) varConstraints.bstrVal,
				    SysStringByteLen(varConstraints.bstrVal),
				    0,
				    &Constraints,
				    &cb))
		{
		    hr = ceHLastError();
		    _JumpError(hr, error, "Policy:CryptDecodeObject");
		}
		fCA = Constraints.fCA;
	    }
	}

	ZeroMemory(abKeyUsage, sizeof(abKeyUsage));
	pbKeyUsage = abKeyUsage;
	cbKeyUsage = sizeof(abKeyUsage);

	hr = polGetCertificateExtension(
				pServer,
				TEXT(szOID_KEY_USAGE),
				PROPTYPE_BINARY,
				&varKeyUsage);
	if (S_OK == hr)
	{
	    if (!ceDecodeObject(
			    X509_ASN_ENCODING,
			    X509_KEY_USAGE,
			    (BYTE const *) varKeyUsage.bstrVal,
			    SysStringByteLen(varKeyUsage.bstrVal),
			    FALSE,
			    (VOID **) &pKeyUsage,
			    &cb))
	    {
		hr = GetLastError();
		_PrintError(hr, "Policy:ceDecodeObject");
	    }
	    else if (0 != cb && NULL != pKeyUsage && 0 != pKeyUsage->cbData)
	    {
		pbKeyUsage = pKeyUsage->pbData;
		cbKeyUsage = pKeyUsage->cbData;
	    }
	}

	if ((CERT_KEY_ENCIPHERMENT_KEY_USAGE & pbKeyUsage[0]) &&
	    (CERT_KEY_AGREEMENT_KEY_USAGE & pbKeyUsage[0]))
	{
	    pbKeyUsage[0] &= ~CERT_KEY_AGREEMENT_KEY_USAGE;
	    pbKeyUsage[0] |= CERT_DIGITAL_SIGNATURE_KEY_USAGE;
	    fModified = TRUE;
	}
	if (fCA)
	{
	    pbKeyUsage[0] |= ceCASIGN_KEY_USAGE;
	    fModified = TRUE;
	}
	if (fModified)
	{
	    hr = CoCreateInstance(
			    CLSID_CCertEncodeBitString,
			    NULL,               // pUnkOuter
			    CLSCTX_INPROC_SERVER,
			    IID_ICertEncodeBitString,
			    (VOID **) &pBitString);
	    _JumpIfError(hr, error, "Policy:CoCreateInstance");

	    if (!ceConvertWszToBstr(
			&strBitString,
			(WCHAR const *) pbKeyUsage,
			cbKeyUsage))
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "Policy:ceConvertWszToBstr");
	    }

	    hr = pBitString->Encode(cbKeyUsage * 8, strBitString, &strExtension);
	    _JumpIfError(hr, error, "Policy:Encode");

	    if (!ceConvertWszToBstr(&strName, TEXT(szOID_KEY_USAGE), -1))
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "Policy:ceConvertWszToBstr");
	    }
	    varExtension.vt = VT_BSTR;
	    varExtension.bstrVal = strExtension;
	    hr = pServer->SetCertificateExtension(
				    strName,
				    PROPTYPE_BINARY,
				    0,
				    &varExtension);
	    _JumpIfError(hr, error, "Policy:SetCertificateExtension");
	}
    }
    hr = S_OK;

error:
    VariantClear(&varConstraints);
    VariantClear(&varKeyUsage);
    if (NULL != pKeyUsage)
    {
        LocalFree(pKeyUsage);
    }
    SysFreeString(strName);
    SysFreeString(strExtension);
    SysFreeString(strBitString);
    if (NULL != pBitString)
    {
        pBitString->Release();
    }
    return(hr);
}


HRESULT
CCertPolicySample::_AddEnhancedKeyUsageExtension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    BSTR strUsage = NULL;
    char *pszUsage = NULL;
    char *psz;
    char *pszNext;
    CERT_ENHKEY_USAGE ceu;
    CERT_POLICIES_INFO cpi;
    BYTE *pbKeyUsage = NULL;
    DWORD cbKeyUsage;
    BYTE *pbPolicies = NULL;
    DWORD cbPolicies;
    CERT_POLICY_INFO *pcpi = NULL;
    DWORD i;
    VARIANT varExtension;

    ZeroMemory(&ceu, sizeof(ceu));
    ZeroMemory(&cpi, sizeof(cpi));
    VariantInit(&varExtension);

    if (0 == (EDITF_ATTRIBUTEEKU & m_dwEditFlags))
    {
	hr = S_OK;
	goto error;
    }
    hr = polGetRequestAttribute(pServer, wszPROPCERTUSAGE, &strUsage);
    if (S_OK != hr)
    {
	hr = S_OK;
	goto error;
    }
    if (!ceConvertWszToSz(&pszUsage, strUsage, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:ceConvertWszToSz");
    }
    for (psz = pszUsage; '\0' != *psz; psz = pszNext)
    {
	pszNext = &psz[strcspn(psz, ",")];
	if ('\0' != *pszNext)
	{
	    pszNext++;
	}
	ceu.cUsageIdentifier++;
    }
    if (0 == ceu.cUsageIdentifier)
    {
	hr = S_OK;
	goto error;
    }

    ceu.rgpszUsageIdentifier = (char **) LocalAlloc(
		LMEM_FIXED,
		ceu.cUsageIdentifier * sizeof(ceu.rgpszUsageIdentifier[0]));
    if (NULL == ceu.rgpszUsageIdentifier)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:myLocalAlloc");
    }

    // Destructively parse comma separated ObjIds into individual strings

    i = 0;
    for (psz = pszUsage; '\0' != *psz; psz = pszNext)
    {
	char *pszEnd;

	assert(i < ceu.cUsageIdentifier);
	pszNext = &psz[strcspn(psz, ",")];
	pszEnd = pszNext;
	if ('\0' != *pszNext)
	{
	    *pszNext++ = '\0';
	}
	while (' ' == *psz)
	{
	    psz++;
	}
	while (pszEnd > psz && ' ' == *--pszEnd)
	{
	    *pszEnd = '\0';
	}
	if ('\0' != *psz)
	{
	    hr = ceVerifyObjIdA(psz);
	    _JumpIfError(hr, error, "Policy:ceVerifyObjIdA");

	    ceu.rgpszUsageIdentifier[i++] = psz;
	}
    }
    ceu.cUsageIdentifier = i;
    if (0 == ceu.cUsageIdentifier)
    {
	hr = S_OK;
	goto error;
    }

    if (!ceEncodeObject(
		    X509_ASN_ENCODING,
		    X509_ENHANCED_KEY_USAGE,
		    &ceu,
		    0,
		    FALSE,
		    &pbKeyUsage,
		    &cbKeyUsage))
    {
	hr = ceHLastError();
	_JumpError(hr, error, "Policy:ceEncodeObject");
    }

    varExtension.bstrVal = NULL;
    if (!ceConvertWszToBstr(
			&varExtension.bstrVal,
			(WCHAR const *) pbKeyUsage,
			cbKeyUsage))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:ceConvertWszToBstr");
    }
    varExtension.vt = VT_BSTR;
    hr = polSetCertificateExtension(
			    pServer,
			    TEXT(szOID_ENHANCED_KEY_USAGE),
			    PROPTYPE_BINARY,
			    0,
			    &varExtension);
    _JumpIfError(hr, error, "Policy:polSetCertificateExtension");

    cpi.cPolicyInfo = ceu.cUsageIdentifier;
    cpi.rgPolicyInfo = (CERT_POLICY_INFO *) LocalAlloc(
			    LMEM_FIXED | LMEM_ZEROINIT,
			    cpi.cPolicyInfo * sizeof(cpi.rgPolicyInfo[0]));
    if (NULL == cpi.rgPolicyInfo)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:LocalAlloc");
    }
    for (i = 0; i < cpi.cPolicyInfo; i++)
    {
	cpi.rgPolicyInfo[i].pszPolicyIdentifier = ceu.rgpszUsageIdentifier[i];
    }
    if (!ceEncodeObject(
		    X509_ASN_ENCODING,
		    X509_CERT_POLICIES,
		    &cpi,
		    0,
		    FALSE,
		    &pbPolicies,
		    &cbPolicies))
    {
	hr = ceHLastError();
	_JumpError(hr, error, "Policy:ceEncodeObject");
    }

    if (!ceConvertWszToBstr(
			&varExtension.bstrVal,
			(WCHAR const *) pbPolicies,
			cbPolicies))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:ceConvertWszToBstr");
    }
    hr = polSetCertificateExtension(
			    pServer,
			    TEXT(szOID_APPLICATION_CERT_POLICIES),
			    PROPTYPE_BINARY,
			    0,
			    &varExtension);
    _JumpIfError(hr, error, "Policy:polSetCertificateExtension");

error:
    if (NULL != pcpi)
    {
	LocalFree(pcpi);
    }
    VariantClear(&varExtension);
    if (NULL != ceu.rgpszUsageIdentifier)
    {
	LocalFree(ceu.rgpszUsageIdentifier);
    }
    if (NULL != pbPolicies)
    {
	LocalFree(pbPolicies);
    }
    if (NULL != pbKeyUsage)
    {
	LocalFree(pbKeyUsage);
    }
    if (NULL != pszUsage)
    {
	LocalFree(pszUsage);
    }
    SysFreeString(strUsage);
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertPolicy::_AddDefaultBasicConstraintsExtension
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

HRESULT
CCertPolicySample::_AddDefaultBasicConstraintsExtension(
    IN ICertServerPolicy *pServer,
    IN BOOL               fCA)
{
    HRESULT hr;
    VARIANT varExtension;
    LONG ExtFlags;
    CERT_EXTENSION Ext;
    CERT_EXTENSION *pExtension = NULL;
    BSTR strCertType = NULL;

    VariantInit(&varExtension);

    if (EDITF_BASICCONSTRAINTSCA & m_dwEditFlags)
    {
        hr = polGetCertificateExtension(
				pServer,
				TEXT(szOID_BASIC_CONSTRAINTS2),
				PROPTYPE_BINARY,
				&varExtension);
        if (S_OK == hr)
        {
	    CERT_BASIC_CONSTRAINTS2_INFO Constraints;
	    DWORD cb;

	    hr = pServer->GetCertificateExtensionFlags(&ExtFlags);
	    if (S_OK == hr)
	    {
                Ext.pszObjId = szOID_BASIC_CONSTRAINTS2;
                Ext.fCritical = FALSE;
                if (EXTENSION_CRITICAL_FLAG & ExtFlags)
                {
                    Ext.fCritical = TRUE;
                }
                Ext.Value.pbData = (BYTE *) varExtension.bstrVal;
                Ext.Value.cbData = SysStringByteLen(varExtension.bstrVal);
		pExtension = &Ext;

		cb = sizeof(Constraints);
		if (!fCA && CryptDecodeObject(
			        X509_ASN_ENCODING,
			        X509_BASIC_CONSTRAINTS2,
			        Ext.Value.pbData,
			        Ext.Value.cbData,
			        0,
			        &Constraints,
			        &cb))
		{
		    fCA = Constraints.fCA;
		}
	    }
	}
    }

    if (EDITF_ATTRIBUTECA & m_dwEditFlags)
    {
        if (!fCA)
        {
	    hr = polGetRequestAttribute(pServer, wszPROPCERTTYPE, &strCertType);
            if (S_OK == hr)
            {
                if (0 == celstrcmpiL(strCertType, L"ca"))
                {
                    fCA = TRUE;
                }
		SysFreeString(strCertType);
		strCertType = NULL;
            }
        }
        if (!fCA)
        {
	    hr = polGetRequestAttribute(pServer, wszPROPCERTTEMPLATE, &strCertType);
            if (S_OK == hr)
            {
                if (0 == celstrcmpiL(strCertType, wszCERTTYPE_SUBORDINATE_CA) ||
		    0 == celstrcmpiL(strCertType, wszCERTTYPE_CROSS_CA))
                {
                    fCA = TRUE;
                }
            }
	}
    }

    // For standalone, the extension is only enabled if it's a CA

    hr = AddBasicConstraintsCommon(pServer, pExtension, fCA, NULL != pExtension);
    _JumpIfError(hr, error, "Policy:AddBasicConstraintsCommon");

error:
    VariantClear(&varExtension);
    SysFreeString(strCertType);
    return(hr);
}


HRESULT
CCertPolicySample::AddBasicConstraintsCommon(
    IN ICertServerPolicy *pServer,
    IN CERT_EXTENSION const *pExtension,
    IN BOOL fCA,
    IN BOOL fEnableExtension)
{
    HRESULT hr;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    CERT_CONTEXT const *pIssuerCert;
    CERT_EXTENSION *pIssuerExtension;
    LONG ExtFlags = 0;
    BYTE *pbConstraints = NULL;
    CERT_BASIC_CONSTRAINTS2_INFO Constraints;
    CERT_BASIC_CONSTRAINTS2_INFO IssuerConstraints;
    ZeroMemory(&IssuerConstraints, sizeof(IssuerConstraints));

    DWORD cb;

    pIssuerCert = _GetIssuer(pServer);
    if (NULL == pIssuerCert)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "_GetIssuer");
    }

    if (NULL != pExtension)
    {
        cb = sizeof(Constraints);
        if (!CryptDecodeObject(
			X509_ASN_ENCODING,
			X509_BASIC_CONSTRAINTS2,
			pExtension->Value.pbData,
			pExtension->Value.cbData,
			0,
			&Constraints,
			&cb))
        {
	    hr = ceHLastError();
	    _JumpError(hr, error, "Policy:CryptDecodeObject");
        }

        // Cert templates use CAPATHLENGTH_INFINITE to indicate
        // fPathLenConstraint should be FALSE.

        if (CAPATHLENGTH_INFINITE == Constraints.dwPathLenConstraint)
        {

            // NOTE: This is ok as certcli already sets fPathLenConstraint to FALSE
            // for templates in this case.
	    Constraints.fPathLenConstraint = FALSE;

            // NOTE: This is ok as autoenrollment ignores dwPathLenConstraint
            // if fPathLenConstraint is FALSE;
	    Constraints.dwPathLenConstraint = 0;
        }
        if (pExtension->fCritical)
        {
	    ExtFlags = EXTENSION_CRITICAL_FLAG;
        }
    }
    else
    {
	Constraints.fCA = fCA;
	Constraints.fPathLenConstraint = FALSE;
	Constraints.dwPathLenConstraint = 0;
    }
    if (EDITF_BASICCONSTRAINTSCRITICAL & m_dwEditFlags)
    {
        ExtFlags = EXTENSION_CRITICAL_FLAG;
    }

    // Check basic constraints against the issuer's cert.

    pIssuerExtension = CertFindExtension(
				szOID_BASIC_CONSTRAINTS2,
				pIssuerCert->pCertInfo->cExtension,
				pIssuerCert->pCertInfo->rgExtension);
    if (NULL != pIssuerExtension)
    {
        cb = sizeof(IssuerConstraints);
        if (!CryptDecodeObject(
			        X509_ASN_ENCODING,
			        X509_BASIC_CONSTRAINTS2,
			        pIssuerExtension->Value.pbData,
			        pIssuerExtension->Value.cbData,
			        0,
			        &IssuerConstraints,
			        &cb))
        {
            hr = ceHLastError();
            _JumpError(hr, error, "Policy:CryptDecodeObject");
        }
        if (!IssuerConstraints.fCA)
        {
            hr = CERTSRV_E_INVALID_CA_CERTIFICATE;
            _JumpError(hr, error, "Policy:CA cert not a CA cert");
        }
    }

    if (Constraints.fCA)
    {
        if (IssuerConstraints.fPathLenConstraint)
        {
            if (0 == IssuerConstraints.dwPathLenConstraint)
            {
                hr = CERTSRV_E_INVALID_CA_CERTIFICATE;
                _JumpError(hr, error, "Policy:CA cert is a leaf CA cert");
            }
            if (!Constraints.fPathLenConstraint ||
                Constraints.dwPathLenConstraint >
	            IssuerConstraints.dwPathLenConstraint - 1)
            {
                Constraints.fPathLenConstraint = TRUE;
                Constraints.dwPathLenConstraint =
                IssuerConstraints.dwPathLenConstraint - 1;
            }
        }
        if (CAPATHLENGTH_INFINITE != m_CAPathLength)
        {
            if (0 == m_CAPathLength)
            {
                hr = CERTSRV_E_INVALID_CA_CERTIFICATE;
                _JumpError(hr, error, "Policy:Registry says not to issue CA certs");
            }
            if (!Constraints.fPathLenConstraint ||
                Constraints.dwPathLenConstraint > m_CAPathLength - 1)
            {
                Constraints.fPathLenConstraint = TRUE;
                Constraints.dwPathLenConstraint = m_CAPathLength - 1;
            }
        }
    }

    if (!fEnableExtension)
    {
        ExtFlags |= EXTENSION_DISABLE_FLAG;
    }

    if (!ceEncodeObject(
		    X509_ASN_ENCODING,
                    X509_BASIC_CONSTRAINTS2,
                    &Constraints,
		    0,
		    FALSE,
                    &pbConstraints,
                    &cb))
    {
        hr = ceHLastError();
        _JumpError(hr, error, "Policy:ceEncodeObject");
    }

    if (!ceConvertWszToBstr(
			&strExtension,
			(WCHAR const *) pbConstraints,
			cb))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:ceConvertWszToBstr");
    }

    varExtension.vt = VT_BSTR;
    varExtension.bstrVal = strExtension;
    hr = polSetCertificateExtension(
			    pServer,
			    TEXT(szOID_BASIC_CONSTRAINTS2),
			    PROPTYPE_BINARY,
			    ExtFlags,
			    &varExtension);
    _JumpIfError(hr, error, "Policy:polSetCertificateExtension");

error:
    if (NULL != pbConstraints)
    {
        LocalFree(pbConstraints);
    }
    SysFreeString(strExtension);
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertPolicy::_SetValidityPeriod
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

HRESULT
CCertPolicySample::_SetValidityPeriod(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    BSTR strPeriodString = NULL;
    BSTR strPeriodCount = NULL;
    BSTR strNameNotBefore = NULL;
    BSTR strNameNotAfter = NULL;
    BSTR strExpirationDate = NULL;
    VARIANT varValue;
    LONG lDelta;
    ENUM_PERIOD enumValidityPeriod;
    BOOL fValidDigitString;
    SYSTEMTIME SysExpirationDate;

    VariantInit(&varValue);

    if (!(EDITF_ATTRIBUTEENDDATE & m_dwEditFlags))
    {
	hr = S_OK;
	goto error;
    }

    hr = polGetRequestAttribute(
                        pServer,
                        wszPROPEXPIRATIONDATE,
                        &strExpirationDate);
    if (SUCCEEDED(hr))
    {
        if (!WinHttpTimeToSystemTime(
                strExpirationDate,
                &SysExpirationDate))
        {
            hr = ceHLastError();
            _JumpError(hr, error, "WinHttpTimeToSystemTime");
        }

        if (!SystemTimeToVariantTime(&SysExpirationDate, &varValue.date))
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            _JumpError(hr, error, "SystemTimeToVariantTime");
        }
        varValue.vt = VT_DATE;
    }
    else
    {
        hr = polGetRequestAttribute(
			    pServer,
			    wszPROPVALIDITYPERIODSTRING,
			    &strPeriodString);
        if (S_OK != hr)
        {
	    _PrintErrorStr2(
		    hr,
		    "Policy:polGetRequestAttribute",
		    wszPROPVALIDITYPERIODSTRING,
		    CERTSRV_E_PROPERTY_EMPTY);
	    if (CERTSRV_E_PROPERTY_EMPTY == hr)
	    {
	        hr = S_OK;
	    }
	    goto error;
        }

        hr = polGetRequestAttribute(
			    pServer,
			    wszPROPVALIDITYPERIODCOUNT,
			    &strPeriodCount);
        if (S_OK != hr)
        {
	    _PrintErrorStr2(
		    hr,
		    "Policy:polGetRequestAttribute",
		    wszPROPVALIDITYPERIODCOUNT,
		    CERTSRV_E_PROPERTY_EMPTY);
	    if (CERTSRV_E_PROPERTY_EMPTY == hr)
	    {
	        hr = S_OK;
	    }
	    goto error;
        }

        // Swap Count and String BSTRs if backwards -- Windows 2000 had it wrong.

        lDelta = ceWtoI(strPeriodCount, &fValidDigitString);
        if (!fValidDigitString)
        {
	    BSTR str = strPeriodCount;

	    strPeriodCount = strPeriodString;
	    strPeriodString = str;

	    lDelta = ceWtoI(strPeriodCount, &fValidDigitString);
	    if (!fValidDigitString)
	    {
	        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	        _JumpError(hr, error, "Policy:ceWtoI");
	    }
        }

        hr = ceTranslatePeriodUnits(strPeriodString, lDelta, &enumValidityPeriod, &lDelta);
        _JumpIfError(hr, error, "Policy:ceTranslatePeriodUnits");

        strNameNotBefore = SysAllocString(wszPROPCERTIFICATENOTBEFOREDATE);
        if (IsNullBStr(strNameNotBefore))
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "Policy:SysAllocString");
        }
        hr = pServer->GetCertificateProperty(
				    strNameNotBefore,
				    PROPTYPE_DATE,
				    &varValue);
        _JumpIfError(hr, error, "Policy:GetCertificateProperty");

        hr = ceMakeExprDate(&varValue.date, lDelta, enumValidityPeriod);
        _JumpIfError(hr, error, "Policy:ceMakeExprDate");
    }

    strNameNotAfter = SysAllocString(wszPROPCERTIFICATENOTAFTERDATE);
    if (IsNullBStr(strNameNotAfter))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:SysAllocString");
    }
    hr = pServer->SetCertificateProperty(
				strNameNotAfter,
				PROPTYPE_DATE,
				&varValue);
    if (S_OK != hr)
    {
        _PrintError(hr, "_SetValidityPeriod");
    }

error:
    VariantClear(&varValue);
    SysFreeString(strPeriodString);
    SysFreeString(strPeriodCount);
    SysFreeString(strNameNotBefore);
    SysFreeString(strNameNotAfter);
    SysFreeString(strExpirationDate);
    return(hr);
}

//+--------------------------------------------------------------------------
// CCertPolicy::_PersistRequest
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------



HRESULT
CCertPolicySample::_PersistRequest(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    DWORD i, j;
    BOOL fMatch = FALSE;
    DWORD cbEKU;
    LONG ExtFlags;
    LONG fArchivedKey = FALSE;
    VARIANT var;
    CERT_ENHKEY_USAGE* pEKU = NULL;
    BSTR strPropName = NULL;
    VariantInit(&var); 

    strPropName = SysAllocString(wszPROPCERTIFICATEGENERALFLAGS);
    if (IsNullBStr(strPropName))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:SysAllocString");
    }


    {
        hr = polGetCertificateExtension(
                     pServer,
                     TEXT(szOID_ENHANCED_KEY_USAGE), 
                     PROPTYPE_BINARY,
                     &var);
        if (CERTSRV_E_PROPERTY_EMPTY == hr)
        {
            hr = S_OK;
            goto error;
        }
    
        hr = pServer->GetCertificateExtensionFlags(&ExtFlags);
        _JumpIfError(hr, error, "Policy:GetCertificateExtensionFlags");
    
        if (EXTENSION_DISABLE_FLAG & ExtFlags)
        {
            hr = S_OK;
            goto error;
        }
    
        if (!ceDecodeObject(
                     X509_ASN_ENCODING,
                     X509_ENHANCED_KEY_USAGE,
                     (BYTE const *)(V_BSTR(&var)),
                     SysStringByteLen(V_BSTR(&var)), 
                     FALSE,
                     (VOID * *)&pEKU, 
                     &cbEKU))
        {
            hr = ceHLastError();
            _JumpError(hr, error, "ceDecodeObject");
        }
    
        if (0 == pEKU->cUsageIdentifier)
        {
            hr = S_OK;
            goto error;
        }

        if (m_fMatchAnyEKU)
        {
            fMatch = TRUE;
        }
        else
        {
            for (i = 0; i < pEKU->cUsageIdentifier; i++)
            {
                for (j = 0; j < m_cEKUOIDsForVolatileRequests; j++)
                {
                    if (0 == _strcmpi(
                                                pEKU->rgpszUsageIdentifier[i],
                                                m_apszEKUOIDsForVolatileRequests[j]) ||
                        0 == _strcmpi(
                                                pEKU->rgpszUsageIdentifier[i],
                                                szOID_ANY_ENHANCED_KEY_USAGE))
                    {
                        fMatch = TRUE;	// found matching EKU OID
                        break;
                    }
                }
            }
        }

        if (fMatch)
        {
            // set a GeneralFlag value to indicate to the CA that this request must not be persisted 
            VariantClear(&var);  
 
            hr = pServer->GetCertificateProperty(strPropName, PROPTYPE_LONG, &var);
            if (CERTSRV_E_PROPERTY_EMPTY == hr)
            {
                hr  = S_OK;
            }
            _JumpIfError(hr, error, "Policy:SetCertificateProperty");
            
            var.vt = VT_I4;
            var.lVal |= CT_FLAG_DONOTPERSISTINDB;

            hr = pServer->SetCertificateProperty(strPropName, PROPTYPE_LONG, &var);
            _JumpIfError(hr, error, "Policy:SetCertificateProperty");

        }

    }
    

    if (fMatch)
    {
        // Fail Key archival requests
        hr = polGetCertificateLongProperty(pServer, wszPROPKEYARCHIVED, &fArchivedKey);
        _JumpIfErrorStr(hr, error, "polGetCertificateLongProperty", wszPROPKEYARCHIVED);
    
        // fail the request as there is an archived key

        if (fArchivedKey)
        {
            hr = CERTSRV_E_BAD_REQUESTSTATUS;
    
            _JumpError(hr, error, "Key Archival request");
        }        
    }

    hr = S_OK;

error:
    VariantClear(&var);
    if (NULL != pEKU)
    {
        LocalFree(pEKU);
    }
    SysFreeString(strPropName);
    return hr;
}



//+--------------------------------------------------------------------------
// CCertPolicySample::_AddV1TemplateNameExtension
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

HRESULT
CCertPolicySample::AddV1TemplateNameExtension(
    IN ICertServerPolicy *pServer,
    OPTIONAL IN WCHAR const *pwszTemplateName)
{
    HRESULT hr;
    BSTR strName = NULL;
    LONG ExtFlags = 0;
    VARIANT varExtension;
    CERT_NAME_VALUE *pName = NULL;
    CERT_NAME_VALUE NameValue;
    DWORD cbEncoded;
    BYTE *pbEncoded = NULL;
    BOOL fUpdate = TRUE;

    VariantInit(&varExtension);

    strName = SysAllocString(TEXT(szOID_ENROLL_CERTTYPE_EXTENSION));
    if (IsNullBStr(strName))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }

    hr = pServer->GetCertificateExtension(
				    strName,
				    PROPTYPE_BINARY,
				    &varExtension);
    _PrintIfError2(hr, "Policy:GetCertificateExtension", hr);
    if (CERTSRV_E_PROPERTY_EMPTY == hr)
    {
	if (NULL == pwszTemplateName)
	{
	    hr = S_OK;
	    goto error;
	}
    }
    else
    {
	_JumpIfError(hr, error, "Policy:GetCertificateExtension");

	hr = pServer->GetCertificateExtensionFlags(&ExtFlags);
	_JumpIfError(hr, error, "Policy:GetCertificateExtensionFlags");

	if (VT_BSTR == varExtension.vt &&
	    0 == (EXTENSION_DISABLE_FLAG & ExtFlags) &&
	    NULL != pwszTemplateName)
	{
	    if (!ceDecodeObject(
			X509_ASN_ENCODING,
			X509_UNICODE_ANY_STRING,
			(BYTE *) varExtension.bstrVal,
			SysStringByteLen(varExtension.bstrVal),
			FALSE,
			(VOID **) &pName,
			&cbEncoded))
	    {
		hr = ceHLastError();
		_JumpError(hr, error, "Policy:ceDecodeObject");
	    }

	    // case sensitive compare -- be sure to match case of template

	    if (0 == lstrcmp(
			(WCHAR const *) pName->Value.pbData,
			pwszTemplateName))
	    {
		fUpdate = FALSE;
	    }
	}
    }
    if (fUpdate)
    {
	if (NULL == pwszTemplateName)
	{
	    ExtFlags |= EXTENSION_DISABLE_FLAG;
	}
	else
	{
	    VariantClear(&varExtension);
	    varExtension.bstrVal = NULL;

	    NameValue.dwValueType = CERT_RDN_UNICODE_STRING;
	    NameValue.Value.pbData = (BYTE *) pwszTemplateName;
	    NameValue.Value.cbData = 0;

	    if (!ceEncodeObject(
			    X509_ASN_ENCODING,
			    X509_UNICODE_ANY_STRING,
			    &NameValue,
			    0,
			    FALSE,
			    &pbEncoded,
			    &cbEncoded))
	    {
		hr = ceHLastError();
		_JumpError(hr, error, "Policy:ceEncodeObject");
	    }
	    if (!ceConvertWszToBstr(
				&varExtension.bstrVal,
				(WCHAR const *) pbEncoded,
				cbEncoded))
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "Policy:ceConvertWszToBstr");
	    }
	    varExtension.vt = VT_BSTR;
	    ExtFlags &= ~EXTENSION_DISABLE_FLAG;
	}
	hr = pServer->SetCertificateExtension(
				strName,
				PROPTYPE_BINARY,
				ExtFlags,
				&varExtension);
	_JumpIfError(hr, error, "Policy:SetCertificateExtension");
    }
    hr = S_OK;

error:
    VariantClear(&varExtension);
    SysFreeString(strName);
    if (NULL != pName)
    {
	LocalFree(pName);
    }
    if (NULL != pbEncoded)
    {
	LocalFree(pbEncoded);
    }
    return(hr);
}

#pragma warning(push)
#pragma warning(disable: 4509) // nonstandard extension used: uses SEH and has destructor

STDMETHODIMP
CCertPolicySample::VerifyRequest(
    /* [in] */ BSTR const, // strConfig
    /* [in] */ LONG Context,
    /* [in] */ LONG bNewRequest,
    /* [in] */ LONG, // Flags
    /* [out, retval] */ LONG __RPC_FAR *pDisposition)
{
    HRESULT hr = E_FAIL;
    HRESULT hr2 = E_FAIL;
    ICertServerPolicy *pServer = NULL;
    LONG lRequestId;
    CRequestInstance Request;
    BSTR strDisposition = NULL;
    BOOL fEnableEnrolleeExtensions;
    DWORD cCriticalExtensions = 0;
    WCHAR const * const *apwszCriticalExtensions = NULL;

    lRequestId = 0;


	if (NULL == pDisposition)
	{
	    hr = E_POINTER;
	    _LeaveError(hr, leave_block, "Policy:pDisposition");
	}
	*pDisposition = VR_INSTANT_BAD;

	hr = polGetServerCallbackInterface(&pServer, Context);
	_LeaveIfError(hr, leave_block, "Policy:polGetServerCallbackInterface");

	hr = GetRequestId(pServer, &lRequestId);
	_JumpIfError(hr, deny, "Policy:GetRequestId");

	// only need to check user access for original submitter:
	// resubmit can only be called by admins

	if (bNewRequest && (0 == (m_dwEditFlags & EDITF_IGNOREREQUESTERGROUP)))
	{
	    BOOL fRequesterAccess = FALSE;

	    // Is this user allowed to request certs?
	    hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPREQUESTERCAACCESS,
				    (LONG *) &fRequesterAccess);
	    _PrintIfErrorStr(
			hr,
			"Policy:polGetCertificateLongProperty",
			wszPROPREQUESTERCAACCESS);
	    if (hr != S_OK || !fRequesterAccess)
	    {
		hr = CERTSRV_E_ENROLL_DENIED;
		_JumpError(hr, deny, "Policy:fRequesterAccess");
	    }
	}


	hr = Request.Initialize(
			    this,
  		           pServer,
			    &fEnableEnrolleeExtensions);
	_LeaveIfError(hr, leave_block, "Policy:VerifyRequest:Request.Initialize");


	hr = _EnumerateExtensions(
			    pServer,
			    bNewRequest,
			    TRUE,
			    fEnableEnrolleeExtensions,
			    0,
			    NULL);
	_LeaveIfError(hr, leave_block, "_EnumerateExtensions");

	{
	    hr = _AddDefaultBasicConstraintsExtension(
						pServer,
						Request.IsCARequest());
	    _LeaveIfError(hr, leave_block, "_AddDefaultBasicConstraintsExtension");

	    hr = _AddDefaultKeyUsageExtension(pServer, Request.IsCARequest());
	    _LeaveIfError(hr, leave_block, "_AddDefaultKeyUsageExtension");

	    hr = _AddEnhancedKeyUsageExtension(pServer);
	    _LeaveIfError(hr, leave_block, "_AddEnhancedKeyUsageExtension");
	}

	hr = _SetValidityPeriod(pServer);
	_LeaveIfError(hr, leave_block, "_SetValidityPeriod");

	hr = EnumerateAttributes(pServer);
	_LeaveIfError(hr, leave_block, "Policy:EnumerateAttributes");

	hr = _AddRevocationExtension(pServer);
	_LeaveIfError(hr, leave_block, "_AddRevocationExtension");

	hr = _AddOldCertTypeExtension(pServer, Request.IsCARequest());
	_LeaveIfError(hr, leave_block, "_AddOldCertTypeExtension");

	hr = _AddAuthorityKeyId(pServer);
	_LeaveIfError(hr, leave_block, "_AddAuthorityKeyId");


	// pass hr as Disposition

	if ((EDITF_DISABLEEXTENSIONLIST & m_dwEditFlags) ||
	    NULL != apwszCriticalExtensions)
	{
	    hr = _EnumerateExtensions(
				pServer,
				bNewRequest,
				FALSE,
				FALSE,
				cCriticalExtensions,
				apwszCriticalExtensions);
	    _LeaveIfError(hr, leave_block, "_EnumerateExtensions");
	}


       if (m_fVolatileMode)
       {
           hr = _PersistRequest(pServer);
           _LeaveIfError(hr, leave_block, "_PersistRequest");
       }
       

	if (bNewRequest &&
	    (
	     (REQDISP_PENDINGFIRST & m_dwDispositionFlags)))
	{
	    *pDisposition = VR_PENDING;
	}
	else switch (REQDISP_MASK & m_dwDispositionFlags)
	{
	    default:
	    case REQDISP_PENDING:
		*pDisposition = VR_PENDING;
		break;

	    case REQDISP_ISSUE:
		*pDisposition = VR_INSTANT_OK;
		break;

	    case REQDISP_DENY:
		*pDisposition = VR_INSTANT_BAD;
		break;

	    case REQDISP_USEREQUESTATTRIBUTE:
		*pDisposition = VR_INSTANT_OK;
		hr = polGetRequestAttribute(
				    pServer,
				    wszPROPDISPOSITION,
				    &strDisposition);
		if (S_OK == hr)
		{
		    if (0 == celstrcmpiL(strDisposition, wszPROPDISPOSITIONDENY))
		    {
			*pDisposition = VR_INSTANT_BAD;
		    }
		    if (0 == celstrcmpiL(strDisposition, wszPROPDISPOSITIONPENDING))
		    {
			*pDisposition = VR_PENDING;
		    }
		}
		hr = S_OK;
		break;
	}
deny:
	if (FAILED(hr))
	{
	    *pDisposition = hr;	// pass failed HRESULT back as Disposition
	}
	else if (hr != S_OK)
	{
	    *pDisposition = VR_INSTANT_BAD;
	}
	hr = S_OK;
leave_block:

    {
	hr2 = hr;
#define wszFORMATREQUESTID	L"RequestId=%u"
	WCHAR wszRequestId[ARRAYSIZE(wszFORMATREQUESTID) + cwcDWORDSPRINTF];

	if (S_OK == hr2 && NULL != pDisposition && FAILED(*pDisposition))
	{
	    hr2 = *pDisposition;
	}
	if (S_OK != hr2)
	{
	    StringCbPrintf(wszRequestId, sizeof(wszRequestId), wszFORMATREQUESTID, lRequestId);
	    _PrintErrorStr(hr2, "VerifyRequest", wszRequestId);
	}
    }
    SysFreeString(strDisposition);
    if (NULL != pServer)
    {
        pServer->Release();
    }
    //_PrintIfError(hr, "Policy:VerifyRequest(hr)");
    //_PrintError(*pDisposition, "Policy:VerifyRequest(*pDisposition)");
    return(hr);
}
#pragma warning(pop)


//+--------------------------------------------------------------------------
// CCertPolicySample::GetDescription
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

STDMETHODIMP
CCertPolicySample::GetDescription(
    /* [out, retval] */ BSTR __RPC_FAR *pstrDescription)
{
    HRESULT hr = S_OK;
    WCHAR sz[MAX_PATH];

    if(!m_strDescription)
    {
	assert(wcslen(wsz_SAMPLE_DESCRIPTION) < ARRAYSIZE(sz));
	StringCbCopy(sz, sizeof(sz), wsz_SAMPLE_DESCRIPTION);
	sz[ARRAYSIZE(sz) - 1] = L'\0';

	m_strDescription = SysAllocString(sz);
	if (IsNullBStr(m_strDescription))
	{
	    hr = E_OUTOFMEMORY;
	    return hr;
	}
    }
    SysFreeString(*pstrDescription);
    *pstrDescription = SysAllocString(m_strDescription);
    if (IsNullBStr(*pstrDescription))
    {
        hr = E_OUTOFMEMORY;
    }
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertPolicySample::ShutDown
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

STDMETHODIMP
CCertPolicySample::ShutDown(VOID)
{
    // called once, as Server unloading policy dll
    _Cleanup();
    return(S_OK);
}


//+--------------------------------------------------------------------------
// CCertPolicySample::GetManageModule
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

STDMETHODIMP
CCertPolicySample::GetManageModule(
    /* [out, retval] */ ICertManageModule **ppManageModule)
{
    HRESULT hr;

    *ppManageModule = NULL;
    hr = CoCreateInstance(
		    CLSID_CCertManagePolicyModuleSample,
                    NULL,               // pUnkOuter
                    CLSCTX_INPROC_SERVER,
		    IID_ICertManageModule,
                    (VOID **) ppManageModule);
    _JumpIfError(hr, error, "CoCreateInstance");

error:
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertPolicySample::_GetIssuer
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

PCCERT_CONTEXT
CCertPolicySample::_GetIssuer(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    VARIANT varValue;
    BSTR strName = NULL;

    VariantInit(&varValue);
    if (NULL != m_pCert)
    {
        hr = S_OK;
	goto error;
    }
    strName = SysAllocString(wszPROPRAWCACERTIFICATE);
    if (IsNullBStr(strName))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    hr = pServer->GetCertificateProperty(strName, PROPTYPE_BINARY, &varValue);
    _JumpIfError(hr, error, "Policy:GetCertificateProperty");

    m_pCert = CertCreateCertificateContext(
				    X509_ASN_ENCODING,
				    (BYTE *) varValue.bstrVal,
				    SysStringByteLen(varValue.bstrVal));
    if (NULL == m_pCert)
    {
	hr = ceHLastError();
	_JumpError(hr, error, "Policy:CertCreateCertificateContext");
    }

error:
    VariantClear(&varValue);
    SysFreeString(strName);
    return(m_pCert);
}


STDMETHODIMP
CCertPolicySample::InterfaceSupportsErrorInfo(
    IN REFIID riid)
{
    static const IID *arr[] =
    {
        &IID_ICertPolicy,
    };

    for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
    {
        if (IsEqualGUID(*arr[i], riid))
        {
            return(S_OK);
        }
    }
    return(S_FALSE);
}

