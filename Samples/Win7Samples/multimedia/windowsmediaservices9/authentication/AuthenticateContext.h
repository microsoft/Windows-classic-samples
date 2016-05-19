//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//
//  File:       AuthenticateContext.h
//
//  Contents:
//
//--------------------------------------------------------------------------


#if !defined(AFX_AUTHENTICATECONTEXT_H__D5FB96E8_39EB_4691_B4CA_6340014B116B__INCLUDED_) 
#define AFX_AUTHENTICATECONTEXT_H__D5FB96E8_39EB_4691_B4CA_6340014B116B__INCLUDED_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CAuthenticateContext
class ATL_NO_VTABLE CAuthenticateContext : 
        public CComObjectRootEx<CComMultiThreadModel>,
        public CComCoClass<CAuthenticateContext, &CLSID_AuthenticateContext>,
        public IWMSAuthenticationContext
{
public:
    CAuthenticateContext();
    ~CAuthenticateContext();

DECLARE_REGISTRY_RESOURCEID(IDR_AUTHENTICATECONTEXT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAuthenticateContext)
    COM_INTERFACE_ENTRY(IWMSAuthenticationContext)
END_COM_MAP()


public:
    STDMETHOD( Initialize )( IWMSAuthenticationPlugin* pAuthenticator );
    
    // IWMSAuthenticationContext methods
    STDMETHOD( GetAuthenticationPlugin )( IWMSAuthenticationPlugin **ppAuthenPlugin );
    STDMETHOD( Authenticate )(
                    VARIANT ResponseBlob,
                    IWMSContext *pUserCtx,
                    IWMSContext *pPresentationCtx,
                    IWMSCommandContext *pCommandContext,
                    IWMSAuthenticationCallback *pCallback,
                    VARIANT Context );
    STDMETHOD( GetLogicalUserID )( BSTR* bstrUserID );
    STDMETHOD( GetImpersonationAccountName )( BSTR* bstrAccountName );
    STDMETHOD( GetImpersonationToken )( long* Token );

private:
    BOOL IsValidUser( char* pszUserName, char* pszPassword );

    IWMSAuthenticationPlugin            *m_pWMSAuthenticationPlugin;
    CRITICAL_SECTION                    m_CritSec;
    HANDLE                              m_hToken;
    DWORD                               m_State;
    CComBSTR                            m_bstrUsername;
};

class CSafeArrayOfBytes {
public:
    CSafeArrayOfBytes(VARIANT* pVariant);

    ~CSafeArrayOfBytes();

    BOOL HasValidData();

    DWORD GetLength();

    BYTE* GetData();

    HRESULT SetData(BYTE* blob, DWORD len);

private:
    SAFEARRAY* m_psaBlob;
    BYTE* m_dataPtr;
    VARIANT* m_pVariant;
};

inline DWORD base64DecodeNeedLength( DWORD BufEncodedLen )
{
    return( ( ( BufEncodedLen + 3 ) / 4 ) * 3 );
}

#endif // !defined(AFX_AUTHENTICATECONTEXT_H__D5FB96E8_39EB_4691_B4CA_6340014B116B__INCLUDED_)
