//+--------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// File:        exit.cpp
//
// Contents:    CCertExitSample implementation
//
//---------------------------------------------------------------------------

#include "pch.cpp"
#pragma hdrstop

#include <assert.h>
#include "celib.h"

#pragma warning(push)
#pragma warning(disable : 4996) // to disable SDK warning from using deprecated APIs with ATL 7.0 and greater
#include "exit.h"
#include "module.h"
#pragma warning(pop)

BOOL fDebug = DBG_CERTSRV;

#ifndef DBG_CERTSRV
#error -- DBG_CERTSRV not defined!
#endif

#define ceEXITEVENTS \
    (EXITEVENT_CERTDENIED | \
    EXITEVENT_CERTISSUED | \
    EXITEVENT_CERTPENDING | \
    EXITEVENT_CERTRETRIEVEPENDING | \
    EXITEVENT_CERTREVOKED | \
    EXITEVENT_CRLISSUED | \
    EXITEVENT_SHUTDOWN | \
    EXITEVENT_CERTIMPORTED)


extern HINSTANCE g_hInstance;
extern LPWSTR g_pwszUnavailable;

HRESULT
GetServerCallbackInterface(
                           OUT ICertServerExit** ppServer,
                           IN LONG Context)
{
    HRESULT hr;

    if (NULL == ppServer)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "Exit:NULL pointer");
    }

    hr = CoCreateInstance(
        CLSID_CCertServerExit,
        NULL,               // pUnkOuter
        CLSCTX_INPROC_SERVER,
        IID_ICertServerExit,
        (VOID **) ppServer);
    _JumpIfError(hr, error, "Exit:CoCreateInstance");

    if (*ppServer == NULL)
    {
        hr = E_UNEXPECTED;
        _JumpError(hr, error, "Exit:NULL *ppServer");
    }

    // only set context if nonzero
    if (0 != Context)
    {
        hr = (*ppServer)->SetContext(Context);
        _JumpIfError(hr, error, "Exit: SetContext");
    }

error:
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertExitSample::~CCertExitSample -- destructor
//
// free memory associated with this instance
//+--------------------------------------------------------------------------

CCertExitSample::~CCertExitSample()
{
    SysFreeString(m_strCAName);
    if (NULL != m_pwszRegStorageLoc)
    {
        LocalFree(m_pwszRegStorageLoc);
    }
    if (NULL != m_hExitKey)
    {
        RegCloseKey(m_hExitKey);
    }
    SysFreeString(m_strDescription);
}


//+--------------------------------------------------------------------------
// CCertExitSample::Initialize -- initialize for a CA & return interesting Event Mask
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

STDMETHODIMP
CCertExitSample::Initialize(
                      /* [in] */ BSTR const strConfig,
                      /* [retval][out] */ LONG __RPC_FAR *pEventMask)
{
    HRESULT hr = S_OK;
    DWORD cbbuf;
    DWORD dwType;
    ENUM_CATYPES CAType;
    ICertServerExit *pServer = NULL;
    VARIANT varValue;
    WCHAR sz[MAX_PATH];
    size_t len;

    VariantInit(&varValue);

    assert(wcslen(wsz_SAMPLE_DESCRIPTION) < ARRAYSIZE(sz));
    StringCchCopy(sz, ARRAYSIZE(sz), wsz_SAMPLE_DESCRIPTION);
    sz[ARRAYSIZE(sz) - 1] = L'\0';

    m_strDescription = SysAllocString(sz);
    if (IsNullBStr(m_strDescription))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Exit:SysAllocString");
    }

    m_strCAName = SysAllocString(strConfig);
    if (IsNullBStr(m_strCAName))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Exit:SysAllocString");
    }

    *pEventMask = ceEXITEVENTS;
    DBGPRINT((fDebug, "Exit:Initialize(%ws) ==> %x\n", m_strCAName, *pEventMask));

    // get server callbacks

    hr = GetServerCallbackInterface(&pServer, 0);
    _JumpIfError(hr, error, "Exit:GetServerCallbackInterface");

    // get storage location

    hr = exitGetProperty(
        pServer,
        FALSE,	// fRequest
        wszPROPMODULEREGLOC,
        PROPTYPE_STRING,
        &varValue);
    _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszPROPMODULEREGLOC);

    len = wcslen(varValue.bstrVal) + 1;
    m_pwszRegStorageLoc = (LPWSTR)LocalAlloc(LMEM_FIXED, len *sizeof(WCHAR));
    if (NULL == m_pwszRegStorageLoc)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Exit:LocalAlloc");
    }
    StringCchCopy(m_pwszRegStorageLoc, len, varValue.bstrVal);
    VariantClear(&varValue);

    // get CA type
    hr = exitGetProperty(
        pServer,
        FALSE,	// fRequest
        wszPROPCATYPE,
        PROPTYPE_LONG,
        &varValue);
    _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszPROPCATYPE);

    CAType = (ENUM_CATYPES) varValue.lVal;
    VariantClear(&varValue);

    hr = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        m_pwszRegStorageLoc,
        0,              // dwReserved
        KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_QUERY_VALUE,
        &m_hExitKey);

    if (S_OK != hr)
    {
        if ((HRESULT) ERROR_FILE_NOT_FOUND == hr)
        {
            hr = S_OK;
            goto error;
        }
        _JumpError(hr, error, "Exit:RegOpenKeyEx");
    }

    hr = exitGetProperty(
        pServer,
        FALSE,	// fRequest
        wszPROPCERTCOUNT,
        PROPTYPE_LONG,
        &varValue);
    _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszPROPCERTCOUNT);

    m_cCACert = varValue.lVal;

    cbbuf = sizeof(m_dwExitPublishFlags);
    hr = RegQueryValueEx(
        m_hExitKey,
        wszREGCERTPUBLISHFLAGS,
        NULL,           // lpdwReserved
        &dwType,
        (BYTE *) &m_dwExitPublishFlags,
        &cbbuf);
    if (S_OK != hr)
    {
        m_dwExitPublishFlags = 0;
    }


    hr = S_OK;

error:
    VariantClear(&varValue);
    if (NULL != pServer)
    {
        pServer->Release();
    }
    return(ceHError(hr));
}


//+--------------------------------------------------------------------------
// CCertExitSample::_ExpandEnvironmentVariables -- Expand environment variables
//
//+--------------------------------------------------------------------------

HRESULT
CCertExitSample::_ExpandEnvironmentVariables(
                                       __in LPCWSTR pwszIn,
                                       __out_ecount(cwcOut) LPWSTR pwszOut,
                                       IN DWORD cwcOut)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    WCHAR awcVar[MAX_PATH];
    LPCWSTR pwszSrc;
    WCHAR *pwszDst;
    WCHAR *pwszVar;
    DWORD cwc;

    pwszSrc = pwszIn;
    pwszDst = pwszOut;
    WCHAR* const pwszDstEnd = &pwszOut[cwcOut] ;

    while (L'\0' != (*pwszDst = *pwszSrc++))
    {
        if ('%' == *pwszDst)
        {
            *pwszDst = L'\0';
            pwszVar = awcVar;

            while (L'\0' != *pwszSrc)
            {
                if ('%' == *pwszSrc)
                {
                    pwszSrc++;
                    break;
                }
                *pwszVar++ = *pwszSrc++;
                if (pwszVar >= &awcVar[sizeof(awcVar)/sizeof(awcVar[0]) - 1])
                {
                    _JumpError(hr, error, "Exit:overflow 1");
                }
            }
            *pwszVar = L'\0';
            cwc = GetEnvironmentVariable(awcVar, pwszDst, SAFE_SUBTRACT_POINTERS(pwszDstEnd, pwszDst));
            if (0 == cwc)
            {
                hr = ceHLastError();
                _JumpError(hr, error, "Exit:GetEnvironmentVariable");
            }
            if ((DWORD) (pwszDstEnd - pwszDst) <= cwc)
            {
                _JumpError(hr, error, "Exit:overflow 2");
            }
            pwszDst += cwc;
        }
        else
        {
            pwszDst++;
        }
        if (pwszDst >= pwszDstEnd)
        {
            _JumpError(hr, error, "Exit:overflow 3");
        }
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
exitGetRequestAttribute(
                        IN ICertServerExit *pServer,
                        IN WCHAR const *pwszAttributeName,
                        OUT BSTR *pstrOut)
{
    HRESULT hr;
    BSTR strName = NULL;

    *pstrOut = NULL;
    strName = SysAllocString(pwszAttributeName);
    if (IsNullBStr(strName))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Exit:SysAllocString");
    }
    hr = pServer->GetRequestAttribute(strName, pstrOut);
    _JumpIfErrorStr2(
        hr,
        error,
        "Exit:GetRequestAttribute",
        pwszAttributeName,
        CERTSRV_E_PROPERTY_EMPTY);

error:
    SysFreeString(strName);
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertExitSample::_WriteCertToFile -- write binary certificate to a file
//
//+--------------------------------------------------------------------------

HRESULT
CCertExitSample::_WriteCertToFile(
                            IN ICertServerExit *pServer,
                            IN BYTE const *pbCert,
                            IN DWORD cbCert)
{
    HRESULT hr;
    BSTR strCertFile = NULL;
    DWORD cbWritten;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WCHAR wszDir[MAX_PATH];
    WCHAR *pwszPath = NULL;
    WCHAR wszFile[cwcDWORDSPRINTF+5]; //format "requestid.cer"
    VARIANT varRequestID;

    VariantInit(&varRequestID);


    hr = exitGetRequestAttribute(pServer, wszPROPEXITCERTFILE, &strCertFile);
    if (S_OK != hr)
    {
        DBGPRINT((
            fDebug,
            "Exit:exitGetRequestAttribute(%ws): %x%hs\n",
            wszPROPEXITCERTFILE,
            hr,
            CERTSRV_E_PROPERTY_EMPTY == hr? " EMPTY VALUE" : ""));
        if (CERTSRV_E_PROPERTY_EMPTY == hr)
        {
            hr = S_OK;
        }
        goto error;
    }

    // build file name as "requestid.cer"

    hr = exitGetProperty(
        pServer,
        TRUE,  // fRequest,
        wszPROPREQUESTREQUESTID,
        PROPTYPE_LONG,
        &varRequestID);
    _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszPROPREQUESTREQUESTID);

    StringCbPrintf(wszFile, sizeof(wszFile), L"%d.cer", V_I4(&varRequestID));

    hr = _ExpandEnvironmentVariables(
        L"%SystemRoot%\\System32\\" wszCERTENROLLSHAREPATH L"\\",
        wszDir,
        ARRAYSIZE(wszDir));
    _JumpIfError(hr, error, "_ExpandEnvironmentVariables");

    hr = ceBuildPathAndExt(wszDir, wszFile, NULL, &pwszPath);
    _JumpIfError(hr, error, "ceBuildPathAndExt");

    // open file & write binary cert out.

    hFile = CreateFile(
        pwszPath,
        GENERIC_WRITE,
        0,			// dwShareMode
        NULL,		// lpSecurityAttributes
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL);		// hTemplateFile
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = ceHLastError();
        _JumpErrorStr(hr, error, "Exit:CreateFile", pwszPath);
    }
    if (!WriteFile(hFile, pbCert, cbCert, &cbWritten, NULL))
    {
        hr = ceHLastError();
        _JumpErrorStr(hr, error, "Exit:WriteFile", pwszPath);
    }
    if (cbWritten != cbCert)
    {
        hr = STG_E_WRITEFAULT;
        DBGPRINT((
            fDebug,
            "Exit:WriteFile(%ws): attempted %x, actual %x bytes: %x\n",
            pwszPath,
            cbCert,
            cbWritten,
            hr));
        goto error;
    }

error:

    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }
    if (NULL != pwszPath)
    {
        LocalFree(pwszPath);
    }
    SysFreeString(strCertFile);
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertExitSample::_NotifyNewCert -- Notify the exit module of a new certificate
//
//+--------------------------------------------------------------------------

HRESULT
CCertExitSample::_NotifyNewCert(
                          /* [in] */ LONG Context)
{
    HRESULT hr;
    VARIANT varCert;
    ICertServerExit *pServer = NULL;

    VariantInit(&varCert);

    // only call write fxns if server policy allows

    if (m_dwExitPublishFlags & EXITPUB_FILE)
    {
        hr = CoCreateInstance(
            CLSID_CCertServerExit,
            NULL,               // pUnkOuter
            CLSCTX_INPROC_SERVER,
            IID_ICertServerExit,
            (VOID **) &pServer);
        _JumpIfError(hr, error, "Exit:CoCreateInstance");

        hr = pServer->SetContext(Context);
        _JumpIfError(hr, error, "Exit:SetContext");

        hr = exitGetProperty(
            pServer,
            FALSE,	// fRequest,
            wszPROPRAWCERTIFICATE,
            PROPTYPE_BINARY,
            &varCert);
        _JumpIfErrorStr(
            hr,
            error,
            "Exit:exitGetProperty",
            wszPROPRAWCERTIFICATE);

        if (VT_BSTR != varCert.vt)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            _JumpError(hr, error, "Exit:BAD cert var type");
        }

        hr = _WriteCertToFile(
            pServer,
            (BYTE const *) varCert.bstrVal,
            SysStringByteLen(varCert.bstrVal));
        _JumpIfError(hr, error, "_WriteCertToFile");
    }

    hr = S_OK;

error:
    VariantClear(&varCert);
    if (NULL != pServer)
    {
        pServer->Release();
    }
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertExitSample::_NotifyCRLIssued -- Notify the exit module of a new certificate
//
//+--------------------------------------------------------------------------

HRESULT
CCertExitSample::_NotifyCRLIssued(
                            /* [in] */ LONG Context)
{
    HRESULT hr;
    ICertServerExit *pServer = NULL;
    DWORD i;
    VARIANT varBaseCRL;
    VARIANT varDeltaCRL;
    BOOL fDeltaCRLsDisabled;

    VariantInit(&varBaseCRL);
    VariantInit(&varDeltaCRL);

    hr = CoCreateInstance(
        CLSID_CCertServerExit,
        NULL,               // pUnkOuter
        CLSCTX_INPROC_SERVER,
        IID_ICertServerExit,
        (VOID **) &pServer);
    _JumpIfError(hr, error, "Exit:CoCreateInstance");

    hr = pServer->SetContext(Context);
    _JumpIfError(hr, error, "Exit:SetContext");


    hr = exitGetProperty(
        pServer,
        FALSE,	// fRequest,
        wszPROPDELTACRLSDISABLED,
        PROPTYPE_LONG,
        &varBaseCRL);
    _JumpIfErrorStr(
        hr,
        error,
        "Exit:exitGetProperty",
        wszPROPDELTACRLSDISABLED);

    fDeltaCRLsDisabled = varBaseCRL.lVal;

    // How many CRLs are there?

    // Loop for each CRL
    for (i = 0; i < m_cCACert; i++)
    {
        // array size for wsprintf("%s.%u")
#define MAX_CRL_PROP \
    (max( \
    max(ARRAYSIZE(wszPROPCRLSTATE), ARRAYSIZE(wszPROPRAWCRL)), \
    ARRAYSIZE(wszPROPRAWDELTACRL)) + \
    1 + cwcDWORDSPRINTF)

        WCHAR wszCRLPROP[MAX_CRL_PROP];

        // Verify the CRL State says we should update this CRL

        StringCbPrintf(wszCRLPROP, sizeof(wszCRLPROP), wszPROPCRLSTATE L".%u", i);
        hr = exitGetProperty(
            pServer,
            FALSE,	// fRequest,
            wszCRLPROP,
            PROPTYPE_LONG,
            &varBaseCRL);
        _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszCRLPROP);

        if (CA_DISP_VALID != varBaseCRL.lVal)
        {
            continue;
        }

        // Grab the raw base CRL

        StringCbPrintf(wszCRLPROP, sizeof(wszCRLPROP), wszPROPRAWCRL L".%u", i);
        hr = exitGetProperty(
            pServer,
            FALSE,	// fRequest,
            wszCRLPROP,
            PROPTYPE_BINARY,
            &varBaseCRL);
        _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszCRLPROP);

        // Grab the raw delta CRL (which may not exist)

        StringCbPrintf(wszCRLPROP, sizeof(wszCRLPROP), wszPROPRAWDELTACRL L".%u", i);
        hr = exitGetProperty(
            pServer,
            FALSE,	// fRequest,
            wszCRLPROP,
            PROPTYPE_BINARY,
            &varDeltaCRL);
        _PrintIfErrorStr2(
            hr,
            "Exit:exitGetProperty",
            wszCRLPROP,
            fDeltaCRLsDisabled?
            HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) : S_OK);
        if (S_OK != hr && !fDeltaCRLsDisabled)
        {
            goto error;
        }

        // Publish the CRL(s) ...
    }

    hr = S_OK;

error:
    if (NULL != pServer)
    {
        pServer->Release();
    }
    VariantClear(&varBaseCRL);
    VariantClear(&varDeltaCRL);
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertExitSample::Notify -- Notify the exit module of an event
//
// Returns S_OK.
//+--------------------------------------------------------------------------

STDMETHODIMP
CCertExitSample::Notify(
                  /* [in] */ LONG ExitEvent,
                  /* [in] */ LONG Context)
{
    char *psz = "UNKNOWN EVENT";
    HRESULT hr = S_OK;

    switch (ExitEvent)
    {
    case EXITEVENT_CERTISSUED:
        hr = _NotifyNewCert(Context);
        psz = "certissued";
        break;

    case EXITEVENT_CERTPENDING:
        psz = "certpending";
        break;

    case EXITEVENT_CERTDENIED:
        psz = "certdenied";
        break;

    case EXITEVENT_CERTREVOKED:
        psz = "certrevoked";
        break;

    case EXITEVENT_CERTRETRIEVEPENDING:
        psz = "retrievepending";
        break;

    case EXITEVENT_CRLISSUED:
        hr = _NotifyCRLIssued(Context);
        psz = "crlissued";
        break;

    case EXITEVENT_SHUTDOWN:
        psz = "shutdown";
        break;

    case EXITEVENT_CERTIMPORTED:
        psz = "certimported";
        break;

    }


    DBGPRINT((
        fDebug,
        "Exit:Notify(%hs=%x, ctx=%x) rc=%x\n",
        psz,
        ExitEvent,
        Context,
        hr));
    return(hr);
}


STDMETHODIMP
CCertExitSample::GetDescription(
                          /* [retval][out] */ BSTR *pstrDescription)
{
    HRESULT hr = S_OK;
    WCHAR sz[MAX_PATH];

    assert(wcslen(wsz_SAMPLE_DESCRIPTION) < ARRAYSIZE(sz));
    StringCbCopy(sz, sizeof(sz), wsz_SAMPLE_DESCRIPTION);

    *pstrDescription = SysAllocString(sz);
    if (IsNullBStr(*pstrDescription))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Exit:SysAllocString");
    }

error:
    return(hr);
}


//+--------------------------------------------------------------------------
// CCertExitSample::GetManageModule
//
// Returns S_OK on success.
//+--------------------------------------------------------------------------

STDMETHODIMP
CCertExitSample::GetManageModule(
                           /* [out, retval] */ ICertManageModule **ppManageModule)
{
    HRESULT hr;

    *ppManageModule = NULL;
    hr = CoCreateInstance(
        CLSID_CCertManageExitModuleSample,
        NULL,               // pUnkOuter
        CLSCTX_INPROC_SERVER,
        IID_ICertManageModule,
        (VOID **) ppManageModule);
    _JumpIfError(hr, error, "CoCreateInstance");

error:
    return(hr);
}


/////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP
CCertExitSample::InterfaceSupportsErrorInfo(REFIID riid)
{
    int i;
    static const IID *arr[] =
    {
        &IID_ICertExit,
    };

    for (i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
    {
        if (IsEqualGUID(*arr[i],riid))
        {
            return(S_OK);
        }
    }
    return(S_FALSE);
}


HRESULT
exitGetProperty(
                IN ICertServerExit *pServer,
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
        _JumpError(hr, error, "Exit:SysAllocString");
    }
    if (fRequest)
    {
        hr = pServer->GetRequestProperty(strName, PropType, pvarOut);
        _JumpIfErrorStr2(
            hr,
            error,
            "Exit:GetRequestProperty",
            pwszPropertyName,
            CERTSRV_E_PROPERTY_EMPTY);
    }
    else
    {
        hr = pServer->GetCertificateProperty(strName, PropType, pvarOut);
        _JumpIfErrorStr2(
            hr,
            error,
            "Exit:GetCertificateProperty",
            pwszPropertyName,
            CERTSRV_E_PROPERTY_EMPTY);
    }

error:
    SysFreeString(strName);
    return(hr);
}

