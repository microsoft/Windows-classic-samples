// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------
//
// OpenQDlg.cpp : implementation file
//




#include "stdafx.h"
#include "MQApitst.h"
#include "OpenQDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpenQueueDialog dialog


COpenQueueDialog::COpenQueueDialog(CArray <ARRAYQ*, ARRAYQ*>* pStrArray, CWnd* pParent /*=NULL*/)
	: CDialog(COpenQueueDialog::IDD, pParent)
{
	m_pStrArray = pStrArray;

	//{{AFX_DATA_INIT(COpenQueueDialog)
	m_szPathName = _T("");
	m_szFormatName = _T("");
	m_iAccessRight = 0;
	//}}AFX_DATA_INIT
}


void COpenQueueDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpenQueueDialog)
	DDX_Control(pDX, IDC_FORMATNAME_COMBO2, m_FormatNameCB);
	DDX_Control(pDX, IDC_PATHNAME_COMBO, m_PathNameCB);
	DDX_CBString(pDX, IDC_PATHNAME_COMBO, m_szPathName);
	DDV_MaxChars(pDX, m_szPathName, 128);
	DDX_CBString(pDX, IDC_FORMATNAME_COMBO2, m_szFormatName);
	DDV_MaxChars(pDX, m_szFormatName, 4000);
	DDX_Radio(pDX, IDC_RADIO1, m_iAccessRight);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpenQueueDialog, CDialog)
	//{{AFX_MSG_MAP(COpenQueueDialog)
	ON_CBN_SELCHANGE(IDC_FORMATNAME_COMBO2, OnSelchangeFormatnameCombo2)
	ON_CBN_SELCHANGE(IDC_PATHNAME_COMBO, OnSelchangePathnameCombo)
	ON_CBN_EDITUPDATE(IDC_FORMATNAME_COMBO2, OnEditupdateFormatnameCombo2)
	ON_CBN_EDITUPDATE(IDC_PATHNAME_COMBO, OnEditupdatePathnameCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenQueueDialog message handlers

BOOL COpenQueueDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	int i;
	for  (i=0 ; i<m_pStrArray->GetSize() ; i++)
	{
		VERIFY (m_PathNameCB.AddString((*m_pStrArray)[i]->szPathName) != CB_ERR);
		VERIFY (m_FormatNameCB.AddString((*m_pStrArray)[i]->szFormatName) != CB_ERR);

	}

    if (m_PathNameCB.GetCount() > 0) m_PathNameCB.SetCurSel(0);        
    if (m_FormatNameCB.GetCount() > 0) m_FormatNameCB.SetCurSel(0);        

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void COpenQueueDialog::GetFormatName(TCHAR szFormatName[], size_t sizeinChars)
{
	ASSERT(szFormatName != NULL);
	
	_tcsncpy_s(szFormatName, sizeinChars, m_szFormatName, sizeinChars-1);
}


void COpenQueueDialog::OnSelchangePathnameCombo() 
{
	m_FormatNameCB.SetCurSel(m_PathNameCB.GetCurSel());
}

void COpenQueueDialog::OnSelchangeFormatnameCombo2() 
{
	m_PathNameCB.SetCurSel(m_FormatNameCB.GetCurSel());
}


void COpenQueueDialog::OnEditupdateFormatnameCombo2() 
{
	m_PathNameCB.SetCurSel(OPENQDLG_CLEAR_COMBOBOX);
}



void COpenQueueDialog::OnEditupdatePathnameCombo() 
{
	m_FormatNameCB.SetCurSel(OPENQDLG_CLEAR_COMBOBOX);	
}
