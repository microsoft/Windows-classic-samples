//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//
//  File:       AuthenticateAdmin.h
//
//  Contents:
//
//--------------------------------------------------------------------------


#if !defined(AFX_AUTHENTICATEADMIN_H__64E4D8A6_C0B8_4A51_8275_FC89BFF207A4__INCLUDED_) 
#define AFX_AUTHENTICATEADMIN_H__64E4D8A6_C0B8_4A51_8275_FC89BFF207A4__INCLUDED_

#include "resource.h"       // main symbols
#include "AuthenticatePlugin.h"

/////////////////////////////////////////////////////////////////////////////
// CAuthenticateAdmin
class ATL_NO_VTABLE CAuthenticateAdmin : 
    public IDispatchImpl<IAuthenticateAdmin, &IID_IAuthenticateAdmin, &LIBID_AUTHENTICATEPLUGINLib, 9, 0 >,
    public ISupportErrorInfo,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAuthenticateAdmin, &CLSID_AuthenticateAdmin>
{
public:
    CAuthenticateAdmin() :
        m_pAuthenticatePlugin( NULL )
    {
    }

    STDMETHOD( Initialize )( CAuthenticatePlugin *pPlugin );


DECLARE_REGISTRY_RESOURCEID(IDR_AUTHENTICATEADMIN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAuthenticateAdmin)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IAuthenticateAdmin)
END_COM_MAP()

// ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IAuthenticateAdmin

public:

private:
    CAuthenticatePlugin   *m_pAuthenticatePlugin;

protected:
};

#endif // !defined(AFX_AUTHENTICATEADMIN_H__64E4D8A6_C0B8_4A51_8275_FC89BFF207A4__INCLUDED_)


