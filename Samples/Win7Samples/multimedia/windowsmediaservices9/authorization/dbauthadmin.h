//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName: DBAuthAdmin.h
//
// Abstract:
//
//*****************************************************************************
#pragma once

#if !defined(AFX_DBAUTHADMIN_H__4FA9EA92_4802_416B_8134_11B639C8217D__INCLUDED_)
#define AFX_DBAUTHADMIN_H__4FA9EA92_4802_416B_8134_11B639C8217D__INCLUDED_

#include "resource.h"       // main symbols
#include "DBAuthPlugin.h"

/////////////////////////////////////////////////////////////////////////////
// CDBAuthAdmin
class ATL_NO_VTABLE CDBAuthAdmin :
    public IDispatchImpl<IDBAuthAdmin, &IID_IDBAuthAdmin, &LIBID_DBAUTHPLUGINLib, 9, 0 >,
    public ISupportErrorInfo,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CDBAuthAdmin, &CLSID_DBAuthAdmin>
{
public:
    CDBAuthAdmin()
    {
        m_pDBAuthPlugin = NULL;
    }

    ~CDBAuthAdmin()
    {
    }

    STDMETHOD( Initialize )( CDBAuthPlugin *pPlugin );


DECLARE_REGISTRY_RESOURCEID( IDR_DBAUTHADMIN )

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP( CDBAuthAdmin )
    COM_INTERFACE_ENTRY( IDispatch )
    COM_INTERFACE_ENTRY( ISupportErrorInfo )
    COM_INTERFACE_ENTRY( IDBAuthAdmin )
END_COM_MAP()

// ISupportsErrorInfo
    STDMETHOD( InterfaceSupportsErrorInfo )( REFIID riid );

// IDBAuthAdmin
public:
    STDMETHOD( AddUser )( BSTR bstrUserName );
    STDMETHOD( RemoveUser )( BSTR bstrUserName );
    STDMETHOD( VerifyUser )( BSTR bstrUserName, /*[out, retval]*/ VARIANT_BOOL *pbFound );
    STDMETHOD( GetUsers )( /*[out, retval]*/ BSTR *pbstrUserNames );
private:
    CDBAuthPlugin       *m_pDBAuthPlugin;

};

#endif // !defined(AFX_DBAUTHADMIN_H__4FA9EA92_4802_416B_8134_11B639C8217D__INCLUDED_)


