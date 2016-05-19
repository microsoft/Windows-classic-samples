//+-------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//  File:       policy.h
//
//--------------------------------------------------------------------------

#include "certpsam.h"
#include "resource.h"

#ifndef wszATTREMAIL1
# define wszATTREMAIL1			TEXT("E")
# define wszATTREMAIL2			TEXT("EMail")
#endif

#ifndef wszCERTTYPE_SUBORDINATE_CA
# define wszCERTTYPE_SUBORDINATE_CA	L"SubCA"
#endif

#ifndef wszCERTTYPE_CROSS_CA
# define wszCERTTYPE_CROSS_CA		L"CrossCA"
#endif

#ifndef CT_FLAG_DONOTPERSISTINDB
#define CT_FLAG_DONOTPERSISTINDB    (0x00000400)
#endif


extern BOOL fDebug;

HRESULT
ReqInitialize(
    IN ICertServerPolicy *pServer);

VOID
ReqCleanup(VOID);


class CRequestInstance;

#ifndef __BSTRC__DEFINED__
#define __BSTRC__DEFINED__
typedef OLECHAR const *BSTRC;
#endif

HRESULT
polGetServerCallbackInterface(
    OUT ICertServerPolicy **ppServer,
    IN LONG Context);

HRESULT
polGetRequestStringProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT BSTR *pstrOut);

HRESULT
polGetCertificateStringProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT BSTR *pstrOut);

HRESULT
polGetRequestLongProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT LONG *plOut);

HRESULT
polGetCertificateLongProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT LONG *plOut);

HRESULT
polGetRequestAttribute(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszAttributeName,
    OUT BSTR *pstrOut);

HRESULT
polGetCertificateExtension(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszExtensionName,
    IN DWORD dwPropType,
    IN OUT VARIANT *pvarOut);

HRESULT
polSetCertificateExtension(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszExtensionName,
    IN DWORD dwPropType,
    IN DWORD dwExtFlags,
    IN VARIANT const *pvarIn);

HRESULT
polSetCertificateStringProperty(
    __in ICertServerPolicy *pServer,
    __in LPCWSTR pwszPropName,
    __in_opt BSTR strPropertyValue);

HRESULT
polSetCertificateLongProperty(
    __in ICertServerPolicy *pServer,
    __in LPCWSTR pwszPropName,
    __in LONG lVal);

DWORD
polFindObjIdInList(
    IN WCHAR const *pwsz,
    IN DWORD count,
    IN WCHAR const * const *ppwsz);

// 
// Class CCertPolicySample
// 
// Actual policy module for a CA Policy
//
//

class CCertPolicySample: 
    public CComDualImpl<ICertPolicy2, &IID_ICertPolicy2, &LIBID_CERTPOLICYSAMPLELib>, 
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<CCertPolicySample, &CLSID_CCertPolicySample>
{
public:
    CCertPolicySample()
    {
	m_strDescription = NULL;

        // RevocationExtension variables:

	m_dwRevocationFlags = 0;
	m_wszASPRevocationURL = NULL;

        m_dwDispositionFlags = 0;
        m_dwEditFlags = 0;

	m_cEnableRequestExtensions = 0;
	m_apwszEnableRequestExtensions = NULL;

	m_cEnableEnrolleeRequestExtensions = 0;
	m_apwszEnableEnrolleeRequestExtensions = NULL;

	m_cEKUOIDsForVolatileRequests = 0;
	m_apszEKUOIDsForVolatileRequests = NULL;
	m_fVolatileMode = FALSE; 
	

	m_cDisableExtensions = 0;
	m_apwszDisableExtensions = NULL;

	// CA Name
        m_strRegStorageLoc = NULL;

	m_strCAName = NULL;
        m_strCASanitizedName = NULL;
        m_strCASanitizedDSName = NULL;
        m_strMachineDNSName = NULL;

        // CA and cert type info

        m_CAType = ENUM_UNKNOWN_CA;

        m_pCert = NULL;
        m_iCRL = 0;

    }
    ~CCertPolicySample();

BEGIN_COM_MAP(CCertPolicySample)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertPolicy)
    COM_INTERFACE_ENTRY(ICertPolicy2)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertPolicySample) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation.  The default is to support it

DECLARE_REGISTRY(
    CCertPolicySample,
    wszCLASS_CERTPOLICYSAMPLE TEXT(".1"),
    wszCLASS_CERTPOLICYSAMPLE,
    IDS_CERTPOLICY_DESC,
    THREADFLAGS_BOTH)

// ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// ICertPolicy
public:
    STDMETHOD(Initialize)( 
		/* [in] */ BSTR const strConfig);

    STDMETHOD(VerifyRequest)( 
		/* [in] */ BSTR const strConfig,
		/* [in] */ LONG Context,
		/* [in] */ LONG bNewRequest,
		/* [in] */ LONG Flags,
		/* [out, retval] */ LONG __RPC_FAR *pDisposition);

    STDMETHOD(GetDescription)( 
		/* [out, retval] */ BSTR __RPC_FAR *pstrDescription);

    STDMETHOD(ShutDown)();

// ICertPolicy2
public:
    STDMETHOD(GetManageModule)(
		/* [out, retval] */ ICertManageModule **ppManageModule);

public:
    HRESULT AddBasicConstraintsCommon(
		IN ICertServerPolicy *pServer,
		IN CERT_EXTENSION const *pExtension,
		IN BOOL fCA,
		IN BOOL fEnableExtension);

    BSTRC GetPolicyDescription() { return(m_strDescription); }


    HRESULT AddV1TemplateNameExtension(
		IN ICertServerPolicy *pServer,
		OPTIONAL IN WCHAR const *pwszTemplateName);

private:
    CERT_CONTEXT const *_GetIssuer(
		IN ICertServerPolicy *pServer);

    HRESULT _EnumerateExtensions(
		IN ICertServerPolicy *pServer,
		IN LONG bNewRequest,
		IN BOOL fFirstPass,
		IN BOOL fEnableEnrolleeExtensions,
		IN DWORD cCriticalExtensions,
		IN WCHAR const * const *apwszCriticalExtensions);

#if DBG_CERTSRV
    VOID _DumpStringArray(
	__in PCSTR pszType,
	__in DWORD count,
	__in_ecount(count) LPWSTR const *apwsz);
#else
    #define _DumpStringArray(pszType, count, apwsz)
#endif

    VOID _FreeStringArray(
        __inout DWORD *pcString,
        __inout LPWSTR **papwsz);

    VOID _FreeStringArray(
        __inout DWORD *pcString,
        __inout LPSTR **papsz);

    VOID _Cleanup();


    HRESULT _SetSystemStringProp(
		IN ICertServerPolicy *pServer,
		IN WCHAR const *pwszName,
		OPTIONAL IN WCHAR const *pwszValue);

    HRESULT _AddStringArray(
        __in CSPCZZWSTR pwszzValue,
        IN BOOL fURL,
        __inout DWORD *pcStrings,
        __deref_inout_ecount(*pcStrings) LPWSTR **papwszRegValues);

    HRESULT _ReadRegistryString(
        IN HKEY hkey,
        IN BOOL fURL,
        __in WCHAR const *pwszRegName,
        __in WCHAR const *pwszSuffix,
        __deref_out LPWSTR *ppwszOut);

    HRESULT _ReadRegistryMultiString(
        __in HKEY hkey,
        __in LPCWSTR pwszRegName,
        __deref_out PZPWSTR  ppwszzRegValue);

    HRESULT _ReadRegistryStringArray(
        IN HKEY hkey,
        IN BOOL fURL,
        IN DWORD dwFlags,
        IN DWORD cRegNames,
        __in DWORD *aFlags,
        __in_ecount(cRegNames) LPCWSTR const *apwszRegNames,
        __inout DWORD *pcStrings,
        __deref_inout_ecount(*pcStrings) LPWSTR **papwszRegValues);

    VOID _InitRevocationExtension(
		IN HKEY hkey);

    VOID _InitRequestExtensionList(
		IN HKEY hkey);

    VOID _InitDisableExtensionList(
		IN HKEY hkey);

    HRESULT _InitEKUExtensionListForVolatileMode(
		IN HKEY hkey
		);

    HRESULT _AddRevocationExtension(
		IN ICertServerPolicy *pServer);

    HRESULT _AddOldCertTypeExtension(
		IN ICertServerPolicy *pServer,
		IN BOOL fCA);

    HRESULT _AddAuthorityKeyId(
		IN ICertServerPolicy *pServer);

    HRESULT _AddDefaultKeyUsageExtension(
		IN ICertServerPolicy *pServer,
		IN BOOL fCA);

    HRESULT _AddEnhancedKeyUsageExtension(
		IN ICertServerPolicy *pServer);

    HRESULT _AddDefaultBasicConstraintsExtension(
		IN ICertServerPolicy *pServer,
		IN BOOL fCA);

    HRESULT _SetValidityPeriod(
		IN ICertServerPolicy *pServer);

    HRESULT _PersistRequest(
		IN ICertServerPolicy *pServer);


private:
    // RevocationExtension variables:

    CERT_CONTEXT const *m_pCert;

    BSTR m_strDescription;

    DWORD m_dwRevocationFlags;
    LPWSTR m_wszASPRevocationURL;

    DWORD m_dwDispositionFlags;
    DWORD m_dwEditFlags;
    DWORD m_CAPathLength;

    DWORD m_cEnableRequestExtensions;
    LPWSTR *m_apwszEnableRequestExtensions;

    DWORD m_cEnableEnrolleeRequestExtensions;
    LPWSTR *m_apwszEnableEnrolleeRequestExtensions;

    DWORD m_cDisableExtensions;
    LPWSTR *m_apwszDisableExtensions;

    DWORD m_cEKUOIDsForVolatileRequests;
    LPSTR *m_apszEKUOIDsForVolatileRequests;
    BOOL m_fMatchAnyEKU;

    // CertTypeExtension variables:

    BSTR m_strRegStorageLoc;
    BSTR m_strCAName;

    BSTR m_strCASanitizedName;
    BSTR m_strCASanitizedDSName;

    BSTR m_strMachineDNSName;

    // CA and cert type info

    ENUM_CATYPES m_CAType;

    DWORD m_iCert;
    DWORD m_iCRL;


    // Database properties
    BOOL m_fVolatileMode;

};

// 
// Class CRequestInstance
// 
// Instance data for a certificate that is being created.
//

class CRequestInstance
{

public:
    CRequestInstance()
    {
        m_strTemplateName = NULL;
	m_strTemplateObjId = NULL;
	m_pPolicy = NULL;

    }

    ~CRequestInstance();

    HRESULT Initialize(
		IN CCertPolicySample *pPolicy,
		IN ICertServerPolicy *pServer,
		OUT BOOL *pfEnableEnrolleeExtensions);

    HRESULT SetTemplateName(
		IN ICertServerPolicy *pServer,
		IN OPTIONAL WCHAR const *pwszTemplateName,
		IN OPTIONAL WCHAR const *pwszTemplateObjId);

    BSTRC GetTemplateName() { return(m_strTemplateName); }
    BSTRC GetTemplateObjId() { return(m_strTemplateObjId); }


    BOOL IsCARequest() { return(m_fCA); }

    CCertPolicySample *GetPolicy() { return(m_pPolicy); }

private:

    HRESULT _SetFlagsProperty(
		IN ICertServerPolicy *pServer,
		IN WCHAR const *pwszPropName,
		IN DWORD dwFlags);

    BOOL _TemplateNamesMatch(
		IN WCHAR const *pwszTemplateName1,
		IN WCHAR const *pwszTemplateName2,
		OUT BOOL *pfTemplateMissing);

    VOID _Cleanup();		
private:			
    CCertPolicySample *m_pPolicy;
    BSTR                   m_strTemplateName;	// certificate type requested
    BSTR                   m_strTemplateObjId;	// certificate type requested
    DWORD                  m_dwTemplateMajorVersion;
    DWORD                  m_dwTemplateMinorVersion;
    BOOL                   m_fCA;
    BOOL                   m_fROBORequest;
};
