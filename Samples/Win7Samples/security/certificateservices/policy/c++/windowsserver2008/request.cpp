//+--------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// File:        request.cpp
//
// Contents:    Cert Server Policy Module implementation
//
//---------------------------------------------------------------------------

#include "pch.cpp"
#pragma hdrstop

#include <assert.h>
#include "celib.h"
#pragma warning(push)
#pragma warning(disable : 4996) // to disable SDK warning from using deprecated APIs with ATL 7.0 and greater
#include "policy.h"
#include "module.h"
#pragma warning(pop)

HRESULT
ReqInitialize(
    IN ICertServerPolicy *
	)
{
    HRESULT hr;

    hr = S_OK;

    return(hr);
}


VOID
ReqCleanup()
{
}


CRequestInstance::~CRequestInstance()
{
    _Cleanup();
}


VOID
CRequestInstance::_Cleanup()
{
    SysFreeString(m_strTemplateName);
    m_strTemplateName = NULL;

    SysFreeString(m_strTemplateObjId);
    m_strTemplateObjId = NULL;

}




static WCHAR const *s_apwszCATypes[] =
{
    wszCERTTYPE_SUBORDINATE_CA,
    wszCERTTYPE_CROSS_CA,
};

//+--------------------------------------------------------------------------
// CRequestInstance::Initialize
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

HRESULT
CRequestInstance::Initialize(
    IN CCertPolicySample *pPolicy,
    IN ICertServerPolicy *pServer,
    OUT BOOL *pfEnableEnrolleeExtensions)
{
    HRESULT hr;
    HRESULT hrTemplate = S_OK;
    CERT_TEMPLATE_EXT *pTemplate = NULL;
    CERT_NAME_VALUE *pName = NULL;
    BSTR strTemplateObjId = NULL;	// from V2 template extension
    BSTR strTemplateName = NULL;	// from V1 template extension
    BSTR strTemplateRA = NULL;		// from request attributes
    WCHAR const *pwszTemplateName;
    WCHAR const *pwszTemplateObjId;
    WCHAR const *pwszV1TemplateClass;
    VARIANT varValue;
    DWORD cbType;
    DWORD i;
    BOOL fConflict;
    BOOL f;
    BOOL fTemplateMissing;
    BOOL fRAObjId = FALSE;

    VariantInit(&varValue);
    *pfEnableEnrolleeExtensions = TRUE
		    ;

    m_pPolicy = pPolicy;
    m_fCA = FALSE;


    // Retrieve the template ObjId from the V2 cert template info extension

    m_dwTemplateMajorVersion = 0;
    m_dwTemplateMinorVersion = 0;
    hr = polGetCertificateExtension(
			    pServer,
			    TEXT(szOID_CERTIFICATE_TEMPLATE),
			    PROPTYPE_BINARY,
			    &varValue);
    _PrintIfErrorStr2(
		hr,
		"Policy:polGetCertificateExtension",
		TEXT(szOID_CERTIFICATE_TEMPLATE),
		CERTSRV_E_PROPERTY_EMPTY);
    if (S_OK == hr)
    {
        // There was a cert type indicator.
        // varValue points to an encoded string

        if (VT_BSTR != varValue.vt)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "Policy:varValue.vt");
	}
        if (!ceDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CERTIFICATE_TEMPLATE,
		    (BYTE *) varValue.bstrVal,
		    SysStringByteLen(varValue.bstrVal),
		    FALSE,
		    (VOID **) &pTemplate,
		    &cbType))
        {
            hr = ceHLastError();
	    _JumpError(hr, error, "Policy:ceDecodeObject");
        }
	if (!ceConvertSzToBstr(&strTemplateObjId, pTemplate->pszObjId, -1))
	{
            hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:ceConvertSzToBstr");
        }
	m_dwTemplateMajorVersion = pTemplate->dwMajorVersion;
	m_dwTemplateMinorVersion = pTemplate->dwMinorVersion;
	DBGPRINT((
	    fDebug,
	    pTemplate->fMinorVersion?
		"Extension Template Info: %ws V%u.%u\n" :
		"Extension Template Info: %ws V%u%\n",
	    strTemplateObjId,
	    m_dwTemplateMajorVersion,
	    m_dwTemplateMinorVersion));
    }
    VariantClear(&varValue);

    // Retrieve template Name from the V1 cert template name extension

    hr = polGetCertificateExtension(
			    pServer,
			    TEXT(szOID_ENROLL_CERTTYPE_EXTENSION),
			    PROPTYPE_BINARY,
			    &varValue);
    _PrintIfErrorStr2(
		hr,
		"Policy:polGetCertificateExtension",
		TEXT(szOID_ENROLL_CERTTYPE_EXTENSION),
		CERTSRV_E_PROPERTY_EMPTY);
    if (S_OK == hr)
    {
        // There was a cert type indicator.
        // varValue points to an encoded string

        if (VT_BSTR != varValue.vt)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "Policy:varValue.vt");
	}
        if (!ceDecodeObject(
		    X509_ASN_ENCODING,
		    X509_UNICODE_ANY_STRING,
		    (BYTE *) varValue.bstrVal,
		    SysStringByteLen(varValue.bstrVal),
		    FALSE,
		    (VOID **) &pName,
		    &cbType))
        {
            hr = ceHLastError();
	    _JumpError(hr, error, "Policy:ceDecodeObject");
        }
        strTemplateName = SysAllocString((WCHAR *) pName->Value.pbData);
        if (IsNullBStr(strTemplateName))
        {
            hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
        }
	DBGPRINT((fDebug, "Extension Template: %ws\n", strTemplateName));
    }

    fConflict = FALSE;
    fTemplateMissing = FALSE;

    // Retrieve the template from the request attributes

    hr = polGetRequestAttribute(pServer, wszPROPCERTTEMPLATE, &strTemplateRA);
    if (S_OK != hr)
    {
	_PrintErrorStr2(
		    hr,
		    "Policy:polGetRequestAttribute",
		    wszPROPCERTTEMPLATE,
		    CERTSRV_E_PROPERTY_EMPTY);
	hr = S_OK;


    }
    else
    {
	DBGPRINT((fDebug, "Attribute Template: %ws\n", strTemplateRA));
	if (!IsNullBStr(strTemplateObjId) &&
	    !_TemplateNamesMatch(strTemplateObjId, strTemplateRA, &f))
	{
	    fConflict = TRUE;
	    if (f)
	    {
		fTemplateMissing = TRUE;
	    }
	}
	if (!IsNullBStr(strTemplateName) &&
	    !_TemplateNamesMatch(strTemplateName, strTemplateRA, &f))
	{
	    fConflict = TRUE;
	    if (f)
	    {
		fTemplateMissing = TRUE;
	    }
	}
	hr = ceVerifyObjId(strTemplateRA);
	fRAObjId = S_OK == hr;
    }

    if (!IsNullBStr(strTemplateObjId) &&
	!IsNullBStr(strTemplateName) &&
	!_TemplateNamesMatch(strTemplateObjId, strTemplateName, &f))
    {
	fConflict = TRUE;
	if (f)
	{
	    fTemplateMissing = TRUE;
	}
    }

    if (fConflict)
    {
	hrTemplate = CERTSRV_E_TEMPLATE_CONFLICT;
	if (!IsNullBStr(strTemplateObjId))
	{
	    _PrintErrorStr(
			hrTemplate,
			"Policy:Extension Template ObjId",
			strTemplateObjId);
	}
	if (!IsNullBStr(strTemplateName))
	{
	    _PrintErrorStr(
			hrTemplate,
			"Policy:Extension Template Name",
			strTemplateName);
	}
	if (!IsNullBStr(strTemplateRA))
	{
	    _PrintErrorStr(
			hrTemplate,
			"Policy:Attribute Template",
			strTemplateRA);
	}
    }

    pwszTemplateName = strTemplateName;
    pwszTemplateObjId = strTemplateObjId;
    if (fRAObjId)
    {
	if (NULL == pwszTemplateObjId)
	{
	    pwszTemplateObjId = strTemplateRA;
	}
    }
    else
    {
	if (NULL == pwszTemplateName)
	{
	    pwszTemplateName = strTemplateRA;
	}
    }


    {
	if (NULL != pwszTemplateName)
	{
	    for (i = 0; i < ARRAYSIZE(s_apwszCATypes); i++)
	    {
		if (0 == celstrcmpiL(s_apwszCATypes[i], pwszTemplateName))
		{
		    m_fCA = TRUE;
		    break;
		}
	    }
	}
    }
    hr = SetTemplateName(pServer, pwszTemplateName, pwszTemplateObjId);
    _JumpIfError(hr, error, "Policy:SetTemplateName");

    pwszV1TemplateClass = pwszTemplateName;


    hr = pPolicy->AddV1TemplateNameExtension(pServer, pwszV1TemplateClass);
    _JumpIfError(hr, error, "AddTemplateNameExtension");

error:
    if (S_OK != hrTemplate)
    {
	hr = hrTemplate;	// override secondary errors

    }
    VariantClear(&varValue);

    if (NULL != pName)
    {
        LocalFree(pName);
    }
    if (NULL != pTemplate)
    {
        LocalFree(pTemplate);
    }
    SysFreeString(strTemplateObjId);
    SysFreeString(strTemplateName);
    SysFreeString(strTemplateRA);
    return(hr);
}




BOOL
CRequestInstance::_TemplateNamesMatch(
    IN WCHAR const *pwszTemplateName1,
    IN WCHAR const *pwszTemplateName2,
    OUT BOOL *pfTemplateMissing)
{
    HRESULT hr1;
    HRESULT hr2;
    BOOL fMatch = TRUE;

    *pfTemplateMissing = FALSE;

    if (0 == celstrcmpiL(pwszTemplateName1, pwszTemplateName2))
    {
	goto done;	// identical names
    }

    {
	hr1 = ceVerifyObjId(pwszTemplateName1);
	hr2 = ceVerifyObjId(pwszTemplateName2);
	if ((S_OK == hr1) ^ (S_OK == hr2))
	{
	    goto done;
	}
    }
    fMatch = FALSE;

done:
    return(fMatch);
}


//+--------------------------------------------------------------------------
// CRequestInstance::SetTemplateName
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

HRESULT
CRequestInstance::SetTemplateName(
    IN ICertServerPolicy *pServer,
    IN OPTIONAL WCHAR const *pwszTemplateName,
    IN OPTIONAL WCHAR const *pwszTemplateObjId)
{
    HRESULT hr;
    BSTR strProp = NULL;
    BSTR strTemplateName = NULL;

    if (NULL != pwszTemplateName)
    {
	m_strTemplateName = SysAllocString(pwszTemplateName);
	if (IsNullBStr(m_strTemplateName))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
	}
	strTemplateName = m_strTemplateName;
    }

    if (NULL != pwszTemplateObjId)
    {
	m_strTemplateObjId = SysAllocString(pwszTemplateObjId);
	if (IsNullBStr(m_strTemplateObjId))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
	}
	strTemplateName = m_strTemplateObjId;
    }

    if (!IsNullBStr(strTemplateName))
    {
	VARIANT var;

	strProp = SysAllocString(wszPROPCERTIFICATETEMPLATE);
	if (IsNullBStr(strProp))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
	}

	var.vt = VT_BSTR;
	var.bstrVal = strTemplateName;

	hr = pServer->SetCertificateProperty(strProp, PROPTYPE_STRING, &var);
	_JumpIfError(hr, error, "Policy:SetCertificateProperty");
    }
    hr = S_OK;

error:
    SysFreeString(strProp);
    return(hr);
}

