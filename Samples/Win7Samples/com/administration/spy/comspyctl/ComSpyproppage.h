// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __COMSPYPROPPAGE_H_
#define __COMSPYPROPPAGE_H_

#include "resource.h"       // main symbols

EXTERN_C const CLSID CLSID_ComSpyPropPage;

 
/////////////////////////////////////////////////////////////////////////////
// CComSpyPropPage
class ATL_NO_VTABLE CComSpyPropPage :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CComSpyPropPage, &CLSID_ComSpyPropPage>,
	public IPropertyPageImpl<CComSpyPropPage>,
	public CDialogImpl<CComSpyPropPage>
{


public:
	CComPtr<IComSpy> m_pSpy;

	CComSpyPropPage() 
	{
		m_dwTitleID = IDS_TITLEComSpyPropPage;
		m_dwHelpFileID = IDS_HELPFILEComSpyPropPage;
		m_dwDocStringID = IDS_DOCSTRINGComSpyPropPage;
		m_bDirty = FALSE;
	}

	enum {IDD = IDD_COMSPYPROPPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_COMSPYPROPPAGE)

BEGIN_COM_MAP(CComSpyPropPage) 
	COM_INTERFACE_ENTRY_IMPL(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CComSpyPropPage)
	CHAIN_MSG_MAP(IPropertyPageImpl<CComSpyPropPage>)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDC_COLUMN_NAMES, OnSelectColumn)
	COMMAND_HANDLER(IDC_WIDTH, EN_CHANGE, OnWidthChange)
	COMMAND_HANDLER(IDC_WIDTH, EN_CHANGE, OnLogFileChange)
	COMMAND_HANDLER(IDC_SHOW_GRID_LINES, BN_CLICKED, OnCheckBoxClick)
	COMMAND_HANDLER(IDC_LOG_TO_FILE, BN_CLICKED, OnCheckBoxClick)
	COMMAND_HANDLER(IDC_SHOW_ON_SCREEN, BN_CLICKED, OnCheckBoxClick)
END_MSG_MAP()

LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
LRESULT OnSelectColumn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

LRESULT OnWidthChange(WORD wNotify, WORD wID, HWND hWnd, BOOL& bHandled)
{
	SetDirty(TRUE);
	int nSel = (int)SendDlgItemMessage(IDC_COLUMN_NAMES, CB_GETCURSEL, 0, 0L);			   
	BOOL bSuccess;
	UINT nVal = GetDlgItemInt(IDC_WIDTH, &bSuccess, FALSE);
	SendDlgItemMessage(IDC_COLUMN_NAMES, CB_SETITEMDATA, nSel, nVal); // indicating that its width changed
	return 0;

}
LRESULT OnLogFileChange(WORD wNotify, WORD wID, HWND hWnd, BOOL& bHandled)
{
	SetDirty(TRUE);
	return 0;

}

LRESULT OnCheckBoxClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SetDirty(TRUE);
	return 0;
	
}
	STDMETHOD(Apply)(void);

};

#endif //__COMSPYPROPPAGE_H_
