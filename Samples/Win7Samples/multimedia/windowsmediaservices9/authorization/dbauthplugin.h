//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName: DBAuthPlugin.h
//
// Abstract:
//
//*****************************************************************************
#pragma once

#if !defined(AFX_DBAUTHPLUGIN_H__7F3F40ED_1634_4AB8_B2C8_1686B8BFC93A__INCLUDED_)
#define AFX_DBAUTHPLUGIN_H__7F3F40ED_1634_4AB8_B2C8_1686B8BFC93A__INCLUDED_

#include "resource.h"       // main symbols

// #import "WMSServerTypeLib.tlb" named_guids no_namespace raw_interfaces_only
#include "wmsdefs.h"

// Forward declare the CDBAuthAdmin interface
class CDBAuthAdmin;

class CSids
{
public:
    CSids() : m_cItems( 0 ), m_pSidList( NULL ) {};
    ~CSids() { CleanUp(); };
    void CleanUp( BOOL fFreeSid = TRUE );
    BOOL MatchSid( PSID pSid, DWORD *pdwIndex );
    HRESULT CopyAndAddSid( PSID pSid, CSids **ppSids );
    HRESULT CopyAndRemoveItem( DWORD dwIndex, CSids **ppSids );
    HRESULT SetAuthorizedUsers( IWMSNamedValues *pNamedValues );
    HRESULT GetAuthorizedUsers( IWMSNamedValues *pNamedValues );
    HRESULT GetUsers( BSTR *pbstrUserNames );

public:
    DWORD   m_cItems;
    PSID    *m_pSidList;
};

/////////////////////////////////////////////////////////////////////////////
// CDBAuthPlugin
class ATL_NO_VTABLE CDBAuthPlugin :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CDBAuthPlugin, &CLSID_DBAuthPlugin>,
    public IWMSEventAuthorizationPlugin,
    public IWMSBasicPlugin,
    public IDBAuthPlugin
{
public:
    CDBAuthPlugin();
    ~CDBAuthPlugin();

DECLARE_REGISTRY_RESOURCEID(IDR_DBAUTHPLUGIN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDBAuthPlugin)
        COM_INTERFACE_ENTRY(IWMSEventAuthorizationPlugin)
        COM_INTERFACE_ENTRY(IWMSBasicPlugin)
        COM_INTERFACE_ENTRY(IDBAuthPlugin)
END_COM_MAP()

public:
    // IWMSBasicPlugin
    STDMETHOD( InitializePlugin )( IWMSContext *pServerContext, IWMSNamedValues *pNamedValues, IWMSClassObject *pClassFactory );
    STDMETHOD( OnHeartbeat )( );
    STDMETHOD( GetCustomAdminInterface )( IDispatch **ppValue );
    STDMETHOD( ShutdownPlugin )();
    STDMETHOD( EnablePlugin ) ( long *pdwFlags, long *pdwHeartbeatPeriod );
    STDMETHOD( DisablePlugin )();

    // IWMSEventAuthorizationPlugin
    STDMETHOD( GetAuthorizedEvents )( VARIANT *pvarAuthorizedEvents );
    STDMETHOD( AuthorizeEvent )( WMS_EVENT *pEvent, IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx, IWMSEventAuthorizationCallback *pCallback, VARIANT Context );

    HRESULT AddUser( LPCWSTR pwszUserName );
    HRESULT RemoveUser( LPCWSTR pwszUserName );
    HRESULT VerifyUserMembership( LPCWSTR pwszUserName, BOOL *pfFound, DWORD *pdwIndex );
    HRESULT GetUsers( BSTR *pbstrUserNames );

private:
    // Authorization Helper Functions
    HRESULT OnAuthorizeOpen ( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );

    // Private Member Functions
    HRESULT CreateArrayOfEvents ( VARIANT *pvarEvents, WMS_EVENT_TYPE *pWMSEvents, long nNumEvents);

    BOOL VerifySidMembership( PSID pSid, DWORD *pdwIndex );
    void Lock();
    void Unlock();
    HRESULT GetSIDFromName( LPCWSTR pwszUserName, PSID *ppSid );

private:
    // Private Member Variables
    CComPtr<IWMSServer>         m_spServer;
    CComPtr<IWMSNamedValues>    m_spNamedValues;
    CSids                       *m_pSids;
    CRITICAL_SECTION            m_CritSec;
};


#endif // !defined(AFX_DBAUTHPLUGIN_H__7F3F40ED_1634_4AB8_B2C8_1686B8BFC93A__INCLUDED_)


