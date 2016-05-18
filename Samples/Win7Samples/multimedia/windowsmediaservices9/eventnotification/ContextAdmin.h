//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//
//  File:       ContextAdmin.h
//
//  Contents:
//
//--------------------------------------------------------------------------
#pragma once

#include "Resource.h"       // main symbols
#include "ContextPlugin.h"

/////////////////////////////////////////////////////////////////////////////
// CContextAdmin
class ATL_NO_VTABLE CContextAdmin : 
    public IDispatchImpl<IWMSContextAdmin, &IID_IWMSContextAdmin, &LIBID_CONTEXTPLUGINLib, 9, 0 >,
    public ISupportErrorInfo,
    public CComObjectRootEx<CComMultiThreadModel>
{
public:
    CContextAdmin()
    {
        m_pContextPlugin = NULL;
    }

    STDMETHOD( Initialize )( CContextPlugin *pPlugin );


DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CContextAdmin)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IWMSContextAdmin)
END_COM_MAP()

// ISupportErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IWMSContextAdmin
    STDMETHOD( put_OutputPath )( BSTR bstrOutputPath );
    STDMETHOD( get_OutputPath )( BSTR *pbstrOutputPath );
    STDMETHOD( put_ContextTypes )( WMS_CONTEXT_PLUGIN_CONTEXT_TYPE wmsContextTypes );
    STDMETHOD( get_ContextTypes )( WMS_CONTEXT_PLUGIN_CONTEXT_TYPE *pwmsContextTypes );

private:
    CContextPlugin   *m_pContextPlugin;

protected:
};

