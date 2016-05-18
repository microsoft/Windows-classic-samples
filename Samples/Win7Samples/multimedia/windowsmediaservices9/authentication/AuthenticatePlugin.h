//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//
//  File:       AuthenticatePlugin.h
//
//  Contents:
//
//--------------------------------------------------------------------------


#if !defined(AFX_AUTHENTICATEPLUGIN_H__CDE2E83E_1C92_4439_BC67_6608A797BCCB__INCLUDED_) 
#define AFX_AUTHENTICATEPLUGIN_H__CDE2E83E_1C92_4439_BC67_6608A797BCCB__INCLUDED_

#include "resource.h"       // main symbols

#include "WMSContextNames.h"

// Forward declare the CAuthenticateAdmin interface
class CAuthenticateAdmin;

/////////////////////////////////////////////////////////////////////////////
// CAuthenticatePlugin
class ATL_NO_VTABLE CAuthenticatePlugin : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAuthenticatePlugin, &CLSID_AuthenticatePlugin>,
    public IWMSAuthenticationPlugin,
    public IWMSBasicPlugin,
    public IAuthenticatePlugin
{
public:
    CAuthenticatePlugin();
    ~CAuthenticatePlugin();

DECLARE_REGISTRY_RESOURCEID(IDR_AUTHENTICATEPLUGIN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAuthenticatePlugin)
        COM_INTERFACE_ENTRY(IWMSAuthenticationPlugin)
        COM_INTERFACE_ENTRY(IWMSBasicPlugin)
        COM_INTERFACE_ENTRY(IAuthenticatePlugin)
END_COM_MAP()

public:
    // IWMSBasicPlugin
    STDMETHOD( InitializePlugin )( IWMSContext *pServerContext, IWMSNamedValues *pNamedValues, IWMSClassObject *pClassFactory );
    STDMETHOD( OnHeartbeat )( );
    STDMETHOD( GetCustomAdminInterface )( IDispatch **ppValue );
    STDMETHOD( ShutdownPlugin )();
    STDMETHOD( EnablePlugin ) ( long *pdwFlags, long *pdwHeartbeatPeriod );
    STDMETHOD( DisablePlugin )();

    // IWMSAuthenticationPlugin
    STDMETHOD( GetPackageName )( BSTR *PackageName );
    STDMETHOD( GetProtocolName )( BSTR *ProtocolName );
    STDMETHOD( GetFlags )( long *Flags );
    STDMETHOD( CreateAuthenticationContext )( IWMSAuthenticationContext **ppAuthenCtx );

private:
    // Private Member Functions

private:
    // Private Member Variables
    CComPtr<IWMSNamedValues>   m_spNamedValues;
};


#endif // !defined(AFX_AUTHENTICATEPLUGIN_H__CDE2E83E_1C92_4439_BC67_6608A797BCCB__INCLUDED_)


