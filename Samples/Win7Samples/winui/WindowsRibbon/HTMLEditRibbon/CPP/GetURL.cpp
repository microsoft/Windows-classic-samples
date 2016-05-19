// GetURL.cpp : implementation file
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "HTMLEdit.h"
#include "GetURL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGetURL dialog


CGetURL::CGetURL(CWnd* pParent /*=NULL*/)
	: CDialog(CGetURL::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGetURL)
	m_URL = _T("");
	//}}AFX_DATA_INIT
}


void CGetURL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetURL)

	//}}AFX_DATA_MAP
	if(::IsWindow(m_AddrCombo.m_hWnd))
		DDX_Text(pDX, IDC_ADDRCOMBO, m_URL);
}


BEGIN_MESSAGE_MAP(CGetURL, CDialog)
	//{{AFX_MSG_MAP(CGetURL)
	//}}AFX_MSG_MAP
	ON_CONTROL(CBN_CLOSEUP,IDC_ADDRCOMBO,OnCloseup)
	ON_CONTROL(BN_CLICKED,IDC_BTNGO,OnGo)
	ON_CONTROL(BN_CLICKED,IDC_BTN_SEARCH, OnSearch)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CGetURL, CDialog)
	ON_EVENT(CGetURL, IDC_EXPLORER, 0x00000103, OnDocumentComplete, VTS_DISPATCH VTS_VARIANT)
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CGetURL message handlers
void CGetURL::OnDocumentComplete(LPDISPATCH /*pDisp*/, LPVARIANT pURL)
{
	CString szURL(COLE2T(pURL->pvarVal->bstrVal));
	if(::IsWindow(m_AddrCombo.m_hWnd))
	{
		m_AddrCombo.SetWindowText(szURL);
		m_szCurAddr = szURL;
	}
	
}
void CGetURL::OnOK() 
{
	UpdateData();
	CDialog::OnOK();
}
void CGetURL::OnSearch()
{
	if(m_spBrowser)
		m_spBrowser->Navigate(L"http://www.yahoo.com",NULL,NULL,NULL,NULL);
}
BOOL CGetURL::OnInitDialog() 
{
	CDialog::OnInitDialog();
	IUnknown *pUnk;
	
	CWnd *pWnd = GetDlgItem(IDC_EXPLORER);
	pUnk = NULL;
	if(pWnd)
		m_spBrowser = pWnd->GetControlUnknown();

	pWnd = NULL;
	CRect rcItem;
	pWnd = GetDlgItem(IDC_PHSTATIC);
	if(pWnd)
	{
		pWnd->GetClientRect(rcItem);
		pWnd->ClientToScreen(rcItem);
		pWnd->DestroyWindow();
		ScreenToClient(rcItem);
		rcItem.bottom += 150;
		if(m_AddrCombo.Create(WS_VSCROLL|WS_CHILD|WS_VISIBLE|CBS_DROPDOWN|CBS_AUTOHSCROLL,rcItem,this,IDC_ADDRCOMBO))
		{
			m_AddrCombo.SetCurSel(0);
			m_AddrCombo.GetWindowText(m_szCurAddr);
			if(m_spBrowser && m_szCurAddr.GetLength())
				m_spBrowser->Navigate(m_szCurAddr.AllocSysString(),NULL,NULL,NULL,NULL);
		}
	}
	return TRUE;
}

void CGetURL::OnCloseup()
{	
	int nSel = m_AddrCombo.GetCurSel();
	if(CB_ERR != nSel)
	{
		m_AddrCombo.GetLBText(nSel,m_szCurAddr);
		if(m_spBrowser)
			m_spBrowser->Navigate(m_szCurAddr.AllocSysString(),NULL,NULL,NULL,NULL);

	}
}

void CGetURL::OnGo()
{
	m_AddrCombo.GetWindowText(m_szCurAddr);
	if(m_spBrowser)
		m_spBrowser->Navigate(m_szCurAddr.AllocSysString(),NULL,NULL,NULL,NULL);

}
