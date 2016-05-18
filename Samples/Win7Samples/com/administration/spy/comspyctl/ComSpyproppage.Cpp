// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "ComSpyCtl.h"
#include "ComSpyAudit.h"
#include "CComSpy.h"
#include "ComSpyPropPage.h"
#include "comsvcs.h"
#include "AppInfo.h"

const LPCWSTR pwszColNames[] = 
{
	L"Count",
	L"Event",
	L"TickCount",
	L"Application",
	L"Parameter",
	L"Value"
};
/////////////////////////////////////////////////////////////////////////////
// CComSpyPropPage

LRESULT CComSpyPropPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	bHandled = FALSE;

	m_ppUnk[0]->QueryInterface(IID_PPV_ARGS(&m_pSpy));
	_ASSERTE(m_pSpy);
	CComBSTR bstr;
	m_pSpy->get_LogFile(&bstr);
	SetDlgItemText(IDC_LOG_FILE, bstr);
	
	// grid lines
	BOOL bVal;
	m_pSpy->get_ShowGridLines(&bVal);
	SendDlgItemMessage(IDC_SHOW_GRID_LINES, BM_SETCHECK,  (bVal) ? BST_CHECKED : BST_UNCHECKED);

	// show on screen
	m_pSpy->get_ShowOnScreen(&bVal);
	SendDlgItemMessage(IDC_SHOW_ON_SCREEN, BM_SETCHECK,  (bVal) ? BST_CHECKED : BST_UNCHECKED);

	// log to file
	m_pSpy->get_LogToFile(&bVal);
	SendDlgItemMessage(IDC_LOG_TO_FILE, BM_SETCHECK,  (bVal) ? BST_CHECKED : BST_UNCHECKED);

	int i;
	for (i=0; i < ARRAYSIZE(pwszColNames); i++)
		SendDlgItemMessage(IDC_COLUMN_NAMES, CB_ADDSTRING, 0, (LPARAM)pwszColNames[i]);


	SendDlgItemMessage(IDC_COLUMN_NAMES, CB_SETCURSEL, 0, 0);
	BOOL bHandledIgnored = TRUE;
	OnSelectColumn(CBN_SELCHANGE, 0,0, bHandledIgnored);	
	return 0;
}

LRESULT CComSpyPropPage::OnSelectColumn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (wNotifyCode == CBN_SELCHANGE)
	{		
		int nSel = (int)SendDlgItemMessage(IDC_COLUMN_NAMES, CB_GETCURSEL, 0, 0L);			   
		if (nSel != CB_ERR)
		{
			// get the width

			long lWidth;
			_ASSERTE(m_pSpy);
			m_pSpy->get_ColWidth(nSel, &lWidth);
			SetDlgItemInt(IDC_WIDTH, lWidth);			
		}

	}

	return 0;
}


STDMETHODIMP CComSpyPropPage::Apply()
{
	ATLTRACE(L"CComSpyPropPage::Apply\n");
	for (UINT i = 0; i < m_nObjects; i++)
	{
		CComPtr<IComSpy> spSpy;
        HRESULT hr = m_ppUnk[i]->QueryInterface(IID_PPV_ARGS(&spSpy));
        _ASSERTE(SUCCEEDED(hr) && spSpy);

		BSTR bstr;
		GetDlgItemText(IDC_LOG_FILE, bstr);
		spSpy->put_LogFile(bstr);
		::SysFreeString(bstr);
		// grid lines
		BOOL bVal = (SendDlgItemMessage(IDC_SHOW_GRID_LINES, BM_GETCHECK) == BST_CHECKED);
		spSpy->put_ShowGridLines(bVal);	

		// log to file
		bVal = (SendDlgItemMessage(IDC_LOG_TO_FILE, BM_GETCHECK) == BST_CHECKED);
		spSpy->put_LogToFile(bVal);	

		// show on screen
		bVal = (SendDlgItemMessage(IDC_SHOW_ON_SCREEN, BM_GETCHECK) == BST_CHECKED);
		spSpy->put_ShowOnScreen(bVal);	

		int j;
		UINT nWidth;
		for (j=0; j < ARRAYSIZE(pwszColNames); j++)
		{
			
			nWidth = (UINT)SendDlgItemMessage(IDC_COLUMN_NAMES, CB_GETITEMDATA, j) ;
			if (nWidth)
			{
				spSpy->put_ColWidth(j,nWidth);
				SendDlgItemMessage(IDC_COLUMN_NAMES, CB_SETITEMDATA, j, 0); 
			}
		}

	}
	SetDirty(FALSE);
	m_bDirty = FALSE;
	return S_OK;
}
