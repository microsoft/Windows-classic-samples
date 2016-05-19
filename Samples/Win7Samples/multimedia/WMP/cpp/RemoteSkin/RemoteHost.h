// RemoteHost.h: interface for the RemoteHost class.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#if !defined(AFX_REMOTEHOST_H__B7D527C5_9FCD_47FB_A217_F081FDEBE700__INCLUDED_)
#define AFX_REMOTEHOST_H__B7D527C5_9FCD_47FB_A217_F081FDEBE700__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wmp.h"

class CRemoteHost : 
    public CComObjectRootEx<CComSingleThreadModel>, 
    public IServiceProvider, 
    public IWMPRemoteMediaServices  
{
public:
    CRemoteHost();
    virtual ~CRemoteHost();

    //DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CRemoteHost)
        COM_INTERFACE_ENTRY(IServiceProvider)
        COM_INTERFACE_ENTRY(IWMPRemoteMediaServices)
    END_COM_MAP()

    // IServiceProvider
    STDMETHOD(QueryService)(REFGUID /*guidService*/, REFIID riid, void ** ppv);
    // IWMPRemoteMediaServices
    STDMETHOD(GetServiceType)(BSTR * pbstrType);
    STDMETHOD(GetApplicationName)(BSTR * pbstrName);
    STDMETHOD(GetScriptableObject)(BSTR * pbstrName, IDispatch ** ppDispatch);
    STDMETHOD(GetCustomUIMode)(BSTR * pbstrFile);
};

#endif // !defined(AFX_REMOTEHOST_H__B7D527C5_9FCD_47FB_A217_F081FDEBE700__INCLUDED_)
