//+--------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// File:        exit.h
//
// Contents:    CCertExitSample definition
//
//---------------------------------------------------------------------------

#include "certxsam.h"
#include "resource.h"       // main symbols


HRESULT
GetServerCallbackInterface(
    OUT ICertServerExit** ppServer,
    IN LONG Context);

HRESULT
exitGetProperty(
    IN ICertServerExit *pServer,
    IN BOOL fRequest,
    IN WCHAR const *pwszPropertyName,
    IN DWORD PropType,
    OUT VARIANT *pvarOut);

/////////////////////////////////////////////////////////////////////////////
// certexit

class CCertExitSample: 
    public CComDualImpl<ICertExit2, &IID_ICertExit2, &LIBID_CERTEXITSAMPLELib>, 
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<CCertExitSample, &CLSID_CCertExitSample>
{
public:
    CCertExitSample() 
    { 
        m_strDescription = NULL;
        m_strCAName = NULL;
        m_pwszRegStorageLoc = NULL;
        m_hExitKey = NULL;
        m_dwExitPublishFlags = 0;
        m_cCACert = 0;
    }
    ~CCertExitSample();

BEGIN_COM_MAP(CCertExitSample)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertExit)
    COM_INTERFACE_ENTRY(ICertExit2)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertExitSample) 

DECLARE_REGISTRY(
    CCertExitSample,
    wszCLASS_CERTEXITSAMPLE TEXT(".1"),
    wszCLASS_CERTEXITSAMPLE,
    IDS_CERTEXIT_DESC,
    THREADFLAGS_BOTH)

    // ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    // ICertExit
public:
    STDMETHOD(Initialize)( 
            /* [in] */ BSTR const strConfig,
            /* [retval][out] */ LONG __RPC_FAR *pEventMask);

    STDMETHOD(Notify)(
            /* [in] */ LONG ExitEvent,
            /* [in] */ LONG Context);

    STDMETHOD(GetDescription)( 
            /* [retval][out] */ BSTR *pstrDescription);

// ICertExit2
public:
    STDMETHOD(GetManageModule)(
		/* [out, retval] */ ICertManageModule **ppManageModule);

private:
    HRESULT _NotifyNewCert(IN LONG Context);

    HRESULT _NotifyCRLIssued(IN LONG Context);

    HRESULT _WriteCertToFile(
	    IN ICertServerExit *pServer,
	    IN BYTE const *pbCert,
	    IN DWORD cbCert);

    HRESULT _ExpandEnvironmentVariables(
	    __in LPCWSTR pwszIn,
	    __out_ecount(cwcOut) LPWSTR pwszOut,
	    IN DWORD cwcOut);

    // Member variables & private methods here:
    BSTR           m_strDescription;
    BSTR           m_strCAName;
    LPWSTR         m_pwszRegStorageLoc;
    HKEY           m_hExitKey;
    DWORD          m_dwExitPublishFlags;
    DWORD          m_cCACert;

};
