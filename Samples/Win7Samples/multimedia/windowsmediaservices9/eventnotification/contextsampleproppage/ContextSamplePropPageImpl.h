//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:  ContextSamplePropPageImpl.h
//
// Abstract:
//
//*****************************************************************************

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include "resource.h"
#include "WMSServer.h"
#include "ContextSamplePropPage.h"
#include "ContextDll.h"
#include <comdef.h>
#include <commctrl.h>


EXTERN_C const CLSID CLSID_ContextSamplePropPage;

/////////////////////////////////////////////////////////////////////////////
// CContextSamplePropPage
class ATL_NO_VTABLE CContextSamplePropPage :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CContextSamplePropPage, &CLSID_ContextSamplePropPage>,
	public IPropertyPageImpl<CContextSamplePropPage>,
	public CDialogImpl<CContextSamplePropPage>
{
public:
	CContextSamplePropPage();
	enum {IDD = IDD_CONTEXTSAMPLEPROPPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_CONTEXTSAMPLEPROPPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CContextSamplePropPage) 
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CContextSamplePropPage)
	COMMAND_HANDLER(IDC_EDIT_OUTPUT_PATH, EN_CHANGE, SetDirtyFlag )
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog )
    COMMAND_HANDLER( IDC_CHECK_USER_CONTEXT, BN_CLICKED, SetDirtyFlag )
    COMMAND_HANDLER( IDC_CHECK_PRESENTATION_CONTEXT, BN_CLICKED, SetDirtyFlag )
    COMMAND_HANDLER( IDC_CHECK_COMMAND_REQUEST_CONTEXT, BN_CLICKED, SetDirtyFlag )
    COMMAND_HANDLER( IDC_CHECK_COMMAND_RESPONSE_CONTEXT, BN_CLICKED, SetDirtyFlag )
	CHAIN_MSG_MAP(IPropertyPageImpl<CContextSamplePropPage>)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

private:
    HRESULT GetPluginSetting();
    HRESULT PopulateControls();
    HRESULT RetrieveDialogInformation();

	void MessageBox(UINT id);
	STDMETHOD	(Apply)(void);
	HRESULT		Connect(void);
	LRESULT		OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT		SetDirtyFlag(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
	    if( !m_fInitializing )
	    {
            SetDirty( TRUE );
	    }

	    return( 0 );
	};

    //
    //data members
    //
    CComQIPtr<IWMSContextAdmin> m_pPluginAdmin;
	BOOL m_fInitializing;
	CComBSTR m_bstrOrigOutputPath;
	CComBSTR m_bstrOutputPath;
	WMS_CONTEXT_PLUGIN_CONTEXT_TYPE m_wmsOrigContextTypes;
	WMS_CONTEXT_PLUGIN_CONTEXT_TYPE m_wmsContextTypes;
};
